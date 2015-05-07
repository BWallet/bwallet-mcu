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

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <libopencm3/stm32/flash.h>

#include "messages.pb.h"
#include "storage.pb.h"

#include "bwallet.h"
#include "sha2.h"
#include "aes.h"
#include "pbkdf2.h"
#include "bip32.h"
#include "bip39.h"
#include "util.h"
#include "memory.h"
#include "rng.h"
#include "storage.h"
#include "debug.h"
#include "protect.h"
#include "layout2.h"
#include "coins.h"

Storage storage;

uint8_t storage_uuid[12];
char    storage_uuid_str[25];

static bool   sessionRootNodeCached;
static HDNode sessionRootNode;

static bool sessionPinCached;

static bool sessionPassphraseCached;
static char sessionPassphrase[51];

/*
 storage layout:

 offset | type/length |  description
--------+-------------+-------------------------------
 0x0000 |  4 bytes    |  magic = 'stor'
 0x0004 |  12 bytes   |  uuid
 0x0010 |  ?          |  Storage structure
 */

#define STORAGE_VERSION 6

void storage_from_flash(uint32_t version)
{
	switch (version) {
		case 1: // copy
			memcpy(&storage, (void *)(FLASH_STORAGE_START + 4 + sizeof(storage_uuid)), sizeof(Storage));
			break;
		case 2: // copy(since 1.3.0)
			memcpy(&storage, (void *)(FLASH_STORAGE_START + 4 + sizeof(storage_uuid)), sizeof(Storage));
			break;
		case 3: // copy(since 1.3.1)
			memcpy(&storage, (void *)(FLASH_STORAGE_START + 4 + sizeof(storage_uuid)), sizeof(Storage));
			break;
		case 4: // copy(since 1.3.2)
			memcpy(&storage, (void *)(FLASH_STORAGE_START + 4 + sizeof(storage_uuid)), sizeof(Storage));
			break;
		case 5: // copy(since 1.3.3)
			memcpy(&storage, (void *)(FLASH_STORAGE_START + 4 + sizeof(storage_uuid)), sizeof(Storage));
			break;
		case 6: // copy(since 1.3.4)
			memcpy(&storage, (void *)(FLASH_STORAGE_START + 4 + sizeof(storage_uuid)), sizeof(Storage));
			break;
	}
	storage.version = STORAGE_VERSION;
}

void storage_init(void)
{
	storage_reset();
	// if magic is ok
	if (memcmp((void *)FLASH_STORAGE_START, "stor", 4) == 0) {
		// load uuid
		memcpy(storage_uuid, (void *)(FLASH_STORAGE_START + 4), sizeof(storage_uuid));
		data2hex(storage_uuid, sizeof(storage_uuid), storage_uuid_str);
		// load storage struct
		uint32_t version = ((Storage *)(FLASH_STORAGE_START + 4 + sizeof(storage_uuid)))->version;
		if (version && version <= STORAGE_VERSION) {
			storage_from_flash(version);
		}
		if (version != STORAGE_VERSION) {
			storage_commit();
		}
	} else {
		storage_reset_uuid();
		storage_commit();
	}
}

void storage_reset_uuid(void)
{
	// set random uuid
	random_buffer(storage_uuid, sizeof(storage_uuid));
	data2hex(storage_uuid, sizeof(storage_uuid), storage_uuid_str);
}

void storage_reset(void)
{
	// reset storage struct
	memset(&storage, 0, sizeof(storage));
	storage.version = STORAGE_VERSION;
	session_clear(true);
}

void session_clear(bool clear_pin)
{
	sessionRootNodeCached = false;   memset(&sessionRootNode, 0, sizeof(sessionRootNode));
	sessionPassphraseCached = false; memset(&sessionPassphrase, 0, sizeof(sessionPassphrase));
	if(clear_pin)
		sessionPinCached = false;
}

static uint8_t meta_backup[FLASH_META_LEN];

