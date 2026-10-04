//
// webui_fs.cpp
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "webui_fs.h"

#if defined(RASPI_C64) || defined(RASPI_C128)

#include "webui_http.h"

#include <circle/net/socket.h>
#include <circle/sched/scheduler.h>
#include <circle/types.h>

#include <stdio.h>
#include <string.h>

#include "../sdcard/sd_fs.h"

// Implemented in src/vice_network.cpp.
extern "C" const char *circle_get_disk_volume(void);
// Queues a file for the emulator main loop to autostart (interrupt safe;
// see third_party/common/ui.c).
extern "C" void emu_autostart_interrupt(const char *path);

using webhttp::CChunkedResponse;

namespace {

const unsigned WEBUI_FS_MAX_ENTRIES = 6000;
const unsigned WEBUI_FS_IO_CHUNK = 32 * 1024;
// Largest file the editor will open or save (the config files are a few KB;
// the body is streamed to a temp file, so this is not a RAM limit).
const unsigned WEBUI_FS_EDIT_MAX = 256 * 1024;

// One request is handled at a time by the web UI task, so a single file
// scratch buffer is safe and keeps it off the task stack.
u8 s_io_buffer[WEBUI_FS_IO_CHUNK];

// CP850 (the FatFs OEM code page in this build) 0x80..0xFF -> Unicode.
const unsigned short kCp850High[128] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    0x00FF, 0x00D6, 0x00DC, 0x00F8, 0x00A3, 0x00D8, 0x00D7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    0x00BF, 0x00AE, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00C1, 0x00C2, 0x00C0,
    0x00A9, 0x2563, 0x2551, 0x2557, 0x255D, 0x00A2, 0x00A5, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x00E3, 0x00C3,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x00A4,
    0x00F0, 0x00D0, 0x00CA, 0x00CB, 0x00C8, 0x0131, 0x00CD, 0x00CE,
    0x00CF, 0x2518, 0x250C, 0x2588, 0x2584, 0x00A6, 0x00CC, 0x2580,
    0x00D3, 0x00DF, 0x00D4, 0x00D2, 0x00F5, 0x00D5, 0x00B5, 0x00FE,
    0x00DE, 0x00DA, 0x00DB, 0x00D9, 0x00FD, 0x00DD, 0x00AF, 0x00B4,
    0x00AD, 0x00B1, 0x2017, 0x00BE, 0x00B6, 0x00A7, 0x00F7, 0x00B8,
    0x00B0, 0x00A8, 0x00B7, 0x00B9, 0x00B3, 0x00B2, 0x25A0, 0x00A0,
};

void EmitUtf8(CChunkedResponse *r, unsigned cp) {
  char out[3];
  if (cp < 0x80) {
    out[0] = (char) cp;
    r->WriteN(out, 1);
  } else if (cp < 0x800) {
    out[0] = (char) (0xC0 | (cp >> 6));
    out[1] = (char) (0x80 | (cp & 0x3F));
    r->WriteN(out, 2);
  } else {
    out[0] = (char) (0xE0 | (cp >> 12));
    out[1] = (char) (0x80 | ((cp >> 6) & 0x3F));
    out[2] = (char) (0x80 | (cp & 0x3F));
    r->WriteN(out, 3);
  }
}

// Write s as a JSON string literal (with surrounding quotes), escaping
// control characters and translating the OEM high range to UTF-8.
void JsonString(CChunkedResponse *r, const char *s) {
  r->Write("\"");
  for (const unsigned char *p = (const unsigned char *) s; *p != '\0'; p++) {
    unsigned char c = *p;
    if (c == '"' || c == '\\') {
      char esc[2] = {'\\', (char) c};
      r->WriteN(esc, 2);
    } else if (c == '\n') {
      r->Write("\\n");
    } else if (c == '\r') {
      r->Write("\\r");
    } else if (c == '\t') {
      r->Write("\\t");
    } else if (c < 0x20) {
      r->Printf("\\u%04x", c);
    } else if (c < 0x80) {
      char ch = (char) c;
      r->WriteN(&ch, 1);
    } else {
      EmitUtf8(r, kCp850High[c - 0x80]);
    }
  }
  r->Write("\"");
}

void WriteDateTime(CChunkedResponse *r, const sd_time *t) {
  if (t->year == 0) {
    return;  // unknown timestamp -> empty string
  }
  r->Printf("%04u-%02u-%02uT%02u:%02u:%02u", t->year, t->month, t->day,
            t->hour, t->minute, t->second);
}

