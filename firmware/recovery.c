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

#include "recovery.h"
#include "fsm.h"
#include "storage.h"
#include "layout2.h"
#include "protect.h"
#include "types.pb.h"
#include "messages.h"
#include "rng.h"
#include "bip39.h"

static uint32_t word_count;
static bool awaiting_word = false;
static bool enforce_wordlist;
static char fake_word[12];
static uint32_t word_pos;
static uint32_t word_index;
static char word_order[24];
static char words[24][12];

void next_word(void) {
	word_pos = word_order[word_index];
	if (word_pos == 0) {
		const char *const *wl = mnemonic_wordlist();
		strlcpy(fake_word, wl[random_uniform(2048)], sizeof(fake_word));
		switch (storage_getLang()) {
			case CHINESE : 
				layoutZhDialogSwipe(DIALOG_ICON_INFO, NULL, NULL, NULL, "请输入单词", NULL, fake_word, NULL);
				break;
			default :
				layoutDialogSwipe(DIALOG_ICON_INFO, NULL, NULL, NULL, "Please enter the word", NULL, fake_word, NULL, "on your computer", NULL);
				break;
		}
	} else {
		fake_word[0] = 0;
		char desc[] = "##th word";
		char zhunitdesc[] = "第# #个单词";
		char zhdesc[] = "第# ## #个单词";
		if(storage_getLang() == CHINESE) {
			if (word_pos < 10) {
				zhunitdesc[4] = '0' + word_pos % 10; 
			} else {
				zhdesc[4] = '0' + word_pos / 10; 
				zhdesc[7] = '0' + word_pos % 10;    
			}
		}else {
			if (word_pos < 10) {
				desc[0] = ' ';
			} else {
				desc[0] = '0' + word_pos / 10;
			}
			desc[1] = '0' + word_pos % 10;
			if (word_pos == 1 || word_pos == 21) {
				desc[2] = 's'; desc[3] = 't';
			} else
			if (word_pos == 2 || word_pos == 22) {
				desc[2] = 'n'; desc[3] = 'd';
			} else
			if (word_pos == 3 || word_pos == 23) {
				desc[2] = 'r'; desc[3] = 'd';
			}
		}

		switch (storage_getLang()) {
			case CHINESE : 
				layoutZhDialogSwipe(DIALOG_ICON_INFO, NULL, NULL, NULL, "请输入您的备份词组", NULL, (word_pos < 10 ? zhunitdesc : zhdesc), NULL);
				break;
			default :
				layoutDialogSwipe(DIALOG_ICON_INFO, NULL, NULL, NULL, "Please enter the", NULL, (word_pos < 10 ? desc + 1 : desc), NULL, "of your mnemonic", NULL);
				break;
		}
	}
	WordRequest resp;
	memset(&resp, 0, sizeof(WordRequest));
	msg_write(MessageType_MessageType_WordRequest, &resp);
}

void recovery_init(uint32_t _word_count, bool passphrase_protection, bool pin_protection, const char *language, const char *label, bool _enforce_wordlist)
{
	if (_word_count != 12 && _word_count != 18 && _word_count != 24) {
		fsm_sendFailure(FailureType_Failure_SyntaxError, "Invalid word count (has to be 12, 18 or 24 bits)");
		layoutHome();
		return;
	}   

	word_count = _word_count;
	enforce_wordlist = _enforce_wordlist;

	if (pin_protection && !protectChangePin()) {
		fsm_sendFailure(FailureType_Failure_ActionCancelled, "PIN change failed");
		layoutHome();
		return;
	}   

	storage.has_passphrase_protection = true;
	storage.passphrase_protection = passphrase_protection;
	storage_setLanguage(language);
	storage_setLabel(label);

	uint32_t i;
	for (i = 0; i < word_count; i++) {
		word_order[i] = i + 1;
	}   
	for (i = word_count; i < 24; i++) {
		word_order[i] = 0;
	}
	random_permute(word_order, 24);
	awaiting_word = true;
	word_index = 0;
	next_word();
}

void recovery_word(const char *word)
{
	if (!awaiting_word) {
		fsm_sendFailure(FailureType_Failure_UnexpectedMessage, "Not in Recovery mode");
		layoutHome();
		return;
	}

	if (word_pos == 0) { // fake word
		if (strcmp(word, fake_word) != 0) {
			storage_reset();
			fsm_sendFailure(FailureType_Failure_SyntaxError, "Wrong word retyped");
			layoutHome();
			return;
		}
	} else { // real word
		if (enforce_wordlist) { // check if word is valid
			const char *const *wl = mnemonic_wordlist();
			bool found = false;
			while (*wl) {
				if (strcmp(word, *wl) == 0) {
					found = true;
					break;
				}
				wl++;
			}
			if (!found) {
				storage_reset();
				fsm_sendFailure(FailureType_Failure_SyntaxError, "Word not found in a wordlist");
				layoutHome();
				return;
			}
		}
		strlcpy(words[word_pos - 1], word, sizeof(words[word_pos - 1]));
	}

	if (word_index + 1 == 24) { // last one
		uint32_t i;
		strlcpy(storage.mnemonic, words[0], sizeof(storage.mnemonic));
		for (i = 1; i < word_count; i++) {
			strlcat(storage.mnemonic, " ", sizeof(storage.mnemonic));
			strlcat(storage.mnemonic, words[i], sizeof(storage.mnemonic));
		}
		if (!enforce_wordlist || mnemonic_check(storage.mnemonic)) {
			storage.has_mnemonic = true;
			storage_commit();
			fsm_sendSuccess("Device recovered");
		} else {
			storage_reset();
			fsm_sendFailure(FailureType_Failure_SyntaxError, "Invalid mnemonic, are words in correct order?");
		}
		awaiting_word = false;
		layoutHome();
	} else {
		word_index++;
		next_word();
	}
}

void recovery_abort(void)
{
	if (awaiting_word) {
		layoutHome();
		awaiting_word = false;
	}
}

const char *recovery_get_fake_word(void)
{
	return fake_word;
}

uint32_t recovery_get_word_pos(void)
{
	return word_pos;
}