void storage_commit(void)
{
	int i;
	uint32_t *w;
	// backup meta
	memcpy(meta_backup, (void *)FLASH_META_START, FLASH_META_LEN);
	flash_clear_status_flags();
	flash_unlock();
	// erase storage
	for (i = FLASH_META_SECTOR_FIRST; i <= FLASH_META_SECTOR_LAST; i++) {
		flash_erase_sector(i, FLASH_CR_PROGRAM_X32);
	}
	// modify storage
	memcpy(meta_backup + FLASH_META_DESC_LEN, "stor", 4);
	memcpy(meta_backup + FLASH_META_DESC_LEN + 4, storage_uuid, sizeof(storage_uuid));
	memcpy(meta_backup + FLASH_META_DESC_LEN + 4 + sizeof(storage_uuid), &storage, sizeof(Storage));
	// copy it back
	for (i = 0; i < FLASH_META_LEN / 4; i++) {
		w = (uint32_t *)(meta_backup + i * 4);
		flash_program_word(FLASH_META_START + i * 4, *w);
	}
	flash_lock();
	// flash operation failed
	if (FLASH_SR & (FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR | FLASH_SR_WRPERR)) {
		layoutDialog(DIALOG_ICON_ERROR, NULL, NULL, NULL, "Storage failure", "detected.", NULL, "Please unplug", "the device.", NULL);
		for (;;) { } 
	}   
}

void storage_loadDevice(LoadDevice *msg)
{
	storage_reset();

	storage.has_imported = true;
	storage.imported = true;

	if (msg->has_pin > 0) {
		storage_setPin(msg->pin);
	}

	if (msg->has_passphrase_protection) {
		storage.has_passphrase_protection = true;
		storage.passphrase_protection = msg->passphrase_protection;
	} else {
		storage.has_passphrase_protection = false;
	}

	if (msg->has_node) {
		storage.has_node = true;
		storage.has_mnemonic = false;
		memcpy(&storage.node, &(msg->node), sizeof(HDNodeType));
		sessionRootNodeCached = false;
		memset(&sessionRootNode, 0, sizeof(sessionRootNode));
	} else if (msg->has_mnemonic) {
		storage.has_mnemonic = true;
		storage.has_node = false;
		strlcpy(storage.mnemonic, msg->mnemonic, sizeof(storage.mnemonic));
		sessionRootNodeCached = false;
		memset(&sessionRootNode, 0, sizeof(sessionRootNode));
	}

	if (msg->has_language) {
		storage_setLanguage(msg->language);
	}

	if (msg->has_label) {
		storage_setLabel(msg->label);
	}
}

void storage_labelInit(void)
{
	int i, j;
	for(i = 0; i < COINS_COUNT; i++){
		storage.label_list[i].count = 0;
		for(j = 0; j < LABEL_COUNT; j++) {
			storage.label_list[i].labels[j].index = 0;
			memset(storage.label_list[i].labels[j].label, 0, 
				   sizeof(storage.label_list[i].labels[j].label));
		}
	}
	storage_commit();
}

uint32_t storage_findAccountLabel(const uint32_t index, const uint32_t coin_index)
{
	uint32_t find_index, count;
	count = storage.label_list[coin_index].count;
	for(find_index = 0; find_index < count; find_index++) {
		if(storage.label_list[coin_index].labels[find_index].index == index)
			return find_index;
	}   
	return INVAILD_INDEX; 
}

void storage_setAccountLabel(const char *label, const uint32_t index, const uint32_t coin_index, const uint32_t count, const uint32_t find_index)
{
	if(!label && (coin_index > COINS_COUNT)) return;  

	if((find_index > LABEL_COUNT) && (storage.label_list[coin_index].count <= LABEL_COUNT)) {
		storage.label_list[coin_index].labels[count].index = index;
		strlcpy(storage.label_list[coin_index].labels[count].label, label, 
				sizeof(storage.label_list[coin_index].labels[count].label));
		storage.label_list[coin_index].count += 1;
	} else if(find_index < LABEL_COUNT){
		memset(storage.label_list[coin_index].labels[find_index].label, 0,  
				sizeof(storage.label_list[coin_index].labels[find_index].label));
		strlcpy(storage.label_list[coin_index].labels[find_index].label, label,
				sizeof(storage.label_list[coin_index].labels[find_index].label));
	} else
		return;
}

