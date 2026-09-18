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

#include <ff.h>
#include <stdio.h>
#include <string.h>

// Implemented in src/vice_network.cpp.
extern "C" const char *circle_get_disk_volume(void);
// Queues a file for the emulator main loop to autostart (interrupt safe;
// see third_party/common/ui.c).
extern "C" void emu_autostart_interrupt(const char *path);

using webhttp::CChunkedResponse;

namespace {

const unsigned WEBUI_FS_MAX_ENTRIES = 6000;
const unsigned WEBUI_FS_IO_CHUNK = 32 * 1024;

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

void WriteFatDateTime(CChunkedResponse *r, unsigned fdate, unsigned ftime) {
  if (fdate == 0) {
    return;  // unknown timestamp -> empty string
  }
  unsigned year = ((fdate >> 9) & 0x7F) + 1980;
  unsigned month = (fdate >> 5) & 0x0F;
  unsigned day = fdate & 0x1F;
  unsigned hour = (ftime >> 11) & 0x1F;
  unsigned minute = (ftime >> 5) & 0x3F;
  unsigned second = (ftime & 0x1F) * 2;
  r->Printf("%04u-%02u-%02uT%02u:%02u:%02u", year, month, day, hour, minute,
            second);
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
    if (seg_len > 200) {
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

int BuildFatPath(const char *vol, const char *clean, char *out,
                 unsigned out_size) {
  int n = snprintf(out, out_size, "%s:%s", vol, clean);
  return (n > 0 && (unsigned) n < out_size) ? 0 : -1;
}

// Resolve and validate the vol/path query parameters. Returns 0 on
// success and fills fatpath / clean; on failure it has already sent an
// error response and returns -1.
int ResolveTarget(CSocket *socket, const char *query, char *clean,
                  unsigned clean_size, char *fatpath, unsigned fatpath_size,
                  boolean require_file) {
  char vol[24];
  char raw[512];
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
  if (BuildFatPath(vol, clean, fatpath, fatpath_size) != 0) {
    webhttp::SendText(socket, 400, "Bad Request", "path too long\n");
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

// Only image/program types that the menu's Autostart accepts as-is.
boolean IsAutostartable(const char *clean) {
  const char *dot = 0;
  for (const char *p = clean; *p != '\0'; p++) {
    if (*p == '/') dot = 0;
    else if (*p == '.') dot = p + 1;
  }
  if (dot == 0) return FALSE;
  static const char *const kExt[] = {
      "d64", "d71", "d81", "d82", "g64", "x64", "t64", "tap", "prg", "p00",
  };
  for (unsigned i = 0; i < sizeof(kExt) / sizeof(kExt[0]); i++) {
    if (CiEqual(dot, kExt[i])) return TRUE;
  }
  return FALSE;
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
  static const char *const kProtected[] = {
      "settings.txt",     "settings-c128.txt",     "settings-vic20.txt",
      "settings-plus4.txt", "settings-plus4emu.txt", "settings-pet.txt",
      "wpa_supplicant.conf", "cmdline.txt",         "config.txt",
      "machines.txt",     "bmc64.log",
  };
  for (unsigned i = 0; i < sizeof(kProtected) / sizeof(kProtected[0]); i++) {
    if (CiEqual(base, kProtected[i])) return TRUE;
  }
  return FALSE;
}

}  // namespace

void WebUiFsVolumes(CSocket *socket) {
  const char *vol = circle_get_disk_volume();
  char prefix[24];
  snprintf(prefix, sizeof(prefix), "%s:", vol);

  DWORD free_clusters = 0;
  FATFS *fs = 0;
  FRESULT fr = f_getfree(prefix, &free_clusters, &fs);

  CChunkedResponse r(socket, 200, "OK", "application/json");
  r.Write("{\"volumes\":[");
  if (fr == FR_OK && fs != 0) {
    unsigned long total_clusters = fs->n_fatent > 2 ? fs->n_fatent - 2 : 0;
    unsigned long total_kb =
        (unsigned long) (((u64) total_clusters * fs->csize) / 2);
    unsigned long free_kb =
        (unsigned long) (((u64) free_clusters * fs->csize) / 2);
    r.Write("{\"id\":");
    JsonString(&r, vol);
    r.Printf(",\"kind\":\"sdcard\",\"total_kb\":%lu,\"free_kb\":%lu}", total_kb,
             free_kb);
  }
  r.Write("]}");
  r.Finish();
}

void WebUiFsList(CSocket *socket, const char *query) {
  char clean[512];
  char fatpath[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), fatpath,
                    sizeof(fatpath), FALSE) != 0) {
    return;
  }

  DIR dir;
  FRESULT fr = f_opendir(&dir, fatpath);
  if (fr == FR_NO_PATH || fr == FR_NO_FILE || fr == FR_INVALID_NAME) {
    webhttp::SendText(socket, 404, "Not Found", "no such directory\n");
    return;
  }
  if (fr != FR_OK) {
    webhttp::SendText(socket, 500, "Internal Server Error",
                      "cannot open directory\n");
    return;
  }

  CChunkedResponse r(socket, 200, "OK", "application/json");
  r.Write("{\"vol\":");
  JsonString(&r, circle_get_disk_volume());
  r.Write(",\"path\":");
  JsonString(&r, clean);
  r.Write(",\"entries\":[");

  unsigned count = 0;
  boolean truncated = FALSE;
  boolean first = TRUE;
  FILINFO info;
  while (f_readdir(&dir, &info) == FR_OK && info.fname[0] != '\0') {
    if (info.fname[0] == '.' &&
        (info.fname[1] == '\0' ||
         (info.fname[1] == '.' && info.fname[2] == '\0'))) {
      continue;
    }
    if (count >= WEBUI_FS_MAX_ENTRIES) {
      truncated = TRUE;
      break;
    }
    if (!first) {
      r.Write(",");
    }
    first = FALSE;

    boolean is_dir = (info.fattrib & AM_DIR) != 0;
    r.Write("{\"name\":");
    JsonString(&r, info.fname);
    r.Printf(",\"size\":%lu,\"dir\":%s,\"mtime\":\"",
             (unsigned long) info.fsize, is_dir ? "true" : "false");
    WriteFatDateTime(&r, info.fdate, info.ftime);
    r.Write("\"}");

    count++;
    if ((count & 63) == 0) {
      CScheduler::Get()->Yield();
    }
  }
  f_closedir(&dir);

  r.Printf("],\"truncated\":%s}", truncated ? "true" : "false");
  r.Finish();
}

void WebUiFsDownload(CSocket *socket, const char *query) {
  char clean[512];
  char fatpath[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), fatpath,
                    sizeof(fatpath), TRUE) != 0) {
    return;
  }