// Parse "YYYY-MM-DDTHH:MM:SS" (local wall-clock time, as WriteDateTime
// emits). Returns FALSE if malformed or outside what FAT can store
// (1980..2107).
boolean ParseDateTime(const char *s, sd_time *t) {
  unsigned year, month, day, hour, minute, second;
  if (sscanf(s, "%4u-%2u-%2uT%2u:%2u:%2u", &year, &month, &day, &hour, &minute,
             &second) != 6) {
    return FALSE;
  }
  if (year < 1980 || year > 2107 || month < 1 || month > 12 || day < 1 ||
      day > 31 || hour > 23 || minute > 59 || second > 59) {
    return FALSE;
  }
  t->year = (uint16_t) year;
  t->month = (uint8_t) month;
  t->day = (uint8_t) day;
  t->hour = (uint8_t) hour;
  t->minute = (uint8_t) minute;
  t->second = (uint8_t) second;
  return TRUE;
}

// Normalise a client path into a sandboxed absolute path within a volume.
// Returns 0 on success (out starts with '/'), -1 if the path is unsafe.
int SanitizeRelPath(const char *in, char *out, unsigned out_size) {
  if (out_size < 2) {
    return -1;
  }
  if (in == 0 || in[0] == '\0') {
    strcpy(out, "/");
    return 0;
  }
  if (in[0] != '/') {
    return -1;
  }

  unsigned o = 1;
  out[0] = '/';
  const char *p = in;
  while (*p != '\0') {
    while (*p == '/') {
      p++;
    }
    const char *seg = p;
    while (*p != '\0' && *p != '/') {
      p++;
    }
    unsigned seg_len = (unsigned) (p - seg);
    if (seg_len == 0) {
      continue;
    }
    if (seg_len > SD_MAX_NAME_LEN) {
      return -1;
    }
    if (seg_len == 1 && seg[0] == '.') {
      continue;
    }
    if (seg_len == 2 && seg[0] == '.' && seg[1] == '.') {
      return -1;
    }
    for (unsigned i = 0; i < seg_len; i++) {
      unsigned char c = (unsigned char) seg[i];
      if (c < 0x20 || c == '\\' || c == ':' || c == '*' || c == '?' ||
          c == '"' || c == '<' || c == '>' || c == '|') {
        return -1;
      }
    }
    if (o > 1) {
      if (o + 1 >= out_size) {
        return -1;
      }
      out[o++] = '/';
    }
    if (o + seg_len >= out_size) {
      return -1;
    }
    memcpy(out + o, seg, seg_len);
    o += seg_len;
  }
  out[o] = '\0';
  return 0;
}

// Resolve and validate the vol/path query parameters. Returns 0 on
// success and fills clean, a path on the card for sd_fs.h; on failure it has
// already sent an error response and returns -1.
int ResolveTarget(CSocket *socket, const char *query, char *clean,
                  unsigned clean_size, boolean require_file) {
  char vol[24];
  char raw[560];
  const char *disk = circle_get_disk_volume();

  if (!webhttp::QueryParam(query, "vol", vol, sizeof(vol)) || vol[0] == '\0') {
    strncpy(vol, disk, sizeof(vol) - 1);
    vol[sizeof(vol) - 1] = '\0';
  }
  if (strcmp(vol, disk) != 0) {
    webhttp::SendText(socket, 404, "Not Found", "unknown volume\n");
    return -1;
  }
  if (!webhttp::QueryParam(query, "path", raw, sizeof(raw))) {
    if (query != 0 &&
        (strncmp(query, "path=", 5) == 0 || strstr(query, "&path=") != 0)) {
      webhttp::SendText(socket, 400, "Bad Request", "path too long or invalid\n");
      return -1;
    }
    strcpy(raw, "/");
  }
  if (SanitizeRelPath(raw, clean, clean_size) != 0) {
    webhttp::SendText(socket, 400, "Bad Request", "bad path\n");
    return -1;
  }
  if (require_file && strcmp(clean, "/") == 0) {
    webhttp::SendText(socket, 400, "Bad Request", "not a file\n");
    return -1;
  }
  return 0;
}

boolean CiEqual(const char *a, const char *b) {
  for (; *a != '\0' && *b != '\0'; a++, b++) {
    char ca = *a;
    char cb = *b;
    if (ca >= 'A' && ca <= 'Z') ca = (char) (ca + 32);
    if (cb >= 'A' && cb <= 'Z') cb = (char) (cb + 32);
    if (ca != cb) return FALSE;
  }
  return *a == *b;
}