void storage_delAccountLabel(const uint32_t coin_index, const uint32_t count, const uint32_t find_index)
{
	uint32_t del_index;
	if(coin_index > COINS_COUNT) return;

	if(find_index > LABEL_COUNT )
		return;
	else {
		if(find_index == (count - 1)) {
			storage.label_list[coin_index].labels[find_index].index = 0;
			memset(storage.label_list[coin_index].labels[find_index].label, 0,
					sizeof(storage.label_list[coin_index].labels[find_index].label));
		} else {
			for(del_index = find_index; del_index < count - 1; del_index++) {
				storage.label_list[coin_index].labels[del_index].index =
					storage.label_list[coin_index].labels[del_index + 1].index;
				memset(storage.label_list[coin_index].labels[del_index].label, 0,
						sizeof(storage.label_list[coin_index].labels[del_index].label));
				strlcpy(storage.label_list[coin_index].labels[del_index].label,
						storage.label_list[coin_index].labels[del_index + 1].label,
						sizeof(storage.label_list[coin_index].labels[del_index].label));
			}
		}
		storage.label_list[coin_index].count -= 1;
	}
}
void storage_setLabel(const char *label)
{
	if (!label) return;
	storage.has_label = true;
	strlcpy(storage.label, label, sizeof(storage.label));
}

void storage_setLanguage(const char *lang)
{
	if (!lang) return;

	if (strcmp(lang, "english") == 0) {
		storage.has_language = true;
		strlcpy(storage.language, lang, sizeof(storage.language));
	}
	if ((strcmp(lang, "chinese") == 0) || (strcmp(lang, "zh") == 0)) {
		storage.has_language = true;
		strlcpy(storage.language, lang, sizeof(storage.language));
	}
	
}

void storage_setPassphraseProtection(bool passphrase_protection)
{
	sessionRootNodeCached = false;
	sessionPassphraseCached = false;

	storage.has_passphrase_protection = true;
	storage.passphrase_protection = passphrase_protection;
}

void storage_setHomescreen(const uint8_t *data, uint32_t size)
{
	if (data && size == 1024) {
		storage.has_homescreen = true;
		memcpy(storage.homescreen.bytes, data, size);
		storage.homescreen.size = size;
	} else {
		storage.has_homescreen = false;
		memset(storage.homescreen.bytes, 0, sizeof(storage.homescreen.bytes));
		storage.homescreen.size = 0;
	}   
}

void get_root_node_callback(uint32_t iter, uint32_t total)
{
	switch (storage_getLang()) {
		case CHINESE :
			layoutProgress("唤醒中#.##.##.#", 1000 * iter / total);
			break;
		default :
			layoutProgress("Waking up", 1000 * iter / total);
			break;
	}
}

uint32_t storage_getAccountCount(const uint32_t coin_index)
{
	if(coin_index > COINS_COUNT) return 0;

	return storage.label_list[coin_index].count;
}

void storage_getAccountLabels(bool all, const uint32_t index, AccountLabels *coin_labels, const uint32_t coin_index) 
{
	if(all) {
		memcpy(coin_labels->labels, &storage.label_list[coin_index].labels, 
				sizeof(storage.label_list[coin_index].labels));
	} else {
		memcpy(coin_labels->labels, &storage.label_list[coin_index].labels[index], 
				sizeof(storage.label_list[coin_index].labels[index]));
	}   
}

