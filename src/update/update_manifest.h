//
// update_manifest.h
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
// bmc64-manifest.txt: every file of every released version with its SHA-256
// and size. Written by tools/update/gen_update_manifest.py.
//
//   # comment
//   format 1
//   target v5.1.11
//   <version> <sha256> <size> <path>
//

#ifndef BMC64_UPDATE_MANIFEST_H
#define BMC64_UPDATE_MANIFEST_H

#include <stdint.h>

#define UM_NAME "bmc64-manifest.txt"
#define UM_FORMAT 1

typedef struct {
  const char *version;
  const char *path;
  uint8_t sha[32];
  uint32_t size;
  int is_target;         // part of the version this zip installs
} um_record;

typedef struct {
  char target[32];
  int legacy;            // the zip had no manifest: a release from before the
                         // updater, recognised from the card's history
  um_record *recs;
  int count;
  int cap;
  char *texts[2];        // parsed text buffers, owned
  int ntexts;
} um_manifest;

// Parses a manifest. The primary one (from the zip) sets the target; a
// secondary one (the card's copy of the installed manifest) only adds
// history. Takes ownership of text, which must be NUL-terminated.
// Returns 0 on success.
int um_parse(um_manifest *m, char *text, int primary, char *err,
             unsigned errlen);
void um_free(um_manifest *m);

// 1 if any release has shipped this path with this hash.
int um_known(const um_manifest *m, const char *path, const uint8_t sha[32]);

// Compares two versions ("v5.1.10", "5.1.9"): < 0, 0 or > 0.
int um_version_cmp(const char *a, const char *b);

#endif
