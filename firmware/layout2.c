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

#include <stdint.h>
#include <string.h>

#include "layout2.h"
#include "storage.h"
#include "oled.h"
#include "bitmaps.h"
#include "string.h"
#include "util.h"
#include "qr_encode.h"

void *layoutLast = layoutHome;

void layoutDialogSwipe(LayoutDialogIcon icon, const char *btnNo, const char *btnYes, const char *desc, const char *line1, const char *line2, const char *line3, const char *line4, const char *line5, const char *line6)
{
	layoutLast = layoutDialogSwipe;
	oledSwipeLeft();
	layoutDialog(icon, btnNo, btnYes, desc, line1, line2, line3, line4, line5, line6);
}

void layoutZhDialogSwipe(LayoutDialogIcon icon, const char *btnNo, const char *btnYes, const char *desc, const char *line1, const char *line2, const char *line3, const char *line4)
{
	layoutLast = layoutDialogSwipe;
	oledSwipeLeft();
	layoutZhDialog(icon, btnNo, btnYes, desc, line1, line2, line3, line4);
}

void layoutProgressSwipe(const char *desc, int permil, int gearstep)
{
	if (layoutLast == layoutProgressSwipe) {
		oledClear();
	} else {
		layoutLast = layoutProgressSwipe;
		oledSwipeLeft();
	}
	layoutProgress(desc, permil, gearstep);
}

void layoutHome(void)
{
	if (layoutLast == layoutHome) {
		oledClear();
	} else {
		layoutLast = layoutHome;
		oledSwipeLeft();
	}
	const char *label = storage_getLabel();
	if (label && strlen(label) > 0) {
		oledDrawBitmap(44, 4, &bmp_logo48);
		oledDrawStringCenter(OLED_HEIGHT - 8, label);
	} else {
		oledDrawBitmap(40, 0, &bmp_logo64);
	}
	oledRefresh();
}

void layoutScreen(void)
{
	int i;
	uint8_t bmp_test_screen_data[1024];

	for (i = 0; i < 1024; i++)
		bmp_test_screen_data[i] = 0xff;

	const BITMAP  bmp_test_screen = {128, 64, bmp_test_screen_data};	

	oledDrawBitmap(0, 0, &bmp_test_screen);
	oledRefresh();
}

const char *str_amount(uint64_t amnt, const char *abbr, char *buf, int len)
{
	memset(buf, 0, len);
	uint64_t a = amnt, b = 1;
	int i;
	for (i = 0; i < 8; i++) {
		buf[16 - i] = '0' + (a / b) % 10;
		b *= 10;
	}
	buf[8] = '.';
	for (i = 0; i < 8; i++) {
		buf[7 - i] = '0' + (a / b) % 10;
		b *= 10;
	}
	i = 17;
	while (i > 10 && buf[i - 1] == '0') { // drop trailing zeroes
		i--;
	}
	if (abbr) {
		buf[i] = ' ';
		strlcpy(buf + i + 1, abbr, len - i - 1);
	} else {
		buf[i] = 0;
	}
	const char *r = buf;
	while (*r == '0' && *(r + 1) != '.') r++; // drop leading zeroes
	return r;
}

static char buf_out[32], buf_fee[32];

void layoutConfirmOutput(const CoinType *coin, const TxOutputType *out)
{
	static char first_half[17 + 1];
	strlcpy(first_half, out->address, sizeof(first_half));
	const char *str_out = str_amount(out->amount, coin->has_coin_shortcut ? coin->coin_shortcut : NULL, buf_out, sizeof(buf_out));
	switch (storage_getLang()) {
		case CHINESE :
			layoutZhDialogSwipe(DIALOG_ICON_QUESTION,
					"取消",
					"确认",
					NULL,
					str_out,
					"发送到#:#",
					first_half,
					out->address + 17
					);
			break;
		default	:
			layoutDialogSwipe(DIALOG_ICON_QUESTION,
					"Cancel",
					"Confirm",
					NULL,
					"Confirm sending",
					str_out,
					"to",
					first_half,
					out->address + 17,
					NULL
					);
			break;
	}
}