// Only image/program types that the menu's Autostart accepts as-is. A .crt
// cartridge image is one: VICE's autostart_autodetect attaches it on the
// C64 and C128, which are the machines the web UI runs on.
boolean IsAutostartable(const char *clean) {
  const char *dot = 0;
  for (const char *p = clean; *p != '\0'; p++) {
    if (*p == '/') dot = 0;
    else if (*p == '.') dot = p + 1;
  }
  if (dot == 0) return FALSE;
  static const char *const kExt[] = {
      "d64", "d71", "d81", "d82", "g64", "x64", "t64", "tap", "prg", "p00",
      "crt",
  };
  for (unsigned i = 0; i < sizeof(kExt) / sizeof(kExt[0]); i++) {
    if (CiEqual(dot, kExt[i])) return TRUE;
  }
  return FALSE;
}

// BMC64's own configuration files. Upload and delete refuse them (in any
// folder); the editor's save endpoint is the only way the web UI changes them.
const char *const kConfigFiles[] = {
    "settings.txt",     "settings-c128.txt",     "settings-vic20.txt",
    "settings-plus4.txt", "settings-plus4emu.txt", "settings-pet.txt",
    "wpa_supplicant.conf", "cmdline.txt",         "config.txt",
    "machines.txt",
};

boolean IsConfigFile(const char *name) {
  for (unsigned i = 0; i < sizeof(kConfigFiles) / sizeof(kConfigFiles[0]); i++) {
    if (CiEqual(name, kConfigFiles[i])) return TRUE;
  }
  return FALSE;
}

// vice.ini is editable too, but (unlike the files above) stays uploadable so
// a prepared copy can still be dropped on the card.
boolean IsEditableName(const char *name) {
  return IsConfigFile(name) || CiEqual(name, "vice.ini");
}

// Keyboard mapping files are plain text that lives in the machine folders
// (C64/rpi_pos.vkm, ...), so they are editable wherever they are on the card.
boolean IsKeymapName(const char *name) {
  size_t length = strlen(name);
  return length > 4 && CiEqual(name + length - 4, ".vkm");
}

// Starts with "/profiles/" (any case); returns the rest, or 0.
const char *InProfilesFolder(const char *clean) {
  static const char kProfiles[] = "/profiles/";
  const unsigned length = sizeof(kProfiles) - 1;
  if (strlen(clean) <= length) return 0;
  char start[sizeof(kProfiles)];
  memcpy(start, clean, length);
  start[length] = '\0';
  return CiEqual(start, kProfiles) ? clean + length : 0;
}

// Profile files (docs/PROFILES.md, Profile files): active.txt in /profiles,
// each profile's profile.txt, settings.txt and vice.ini, and Main's
// /profiles/main/<machine>.txt.
boolean IsProfileFilePath(const char *clean) {
  const char *rest = InProfilesFolder(clean);
  if (rest == 0) return FALSE;
  const char *slash = strchr(rest, '/');
  if (slash == 0) {
    return CiEqual(rest, "active.txt");
  }
  const char *name = slash + 1;
  if (*name == '\0' || strchr(name, '/') != 0) return FALSE;  // one level only
  char folder[SD_MAX_NAME_LEN + 1];
  unsigned folder_length = (unsigned) (slash - rest);
  if (folder_length == 0 || folder_length > SD_MAX_NAME_LEN) return FALSE;
  memcpy(folder, rest, folder_length);
  folder[folder_length] = '\0';
  if (CiEqual(folder, "main")) {
    size_t length = strlen(name);
    return length > 4 && CiEqual(name + length - 4, ".txt");
  }
  return CiEqual(name, "profile.txt") || CiEqual(name, "settings.txt") ||
         CiEqual(name, "vice.ini");
}

// The config files are editable only in the volume root; keymaps anywhere;
// profile files in /profiles.
boolean IsEditablePath(const char *clean) {
  if (clean[0] != '/') return FALSE;
  const char *base = strrchr(clean, '/') + 1;
  return IsKeymapName(base) || (base == clean + 1 && IsEditableName(base)) ||
         IsProfileFilePath(clean);
}

