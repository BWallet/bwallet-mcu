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

#include "signatures.h"
#include "ecdsa.h"
#include "bootloader.h"

#define PUBKEYS 5

static const uint8_t *pubkey[PUBKEYS] = {
	(uint8_t *)"\x04\xb5\xbe\xdc\x48\x54\xac\x22\xaf\x4f\x88\x95\xed\x7a\xa4\x35\xec\xac\xed\x4f\x3e\xab\x41\x73\x5d\x5c\xfb\xed\x52\x77\xdb\x35\xc0\x8c\xd0\x1f\x2a\xea\xe7\xbb\x81\xaf\x67\x56\xc7\xc1\x9c\x9b\x9c\xa0\xc5\xa6\x3c\x06\xe3\x13\x4c\xb7\x95\xa0\xc2\x92\x90\x12\xe0",
	(uint8_t *)"\x04\xdc\xb8\x5c\x8b\x2e\x1c\x16\xd9\x35\xf1\xf8\xc5\xee\x24\x1f\x31\x4b\x3c\x64\x76\x77\x45\x99\xbc\xd1\x39\x13\xac\x9c\x29\xe5\x7a\x83\x6b\x76\x68\x59\x62\x6b\xc9\x84\x92\xc6\x78\xd4\x28\xb7\x5c\xc0\x58\x91\xfb\x9c\x8d\x8b\x3f\xe5\xf3\xfa\x9d\xe1\x5b\x6a\x21",
	(uint8_t *)"\x04\x42\x8c\x72\xf1\xb3\x7b\x2e\xc9\xbd\xd5\x55\x3e\x94\x44\x3d\x3f\x26\x29\xd7\x70\x2b\x5b\x8f\x63\xe7\xab\x77\xa7\x86\x07\x3e\xc8\x18\x49\x21\xfd\x2d\x2c\x60\x05\x1e\x66\x76\x89\xf1\x30\x19\xcf\x4a\xc9\x1e\x26\x7d\x98\x85\x7e\x05\x17\xf3\xb7\x6f\xec\xb9\xd5",
	(uint8_t *)"\x04\x90\xf1\xea\x6d\x4b\x2a\x06\xa1\xed\xcb\xc8\x14\x5c\x2b\x0c\x23\xf7\x29\xa2\xa4\x3b\x39\x2b\x7c\xd6\xac\x26\x19\xa7\x56\x78\x02\x05\x24\x3c\xd8\x5b\x89\x09\xb7\x4b\xb2\x19\x55\xa5\x80\x87\xba\x66\x6c\x9b\x4f\x84\x51\xd5\xea\xa1\xbc\x7f\x66\x46\x0f\x57\x93",
	(uint8_t *)"\x04\xae\xb9\xf3\xaf\x61\x29\xe4\x4a\x83\xb8\x97\x64\x8f\x91\x30\x7f\xc6\x4a\xe5\xd9\xa8\x5f\xee\xbf\x44\x2e\xba\x34\xa6\x91\xe4\x81\x97\xc8\x60\x07\x62\xf7\x78\x9a\x01\x74\x4f\xc9\xf8\x7c\x05\xc7\x89\x43\xf1\x5c\xb7\xee\xba\xe7\x91\x73\x49\x94\xa0\x00\xa3\x5b",
};

#define SIGNATURES 3

int signatures_ok(void)
{
	uint32_t codelen = *((uint32_t *)FLASH_META_CODELEN);
	uint8_t sigindex1, sigindex2, sigindex3;

	sigindex1 = *((uint8_t *)FLASH_META_SIGINDEX1);
	sigindex2 = *((uint8_t *)FLASH_META_SIGINDEX2);
	sigindex3 = *((uint8_t *)FLASH_META_SIGINDEX3);

	if (sigindex1 < 1 || sigindex1 > PUBKEYS) return 0; // invalid index
	if (sigindex2 < 1 || sigindex2 > PUBKEYS) return 0; // invalid index
	if (sigindex3 < 1 || sigindex3 > PUBKEYS) return 0; // invalid index

	if (sigindex1 == sigindex2) return 0; // duplicate use
	if (sigindex1 == sigindex3) return 0; // duplicate use
	if (sigindex2 == sigindex3) return 0; // duplicate use

	if (ecdsa_verify(pubkey[sigindex1 - 1], (uint8_t *)FLASH_META_SIG1, (uint8_t *)FLASH_APP_START, codelen) != 0) { // failure
		return 0;
	}
	if (ecdsa_verify(pubkey[sigindex2 - 1], (uint8_t *)FLASH_META_SIG2, (uint8_t *)FLASH_APP_START, codelen) != 0) { // failure
		return 0;
	}
	if (ecdsa_verify(pubkey[sigindex3 - 1], (uint8_t *)FLASH_META_SIG3, (uint8_t *)FLASH_APP_START, codelen) != 0) { // failture
		return 0;
	}

	return 1;
}