void layoutConfirmTx(const CoinType *coin, uint64_t amount_out, uint64_t amount_fee)
{
	const char *str_out = str_amount(amount_out, coin->has_coin_shortcut ? coin->coin_shortcut : NULL, buf_out, sizeof(buf_out));
	const char *str_fee = str_amount(amount_fee, coin->has_coin_shortcut ? coin->coin_shortcut : NULL, buf_fee, sizeof(buf_fee));
	switch (storage_getLang()) {
		case CHINESE :
			layoutZhDialogSwipe(DIALOG_ICON_QUESTION,
					"取消",
					"确认",
					NULL,
					"你的钱包将发送#:#",
					str_out,
					"手续费#:#",
					str_fee
					);
			break;
		default :
			layoutDialogSwipe(DIALOG_ICON_QUESTION,
					"Cancel",
					"Confirm",
					NULL,
					"Really send",
					str_out,
					"from your wallet?",
					"Fee will be",
					str_fee,
					NULL
					);
			break;

	}
}

void layoutFeeOverThreshold(const CoinType *coin, uint64_t fee, uint32_t kb)
{
	(void)kb;
	const char *str_out = str_amount(fee, coin->has_coin_shortcut ? coin->coin_shortcut : NULL, buf_out, sizeof(buf_out));
	switch (storage_getLang()) {
		case CHINESE :
			layoutZhDialogSwipe(DIALOG_ICON_QUESTION,
					"取消",
					"确认",
					NULL,
					str_out,
					"的手续费过高",
					NULL,
					"确认发送吗#?#"
					);
			break;
		default :
			layoutDialogSwipe(DIALOG_ICON_QUESTION,
					"Cancel",
					"Confirm",
					NULL,
					"Fee",
					str_out,
					"is unexpectedly high.",
					NULL,
					"Send anyway?",
					NULL
					);
			break;
	
	}
}

void layoutSignMessage(const uint8_t *msg, uint32_t len)
{
	bool ascii = true;
	uint32_t i;
	for (i = 0; i < len; i++) {
		if (msg[i] < 0x20 || msg[i] >= 0x80) {
			ascii = false;
			break;
		}
	}

	char str[4][17];
	memset(str, 0, sizeof(str));
	if (ascii) {
		strlcpy(str[0], (char *)msg, 17);
		if (len > 16) {
			strlcpy(str[1], (char *)msg + 16, 17);
		}
		if (len > 32) {
			strlcpy(str[2], (char *)msg + 32, 17);
		}
		if (len > 48) {
			strlcpy(str[3], (char *)msg + 48, 17);
		}
	} else {
		data2hex(msg, len > 8 ? 8 : len, str[0]);
		if (len > 8) {
			data2hex(msg + 8, len > 16 ? 8 : len - 8, str[1]);
		}
		if (len > 16) {
			data2hex(msg + 16, len > 24 ? 8 : len - 16, str[2]);
		}
		if (len > 24) {
			data2hex(msg + 24, len > 32 ? 8 : len - 24, str[3]);
		}
	}
	
	switch (storage_getLang()) {
		case CHINESE :
			layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认",
					ascii ? "签名文本信息#?#" : "签名二进制信息#?#",
					str[0], str[1], str[2], str[3]);
			break;
		default :
			layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm",
					ascii ? "Sign text message?" : "Sign binary message?",
					str[0], str[1], str[2], str[3], NULL, NULL);
			break;
	}
}

void layoutVerifyMessage(const uint8_t *msg, uint32_t len)
{
	bool ascii = true;
	uint32_t i;
	for (i = 0; i < len; i++) {
		if (msg[i] < 0x20 || msg[i] >= 0x80) {
			ascii = false;
			break;
		}
	}

	char str[4][17];
	memset(str, 0, sizeof(str));
	if (ascii) {
		strlcpy(str[0], (char *)msg, 17);
		if (len > 16) {
			strlcpy(str[1], (char *)msg + 16, 17);
		}
		if (len > 32) {
			strlcpy(str[2], (char *)msg + 32, 17);
		}
		if (len > 48) {
			strlcpy(str[3], (char *)msg + 48, 17);
		}
	} else {
		data2hex(msg, len > 8 ? 8 : len, str[0]);
		if (len > 8) {
			data2hex(msg + 8, len > 16 ? 8 : len - 8, str[1]);
		}
		if (len > 16) {
			data2hex(msg + 16, len > 24 ? 8 : len - 16, str[2]);
		}
		if (len > 24) {
			data2hex(msg + 24, len > 32 ? 8 : len - 24, str[3]);
		}
	}

	switch (storage_getLang()) {
		case CHINESE :
			layoutZhDialogSwipe(DIALOG_ICON_QUESTION, NULL, "确认",
					ascii ? "消息内容" : "二进制消息内容",
					str[0], str[1], str[2], str[3]);
			break;
		default :
			layoutDialogSwipe(DIALOG_ICON_INFO, NULL, "OK",
					ascii ? "Message contents" : "Binary message contents",
					str[0], str[1], str[2], str[3], NULL, NULL);
			break;
	}
}

