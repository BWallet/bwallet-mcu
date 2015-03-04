/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.1 */

#include "messages.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

const char GetAddress_coin_name_default[17] = "Bitcoin";
const char LoadDevice_language_default[17] = "english";
const uint32_t ResetDevice_strength_default = 128u;
const char ResetDevice_language_default[17] = "english";
const char RecoveryDevice_language_default[17] = "english";
const char SignMessage_coin_name_default[17] = "Bitcoin";
const char EncryptMessage_coin_name_default[17] = "Bitcoin";
const char EstimateTxSize_coin_name_default[17] = "Bitcoin";
const char SignTx_coin_name_default[17] = "Bitcoin";
const char SimpleSignTx_coin_name_default[17] = "Bitcoin";


const pb_field_t Initialize_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t Features_fields[16] = {
    PB_FIELD(  1, STRING  , OPTIONAL, STATIC  , FIRST, Features, vendor, vendor, 0),
    PB_FIELD(  2, UINT32  , OPTIONAL, STATIC  , OTHER, Features, major_version, vendor, 0),
    PB_FIELD(  3, UINT32  , OPTIONAL, STATIC  , OTHER, Features, minor_version, major_version, 0),
    PB_FIELD(  4, UINT32  , OPTIONAL, STATIC  , OTHER, Features, patch_version, minor_version, 0),
    PB_FIELD(  5, BOOL    , OPTIONAL, STATIC  , OTHER, Features, bootloader_mode, patch_version, 0),
    PB_FIELD(  6, STRING  , OPTIONAL, STATIC  , OTHER, Features, device_id, bootloader_mode, 0),
    PB_FIELD(  7, BOOL    , OPTIONAL, STATIC  , OTHER, Features, pin_protection, device_id, 0),
    PB_FIELD(  8, BOOL    , OPTIONAL, STATIC  , OTHER, Features, passphrase_protection, pin_protection, 0),
    PB_FIELD(  9, STRING  , OPTIONAL, STATIC  , OTHER, Features, language, passphrase_protection, 0),
    PB_FIELD( 10, STRING  , OPTIONAL, STATIC  , OTHER, Features, label, language, 0),
    PB_FIELD( 11, MESSAGE , REPEATED, STATIC  , OTHER, Features, coins, label, &CoinType_fields),
    PB_FIELD( 12, BOOL    , OPTIONAL, STATIC  , OTHER, Features, initialized, coins, 0),
    PB_FIELD( 13, BYTES   , OPTIONAL, STATIC  , OTHER, Features, revision, initialized, 0),
    PB_FIELD( 14, BYTES   , OPTIONAL, STATIC  , OTHER, Features, bootloader_hash, revision, 0),
    PB_FIELD( 15, BOOL    , OPTIONAL, STATIC  , OTHER, Features, imported, bootloader_hash, 0),
    PB_LAST_FIELD
};

const pb_field_t ClearSession_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t ApplySettings_fields[4] = {
    PB_FIELD(  1, STRING  , OPTIONAL, STATIC  , FIRST, ApplySettings, language, language, 0),
    PB_FIELD(  2, STRING  , OPTIONAL, STATIC  , OTHER, ApplySettings, label, language, 0),
    PB_FIELD(  3, BOOL    , OPTIONAL, STATIC  , OTHER, ApplySettings, use_passphrase, label, 0),
    PB_LAST_FIELD
};

const pb_field_t ChangePin_fields[2] = {
    PB_FIELD(  1, BOOL    , OPTIONAL, STATIC  , FIRST, ChangePin, remove, remove, 0),
    PB_LAST_FIELD
};

const pb_field_t Ping_fields[5] = {
    PB_FIELD(  1, STRING  , OPTIONAL, STATIC  , FIRST, Ping, message, message, 0),
    PB_FIELD(  2, BOOL    , OPTIONAL, STATIC  , OTHER, Ping, button_protection, message, 0),
    PB_FIELD(  3, BOOL    , OPTIONAL, STATIC  , OTHER, Ping, pin_protection, button_protection, 0),
    PB_FIELD(  4, BOOL    , OPTIONAL, STATIC  , OTHER, Ping, passphrase_protection, pin_protection, 0),
    PB_LAST_FIELD
};

