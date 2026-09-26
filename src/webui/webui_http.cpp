//
// webui_http.cpp
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

#include "webui_http.h"

#if defined(RASPI_C64) || defined(RASPI_C128)

#include <circle/net/socket.h>
#include <circle/sched/scheduler.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace webhttp {

boolean SendAll(CSocket *socket, const void *data, unsigned length) {
  const u8 *bytes = (const u8 *) data;
  unsigned sent = 0;
  while (sent < length) {
    int n = socket->Send(bytes + sent, length - sent, 0);
    if (n <= 0) {
      return FALSE;
    }
    sent += (unsigned) n;
    CScheduler::Get()->Yield();
  }
  return TRUE;
}

void SendResponse(CSocket *socket, int status, const char *reason,
                  const char *content_type, const void *body,
                  unsigned body_length) {
  char header[256];
  int header_length = snprintf(
      header, sizeof(header),
      "HTTP/1.1 %d %s\r\n"
      "Content-Type: %s\r\n"
      "Content-Length: %u\r\n"
      "Connection: close\r\n"
      "Cache-Control: no-store\r\n"
      "\r\n",
      status, reason, content_type, body_length);
  if (header_length <= 0 || (unsigned) header_length >= sizeof(header)) {
    return;
  }
  if (!SendAll(socket, header, (unsigned) header_length)) {
    return;
  }
  if (body_length != 0) {
    SendAll(socket, body, body_length);
  }
}

void SendText(CSocket *socket, int status, const char *reason,
              const char *text) {
  SendResponse(socket, status, reason, "text/plain; charset=utf-8", text,
               (unsigned) strlen(text));
}

static int HexValue(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

unsigned UrlDecode(char *dst, unsigned dst_size, const char *src) {
  unsigned out = 0;
  if (dst_size == 0) {
    return 0;
  }
  for (const char *p = src; *p != '\0'; p++) {
    char decoded;
    if (*p == '+') {
      decoded = ' ';
    } else if (*p == '%' && p[1] != '\0' && p[2] != '\0' &&
               HexValue(p[1]) >= 0 && HexValue(p[2]) >= 0) {
      decoded = (char) ((HexValue(p[1]) << 4) | HexValue(p[2]));
      p += 2;
    } else {
      decoded = *p;
    }
    if (out + 1 < dst_size) dst[out] = decoded;
    out++;
  }
  dst[out < dst_size ? out : dst_size - 1] = '\0';
  return out;
}

boolean QueryParam(const char *query, const char *key,
                   char *out, unsigned out_size) {
  if (query == 0 || out_size == 0) {
    return FALSE;
  }
  unsigned key_len = (unsigned) strlen(key);
  const char *p = query;
  while (*p != '\0') {
    const char *amp = strchr(p, '&');
    unsigned pair_len = amp != 0 ? (unsigned) (amp - p) : (unsigned) strlen(p);
    if (pair_len > key_len && p[key_len] == '=' &&
        strncmp(p, key, key_len) == 0) {
      char raw[2048];
      unsigned n = pair_len - key_len - 1;
      if (n >= sizeof(raw)) {
        return FALSE;
      }
      memcpy(raw, p + key_len + 1, n);
      raw[n] = '\0';
      unsigned decoded_len = UrlDecode(out, out_size, raw);
      return decoded_len < out_size && strlen(out) == decoded_len;
    }
    if (amp == 0) {
      break;
    }
    p = amp + 1;
  }
  return FALSE;
}

//
// CChunkedResponse
//

CChunkedResponse::CChunkedResponse(CSocket *socket, int status,
                                   const char *reason, const char *content_type)
    : m_socket(socket), m_status(status), m_reason(reason),
      m_content_type(content_type), m_began(FALSE), m_failed(FALSE), m_len(0) {}

boolean CChunkedResponse::Begin(void) {
  if (m_began) {
    return !m_failed;
  }
  m_began = TRUE;
  char header[256];
  int n = snprintf(header, sizeof(header),
                   "HTTP/1.1 %d %s\r\n"
                   "Content-Type: %s\r\n"
                   "Transfer-Encoding: chunked\r\n"
                   "Connection: close\r\n"
                   "Cache-Control: no-store\r\n"
                   "\r\n",
                   m_status, m_reason, m_content_type);
  if (n <= 0 || (unsigned) n >= sizeof(header) ||
      !SendAll(m_socket, header, (unsigned) n)) {
    m_failed = TRUE;
  }
  return !m_failed;
}

boolean CChunkedResponse::Flush(void) {
  if (m_failed) {
    return FALSE;
  }
  if (m_len == 0) {
    return TRUE;
  }
  char size_line[16];
  int n = snprintf(size_line, sizeof(size_line), "%x\r\n", m_len);
  if (n <= 0 || !SendAll(m_socket, size_line, (unsigned) n) ||
      !SendAll(m_socket, m_buf, m_len) || !SendAll(m_socket, "\r\n", 2)) {
    m_failed = TRUE;
    return FALSE;
  }
  m_len = 0;
  return TRUE;
}

boolean CChunkedResponse::WriteN(const void *data, unsigned length) {
  if (!m_began && !Begin()) {
    return FALSE;
  }
  const u8 *bytes = (const u8 *) data;
  while (length > 0 && !m_failed) {
    unsigned space = sizeof(m_buf) - m_len;
    unsigned take = length < space ? length : space;
    memcpy(m_buf + m_len, bytes, take);
    m_len += take;
    bytes += take;
    length -= take;
    if (m_len == sizeof(m_buf)) {
      Flush();
    }
  }
  return !m_failed;
}

boolean CChunkedResponse::Write(const char *text) {
  return WriteN(text, (unsigned) strlen(text));
}

boolean CChunkedResponse::Printf(const char *format, ...) {
  char line[1024];
  va_list args;
  va_start(args, format);
  int n = vsnprintf(line, sizeof(line), format, args);
  va_end(args);
  if (n < 0) {
    return !m_failed;
  }
  if ((unsigned) n >= sizeof(line)) {
    n = sizeof(line) - 1;
  }
  return WriteN(line, (unsigned) n);
}

boolean CChunkedResponse::Finish(void) {
  if (!m_began && !Begin()) {
    return FALSE;
  }
  if (!Flush()) {
    return FALSE;
  }
  if (!SendAll(m_socket, "0\r\n\r\n", 5)) {
    m_failed = TRUE;
  }
  return !m_failed;
}

}  // namespace webhttp

#endif  // RASPI_C64 || RASPI_C128
