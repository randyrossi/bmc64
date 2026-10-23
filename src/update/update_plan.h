//
// update_plan.h
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
// Compares the SD card with the manifest and decides, per file, what the
// update view offers (New / Update / Changed by you) and the default choice.
//

#ifndef BMC64_UPDATE_PLAN_H
#define BMC64_UPDATE_PLAN_H

#include "update_manifest.h"
#include "update_zip.h"

enum { UP_GROUP_KERNEL, UP_GROUP_FIRMWARE, UP_GROUP_CONFIG, UP_GROUPS };

enum {
  UP_NEW,       // not on the card
  UP_UPDATE,    // a file from some release, never changed by the user
  UP_CHANGED,   // matches no release: the user changed it
};

typedef struct {
  const um_record *rec;  // the target version's record
  int zip_index;
  int group;
  int status;
  int replace;           // the user's choice; defaults from status
  int this_board;        // a kernel of the board BMC64 is running on
  int order;             // when it is moved into place: boot files last
} up_item;

typedef struct {
  up_item *items;        // only files that differ, sorted by group then path
  int count;
  int up_to_date;        // files already identical to the target
  const char **dirs;     // folders in the zip (created if missing)
  int ndirs;
  char running[32];      // "5.1.10"
  char target[32];       // "v5.1.11"
  int direction;         // < 0 update, 0 same version, > 0 downgrade
  int legacy;            // a release from before the updater (no manifest in
                         // the zip): once installed, the updater is gone
} up_plan;

// Called while card files are being checked.
typedef void (*up_progress_fn)(void *ctx, const char *name, int done,
                               int total);

// Loads the zip's manifest, then the card's copy of the installed one, which
// adds the history a downgrade needs to recognise the newer release's files.
// A zip without a manifest (a release from before the updater) is recognised
// from the card's copy instead, which lists every earlier release.
// Returns 0 on success; err explains a failure in a sentence for the user.
int up_load_manifests(uz_zip *z, um_manifest *m, char *err, unsigned errlen);

// Builds the plan. board_kernel is "kernel.img", "kernel7.img" or
// "kernel8-32.img". Returns 0 on success.
int up_build(up_plan *p, uz_zip *z, const um_manifest *m, const char *running,
             const char *board_kernel, up_progress_fn progress, void *ctx,
             char *err, unsigned errlen);
void up_free(up_plan *p);

// Hashes a file on the card. Returns 0 on success.
int up_hash_file(const char *path, uint8_t sha[32]);

// 1 if path is safe to write on the card: relative, printable ASCII, no
// "..", no characters FAT can't store.
int up_path_ok(const char *path);

#endif