// Uploads must not clobber BMC64's own configuration or the Wi-Fi
// firmware directory.
boolean IsProtectedPath(const char *clean) {
  const char *seg = clean;
  while (*seg == '/') seg++;
  const char *seg_end = seg;
  while (*seg_end != '\0' && *seg_end != '/') seg_end++;
  if ((unsigned) (seg_end - seg) == 8) {
    char first[9];
    memcpy(first, seg, 8);
    first[8] = '\0';
    if (CiEqual(first, "firmware")) return TRUE;
  }

  const char *base = clean;
  for (const char *p = clean; *p != '\0'; p++) {
    if (*p == '/') base = p + 1;
  }
  return IsConfigFile(base) || CiEqual(base, "bmc64.log");
}

// A name typed into New folder / Rename: one path segment that FAT stores
// exactly as written. Printable ASCII only, because paths reach FatFs
// unconverted (its OEM code page is CP850, and only the listing translates
// to UTF-8), so a UTF-8 name would be stored garbled. FatFs would also
// silently trim a trailing dot or space, so those are refused rather than
// creating a different name than the one asked for.
boolean IsValidEntryName(const char *name) {
  size_t length = strlen(name);
  if (length == 0 || length > SD_MAX_NAME_LEN) return FALSE;
  if (name[0] == ' ' || name[length - 1] == ' ' || name[length - 1] == '.') {
    return FALSE;
  }
  for (const char *p = name; *p != '\0'; p++) {
    unsigned char c = (unsigned char) *p;
    if (c < 0x20 || c >= 0x7F || c == '/' || c == '\\' || c == ':' ||
        c == '*' || c == '?' || c == '"' || c == '<' || c == '>' ||
        c == '|') {
      return FALSE;
    }
  }
  return TRUE;
}

const char *const kBadNameText =
    "invalid name: use plain ASCII without / \\ : * ? \" < > |, and no "
    "leading space or trailing space or dot\n";

enum BodyResult {
  BODY_OK,
  BODY_CREATE_FAILED,
  BODY_WRITE_FAILED,  // write error (disk full?)
  BODY_TRUNCATED,     // client aborted or timed out
  BODY_HAS_NUL,       // only when reject_nul is set
};

// Stream a request body into a new file at temppath: prefetched bytes (read
// with the headers) first, then the rest from the socket, `total` bytes in
// all. On anything but BODY_OK the temp file is removed.
BodyResult ReceiveBodyToFile(CSocket *socket, const char *temppath,
                             const unsigned char *prefetched,
                             unsigned prefetched_len, unsigned long total,
                             boolean reject_nul) {
  sd_file *file = sd_open(temppath, SD_WRITE);
  if (file == 0) {
    return BODY_CREATE_FAILED;
  }

  unsigned long written = 0;
  BodyResult result = BODY_OK;

  if (prefetched_len > total) {
    prefetched_len = (unsigned) total;
  }
  if (prefetched_len > 0) {
    if (reject_nul && memchr(prefetched, 0, prefetched_len) != 0) {
      result = BODY_HAS_NUL;
    } else if (sd_write(file, prefetched, prefetched_len) != SD_OK) {
      result = BODY_WRITE_FAILED;
    }
    written += prefetched_len;
  }

  while (result == BODY_OK && written < total) {
    unsigned long remain = total - written;
    unsigned want = remain < sizeof(s_io_buffer) ? (unsigned) remain
                                                 : sizeof(s_io_buffer);
    int n = socket->Receive(s_io_buffer, want, 0);
    if (n <= 0) {
      result = BODY_TRUNCATED;
      break;
    }
    if (reject_nul && memchr(s_io_buffer, 0, (unsigned) n) != 0) {
      result = BODY_HAS_NUL;
      break;
    }
    if (sd_write(file, s_io_buffer, (unsigned) n) != SD_OK) {
      result = BODY_WRITE_FAILED;
      break;
    }
    written += (unsigned) n;
    CScheduler::Get()->Yield();
  }

  if (sd_close(file) != SD_OK && result == BODY_OK) {
    result = BODY_WRITE_FAILED;
  }
  if (result != BODY_OK) {
    sd_unlink(temppath);
  }
  return result;
}

void SendBodyFailure(CSocket *socket, BodyResult result) {
  switch (result) {
  case BODY_CREATE_FAILED:
    webhttp::SendText(socket, 500, "Internal Server Error",
                      "cannot create file\n");
    break;
  case BODY_WRITE_FAILED:
    webhttp::SendText(socket, 507, "Insufficient Storage",
                      "write failed (disk full?)\n");
    break;
  case BODY_HAS_NUL:
    webhttp::SendText(socket, 400, "Bad Request", "not a text file\n");
    break;
  default:
    webhttp::SendText(socket, 400, "Bad Request", "upload truncated\n");
    break;
  }
}

