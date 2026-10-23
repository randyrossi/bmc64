//
// webui_http.h
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
// Small HTTP/1.1 response helpers shared by the web UI request handlers:
// fixed-length responses, a chunked-transfer response builder, and query
// string parsing. Socket sends yield to the Circle scheduler so a large
// body never starves the network stack or the emulator.
//

#ifndef _webui_http_h
#define _webui_http_h

#include <circle/types.h>

class CSocket;

namespace webhttp {

// Send every byte, yielding between sends. FALSE on socket error.
boolean SendAll(CSocket *socket, const void *data, unsigned length);

// One-shot response with an in-memory body and a Content-Length header.
void SendResponse(CSocket *socket, int status, const char *reason,
                  const char *content_type, const void *body,
                  unsigned body_length);

// text/plain helper (body is a NUL-terminated string).
void SendText(CSocket *socket, int status, const char *reason,
              const char *text);

// Percent-decode src into dst (always NUL-terminated). Returns the full
// decoded length, even if dst is too small to hold it.
unsigned UrlDecode(char *dst, unsigned dst_size, const char *src);

// Look up key in an "a=b&c=d" query string and percent-decode its value
// into out. Returns TRUE when the key is present and the value fits.
boolean QueryParam(const char *query, const char *key,
                   char *out, unsigned out_size);

// Transfer-Encoding: chunked response builder. Small writes are buffered
// and flushed as HTTP chunks; Finish() emits the terminating chunk.
class CChunkedResponse {
public:
  CChunkedResponse(CSocket *socket, int status, const char *reason,
                   const char *content_type);

  boolean Begin(void);                              // send response header
  boolean Write(const char *text);                  // append a C string
  boolean WriteN(const void *data, unsigned length);
  boolean Printf(const char *format, ...);
  boolean Finish(void);                             // flush + close stream
  boolean Failed(void) const { return m_failed; }

private:
  boolean Flush(void);

  CSocket *m_socket;
  int m_status;
  const char *m_reason;
  const char *m_content_type;
  boolean m_began;
  boolean m_failed;
  unsigned m_len;
  char m_buf[4096];
};

}  // namespace webhttp

#endif
