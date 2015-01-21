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

#include <libopencm3/stm32/desig.h>

#include "serialno.h"
#include "util.h"
#include "sha2.h"

void fill_serialno_fixed(char *s)
{
	uint8_t uuid[32];
	desig_get_unique_id((uint32_t *)uuid);
	sha256_Raw(uuid, 12, uuid);
	sha256_Raw(uuid, 32, uuid);
	data2hex(uuid, 12, s);
}