// Lets other tasks run while a big folder is deleted.
void YieldWhileDeleting(void *ctx) {
  (void) ctx;
  CScheduler::Get()->Yield();
}

// ---- listing a folder ----

struct ListState {
  CChunkedResponse *r;
  const char *clean;
  boolean at_root;
  boolean first;
  boolean truncated;
  unsigned count;
};

int ListEntry(void *ctx, const sd_info *info) {
  ListState *state = (ListState *) ctx;
  CChunkedResponse *r = state->r;
  if (state->count >= WEBUI_FS_MAX_ENTRIES) {
    state->truncated = TRUE;
    return 1;
  }
  if (!state->first) {
    r->Write(",");
  }
  state->first = FALSE;

  r->Write("{\"name\":");
  JsonString(r, info->name);
  r->Printf(",\"size\":%lu,\"dir\":%s,\"mtime\":\"",
            (unsigned long) info->size, info->is_dir ? "true" : "false");
  WriteDateTime(r, &info->mtime);
  r->Write("\"");
  char child[560];
  boolean child_ok = (unsigned) snprintf(child, sizeof(child), "%s%s%s",
                                         state->clean,
                                         state->at_root ? "" : "/",
                                         info->name) < sizeof(child);
  if (child_ok && !info->is_dir && info->size <= WEBUI_FS_EDIT_MAX &&
      IsEditablePath(child)) {
    r->Write(",\"edit\":true");
  }
  // Entries the web UI won't rename or delete, so the page can leave
  // those actions out of the row's menu.
  if (child_ok && IsProtectedPath(child)) {
    r->Write(",\"protected\":true");
  }
  r->Write("}");

  state->count++;
  if ((state->count & 63) == 0) {
    CScheduler::Get()->Yield();
  }
  return 0;
}

}  // namespace

void WebUiFsVolumes(CSocket *socket) {
  const char *vol = circle_get_disk_volume();
  uint32_t total_kb = 0;
  uint32_t free_kb = 0;
  int rc = sd_space_kb(&total_kb, &free_kb);

  CChunkedResponse r(socket, 200, "OK", "application/json");
  r.Write("{\"volumes\":[");
  if (rc == SD_OK) {
    r.Write("{\"id\":");
    JsonString(&r, vol);
    r.Printf(",\"kind\":\"sdcard\",\"total_kb\":%lu,\"free_kb\":%lu}",
             (unsigned long) total_kb, (unsigned long) free_kb);
  }
  r.Write("]}");
  r.Finish();
}

void WebUiFsList(CSocket *socket, const char *query) {
  char clean[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), FALSE) != 0) {
    return;
  }

  // The root always exists; anything else must be a folder.
  if (strcmp(clean, "/") != 0) {
    sd_info info;
    int rc = sd_stat_info(clean, &info);
    if (rc == SD_NOT_FOUND || rc == SD_INVALID || (rc == SD_OK && !info.is_dir)) {
      webhttp::SendText(socket, 404, "Not Found", "no such directory\n");
      return;
    }
    if (rc != SD_OK) {
      webhttp::SendText(socket, 500, "Internal Server Error",
                        "cannot open directory\n");
      return;
    }
  }

  CChunkedResponse r(socket, 200, "OK", "application/json");
  r.Write("{\"vol\":");
  JsonString(&r, circle_get_disk_volume());
  r.Write(",\"path\":");
  JsonString(&r, clean);
  r.Write(",\"entries\":[");

  ListState state;
  state.r = &r;
  state.clean = clean;
  state.at_root = strcmp(clean, "/") == 0;
  state.first = TRUE;
  state.truncated = FALSE;
  state.count = 0;
  sd_list_info(clean, ListEntry, &state);

  r.Printf("],\"truncated\":%s}", state.truncated ? "true" : "false");
  r.Finish();
}

