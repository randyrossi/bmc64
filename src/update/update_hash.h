// SHA-256 (FIPS 180-4) for the updater. (CRC-32 comes from zlib.)

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