const pb_field_t Success_fields[2] = {
    PB_FIELD(  1, STRING  , OPTIONAL, STATIC  , FIRST, Success, message, message, 0),
    PB_LAST_FIELD
};

const pb_field_t Failure_fields[3] = {
    PB_FIELD(  1, ENUM    , OPTIONAL, STATIC  , FIRST, Failure, code, code, 0),
    PB_FIELD(  2, STRING  , OPTIONAL, STATIC  , OTHER, Failure, message, code, 0),
    PB_LAST_FIELD
};

const pb_field_t ButtonRequest_fields[3] = {
    PB_FIELD(  1, ENUM    , OPTIONAL, STATIC  , FIRST, ButtonRequest, code, code, 0),
    PB_FIELD(  2, STRING  , OPTIONAL, STATIC  , OTHER, ButtonRequest, data, code, 0),
    PB_LAST_FIELD
};

const pb_field_t ButtonAck_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t PinMatrixRequest_fields[2] = {
    PB_FIELD(  1, ENUM    , OPTIONAL, STATIC  , FIRST, PinMatrixRequest, type, type, 0),
    PB_LAST_FIELD
};

const pb_field_t PinMatrixAck_fields[2] = {
    PB_FIELD(  1, STRING  , REQUIRED, STATIC  , FIRST, PinMatrixAck, pin, pin, 0),
    PB_LAST_FIELD
};

const pb_field_t Cancel_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t PassphraseRequest_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t PassphraseAck_fields[2] = {
    PB_FIELD(  1, STRING  , REQUIRED, STATIC  , FIRST, PassphraseAck, passphrase, passphrase, 0),
    PB_LAST_FIELD
};

const pb_field_t GetEntropy_fields[2] = {
    PB_FIELD(  1, UINT32  , REQUIRED, STATIC  , FIRST, GetEntropy, size, size, 0),
    PB_LAST_FIELD
};

const pb_field_t Entropy_fields[2] = {
    PB_FIELD(  1, BYTES   , REQUIRED, STATIC  , FIRST, Entropy, entropy, entropy, 0),
    PB_LAST_FIELD
};

const pb_field_t GetPublicKey_fields[2] = {
    PB_FIELD(  1, UINT32  , REPEATED, STATIC  , FIRST, GetPublicKey, address_n, address_n, 0),
    PB_LAST_FIELD
};

const pb_field_t PublicKey_fields[3] = {
    PB_FIELD(  1, MESSAGE , REQUIRED, STATIC  , FIRST, PublicKey, node, node, &HDNodeType_fields),
    PB_FIELD(  2, STRING  , OPTIONAL, STATIC  , OTHER, PublicKey, xpub, node, 0),
    PB_LAST_FIELD
};

const pb_field_t GetAddress_fields[5] = {
    PB_FIELD(  1, UINT32  , REPEATED, STATIC  , FIRST, GetAddress, address_n, address_n, 0),
    PB_FIELD(  2, STRING  , OPTIONAL, STATIC  , OTHER, GetAddress, coin_name, address_n, &GetAddress_coin_name_default),
    PB_FIELD(  3, BOOL    , OPTIONAL, STATIC  , OTHER, GetAddress, show_display, coin_name, 0),
    PB_FIELD(  4, MESSAGE , OPTIONAL, STATIC  , OTHER, GetAddress, multisig, show_display, &MultisigRedeemScriptType_fields),
    PB_LAST_FIELD
};

const pb_field_t Address_fields[2] = {
    PB_FIELD(  1, STRING  , REQUIRED, STATIC  , FIRST, Address, address, address, 0),
    PB_LAST_FIELD
};

