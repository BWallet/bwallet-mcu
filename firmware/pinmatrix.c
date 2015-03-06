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

#include "pinmatrix.h"
#include "oled.h"
#include "rng.h"

static char pinmatrix_perm[10] = "XXXXXXXXX";

void pinmatrix_draw(const char *text)
{
	const BITMAP *bmp_digits[10] = {
		&bmp_digit0, &bmp_digit1, &bmp_digit2, &bmp_digit3, &bmp_digit4,
		&bmp_digit5, &bmp_digit6, &bmp_digit7, &bmp_digit8, &bmp_digit9,
	};
	oledSwipeLeft();
	const int w = bmp_digit0.width, h = bmp_digit0.height, pad = 2;
	int i, j, k;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			// use (2 - j) instead of j to achieve 789456123 layout
			k = pinmatrix_perm[i + (2 - j) * 3] - '0';
			if (*text < 128) {
				oledDrawStringCenter(0, text);
			}
			else {
				oledDrawZhCenter(0, text);
			}
			oledDrawBitmap((OLED_WIDTH - 3 * w - 2 * pad) / 2 + i * (w + pad), OLED_HEIGHT - 3 * h - 2 * pad + j * (h + pad), bmp_digits[k]);
		}
	}
	oledRefresh();
}

void pinmatrix_start(const char *text)
{
	int i;
	for (i = 0; i < 9; i++) {
		pinmatrix_perm[i] = '1' + i;
	}   
	pinmatrix_perm[9] = 0;
	random_permute(pinmatrix_perm, 9); 
	pinmatrix_draw(text);
}

void pinmatrix_done(char *pin)
{
	int k, i = 0;
	while (pin && pin[i]) {
		k = pin[i] - '1';
		if (k >= 0 && k <= 8) {
			pin[i] = pinmatrix_perm[k];
		} else {
			pin[i] = 'X';
		}
		i++;
	}
	memset(pinmatrix_perm, 'X', sizeof(pinmatrix_perm));
}

const char *pinmatrix_get(void)
{
	return pinmatrix_perm;
}
