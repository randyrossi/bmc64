//
// webui_profiles.cpp
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
// GET /api/profiles/files: every profile file's text in one response. The
// device only reads and sends the files; the page parses them, as it does
// files it downloads one at a time.
//

#include "webui_profiles.h"
#include "webui.h"

#if BMC64_WEBUI

#include "webui_fs.h"
#include "webui_http.h"
#include "../profiles/profiles.h"

#include <circle/net/socket.h>
#include <circle/sched/scheduler.h>
#include <circle/types.h>

#include <stdio.h>
#include <string.h>

#include "../sdcard/sd_fs.h"

// Implemented in src/vice_network.cpp.
extern "C" const char *circle_get_disk_volume(void);
// src/profiles/profile_store.c
extern "C" int profiles_id_valid(const char *id);

using webhttp::CChunkedResponse;

namespace {

const unsigned kMaxProfiles = 256;
// Files kept from one folder: a profile has at most a handful.
const unsigned kMaxFolderFiles = 32;
const unsigned kMaxNameLen = 63;
const unsigned kMaxFileBytes = 64 * 1024;

// One request is handled at a time by the web UI task, so these are safe
// as statics and stay off the task stack. Folders are listed into them
// first and read after, so no file is opened while a listing is open.
char s_ids[kMaxProfiles][PROFILES_MAX_ID_LEN + 1];
unsigned s_id_count;
boolean s_ids_truncated;
char s_names[kMaxFolderFiles][kMaxNameLen + 1];
unsigned s_name_count;
u8 s_buffer[4096];

// The profile folders, as the Profiles page lists them.
int CollectId(void *ctx, const sd_info *info) {
  (void) ctx;
  if (!info->is_dir || !profiles_id_valid(info->name) ||
      strcmp(info->name, PROFILES_MAIN_ID) == 0) {
    return 0;
  }
  if (s_id_count >= kMaxProfiles) {
    s_ids_truncated = TRUE;
    return 1;
  }
  strcpy(s_ids[s_id_count++], info->name);
  return 0;
}

// The profile files in one folder (WebUiFsIsProfileFile). Names outside
// plain ASCII are skipped: none of the profile files has one.
int CollectName(void *ctx, const sd_info *info) {
  const char *folder = (const char *) ctx;
  size_t length = strlen(info->name);
  if (info->is_dir || info->size > kMaxFileBytes || length > kMaxNameLen) {
    return 0;
  }
  for (size_t i = 0; i < length; i++) {
    if ((unsigned char) info->name[i] >= 0x80) return 0;
  }
  char path[PROFILES_MAX_PATH_LEN];
  if ((unsigned) snprintf(path, sizeof(path), "%s/%s", folder, info->name) >=
          sizeof(path) ||
      !WebUiFsIsProfileFile(path)) {
    return 0;
  }
  if (s_name_count >= kMaxFolderFiles) {
    return 1;
  }
  strcpy(s_names[s_name_count++], info->name);
  return 0;
}

// The inside of a JSON string: quotes, backslashes and control characters
// escaped, every other byte as it is, so UTF-8 text stays UTF-8.
void WriteJsonText(CChunkedResponse *r, const u8 *data, unsigned length) {
  unsigned run = 0;  // start of the bytes not written yet
  for (unsigned i = 0; i < length; i++) {
    u8 c = data[i];
    if (c >= 0x20 && c != '"' && c != '\\') {
      continue;
    }
    r->WriteN(data + run, i - run);
    run = i + 1;
    if (c == '"' || c == '\\') {
      char escaped[2] = {'\\', (char) c};
      r->WriteN(escaped, 2);
    } else if (c == '\n') {
      r->Write("\\n");
    } else if (c == '\r') {
      r->Write("\\r");
    } else if (c == '\t') {
      r->Write("\\t");
    } else {
      r->Printf("\\u%04x", c);
    }
  }
  r->WriteN(data + run, length - run);
}

void WriteJsonText(CChunkedResponse *r, const char *text) {
  WriteJsonText(r, (const u8 *) text, (unsigned) strlen(text));
}

// "path":"text" for one file; nothing if it can't be opened or is too big.
void WriteFile(CChunkedResponse *r, const char *path, boolean *first) {
  sd_file *file = sd_open(path, SD_READ);
  if (file == 0) {
    return;
  }
  if (sd_size(file) > kMaxFileBytes) {
    sd_close(file);
    return;
  }
  r->Write(*first ? "\"" : ",\"");
  *first = FALSE;
  WriteJsonText(r, path);
  r->Write("\":\"");
  for (;;) {
    unsigned got = 0;
    if (sd_read(file, s_buffer, sizeof(s_buffer), &got) != SD_OK || got == 0) {
      break;
    }
    WriteJsonText(r, s_buffer, got);
  }
  r->Write("\"");
  sd_close(file);
  CScheduler::Get()->Yield();
}

void WriteFolder(CChunkedResponse *r, const char *folder, boolean *first) {
  s_name_count = 0;
  sd_list_info(folder, CollectName, (void *) folder);
  char path[PROFILES_MAX_PATH_LEN];
  for (unsigned i = 0; i < s_name_count; i++) {
    snprintf(path, sizeof(path), "%s/%s", folder, s_names[i]);
    WriteFile(r, path, first);
  }
}

}  // namespace

void WebUiProfileFiles(CSocket *socket) {
  sd_info info;
  boolean folder = sd_stat_info(PROFILES_DIR, &info) == SD_OK && info.is_dir;
  s_id_count = 0;
  s_ids_truncated = FALSE;
  if (folder) {
    sd_list_info(PROFILES_DIR, CollectId, 0);
  }

  CChunkedResponse r(socket, 200, "OK", "application/json");
  r.Write("{\"vol\":\"");
  WriteJsonText(&r, circle_get_disk_volume());
  r.Printf("\",\"folder\":%s,\"ids\":[", folder ? "true" : "false");
  for (unsigned i = 0; i < s_id_count; i++) {
    r.Write(i ? ",\"" : "\"");
    r.Write(s_ids[i]);  // a valid id is [a-z0-9-], nothing to escape
    r.Write("\"");
  }
  r.Printf("],\"truncated\":%s,\"files\":{", s_ids_truncated ? "true" : "false");

  boolean first = TRUE;
  if (folder) {
    WriteFile(&r, PROFILES_DIR "/active.txt", &first);
    WriteFolder(&r, PROFILES_DIR "/" PROFILES_MAIN_ID, &first);
    char path[PROFILES_MAX_PATH_LEN];
    for (unsigned i = 0; i < s_id_count; i++) {
      snprintf(path, sizeof(path), "%s/%s", PROFILES_DIR, s_ids[i]);
      WriteFolder(&r, path, &first);
    }
  }
  r.Write("}}");
  r.Finish();
}

#endif  // BMC64_WEBUI