const pb_field_t WipeDevice_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t LoadDevice_fields[8] = {
    PB_FIELD(  1, STRING  , OPTIONAL, STATIC  , FIRST, LoadDevice, mnemonic, mnemonic, 0),
    PB_FIELD(  2, MESSAGE , OPTIONAL, STATIC  , OTHER, LoadDevice, node, mnemonic, &HDNodeType_fields),
    PB_FIELD(  3, STRING  , OPTIONAL, STATIC  , OTHER, LoadDevice, pin, node, 0),
    PB_FIELD(  4, BOOL    , OPTIONAL, STATIC  , OTHER, LoadDevice, passphrase_protection, pin, 0),
    PB_FIELD(  5, STRING  , OPTIONAL, STATIC  , OTHER, LoadDevice, language, passphrase_protection, &LoadDevice_language_default),
    PB_FIELD(  6, STRING  , OPTIONAL, STATIC  , OTHER, LoadDevice, label, language, 0),
    PB_FIELD(  7, BOOL    , OPTIONAL, STATIC  , OTHER, LoadDevice, skip_checksum, label, 0),
    PB_LAST_FIELD
};

const pb_field_t ResetDevice_fields[7] = {
    PB_FIELD(  1, BOOL    , OPTIONAL, STATIC  , FIRST, ResetDevice, display_random, display_random, 0),
    PB_FIELD(  2, UINT32  , OPTIONAL, STATIC  , OTHER, ResetDevice, strength, display_random, &ResetDevice_strength_default),
    PB_FIELD(  3, BOOL    , OPTIONAL, STATIC  , OTHER, ResetDevice, passphrase_protection, strength, 0),
    PB_FIELD(  4, BOOL    , OPTIONAL, STATIC  , OTHER, ResetDevice, pin_protection, passphrase_protection, 0),
    PB_FIELD(  5, STRING  , OPTIONAL, STATIC  , OTHER, ResetDevice, language, pin_protection, &ResetDevice_language_default),
    PB_FIELD(  6, STRING  , OPTIONAL, STATIC  , OTHER, ResetDevice, label, language, 0),
    PB_LAST_FIELD
};

const pb_field_t EntropyRequest_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t EntropyAck_fields[2] = {
    PB_FIELD(  1, BYTES   , OPTIONAL, STATIC  , FIRST, EntropyAck, entropy, entropy, 0),
    PB_LAST_FIELD
};

const pb_field_t RecoveryDevice_fields[7] = {
    PB_FIELD(  1, UINT32  , OPTIONAL, STATIC  , FIRST, RecoveryDevice, word_count, word_count, 0),
    PB_FIELD(  2, BOOL    , OPTIONAL, STATIC  , OTHER, RecoveryDevice, passphrase_protection, word_count, 0),
    PB_FIELD(  3, BOOL    , OPTIONAL, STATIC  , OTHER, RecoveryDevice, pin_protection, passphrase_protection, 0),
    PB_FIELD(  4, STRING  , OPTIONAL, STATIC  , OTHER, RecoveryDevice, language, pin_protection, &RecoveryDevice_language_default),
    PB_FIELD(  5, STRING  , OPTIONAL, STATIC  , OTHER, RecoveryDevice, label, language, 0),
    PB_FIELD(  6, BOOL    , OPTIONAL, STATIC  , OTHER, RecoveryDevice, enforce_wordlist, label, 0),
    PB_LAST_FIELD
};

const pb_field_t WordRequest_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t WordAck_fields[2] = {
    PB_FIELD(  1, STRING  , REQUIRED, STATIC  , FIRST, WordAck, word, word, 0),
    PB_LAST_FIELD
};

const pb_field_t SignMessage_fields[4] = {
    PB_FIELD(  1, UINT32  , REPEATED, STATIC  , FIRST, SignMessage, address_n, address_n, 0),
    PB_FIELD(  2, BYTES   , REQUIRED, STATIC  , OTHER, SignMessage, message, address_n, 0),
    PB_FIELD(  3, STRING  , OPTIONAL, STATIC  , OTHER, SignMessage, coin_name, message, &SignMessage_coin_name_default),
    PB_LAST_FIELD
};