  FIL file;
  if (f_open(&file, fatpath, FA_READ) != FR_OK) {
    webhttp::SendText(socket, 404, "Not Found", "cannot open file\n");
    return;
  }
  DWORD size = f_size(&file);

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
    f_close(&file);
    return;
  }

  for (;;) {
    UINT read_bytes = 0;
    if (f_read(&file, s_io_buffer, sizeof(s_io_buffer), &read_bytes) != FR_OK) {
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
  f_close(&file);
}

void WebUiFsUpload(CSocket *socket, const char *query,
                   const unsigned char *prefetched, unsigned prefetched_len,
                   long content_length) {
  char clean[512];
  char fatpath[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), fatpath,
                    sizeof(fatpath), TRUE) != 0) {
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

  FILINFO existing;
  if (f_stat(fatpath, &existing) == FR_OK) {
    if (existing.fattrib & AM_DIR) {
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
  if ((unsigned) snprintf(temppath, sizeof(temppath), "%s.part", fatpath) >=
      sizeof(temppath)) {
    webhttp::SendText(socket, 400, "Bad Request", "path too long\n");
    return;
  }

  FIL file;
  if (f_open(&file, temppath, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
    webhttp::SendText(socket, 500, "Internal Server Error",
                      "cannot create file\n");
    return;
  }

  unsigned long total = (unsigned long) content_length;
  unsigned long written = 0;
  boolean write_failed = FALSE;

  if (prefetched_len > total) {
    prefetched_len = (unsigned) total;
  }
  if (prefetched_len > 0) {
    UINT bw = 0;
    if (f_write(&file, prefetched, prefetched_len, &bw) != FR_OK ||
        bw != prefetched_len) {
      write_failed = TRUE;
    }
    written += prefetched_len;
  }

  while (!write_failed && written < total) {
    unsigned long remain = total - written;
    unsigned want = remain < sizeof(s_io_buffer) ? (unsigned) remain
                                                 : sizeof(s_io_buffer);
    int n = socket->Receive(s_io_buffer, want, 0);
    if (n <= 0) {
      break;  // client aborted or timed out
    }
    UINT bw = 0;
    if (f_write(&file, s_io_buffer, (UINT) n, &bw) != FR_OK ||
        bw != (UINT) n) {
      write_failed = TRUE;
      break;
    }
    written += (unsigned) n;
    CScheduler::Get()->Yield();
  }

  f_close(&file);

  if (write_failed) {
    f_unlink(temppath);
    webhttp::SendText(socket, 507, "Insufficient Storage",
                      "write failed (disk full?)\n");
    return;
  }
  if (written != total) {
    f_unlink(temppath);
    webhttp::SendText(socket, 400, "Bad Request", "upload truncated\n");
    return;
  }

  f_unlink(fatpath);  // ignore result: file may not exist
  if (f_rename(temppath, fatpath) != FR_OK) {
    f_unlink(temppath);
    webhttp::SendText(socket, 500, "Internal Server Error",
                      "cannot finalise upload\n");
    return;
  }

  char body[64];
  int bn = snprintf(body, sizeof(body), "{\"ok\":true,\"size\":%lu}", written);
  webhttp::SendResponse(socket, 200, "OK", "application/json", body,
                        bn > 0 ? (unsigned) bn : 0);
}

void WebUiFsDelete(CSocket *socket, const char *query) {
  char clean[512];
  char fatpath[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), fatpath,
                    sizeof(fatpath), TRUE) != 0) {
    return;
  }
  if (IsProtectedPath(clean)) {
    webhttp::SendText(socket, 403, "Forbidden", "that path is protected\n");
    return;
  }

  FRESULT fr = f_unlink(fatpath);
  if (fr == FR_NO_FILE || fr == FR_NO_PATH || fr == FR_INVALID_NAME) {
    webhttp::SendText(socket, 404, "Not Found", "no such file\n");
    return;
  }
  if (fr == FR_DENIED) {
    webhttp::SendText(socket, 409, "Conflict",
                      "cannot delete (directory not empty or read-only)\n");
    return;
  }
  if (fr != FR_OK) {
    webhttp::SendText(socket, 500, "Internal Server Error", "delete failed\n");
    return;
  }

  const char *ok = "{\"ok\":true}";
  webhttp::SendResponse(socket, 200, "OK", "application/json", ok,
                        (unsigned) strlen(ok));
}

// ---- autostart ----

void WebUiFsAutostart(CSocket *socket, const char *query) {
  char clean[512];
  char fatpath[560];
  if (ResolveTarget(socket, query, clean, sizeof(clean), fatpath,
                    sizeof(fatpath), TRUE) != 0) {
    return;
  }
  if (!IsAutostartable(clean)) {
    webhttp::SendText(socket, 400, "Bad Request",
                      "not an autostartable file type\n");
    return;
  }

  FILINFO info;
  if (f_stat(fatpath, &info) != FR_OK || (info.fattrib & AM_DIR)) {
    webhttp::SendText(socket, 404, "Not Found", "no such file\n");
    return;
  }

  // Runs later on the emulator core; failures are only logged there.
  emu_autostart_interrupt(fatpath);

  const char *ok = "{\"ok\":true}";
  webhttp::SendResponse(socket, 202, "Accepted", "application/json", ok,
                        (unsigned) strlen(ok));
}

#endif  // RASPI_C64 || RASPI_C128
