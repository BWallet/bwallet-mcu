/*
 * This file is part of the TREZOR project.
 *
 * Copyright (C) 2014 Pavol Rusnak <stick@satoshilabs.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "bwallet.h"
#include "fsm.h"
#include "messages.h"
#include "bip32.h"
#include "storage.h"
#include "coins.h"
#include "debug.h"
#include "transaction.h"
#include "rng.h"
#include "storage.h"
#include "oled.h"
#include "protect.h"
#include "pinmatrix.h"
#include "layout2.h"
#include "ecdsa.h"
#include "reset.h"
#include "recovery.h"
#include "memory.h"
#include "usb.h"
#include "util.h"
#include "signing.h"
#include "aes.h"
#include "hmac.h"
#include "serialno.h"
#include "crypto.h"
#include "base58.h"
#include "bip39.h"
#include "ripemd160.h"

// message methods

static uint8_t msg_resp[MSG_OUT_SIZE];

#define RESP_INIT(TYPE) TYPE *resp = (TYPE *)msg_resp; memset(resp, 0, sizeof(TYPE));

void fsm_sendSuccess(const char *text)
{
	RESP_INIT(Success);
	if (text) {
		resp->has_message = true;
		strlcpy(resp->message, text, sizeof(resp->message));
	}
	msg_write(MessageType_MessageType_Success, resp);
}

void fsm_sendFailure(FailureType code, const char *text)
{
	if (protectAbortedByInitialize) {
		fsm_msgInitialize((Initialize *)0);
		protectAbortedByInitialize = false;
		return;
	}
	RESP_INIT(Failure);
	resp->has_code = true;
	resp->code = code;
	if (text) {
		resp->has_message = true;
		strlcpy(resp->message, text, sizeof(resp->message));
	}
	msg_write(MessageType_MessageType_Failure, resp);
}

const CoinType *fsm_getCoin(const char *name)
{
	const CoinType *coin = coinByName(name);
	if (!coin) {
		fsm_sendFailure(FailureType_Failure_Other, "Invalid coin name");
		layoutHome();
		return 0;
	}   
	return coin;
}

const HDNode *fsm_getDerivedNode(uint32_t *address_n, size_t address_n_count)
{
	static HDNode node;
	if (!storage_getRootNode(&node)) {
		fsm_sendFailure(FailureType_Failure_NotInitialized, "Device not initialized or passphrase request cancelled");
		layoutHome();
		return 0;
	}
	if (!address_n || address_n_count == 0) {
		return &node;
	}

	if (hdnode_private_ckd_cached(&node, address_n, address_n_count) == 0) {
		fsm_sendFailure(FailureType_Failure_Other, "Failed to derive private key");
		layoutHome();
		return 0;
	}   
	return &node;
}

void fsm_msgInitialize(Initialize *msg)
{
	(void)msg;
	recovery_abort();
	signing_abort();
	session_clear(false);
	fsm_msgGetFeatures(0);
}

void fsm_msgGetFeatures(GetFeatures *msg)
{	
	(void)msg;	
	RESP_INIT(Features);
	resp->has_vendor = true;         strlcpy(resp->vendor, "bitcointrezor.com", sizeof(resp->vendor));
	resp->has_major_version = true;  resp->major_version = VERSION_MAJOR;
	resp->has_minor_version = true;  resp->minor_version = VERSION_MINOR;
	resp->has_patch_version = true;  resp->patch_version = VERSION_PATCH;
	resp->has_device_id = true;      strlcpy(resp->device_id, storage_uuid_str, sizeof(resp->device_id));
	resp->has_pin_protection = true; resp->pin_protection = storage.has_pin;
	resp->has_passphrase_protection = true; resp->passphrase_protection = storage.has_passphrase_protection && storage.passphrase_protection;
#ifdef SCM_REVISION
	int len = sizeof(SCM_REVISION) - 1;
	resp->has_revision = true; memcpy(resp->revision.bytes, SCM_REVISION, len); resp->revision.size = len;
#endif
	resp->has_bootloader_hash = true; resp->bootloader_hash.size = memory_bootloader_hash(resp->bootloader_hash.bytes);
	if (storage.has_language) {
		resp->has_language = true;
		strlcpy(resp->language, storage.language, sizeof(resp->language));
	}
	if (storage.has_label) {
		resp->has_label = true;
		strlcpy(resp->label, storage.label, sizeof(resp->label));
	}
	resp->coins_count = COINS_COUNT;
	memcpy(resp->coins, coins, COINS_COUNT * sizeof(CoinType));
	resp->has_initialized = true; resp->initialized = storage_isInitialized();
	resp->has_imported = true; resp->imported = storage.has_imported && storage.imported;
	resp->has_pin_cached = true; resp->pin_cached = session_isPinCached();
	resp->has_passphrase_cached = true; resp->passphrase_cached = session_isPassphraseCached();
	msg_write(MessageType_MessageType_Features, resp);
}

void fsm_msgPing(Ping *msg)
{
	RESP_INIT(Success);

	if (msg->has_button_protection && msg->button_protection) {
		layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you really want to", "answer to ping?", NULL, NULL, NULL, NULL);
		if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "Ping cancelled");
			layoutHome();
			return;
		}
	}

	if (msg->has_pin_protection && msg->pin_protection) {
		if (!protectPin(true)) {
			layoutHome();
			return;
		}
	}

	if (msg->has_passphrase_protection && msg->passphrase_protection) {
		if (!protectPassphrase()) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "Ping cancelled");
			return;
		}
	}

	if (msg->has_message) {
		resp->has_message = true;
		memcpy(&(resp->message), &(msg->message), sizeof(resp->message));
	}
	msg_write(MessageType_MessageType_Success, resp);
	layoutHome();
}

void fsm_msgChangePin(ChangePin *msg)
{
	bool removal = msg->has_remove && msg->remove;
	if (removal) {
		if (storage_hasPin()) {
			switch (storage_getLang()) {
				case CHINESE : 
					layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, "移除#P##I##N#码#?#", NULL, NULL, NULL);
					break;
				default :
					layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you really want to", "remove current PIN?", NULL, NULL, NULL, NULL);
					break;
			}
		} else {
			fsm_sendSuccess("PIN removed");
			return;
		}
	} else {
		if (storage_hasPin()) {
			switch (storage_getLang()) {
				case CHINESE : 
					layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, "重置#P##I##N#码#?#", NULL, NULL, NULL);
					break;
				default :
					layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you really want to", "change current PIN?", NULL, NULL, NULL, NULL);
					break;
			}
		} else {
			switch (storage_getLang()) {
				case CHINESE : 
					layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, "设置新的#P##I##N#码#?#", NULL, NULL, NULL);
					break;
				default :
					layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you really want to", "set new PIN?", NULL, NULL, NULL, NULL);
					break;
			}
		}
	}
	if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
		fsm_sendFailure(FailureType_Failure_ActionCancelled, removal ? "PIN removal cancelled" : "PIN change cancelled");
		layoutHome();
		return;
	}
	if (!protectPin(false)) {
		layoutHome();
		return;
	}
	if (removal) {
		storage_setPin(0);
		fsm_sendSuccess("PIN removed");
	} else {
		if (protectChangePin()) {
			fsm_sendSuccess("PIN changed");
		} else {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "PIN change failed");
		}
	}
	layoutHome();
}

void fsm_msgWipeDevice(WipeDevice *msg)
{
	(void)msg;
	switch (storage_getLang()) {
		case CHINESE : 
			layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, "删除钱包#?#", NULL, "所有数据将丢失#.#", NULL);
			break;
		default :
			layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you really want to", "wipe the device?", NULL, "All data will be lost.", NULL, NULL);
			break;
	}
	if (!protectButton(ButtonRequestType_ButtonRequest_WipeDevice, false)) {
		fsm_sendFailure(FailureType_Failure_ActionCancelled, "Wipe cancelled");
		layoutHome();
		return;
	}
	storage_reset();
	storage_reset_uuid();
	storage_commit();
	// the following does not work on Mac anyway :-/ Linux/Windows are fine, so it is not needed
	// usbReconnect(); // force re-enumeration because of the serial number change
	fsm_sendSuccess("Device wiped");
	layoutHome();
}

void fsm_msgFirmwareErase(FirmwareErase *msg)
{
	(void)msg;
	fsm_sendFailure(FailureType_Failure_UnexpectedMessage, "Not in bootloader mode");
}

void fsm_msgFirmwareUpload(FirmwareUpload *msg)
{
	(void)msg;
	fsm_sendFailure(FailureType_Failure_UnexpectedMessage, "Not in bootloader mode");
}

void fsm_msgGetEntropy(GetEntropy *msg)
{
		switch (storage_getLang()) {
			case CHINESE : 
				layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, "发送公钥#?#", NULL, NULL, NULL);
				break;
			default :
				layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you really want to", "send entropy?", NULL, NULL, NULL, NULL);
				break;
		}
	if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
		fsm_sendFailure(FailureType_Failure_ActionCancelled, "Entropy cancelled");
		layoutHome();
		return;
	}
	RESP_INIT(Entropy);
	uint32_t len = msg->size;
	if (len > 1024) {
		len = 1024;
	}
	resp->entropy.size = len;
	random_buffer(resp->entropy.bytes, len);
	msg_write(MessageType_MessageType_Entropy, resp);
	layoutHome();
}

void fsm_msgGetPublicKey(GetPublicKey *msg)
{
	RESP_INIT(PublicKey);

	if (!protectPin(true)) {
		layoutHome();
		return;
	}   

	const HDNode *node = fsm_getDerivedNode(msg->address_n, msg->address_n_count);
	if (!node) return;

	resp->node.depth = node->depth;
	resp->node.fingerprint = node->fingerprint;
	resp->node.child_num = node->child_num;
	resp->node.chain_code.size = 32;
	memcpy(resp->node.chain_code.bytes, node->chain_code, 32);
	resp->node.has_private_key = false;
	resp->node.has_public_key = true;
	resp->node.public_key.size = 33;
	memcpy(resp->node.public_key.bytes, node->public_key, 33);
	resp->has_xpub = true;
	hdnode_serialize_public(node, resp->xpub, sizeof(resp->xpub));
	msg_write(MessageType_MessageType_PublicKey, resp);
	layoutHome();
}

void fsm_msgLoadDevice(LoadDevice *msg)
{
	if (storage_isInitialized()) {
		fsm_sendFailure(FailureType_Failure_UnexpectedMessage, "Device is already initialized. Use Wipe first.");
		return;
	}

	switch (storage_getLang()) {
		case CHINESE : 
			layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "继续", NULL, "不推荐载入私钥#.#", NULL, "是否继续#!#", NULL);
			break;
		default :
			layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "I take the risk", NULL, "Loading private seed", "is not recommended.", "Continue only if you", "know what you are", "doing!", NULL);
			break;
	}
	if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
		fsm_sendFailure(FailureType_Failure_ActionCancelled, "Load cancelled");
		layoutHome();
		return;
	}

	if (msg->has_mnemonic && !(msg->has_skip_checksum && msg->skip_checksum) ) { 
		if (!mnemonic_check(msg->mnemonic)) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "Mnemonic with wrong checksum provided");
			layoutHome();
			return;
		}   
	}   
	storage_loadDevice(msg);
	storage_commit();
	fsm_sendSuccess("Device loaded");
	layoutHome();
}

void fsm_msgResetDevice(ResetDevice *msg)
{
	if (storage_isInitialized()) {
		fsm_sendFailure(FailureType_Failure_UnexpectedMessage, "Device is already initialized. Use Wipe first.");
		return;
	}
	reset_init(
		msg->has_display_random && msg->display_random,
		msg->has_strength ? msg->strength : 128,
		msg->has_passphrase_protection && msg->passphrase_protection,
		msg->has_pin_protection && msg->pin_protection,
		msg->has_language ? msg->language : 0,
		msg->has_label ? msg->label : 0
	);
}

void fsm_msgSignTx(SignTx *msg)
{
	if (msg->inputs_count < 1) {
		fsm_sendFailure(FailureType_Failure_Other, "Transaction must have at least one input");
		layoutHome();
		return;
	}

	if (msg->outputs_count < 1) {
		fsm_sendFailure(FailureType_Failure_Other, "Transaction must have at least one output");
		layoutHome();
		return;
	}

	if (!protectPin(true)) {
		layoutHome();
		return;
	}

	const CoinType *coin = fsm_getCoin(msg->coin_name);
	if (!coin) return;
	const HDNode *node = fsm_getDerivedNode(0, 0);
	if (!node) return;

	signing_init(msg->inputs_count, msg->outputs_count, coin, node);
}

void fsm_msgCancel(Cancel *msg)
{
	(void)msg;
	recovery_abort();
	signing_abort();
}

void fsm_msgTxAck(TxAck *msg)
{
	if (msg->has_tx) {
		signing_txack(&(msg->tx));
	} else {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No transaction provided");
	}
}

void fsm_msgCipherKeyValue(CipherKeyValue *msg)
{
	if (!msg->has_key) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No key provided");
		return;
	}
	if (!msg->has_value) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No value provided");
		return;
	}
	if (msg->value.size % 16) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "Value length must be a multiple of 16");
		return;
	}
	if (!protectPin(true)) {
		layoutHome();
		return;
	}
	const HDNode *node = fsm_getDerivedNode(msg->address_n, msg->address_n_count);
	if (!node) return;

	bool encrypt = msg->has_encrypt && msg->encrypt;
	bool ask_on_encrypt = msg->has_ask_on_encrypt && msg->ask_on_encrypt;
	bool ask_on_decrypt = msg->has_ask_on_decrypt && msg->ask_on_decrypt;
	if ((encrypt && ask_on_encrypt) || (!encrypt && ask_on_decrypt)) {
		layoutCipherKeyValue(encrypt, msg->key);
		if (!protectButton(ButtonRequestType_ButtonRequest_Other, false)) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "CipherKeyValue cancelled");
			layoutHome();
			return;
		}
	}

	uint8_t data[256 + 4];
	strlcpy((char *)data, msg->key, sizeof(data));
	strlcat((char *)data, ask_on_encrypt ? "E1" : "E0", sizeof(data));
	strlcat((char *)data, ask_on_decrypt ? "D1" : "D0", sizeof(data));

	hmac_sha512(node->private_key, 32, data, strlen((char *)data), data);

	RESP_INIT(CipheredKeyValue);
	if (encrypt) {
		aes_encrypt_ctx ctx;
		aes_encrypt_key256(data, &ctx);
		aes_cbc_encrypt(msg->value.bytes, resp->value.bytes, msg->value.size, data + 32, &ctx);
	} else {
		aes_decrypt_ctx ctx;
		aes_decrypt_key256(data, &ctx);
		aes_cbc_decrypt(msg->value.bytes, resp->value.bytes, msg->value.size, data + 32, &ctx);
	}
	resp->has_value = true;
	resp->value.size = msg->value.size;
	msg_write(MessageType_MessageType_CipheredKeyValue, resp);
	layoutHome();
}

void fsm_msgClearSession(ClearSession *msg)
{
	(void)msg;
	session_clear(true);
	fsm_sendSuccess("Session cleared");
}

bool fsm_getLang(ApplySettings *msg)
{
	if(!strcmp(msg->language, "zh") || !strcmp(msg->language, "chinese"))
		return true;
	else
		return false;
}

void fsm_msgApplySettings(ApplySettings *msg)
{
	if (msg->has_label) {
		switch (storage_getLang()) {
			case CHINESE : 
				layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, "设置钱包名称为#:#", NULL, msg->label, NULL);
				break;
			default :
				layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you really want to", "change label to", msg->label, "?", NULL, NULL);
				break;
		}
		if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "Apply settings cancelled");
			layoutHome();
			return;
		}
	} 

	if (msg->has_language) {
		switch (storage_getLang()) {
			case CHINESE : 
				layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, "设置固件语言为#:#", NULL, fsm_getLang(msg) ? "中文" : "英语", NULL);
				break;
			default :
				layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you really want to", "change language to", msg->language, "?", NULL, NULL);
				break;
		}
		if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "Apply settings cancelled");
			layoutHome();
			return;
		}   
	}

	if (msg->has_use_passphrase) {
		switch (storage_getLang()) {
			case CHINESE :
				layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, NULL, msg->use_passphrase ? "开启密码" : "禁止密码", "加密#?#", NULL);
				break;
			default :
				layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you really want to", msg->use_passphrase ? "enable passphrase" : "disable passphrase", "encryption?", NULL, NULL, NULL);
				break;
		}
		if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "Apply settings cancelled");
			layoutHome();
			return;
		}   
	}   

	if (msg->has_homescreen) {
		switch (storage_getLang()) {
			case CHINESE :
				layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, NULL, "改变屏幕显示", NULL, NULL);
				break;
			default :
				layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you really want to", "change the home", "screen ?", NULL, NULL, NULL);
				break;
		}
		if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "Apply settings cancelled");
			layoutHome();
			return;
		}
	}

	if (!msg->has_label && !msg->has_language && !msg->has_use_passphrase && !msg->has_homescreen) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No setting provided");
		return;
	}   
	if (!protectPin(true)) {
		layoutHome();
		return;
	}
	if (msg->has_label) {
		storage_setLabel(msg->label);
	}
	if (msg->has_language) {
		storage_setLanguage(msg->language);
	}
	if (msg->has_use_passphrase) {
		storage_setPassphraseProtection(msg->use_passphrase);
	}
	if (msg->has_homescreen) {
		storage_setHomescreen(msg->homescreen.bytes, msg->homescreen.size);
	}

	storage_commit();
	fsm_sendSuccess("Settings applied");
	layoutHome();
}

void fsm_msgGetAddress(GetAddress *msg)
{
	RESP_INIT(Address);

	if (!protectPin(true)) {
		layoutHome();
		return;
	}   

	const CoinType *coin = fsm_getCoin(msg->coin_name);
	if (!coin) return;
	const HDNode *node = fsm_getDerivedNode(msg->address_n, msg->address_n_count);
	if (!node) return;


	if (msg->has_multisig) {
		switch (storage_getLang()) {
			case CHINESE :
				layoutProgressSwipe("准备#.##.##.#", 0);
				break;
			default :
				layoutProgressSwipe("Preparing", 0);
				break;

		}
		if (cryptoMultisigPubkeyIndex(&(msg->multisig), node->public_key) < 0) {
			fsm_sendFailure(FailureType_Failure_Other, "Pubkey not found in multisig script");
			layoutHome();
			return;
		}   
		uint8_t buf[32];
		if (compile_script_multisig_hash(&(msg->multisig), buf) == 0) {
			fsm_sendFailure(FailureType_Failure_Other, "Invalid multisig script");
			layoutHome();
			return;
		}   
		ripemd160(buf, 32, buf + 1); 
		buf[0] = coin->address_type_p2sh;
		base58_encode_check(buf, 21, resp->address, sizeof(resp->address));
	} else {
		ecdsa_get_address(node->public_key, coin->address_type, resp->address, sizeof(resp->address));
	}

	if (msg->has_show_display && msg->show_display) {
		char desc[16];
		if (msg->has_multisig) {
			strlcpy(desc, "Msig __ of __:", sizeof(desc));
			const uint32_t m = msg->multisig.m;
			const uint32_t n = msg->multisig.pubkeys_count;
			desc[5] = (m < 10) ? ' ': ('0' + (m / 10));
			desc[6] = '0' + (m % 10);
			desc[11] = (n < 10) ? ' ': ('0' + (n / 10));
			desc[12] = '0' + (n % 10);

		} else {
			strlcpy(desc, "Address:", sizeof(desc));
		}
		layoutAddress(resp->address);
		if (!protectButton(ButtonRequestType_ButtonRequest_Address, true)) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "Show address cancelled");
			layoutHome();
			return;
		}
	}

	msg_write(MessageType_MessageType_Address, resp);
	layoutHome();
}	

void fsm_msgEntropyAck(EntropyAck *msg)
{
	if (msg->has_entropy) {
		reset_entropy(msg->entropy.bytes, msg->entropy.size);
	} else {
		reset_entropy(0, 0);
	}
}

void fsm_msgSignMessage(SignMessage *msg)
{
	RESP_INIT(MessageSignature);

	layoutSignMessage(msg->message.bytes, msg->message.size);
	if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
		fsm_sendFailure(FailureType_Failure_ActionCancelled, "Sign message cancelled");
		layoutHome();
		return;
	}

	if (!protectPin(true)) {
		layoutHome();
		return;
	}

	const CoinType *coin = fsm_getCoin(msg->coin_name);
	if(!coin) return;
	const HDNode *node = fsm_getDerivedNode(msg->address_n, msg->address_n_count);
	if (!node) return;

	switch (storage_getLang()) {
		case CHINESE:
			layoutProgressSwipe("签名#.##.##.#", 0);
			break;
		default :
			layoutProgressSwipe("Signing", 0);
			break;
	}
	if (cryptoMessageSign(msg->message.bytes, msg->message.size, node->private_key, resp->signature.bytes) == 0) {
		resp->has_address = true;
		uint8_t addr_raw[21];
		ecdsa_get_address_raw(node->public_key, coin->address_type, addr_raw);
		base58_encode_check(addr_raw, 21, resp->address, sizeof(resp->address));
		resp->has_signature = true;
		resp->signature.size = 65; 
		msg_write(MessageType_MessageType_MessageSignature, resp);
	} else {
		fsm_sendFailure(FailureType_Failure_Other, "Error signing message");
	}   
	layoutHome();
}

void fsm_msgVerifyMessage(VerifyMessage *msg)
{
	if (!msg->has_address) {
		fsm_sendFailure(FailureType_Failure_Other, "No address provided");
		return;
	}   
	if (!msg->has_message) {
		fsm_sendFailure(FailureType_Failure_Other, "No message provided");
		return;
	} 
	switch (storage_getLang()) {
		case CHINESE:
			layoutProgressSwipe("验证#.##.##.#", 0);
			break;
		default :
			layoutProgressSwipe("Verifying", 0);
			break;
	}
	uint8_t addr_raw[21];
	if (!ecdsa_address_decode(msg->address, addr_raw)) {
		fsm_sendFailure(FailureType_Failure_InvalidSignature, "Invalid address");
	}
	if (msg->signature.size == 65 && cryptoMessageVerify(msg->message.bytes, msg->message.size, addr_raw, msg->signature.bytes) == 0) {
		layoutVerifyMessage(msg->message.bytes, msg->message.size);
		protectButton(ButtonRequestType_ButtonRequest_Other, true);
		fsm_sendSuccess("Message verified");
	} else {
		fsm_sendFailure(FailureType_Failure_InvalidSignature, "Invalid signature");
	}
	layoutHome();
}

void fsm_msgSignIdentity(SignIdentity *msg)
{
	RESP_INIT(SignedIdentity);

	layoutSignIdentity(&(msg->identity), msg->has_challenge_visual ? msg->challenge_visual : 0);
	if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
		fsm_sendFailure(FailureType_Failure_ActionCancelled, "Sign identity cancelled");
		layoutHome();
		return;
	}

	if (!protectPin(true)) {
		layoutHome();
		return;
	}

	uint8_t hash[32];
	if (!msg->has_identity || cryptoIdentityFingerprint(&(msg->identity), hash) == 0) {
		fsm_sendFailure(FailureType_Failure_Other, "Invalid identity");
		layoutHome();
		return;
	}
	uint32_t address_n[5];
	address_n[0] = 0x80000000 | 13;
	address_n[1] = 0x80000000 | hash[ 0] | (hash[ 1] << 8) | (hash[ 2] << 16) | (hash[ 3] << 24);
	address_n[2] = 0x80000000 | hash[ 4] | (hash[ 5] << 8) | (hash[ 6] << 16) | (hash[ 7] << 24);
	address_n[3] = 0x80000000 | hash[ 8] | (hash[ 9] << 8) | (hash[10] << 16) | (hash[11] << 24);
	address_n[4] = 0x80000000 | hash[12] | (hash[13] << 8) | (hash[14] << 16) | (hash[15] << 24);

	const HDNode *node = fsm_getDerivedNode(address_n, 5);
	if (!node) return;

	uint8_t message[256 + 256];
	memcpy(message, msg->challenge_hidden.bytes, msg->challenge_hidden.size);
	const int len = strlen(msg->challenge_visual);
	memcpy(message + msg->challenge_hidden.size, msg->challenge_visual, len);

	switch (storage_getLang()) {
		case CHINESE:
			layoutProgressSwipe("签名#.##.##.#", 0);
			break;
		default :
			layoutProgressSwipe("Signing", 0);
			break;
	}
	if (cryptoMessageSign(message, msg->challenge_hidden.size + len, node->private_key, resp->signature.bytes) == 0) {
		resp->has_address = true;
		uint8_t addr_raw[21];
		ecdsa_get_address_raw(node->public_key, 0x00, addr_raw); // hardcoded Bitcoin address type
		base58_encode_check(addr_raw, 21, resp->address, sizeof(resp->address));
		resp->has_public_key = true;
		resp->public_key.size = 33;
		memcpy(resp->public_key.bytes, node->public_key, 33);
		resp->has_signature = true;
		resp->signature.size = 65;
		msg_write(MessageType_MessageType_SignedIdentity, resp);
	} else {
		fsm_sendFailure(FailureType_Failure_Other, "Error signing identity");
	}
	layoutHome();
}

void fsm_msgEncryptMessage(EncryptMessage *msg)
{
	if (!msg->has_pubkey) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No public key provided");
		return;
	}
	if (!msg->has_message) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No message provided");
		return;
	}
	curve_point pubkey;
	if (msg->pubkey.size != 33 || ecdsa_read_pubkey(msg->pubkey.bytes, &pubkey) == 0) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "Invalid public key provided");
		return;
	}
	bool display_only = msg->has_display_only && msg->display_only;
	bool signing = msg->address_n_count > 0;
	RESP_INIT(EncryptedMessage);
	const CoinType *coin = 0;
	const HDNode *node = 0;
	uint8_t address_raw[21];
	if (signing) {
		coin = coinByName(msg->coin_name);
		if (!coin) {
			fsm_sendFailure(FailureType_Failure_Other, "Invalid coin name");
			return;
		}   
		if (!protectPin(true)) {
			layoutHome();
			return;
		}
		node = fsm_getDerivedNode(msg->address_n, msg->address_n_count);
		if (!node) return;
		uint8_t public_key[33];
		ecdsa_get_public_key33(node->private_key, public_key);
		ecdsa_get_address_raw(public_key, coin->address_type, address_raw);
	}
	layoutEncryptMessage(msg->message.bytes, msg->message.size, signing);
	if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
		fsm_sendFailure(FailureType_Failure_ActionCancelled, "Encrypt message cancelled");
		layoutHome();
		return;
	}
	switch (storage_getLang()) {
		case CHINESE:
			layoutProgressSwipe("加密#.##.##.#", 0);
			break;
		default :
			layoutProgressSwipe("Encrypting", 0);
			break;
	}
	if (cryptoMessageEncrypt(&pubkey, msg->message.bytes, msg->message.size, display_only, 
				resp->nonce.bytes, &(resp->nonce.size), resp->message.bytes, &(resp->message.size), 
				resp->hmac.bytes, &(resp->hmac.size), signing ? node->private_key : 0, signing ? address_raw : 0) != 0) {
		fsm_sendFailure(FailureType_Failure_ActionCancelled, "Error encrypting message");
		layoutHome();
		return;
	}
	resp->has_nonce = true;
	resp->has_message = true;
	resp->has_hmac = true;
	msg_write(MessageType_MessageType_EncryptedMessage, resp);
	layoutHome();
}

void fsm_msgDecryptMessage(DecryptMessage *msg)
{
	if (!msg->has_nonce) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No nonce provided");
		return;
	}
	if (!msg->has_message) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No message provided");
		return;
	}
	if (!msg->has_hmac) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No message hmac provided");
		return;
	}
	curve_point nonce_pubkey;
	if (msg->nonce.size != 33 || ecdsa_read_pubkey(msg->nonce.bytes, &nonce_pubkey) == 0) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "Invalid nonce provided");
		return;
	}
	if (!protectPin(true)) {
		layoutHome();
		return;
	}
	const HDNode *node = fsm_getDerivedNode(msg->address_n, msg->address_n_count);
	if (!node) return;

	switch (storage_getLang()) {
		case CHINESE:
			layoutProgressSwipe("解密#.##.##.#", 0);
			break;
		default :
			layoutProgressSwipe("Decrypting", 0);
			break;
	}
	RESP_INIT(DecryptedMessage);
	bool display_only = false;
	bool signing = false;
	uint8_t address_raw[21];
	if (cryptoMessageDecrypt(&nonce_pubkey, msg->message.bytes, msg->message.size, msg->hmac.bytes, 
		msg->hmac.size, node->private_key, resp->message.bytes, &(resp->message.size), &display_only, &signing, address_raw) != 0) {
		fsm_sendFailure(FailureType_Failure_ActionCancelled, "Error decrypting message");
		layoutHome();
		return;
	}
	if (signing) {
		base58_encode_check(address_raw, 21, resp->address, sizeof(resp->address));
	}
	layoutDecryptMessage(resp->message.bytes, resp->message.size, signing ? resp->address : 0);
	protectButton(ButtonRequestType_ButtonRequest_Other, true);
	if (display_only) {
		resp->has_address = false;
		resp->has_message = false;
		memset(resp->address, sizeof(resp->address), 0);
		memset(&(resp->message), sizeof(resp->message), 0);
	} else {
		resp->has_address = signing;
		resp->has_message = true;
	}
	msg_write(MessageType_MessageType_DecryptedMessage, resp);
	layoutHome();
}

void fsm_msgEstimateTxSize(EstimateTxSize *msg)
{
	RESP_INIT(TxSize);
	resp->has_tx_size = true;
	resp->tx_size = transactionEstimateSize(msg->inputs_count, msg->outputs_count);
	msg_write(MessageType_MessageType_TxSize, resp);
}

void fsm_msgRecoveryDevice(RecoveryDevice *msg)
{
	if (storage_isInitialized()) {
		fsm_sendFailure(FailureType_Failure_UnexpectedMessage, "Device is already initialized. Use Wipe first.");
		return;
	}
	recovery_init(
		msg->has_word_count ? msg->word_count : 12,
		msg->has_passphrase_protection && msg->passphrase_protection,
		msg->has_pin_protection && msg->pin_protection,
		msg->has_language ? msg->language : 0,
		msg->has_label ? msg->label : 0,
		msg->has_enforce_wordlist ? msg->enforce_wordlist : false
	);
}

void fsm_msgWordAck(WordAck *msg)
{
	recovery_word(msg->word);
}

const char *fsm_msgStrIndex(uint32_t label_index)
{
	static char str_index[16];
    char temp;
    int	i , j, index = 0; 
	uint32_t value_index;

	value_index = label_index;
	do{
		str_index[index++] = '0' + label_index % 10;
	}while((label_index /= 10) > 0);

	str_index[index] = '\0';

	for(i = 0, j = index-1; i < j; i++, j--){
		temp = str_index[i];
		str_index[i] = str_index[j];
		str_index[j] = temp;
	}

	if(value_index == 11 || value_index == 12 || value_index == 13) {
		strcat(str_index, "th");
	} else {
		if((str_index[index - 1] == '1') || 
				(str_index[index - 1] == '2') || 
				(str_index[index - 1] == '3')) {
			if(str_index[index - 1] == '1')
				strcat(str_index, "st");
			if(str_index[index - 1] == '2')
				strcat(str_index, "nd");
			if(str_index[index - 1] == '3')
				strcat(str_index, "rd");
		} else
			strcat(str_index, "th");
	}

	return str_index;
}

void fsm_msgSetAccountLabel(SetAccountLabel *msg)
{
	if (!msg->coin_name) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No coin name provided");
		return;
	}

	const uint32_t coin_index = coinIndex(msg->coin_name);
	if(coin_index > COINS_COUNT) { 
		fsm_sendFailure(FailureType_Failure_Other, "Invalid coin name");
		return ;
	}    

	const uint32_t labels_count_init = storage_getAccountCount(coin_index);
	if(labels_count_init == 0xffffffff)
		storage_labelInit();
	const uint32_t labels_count = storage_getAccountCount(coin_index);
	if(labels_count >= LABEL_COUNT) {
		fsm_sendFailure(FailureType_Failure_Other, "Label cannot be more than 32");
		return ;
	}

	const char *str_index = fsm_msgStrIndex(msg->index);
	char zhstr_index[16];
	memset(zhstr_index, 0, sizeof(zhstr_index));
	strncpy(zhstr_index, str_index, (strlen(str_index) - 2));

	if(msg->has_label) {
		switch(storage_getLang()) {
			case CHINESE :
				layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, "设置第", zhstr_index, "个账户名称为#:#", msg->label);
				break;
			default :
				layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you want to set ", str_index, "account label to", msg->label, "?", NULL);
				break;
		}
		if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "Account label settings cancelled");
			layoutHome();
			return;
		}
	}    

	if (!msg->has_label && msg->has_index) {
		switch(storage_getLang()) {
			case CHINESE :
				layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认", NULL, "删除第", zhstr_index, "个账户#:#", msg->label);
				break;
			default :
				layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm", NULL, "Do you want to delete", str_index, msg->label, "account label to", "?", NULL);
				break;
		}
		if (!protectButton(ButtonRequestType_ButtonRequest_ProtectCall, false)) {
			fsm_sendFailure(FailureType_Failure_ActionCancelled, "Account label delete cancelled");
			layoutHome();
			return;
		}
	}    

	if ((!msg->has_label && !msg->has_index) || (msg->has_label && !msg->has_index)) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No setting provided");
		return;
	}

	if (!protectPin(true)) {
		layoutHome();
		return;
	}

	const uint32_t find_index = storage_findAccountLabel(msg->index, coin_index);
	if(msg->has_label && msg->has_index) {
		storage_setAccountLabel(msg->label, msg->index, coin_index, labels_count, find_index);
	}

	if(!msg->has_label && msg->has_index) {
		storage_delAccountLabel(coin_index, labels_count, find_index);
	}

	storage_commit();
	fsm_sendSuccess("Settings applied");
	layoutHome();
}

void fsm_msgGetAccountLabels(GetAccountLabels *msg)
{
	if (!msg->coin_name) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "No coin name provided");
		return;
	}
	const uint32_t coin_index = coinIndex(msg->coin_name);
	if(coin_index > COINS_COUNT) {
		fsm_sendFailure(FailureType_Failure_Other, "Invalid coin name");
		return ;
	}
	

	const uint32_t labels_count_init = storage_getAccountCount(coin_index);
	if(labels_count_init == 0xffffffff)
		storage_labelInit();

	const uint32_t find_index = storage_findAccountLabel(msg->index, coin_index);

	RESP_INIT(AccountLabels);
	resp->has_coin_name = true;
	strlcpy(resp->coin_name, msg->coin_name, sizeof(msg->coin_name));

	if(msg->all)
		resp->labels_count = storage_getAccountCount(coin_index);		
	else if(find_index > LABEL_COUNT)
		resp->labels_count = 0;
	else
		resp->labels_count = 1;

	if(resp->labels_count)
		storage_getAccountLabels(msg->all, find_index, resp, coin_index);

	msg_write(MessageType_MessageType_AccountLabels, resp);
	layoutHome();

}

void fsm_msgTestScreen(TestScreen *msg)
{
	uint32_t delay_time = msg->delay_time * 20000000;
	layoutScreen();
	while(delay_time--)
		__asm__("nop");
	layoutHome();
	fsm_sendSuccess("Test finish!");
}

#if DEBUG_LINK

void fsm_msgDebugLinkGetState(DebugLinkGetState *msg)
{
	(void)msg;
	RESP_INIT(DebugLinkState);

	resp->has_layout = true;
	resp->layout.size = OLED_BUFSIZE;
	memcpy(resp->layout.bytes, oledGetBuffer(), OLED_BUFSIZE);

	if (storage.has_pin) {
		resp->has_pin = true;
		strlcpy(resp->pin, storage.pin, sizeof(resp->pin));
	}

	resp->has_matrix = true;
	strlcpy(resp->matrix, pinmatrix_get(), sizeof(resp->matrix));

	resp->has_reset_entropy = true;
	resp->reset_entropy.size = reset_get_int_entropy(resp->reset_entropy.bytes);

	resp->has_reset_word = true;
	strlcpy(resp->reset_word, reset_get_word(), sizeof(resp->reset_word));

	resp->has_recovery_fake_word = true;
	strlcpy(resp->recovery_fake_word, recovery_get_fake_word(), sizeof(resp->recovery_fake_word));

	resp->has_recovery_word_pos = true;
	resp->recovery_word_pos = recovery_get_word_pos();

	if (storage.has_mnemonic) {
		resp->has_mnemonic = true;
		strlcpy(resp->mnemonic, storage.mnemonic, sizeof(resp->mnemonic));
	}

	if (storage.has_node) {
		resp->has_node = true;
		memcpy(&(resp->node), &(storage.node), sizeof(HDNode));
	}

	resp->has_passphrase_protection = true;
	resp->passphrase_protection = storage.has_passphrase_protection && storage.passphrase_protection;

	msg_debug_write(MessageType_MessageType_DebugLinkState, resp);
}

void fsm_msgDebugLinkStop(DebugLinkStop *msg)
{
	(void)msg;
}

#endif