const pb_field_t VerifyMessage_fields[4] = {
    PB_FIELD(  1, STRING  , OPTIONAL, STATIC  , FIRST, VerifyMessage, address, address, 0),
    PB_FIELD(  2, BYTES   , OPTIONAL, STATIC  , OTHER, VerifyMessage, signature, address, 0),
    PB_FIELD(  3, BYTES   , OPTIONAL, STATIC  , OTHER, VerifyMessage, message, signature, 0),
    PB_LAST_FIELD
};

const pb_field_t MessageSignature_fields[3] = {
    PB_FIELD(  1, STRING  , OPTIONAL, STATIC  , FIRST, MessageSignature, address, address, 0),
    PB_FIELD(  2, BYTES   , OPTIONAL, STATIC  , OTHER, MessageSignature, signature, address, 0),
    PB_LAST_FIELD
};

const pb_field_t EncryptMessage_fields[6] = {
    PB_FIELD(  1, BYTES   , OPTIONAL, STATIC  , FIRST, EncryptMessage, pubkey, pubkey, 0),
    PB_FIELD(  2, BYTES   , OPTIONAL, STATIC  , OTHER, EncryptMessage, message, pubkey, 0),
    PB_FIELD(  3, BOOL    , OPTIONAL, STATIC  , OTHER, EncryptMessage, display_only, message, 0),
    PB_FIELD(  4, UINT32  , REPEATED, STATIC  , OTHER, EncryptMessage, address_n, display_only, 0),
    PB_FIELD(  5, STRING  , OPTIONAL, STATIC  , OTHER, EncryptMessage, coin_name, address_n, &EncryptMessage_coin_name_default),
    PB_LAST_FIELD
};

const pb_field_t EncryptedMessage_fields[4] = {
    PB_FIELD(  1, BYTES   , OPTIONAL, STATIC  , FIRST, EncryptedMessage, nonce, nonce, 0),
    PB_FIELD(  2, BYTES   , OPTIONAL, STATIC  , OTHER, EncryptedMessage, message, nonce, 0),
    PB_FIELD(  3, BYTES   , OPTIONAL, STATIC  , OTHER, EncryptedMessage, hmac, message, 0),
    PB_LAST_FIELD
};

const pb_field_t DecryptMessage_fields[5] = {
    PB_FIELD(  1, UINT32  , REPEATED, STATIC  , FIRST, DecryptMessage, address_n, address_n, 0),
    PB_FIELD(  2, BYTES   , OPTIONAL, STATIC  , OTHER, DecryptMessage, nonce, address_n, 0),
    PB_FIELD(  3, BYTES   , OPTIONAL, STATIC  , OTHER, DecryptMessage, message, nonce, 0),
    PB_FIELD(  4, BYTES   , OPTIONAL, STATIC  , OTHER, DecryptMessage, hmac, message, 0),
    PB_LAST_FIELD
};

const pb_field_t DecryptedMessage_fields[3] = {
    PB_FIELD(  1, BYTES   , OPTIONAL, STATIC  , FIRST, DecryptedMessage, message, message, 0),
    PB_FIELD(  2, STRING  , OPTIONAL, STATIC  , OTHER, DecryptedMessage, address, message, 0),
    PB_LAST_FIELD
};

const pb_field_t CipherKeyValue_fields[7] = {
    PB_FIELD(  1, UINT32  , REPEATED, STATIC  , FIRST, CipherKeyValue, address_n, address_n, 0),
    PB_FIELD(  2, STRING  , OPTIONAL, STATIC  , OTHER, CipherKeyValue, key, address_n, 0),
    PB_FIELD(  3, BYTES   , OPTIONAL, STATIC  , OTHER, CipherKeyValue, value, key, 0),
    PB_FIELD(  4, BOOL    , OPTIONAL, STATIC  , OTHER, CipherKeyValue, encrypt, value, 0),
    PB_FIELD(  5, BOOL    , OPTIONAL, STATIC  , OTHER, CipherKeyValue, ask_on_encrypt, encrypt, 0),
    PB_FIELD(  6, BOOL    , OPTIONAL, STATIC  , OTHER, CipherKeyValue, ask_on_decrypt, ask_on_encrypt, 0),
    PB_LAST_FIELD
};

