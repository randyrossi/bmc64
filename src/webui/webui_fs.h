//
// webui_fs.h
//
// Filesystem endpoints for the web UI: directory listing and download
// (read-only) plus a single streamed upload path. All access is confined
// to the configured SD card volume and sandboxed against path traversal;
// upload additionally refuses a small denylist of config/firmware files.
// FatFs calls are serialised with the emulator by FF_FS_REENTRANT and
// chunked so a transfer never holds the volume lock for long.
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

#ifndef _webui_fs_h
#define _webui_fs_h

class CSocket;

// GET /api/volumes  -> JSON list of browsable volumes with size/free.
void WebUiFsVolumes(CSocket *socket);

// GET /api/fs/list?vol=SD&path=/dir  -> JSON directory listing.
void WebUiFsList(CSocket *socket, const char *query);

// GET /api/fs/download?vol=SD&path=/dir/file  -> raw file bytes.
void WebUiFsDownload(CSocket *socket, const char *query);

// POST /api/fs/upload?vol=SD&path=/dir/file  <- the raw request body is
// the file. prefetched[0..prefetched_len) are body bytes already read
// with the request headers; the remainder (up to content_length total)
// is streamed from the socket. content_length < 0 means no
// Content-Length header was sent (rejected with 411). An optional
// &mtime=YYYY-MM-DDTHH:MM:SS (local time) query parameter sets the
// file's modified time; without it the file gets the current FAT time.
void WebUiFsUpload(CSocket *socket, const char *query,
                   const unsigned char *prefetched, unsigned prefetched_len,
                   long content_length);

// POST /api/fs/delete?vol=SD&path=/dir/file  -> remove a file (or an
// empty directory). The same protected paths as upload are refused.
void WebUiFsDelete(CSocket *socket, const char *query);

// POST /api/fs/autostart?vol=SD&path=/dir/file.d64  -> queue the file to
// be autostarted on the emulator (disk/tape image or PRG), like the
// menu's Autostart. Answers 202 once queued; the outcome isn't reported.
void WebUiFsAutostart(CSocket *socket, const char *query);

#endif