void WebUiFsDownload(CSocket *socket, const char *query) {
  char clean[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), TRUE) != 0) {
    return;
  }

  sd_file *file = sd_open(clean, SD_READ);
  if (file == 0) {
    webhttp::SendText(socket, 404, "Not Found", "cannot open file\n");
    return;
  }
  uint32_t size = sd_size(file);

  const char *base = clean;
  for (const char *p = clean; *p != '\0'; p++) {
    if (*p == '/') {
      base = p + 1;
    }
  }
  char safe_name[128];
  unsigned si = 0;
  for (const char *p = base; *p != '\0' && si + 1 < sizeof(safe_name); p++) {
    unsigned char c = (unsigned char) *p;
    if (c == '"' || c == '\\' || c < 0x20) {
      continue;
    }
    safe_name[si++] = (char) c;
  }
  safe_name[si] = '\0';
  if (si == 0) {
    strcpy(safe_name, "download");
  }

  char header[400];
  int hn = snprintf(header, sizeof(header),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/octet-stream\r\n"
                    "Content-Length: %lu\r\n"
                    "Content-Disposition: attachment; filename=\"%s\"\r\n"
                    "Connection: close\r\n"
                    "Cache-Control: no-store\r\n"
                    "\r\n",
                    (unsigned long) size, safe_name);
  if (hn <= 0 || (unsigned) hn >= sizeof(header) ||
      !webhttp::SendAll(socket, header, (unsigned) hn)) {
    sd_close(file);
    return;
  }

  for (;;) {
    unsigned read_bytes = 0;
    if (sd_read(file, s_io_buffer, sizeof(s_io_buffer), &read_bytes) != SD_OK) {
      break;
    }
    if (read_bytes == 0) {
      break;
    }
    if (!webhttp::SendAll(socket, s_io_buffer, read_bytes)) {
      break;
    }
    CScheduler::Get()->Yield();
  }
  sd_close(file);
}

void WebUiFsUpload(CSocket *socket, const char *query,
                   const unsigned char *prefetched, unsigned prefetched_len,
                   long content_length) {
  char clean[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), TRUE) != 0) {
    return;
  }
  if (content_length < 0) {
    webhttp::SendText(socket, 411, "Length Required",
                      "Content-Length header required\n");
    return;
  }
  if (IsProtectedPath(clean)) {
    webhttp::SendText(socket, 403, "Forbidden", "that path is protected\n");
    return;
  }

  char overwrite[4];
  boolean allow_overwrite =
      webhttp::QueryParam(query, "overwrite", overwrite, sizeof(overwrite)) &&
      overwrite[0] == '1';

  sd_info existing;
  if (sd_stat_info(clean, &existing) == SD_OK) {
    if (existing.is_dir) {
      webhttp::SendText(socket, 409, "Conflict", "target is a directory\n");
      return;
    }
    if (!allow_overwrite) {
      webhttp::SendText(socket, 409, "Conflict", "file exists\n");
      return;
    }
  }

  // Write to "<path>.part" and swap it into place on success, so an
  // aborted upload never leaves a truncated file.
  char temppath[576];
  if ((unsigned) snprintf(temppath, sizeof(temppath), "%s.part", clean) >=
      sizeof(temppath)) {
    webhttp::SendText(socket, 400, "Bad Request", "path too long\n");
    return;
  }

  unsigned long total = (unsigned long) content_length;
  BodyResult body_result = ReceiveBodyToFile(
      socket, temppath, prefetched, prefetched_len, total, FALSE);
  if (body_result != BODY_OK) {
    SendBodyFailure(socket, body_result);
    return;
  }

  sd_unlink(clean);  // ignore result: file may not exist
  if (sd_rename(temppath, clean) != SD_OK) {
    sd_unlink(temppath);
    webhttp::SendText(socket, 500, "Internal Server Error",
                      "cannot finalise upload\n");
    return;
  }

  // Keep the file's original modified time when the client supplies it;
  // otherwise (or if it can't be applied) the file keeps the upload time.
  char mtime[24];
  sd_time stamp;
  if (webhttp::QueryParam(query, "mtime", mtime, sizeof(mtime)) &&
      ParseDateTime(mtime, &stamp)) {
    sd_set_mtime(clean, &stamp);
  }

  char body[64];
  int bn = snprintf(body, sizeof(body), "{\"ok\":true,\"size\":%lu}", total);
  webhttp::SendResponse(socket, 200, "OK", "application/json", body,
                        bn > 0 ? (unsigned) bn : 0);
}