const pb_field_t CipheredKeyValue_fields[2] = {
    PB_FIELD(  1, BYTES   , OPTIONAL, STATIC  , FIRST, CipheredKeyValue, value, value, 0),
    PB_LAST_FIELD
};

const pb_field_t EstimateTxSize_fields[4] = {
    PB_FIELD(  1, UINT32  , REQUIRED, STATIC  , FIRST, EstimateTxSize, outputs_count, outputs_count, 0),
    PB_FIELD(  2, UINT32  , REQUIRED, STATIC  , OTHER, EstimateTxSize, inputs_count, outputs_count, 0),
    PB_FIELD(  3, STRING  , OPTIONAL, STATIC  , OTHER, EstimateTxSize, coin_name, inputs_count, &EstimateTxSize_coin_name_default),
    PB_LAST_FIELD
};

const pb_field_t TxSize_fields[2] = {
    PB_FIELD(  1, UINT32  , OPTIONAL, STATIC  , FIRST, TxSize, tx_size, tx_size, 0),
    PB_LAST_FIELD
};

const pb_field_t SignTx_fields[4] = {
    PB_FIELD(  1, UINT32  , REQUIRED, STATIC  , FIRST, SignTx, outputs_count, outputs_count, 0),
    PB_FIELD(  2, UINT32  , REQUIRED, STATIC  , OTHER, SignTx, inputs_count, outputs_count, 0),
    PB_FIELD(  3, STRING  , OPTIONAL, STATIC  , OTHER, SignTx, coin_name, inputs_count, &SignTx_coin_name_default),
    PB_LAST_FIELD
};

const pb_field_t SimpleSignTx_fields[5] = {
    PB_FIELD(  1, MESSAGE , REPEATED, STATIC  , FIRST, SimpleSignTx, inputs, inputs, &TxInputType_fields),
    PB_FIELD(  2, MESSAGE , REPEATED, STATIC  , OTHER, SimpleSignTx, outputs, inputs, &TxOutputType_fields),
    PB_FIELD(  3, MESSAGE , REPEATED, STATIC  , OTHER, SimpleSignTx, transactions, outputs, &TransactionType_fields),
    PB_FIELD(  4, STRING  , OPTIONAL, STATIC  , OTHER, SimpleSignTx, coin_name, transactions, &SimpleSignTx_coin_name_default),
    PB_LAST_FIELD
};

const pb_field_t TxRequest_fields[4] = {
    PB_FIELD(  1, ENUM    , OPTIONAL, STATIC  , FIRST, TxRequest, request_type, request_type, 0),
    PB_FIELD(  2, MESSAGE , OPTIONAL, STATIC  , OTHER, TxRequest, details, request_type, &TxRequestDetailsType_fields),
    PB_FIELD(  3, MESSAGE , OPTIONAL, STATIC  , OTHER, TxRequest, serialized, details, &TxRequestSerializedType_fields),
    PB_LAST_FIELD
};

const pb_field_t TxAck_fields[2] = {
    PB_FIELD(  1, MESSAGE , OPTIONAL, STATIC  , FIRST, TxAck, tx, tx, &TransactionType_fields),
    PB_LAST_FIELD
};

const pb_field_t FirmwareErase_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t FirmwareUpload_fields[2] = {
    PB_FIELD(  1, BYTES   , REQUIRED, STATIC  , FIRST, FirmwareUpload, payload, payload, 0),
    PB_LAST_FIELD
};

const pb_field_t DebugLinkDecision_fields[2] = {
    PB_FIELD(  1, BOOL    , REQUIRED, STATIC  , FIRST, DebugLinkDecision, yes_no, yes_no, 0),
    PB_LAST_FIELD
};

