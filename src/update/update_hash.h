//
// update_hash.h
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// SHA-256 (FIPS 180-4) for the updater. (CRC-32 comes from zlib.)
//

#ifndef BMC64_UPDATE_HASH_H
#define BMC64_UPDATE_HASH_H

#include <stdint.h>

typedef struct {
  uint32_t state[8];
  uint64_t length;
  uint8_t block[64];
  unsigned used;
} uh_sha256;

void uh_sha256_init(uh_sha256 *s);
void uh_sha256_update(uh_sha256 *s, const void *data, unsigned len);
void uh_sha256_final(uh_sha256 *s, uint8_t digest[32]);

// Hex helpers for the manifest: 64 lower-case hex digits <-> 32 bytes.
int uh_hex_to_digest(const char *hex, uint8_t digest[32]);
void uh_digest_to_hex(const uint8_t digest[32], char hex[65]);

#endif