void WebUiFsSave(CSocket *socket, const char *query,
                 const unsigned char *prefetched, unsigned prefetched_len,
                 long content_length) {
  char clean[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), TRUE) != 0) {
    return;
  }
  if (!IsEditablePath(clean)) {
    webhttp::SendText(socket, 403, "Forbidden", "that file cannot be edited\n");
    return;
  }
  if (content_length < 0) {
    webhttp::SendText(socket, 411, "Length Required",
                      "Content-Length header required\n");
    return;
  }
  if ((unsigned long) content_length > WEBUI_FS_EDIT_MAX) {
    webhttp::SendText(socket, 413, "Payload Too Large",
                      "file too large to edit\n");
    return;
  }

  sd_info existing;
  boolean exists = sd_stat_info(clean, &existing) == SD_OK;
  if (exists && existing.is_dir) {
    webhttp::SendText(socket, 409, "Conflict", "target is a directory\n");
    return;
  }

  // A keymap can be in a nested folder, so size these for the longest
  // resolved path plus the ".part" / ".bak" suffix.
  char temppath[sizeof(clean) + 8];
  char bakpath[sizeof(clean) + 8];
  if ((unsigned) snprintf(temppath, sizeof(temppath), "%s.part", clean) >=
          sizeof(temppath) ||
      (unsigned) snprintf(bakpath, sizeof(bakpath), "%s.bak", clean) >=
          sizeof(bakpath)) {
    webhttp::SendText(socket, 400, "Bad Request", "path too long\n");
    return;
  }

  unsigned long total = (unsigned long) content_length;
  BodyResult body_result = ReceiveBodyToFile(
      socket, temppath, prefetched, prefetched_len, total, TRUE);
  if (body_result != BODY_OK) {
    SendBodyFailure(socket, body_result);
    return;
  }

  // Keep the previous version as "<name>.bak". The original is only moved
  // aside once the new one is fully written, and is put back if the final
  // rename fails, so a failed save never leaves a config file missing.
  if (exists) {
    sd_unlink(bakpath);  // ignore result: an older backup may not exist
    if (sd_rename(clean, bakpath) != SD_OK) {
      sd_unlink(temppath);
      webhttp::SendText(socket, 500, "Internal Server Error",
                        "cannot back up the existing file\n");
      return;
    }
  }
  if (sd_rename(temppath, clean) != SD_OK) {
    if (exists) {
      sd_rename(bakpath, clean);
    }
    sd_unlink(temppath);
    webhttp::SendText(socket, 500, "Internal Server Error",
                      "cannot finalise save\n");
    return;
  }

  char body[64];
  int bn = snprintf(body, sizeof(body), "{\"ok\":true,\"size\":%lu}", total);
  webhttp::SendResponse(socket, 200, "OK", "application/json", body,
                        bn > 0 ? (unsigned) bn : 0);
}

void WebUiFsDelete(CSocket *socket, const char *query) {
  char clean[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), TRUE) != 0) {
    return;
  }
  if (IsProtectedPath(clean)) {
    webhttp::SendText(socket, 403, "Forbidden", "that path is protected\n");
    return;
  }

  // &recursive=1 deletes a folder together with its contents; without it
  // only an empty folder can be removed.
  char recursive[4];
  sd_info stat;
  if (webhttp::QueryParam(query, "recursive", recursive, sizeof(recursive)) &&
      recursive[0] == '1' && sd_stat_info(clean, &stat) == SD_OK &&
      stat.is_dir) {
    unsigned removed = 0;
    if (sd_remove_tree(clean, &removed, YieldWhileDeleting, 0) == SD_OK) {
      const char *ok = "{\"ok\":true}";
      webhttp::SendResponse(socket, 200, "OK", "application/json", ok,
                            (unsigned) strlen(ok));
      return;
    }
    char text[128];
    snprintf(text, sizeof(text),
             "could not delete everything (a file may be read-only): %u "
             "item%s removed, the rest of the folder is still there\n",
             removed, removed == 1 ? "" : "s");
    webhttp::SendText(socket, 409, "Conflict", text);
    return;
  }

  int rc = sd_unlink(clean);
  if (rc == SD_NOT_FOUND || rc == SD_INVALID) {
    webhttp::SendText(socket, 404, "Not Found", "no such file\n");
    return;
  }
  if (rc == SD_DENIED) {
    webhttp::SendText(socket, 409, "Conflict",
                      "cannot delete (directory not empty or read-only)\n");
    return;
  }
  if (rc != SD_OK) {
    webhttp::SendText(socket, 500, "Internal Server Error", "delete failed\n");
    return;
  }

  const char *ok = "{\"ok\":true}";
  webhttp::SendResponse(socket, 200, "OK", "application/json", ok,
                        (unsigned) strlen(ok));
}

// ---- new folder / rename ----