const pb_field_t DebugLinkGetState_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t DebugLinkState_fields[11] = {
    PB_FIELD(  1, BYTES   , OPTIONAL, STATIC  , FIRST, DebugLinkState, layout, layout, 0),
    PB_FIELD(  2, STRING  , OPTIONAL, STATIC  , OTHER, DebugLinkState, pin, layout, 0),
    PB_FIELD(  3, STRING  , OPTIONAL, STATIC  , OTHER, DebugLinkState, matrix, pin, 0),
    PB_FIELD(  4, STRING  , OPTIONAL, STATIC  , OTHER, DebugLinkState, mnemonic, matrix, 0),
    PB_FIELD(  5, MESSAGE , OPTIONAL, STATIC  , OTHER, DebugLinkState, node, mnemonic, &HDNodeType_fields),
    PB_FIELD(  6, BOOL    , OPTIONAL, STATIC  , OTHER, DebugLinkState, passphrase_protection, node, 0),
    PB_FIELD(  7, STRING  , OPTIONAL, STATIC  , OTHER, DebugLinkState, reset_word, passphrase_protection, 0),
    PB_FIELD(  8, BYTES   , OPTIONAL, STATIC  , OTHER, DebugLinkState, reset_entropy, reset_word, 0),
    PB_FIELD(  9, STRING  , OPTIONAL, STATIC  , OTHER, DebugLinkState, recovery_fake_word, reset_entropy, 0),
    PB_FIELD( 10, UINT32  , OPTIONAL, STATIC  , OTHER, DebugLinkState, recovery_word_pos, recovery_fake_word, 0),
    PB_LAST_FIELD
};

const pb_field_t DebugLinkStop_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t DebugLinkLog_fields[4] = {
    PB_FIELD(  1, UINT32  , OPTIONAL, STATIC  , FIRST, DebugLinkLog, level, level, 0),
    PB_FIELD(  2, STRING  , OPTIONAL, STATIC  , OTHER, DebugLinkLog, bucket, level, 0),
    PB_FIELD(  3, STRING  , OPTIONAL, STATIC  , OTHER, DebugLinkLog, text, bucket, 0),
    PB_LAST_FIELD
};


/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_32BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in 8 or 16 bit
 * field descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(Features, coins[0]) < 65536 && pb_membersize(PublicKey, node) < 65536 && pb_membersize(GetAddress, multisig) < 65536 && pb_membersize(LoadDevice, node) < 65536 && pb_membersize(SimpleSignTx, inputs[0]) < 65536 && pb_membersize(SimpleSignTx, outputs[0]) < 65536 && pb_membersize(SimpleSignTx, transactions[0]) < 65536 && pb_membersize(TxRequest, details) < 65536 && pb_membersize(TxRequest, serialized) < 65536 && pb_membersize(TxAck, tx) < 65536 && pb_membersize(DebugLinkState, node) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_Initialize_Features_ClearSession_ApplySettings_ChangePin_Ping_Success_Failure_ButtonRequest_ButtonAck_PinMatrixRequest_PinMatrixAck_Cancel_PassphraseRequest_PassphraseAck_GetEntropy_Entropy_GetPublicKey_PublicKey_GetAddress_Address_WipeDevice_LoadDevice_ResetDevice_EntropyRequest_EntropyAck_RecoveryDevice_WordRequest_WordAck_SignMessage_VerifyMessage_MessageSignature_EncryptMessage_EncryptedMessage_DecryptMessage_DecryptedMessage_CipherKeyValue_CipheredKeyValue_EstimateTxSize_TxSize_SignTx_SimpleSignTx_TxRequest_TxAck_FirmwareErase_FirmwareUpload_DebugLinkDecision_DebugLinkGetState_DebugLinkState_DebugLinkStop_DebugLinkLog)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
#error Field descriptor for EncryptedMessage.message is too large. Define PB_FIELD_16BIT to fix this.
#endif