void layoutCipherKeyValue(bool encrypt, const char *key)
{
	int len = strlen(key);
	char str[4][17];
	memset(str, 0, sizeof(str));
	strlcpy(str[0], (char *)key, 17);
	if (len > 16) {
		strlcpy(str[1], (char *)key + 16, 17);
	}
	if (len > 32) {
		strlcpy(str[2], (char *)key + 32, 17);
	}
	if (len > 48) {
		strlcpy(str[3], (char *)key + 48, 17);
	}
	switch (storage_getLang()) {
		case CHINESE :
			layoutZhDialogSwipe(DIALOG_ICON_QUESTION, "取消", "确认",
					encrypt ? "加密" : "解密",
					str[0], str[1], str[2], str[3]);
			break;
		default :
			layoutDialogSwipe(DIALOG_ICON_QUESTION, "Cancel", "Confirm",
					encrypt ? "Encrypt?" : "Decrypt?",
					str[0], str[1], str[2], str[3], NULL, NULL);
			break;
	}
}

void layoutAddress(const char *address)
{
	oledSwipeLeft();
	layoutLast = layoutAddress;

	static unsigned char bitdata[QR_MAX_BITDATA];
	int a, i, j;
	int side = qr_encode(QR_LEVEL_M, 0, address, 0, bitdata);

	if (side > 0 && side <= 29) {
		oledInvert(0, 0, (side + 2) * 2, (side + 2) * 2);
		for (i = 0; i < side; i++) {
			for (j = 0; j< side; j++) {
				a = i * side + j;
				if (bitdata[a / 8] & (1 << (7 - a % 8))) {
					oledClearPixel(2 + i * 2, 2 + j * 2);
					oledClearPixel(3 + i * 2, 2 + j * 2);
					oledClearPixel(2 + i * 2, 3 + j * 2);
					oledClearPixel(3 + i * 2, 3 + j * 2);
				}
			}
		}
	}

	int len = strlen(address);
	char str[4][10];
	memset(str, 0, sizeof(str));

	strlcpy(str[0], (char *)address, 10);
	if (len > 9) {
		strlcpy(str[1], (char *)address + 9, 10);
	}
	if (len > 18) {
		strlcpy(str[2], (char *)address + 18, 10);
	}
	if (len > 27) {
		strlcpy(str[3], (char *)address + 27, 10);
	}

	oledDrawString(68, 0 * 9, str[0]);
	oledDrawString(68, 1 * 9, str[1]);
	oledDrawString(68, 2 * 9, str[2]);
	oledDrawString(68, 3 * 9, str[3]);
	
	static const char *btnYes = "Continue";
	static const char *btnZhYes = "继续";
	switch (storage_getLang()) {
		case CHINESE :
			oledDrawZh(OLED_WIDTH - 9, OLED_HEIGHT - 12, "#}#");
			oledDrawZh(OLED_WIDTH - ((strlen(btnZhYes) / 3) * 12) - 11, OLED_HEIGHT - 12, btnZhYes);
			oledInvert(OLED_WIDTH - ((strlen(btnZhYes) / 3) * 12) - 12, OLED_HEIGHT - 13, OLED_WIDTH - 1, OLED_HEIGHT - 1);
			break;
		default :
			oledDrawString(OLED_WIDTH - fontCharWidth('}') - 1, OLED_HEIGHT - 8, "}");
			oledDrawString(OLED_WIDTH - fontStringWidth(btnYes) - fontCharWidth('}') - 3, OLED_HEIGHT - 8, btnYes);
			oledInvert(OLED_WIDTH - fontStringWidth(btnYes) - fontCharWidth('}') - 4, OLED_HEIGHT - 9, OLED_WIDTH - 1, OLED_HEIGHT - 1);
			break;
	}

	oledRefresh();
}