void WebUiFsMkdir(CSocket *socket, const char *query) {
  char clean[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), TRUE) != 0) {
    return;
  }
  if (!IsValidEntryName(strrchr(clean, '/') + 1)) {
    webhttp::SendText(socket, 400, "Bad Request", kBadNameText);
    return;
  }
  if (IsProtectedPath(clean)) {
    webhttp::SendText(socket, 403, "Forbidden", "that path is protected\n");
    return;
  }

  if (sd_stat_info(clean, 0) == SD_OK) {
    webhttp::SendText(socket, 409, "Conflict", "already exists\n");
    return;
  }

  int rc = sd_mkdir(clean);
  if (rc == SD_EXISTS) {
    webhttp::SendText(socket, 409, "Conflict", "already exists\n");
    return;
  }
  if (rc == SD_NOT_FOUND) {
    webhttp::SendText(socket, 404, "Not Found", "parent folder not found\n");
    return;
  }
  if (rc != SD_OK) {
    webhttp::SendText(socket, 500, "Internal Server Error",
                      "cannot create folder\n");
    return;
  }

  const char *ok = "{\"ok\":true}";
  webhttp::SendResponse(socket, 200, "OK", "application/json", ok,
                        (unsigned) strlen(ok));
}

// POST /api/fs/rename?path=/dir/old&to=new: renames within the same folder,
// so `to` is a bare name, never a path.
void WebUiFsRename(CSocket *socket, const char *query) {
  char clean[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), TRUE) != 0) {
    return;
  }

  char new_name[256];
  if (!webhttp::QueryParam(query, "to", new_name, sizeof(new_name)) ||
      !IsValidEntryName(new_name)) {
    webhttp::SendText(socket, 400, "Bad Request", kBadNameText);
    return;
  }

  const char *old_name = strrchr(clean, '/') + 1;
  char new_clean[560];
  if ((unsigned) snprintf(new_clean, sizeof(new_clean), "%.*s/%s",
                          (int) (old_name - 1 - clean), clean,
                          new_name) >= sizeof(new_clean)) {
    webhttp::SendText(socket, 400, "Bad Request", "path too long\n");
    return;
  }

  // Neither the source nor the new name may be a protected one (that would
  // let a rename create or replace settings.txt, or touch /firmware).
  if (IsProtectedPath(clean) || IsProtectedPath(new_clean)) {
    webhttp::SendText(socket, 403, "Forbidden", "that path is protected\n");
    return;
  }

  if (sd_stat_info(clean, 0) != SD_OK) {
    webhttp::SendText(socket, 404, "Not Found", "no such file\n");
    return;
  }

  boolean unchanged = strcmp(old_name, new_name) == 0;
  boolean case_only = !unchanged && CiEqual(old_name, new_name);
  if (!unchanged && !case_only && sd_stat_info(new_clean, 0) == SD_OK) {
    webhttp::SendText(socket, 409, "Conflict", "already exists\n");
    return;
  }

  int rc = unchanged ? SD_OK : sd_rename(clean, new_clean);
  if (rc == SD_EXISTS) {
    webhttp::SendText(socket, 409, "Conflict", "already exists\n");
    return;
  }
  if (rc == SD_NOT_FOUND) {
    webhttp::SendText(socket, 404, "Not Found", "no such file\n");
    return;
  }
  if (rc != SD_OK) {
    webhttp::SendText(socket, 500, "Internal Server Error",
                      "rename failed\n");
    return;
  }

  const char *ok = "{\"ok\":true}";
  webhttp::SendResponse(socket, 200, "OK", "application/json", ok,
                        (unsigned) strlen(ok));
}

// ---- autostart ----

void WebUiFsAutostart(CSocket *socket, const char *query) {
  char clean[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), TRUE) != 0) {
    return;
  }
  if (!IsAutostartable(clean)) {
    webhttp::SendText(socket, 400, "Bad Request",
                      "not an autostartable file type\n");
    return;
  }

  sd_info info;
  if (sd_stat_info(clean, &info) != SD_OK || info.is_dir) {
    webhttp::SendText(socket, 404, "Not Found", "no such file\n");
    return;
  }

  // Runs later on the emulator core, through stdio, so it gets the path
  // with the card's volume ("SD:/games/x.d64").
  char volume_path[600];
  if ((unsigned) snprintf(volume_path, sizeof(volume_path), "%s:%s",
                          circle_get_disk_volume(), clean) >=
      sizeof(volume_path)) {
    webhttp::SendText(socket, 400, "Bad Request", "path too long\n");
    return;
  }
  // Failures are only logged there.
  emu_autostart_interrupt(volume_path);

  const char *ok = "{\"ok\":true}";
  webhttp::SendResponse(socket, 202, "Accepted", "application/json", ok,
                        (unsigned) strlen(ok));
}

#endif  // RASPI_C64 || RASPI_C128