bool storage_getRootNode(HDNode *node)
{
	// root node is properly cached
	if (sessionRootNodeCached) {
		memcpy(node, &sessionRootNode, sizeof(HDNode));
		return true;
	}

	// if storage has node, decrypt and use it
	if (storage.has_node) {
		if (!protectPassphrase()) {
			return false;
		}
		if (hdnode_from_xprv(storage.node.depth, storage.node.fingerprint, storage.node.child_num, storage.node.chain_code.bytes, storage.node.private_key.bytes, &sessionRootNode) == 0) { 
			return false;
		}   
		if (storage.has_passphrase_protection && storage.passphrase_protection && strlen(sessionPassphrase)) {
			// decrypt hd node
			uint8_t secret[64];
			switch (storage_getLang()) {
				case CHINESE :
					layoutProgressSwipe("唤醒中#.##.##.#", 0);
					break;
				default :
					layoutProgressSwipe("Waking up", 0);
					break;
			}
			pbkdf2_hmac_sha512((const uint8_t *)sessionPassphrase, strlen(sessionPassphrase), (uint8_t *)"BWALLETHD", 8, BIP39_PBKDF2_ROUNDS, secret, 64, get_root_node_callback);
			aes_decrypt_ctx ctx;
			aes_decrypt_key256(secret, &ctx);
			aes_cbc_decrypt(sessionRootNode.chain_code, sessionRootNode.chain_code, 32, secret + 32, &ctx);
			aes_cbc_decrypt(sessionRootNode.private_key, sessionRootNode.private_key, 32, secret + 32, &ctx);
		}
		memcpy(node, &sessionRootNode, sizeof(HDNode));
		sessionRootNodeCached = true;
		return true;
	}

	// if storage has mnemonic, convert it to node and use it
	if (storage.has_mnemonic) {
		if (!protectPassphrase()) {
			return false;
		}
		uint8_t seed[64];
		switch (storage_getLang()) {
			case CHINESE :
				layoutProgressSwipe("唤醒中#.##.##.#", 0);
				break;
			default :
				layoutProgressSwipe("Waking up", 0);
				break;
		}
		mnemonic_to_seed(storage.mnemonic, sessionPassphrase, seed, get_root_node_callback); // BIP-0039
		if (hdnode_from_seed(seed, sizeof(seed), &sessionRootNode) == 0) {
			return false;
		}
		memcpy(node, &sessionRootNode, sizeof(HDNode));
		sessionRootNodeCached = true;
		return true;
	}

	return false;
}

int storage_getLang(void)
{
	if((strcmp(storage_getLanguage(), "chinese") == 0) || (strcmp(storage_getLanguage(), "zh") == 0))
		return CHINESE;
	else
		return ENGLISH;
}

const char *storage_getLabel(void)
{
	return storage.has_label ? storage.label : 0;
}

const char *storage_getLanguage(void)
{
	return storage.has_language ? storage.language : 0;
}

const uint8_t *storage_getHomescreen(void)
{
	return (storage.has_homescreen && storage.homescreen.size == 1024) ? storage.homescreen.bytes : 0;
}

bool storage_isPinCorrect(const char *pin)
{
	 /* The execution time of the following code only depends on the
	  * (public) input.  This avoids timing attacks.
	  */
	char diff = 0;
	uint32_t i = 0;
	while (pin[i]) {
		diff |= storage.pin[i] - pin[i];
		i++;

	}   
	diff |= storage.pin[i];
	return diff == 0;
}

bool storage_hasPin(void)
{
	return storage.has_pin && storage.pin[0] != 0;
}

void storage_setPin(const char *pin)
{
	if (pin && strlen(pin) > 0) {
		storage.has_pin = true;
		strlcpy(storage.pin, pin, sizeof(storage.pin));
	} else {
		storage.has_pin = false;
		storage.pin[0] = 0;
	}
	storage_commit();
	sessionPinCached = false;
}

void session_cachePassphrase(const char *passphrase)
{
	strlcpy(sessionPassphrase, passphrase, sizeof(sessionPassphrase));
	sessionPassphraseCached = true;
}

bool session_isPassphraseCached(void)
{
	return sessionPassphraseCached;
}

void session_cachePin(void)
{
	sessionPinCached = true;
}

bool session_isPinCached(void)
{
	return sessionPinCached;
}

void storage_resetPinFails(void)
{
	storage.has_pin_failed_attempts = true;
	storage.pin_failed_attempts = 0;
	storage_commit();
}

void storage_increasePinFails(void)
{
	if (!storage.has_pin_failed_attempts) {
		storage.has_pin_failed_attempts = true;
		storage.pin_failed_attempts = 1;
	} else {
		storage.pin_failed_attempts++;
	}
	storage_commit();
}

uint32_t storage_getPinFails(void)
{
	storage_from_flash(STORAGE_VERSION);
	return storage.has_pin_failed_attempts ? storage.pin_failed_attempts : 0;
}

bool storage_isInitialized(void)
{
	return storage.has_node || storage.has_mnemonic;
}
