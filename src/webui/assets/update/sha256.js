// SHA-256 in plain JavaScript. The browser's crypto.subtle only works on
// https:// pages, and the web UI is served over plain http:// on the LAN.

const K = new Uint32Array([
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
]);

export class Sha256 {
  constructor() {
    this.h = new Uint32Array([
      0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
      0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
    ]);
    this.w = new Uint32Array(64);
    this.block = new Uint8Array(64);
    this.used = 0;
    this.length = 0;
  }

  // bytes: Uint8Array
  update(bytes) {
    let pos = 0;
    this.length += bytes.length;
    if (this.used) {
      const take = Math.min(64 - this.used, bytes.length);
      this.block.set(bytes.subarray(0, take), this.used);
      this.used += take;
      pos = take;
      if (this.used < 64) return this;
      this.compress(this.block, 0);
      this.used = 0;
    }
    for (; pos + 64 <= bytes.length; pos += 64) this.compress(bytes, pos);
    this.block.set(bytes.subarray(pos), 0);
    this.used = bytes.length - pos;
    return this;
  }

  compress(p, o) {
    const w = this.w;
    for (let i = 0; i < 16; i++, o += 4) {
      w[i] = (p[o] << 24) | (p[o + 1] << 16) | (p[o + 2] << 8) | p[o + 3];
    }
    for (let i = 16; i < 64; i++) {
      const a = w[i - 15], b = w[i - 2];
      const s0 = ((a >>> 7) | (a << 25)) ^ ((a >>> 18) | (a << 14)) ^ (a >>> 3);
      const s1 = ((b >>> 17) | (b << 15)) ^ ((b >>> 19) | (b << 13)) ^ (b >>> 10);
      w[i] = (w[i - 16] + s0 + w[i - 7] + s1) | 0;
    }
    const h = this.h;
    let a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], k = h[7];
    for (let i = 0; i < 64; i++) {
      const t1 = (k + (((e >>> 6) | (e << 26)) ^ ((e >>> 11) | (e << 21)) ^ ((e >>> 25) | (e << 7))) +
                  ((e & f) ^ (~e & g)) + K[i] + w[i]) | 0;
      const t2 = ((((a >>> 2) | (a << 30)) ^ ((a >>> 13) | (a << 19)) ^ ((a >>> 22) | (a << 10))) +
                  ((a & b) ^ (a & c) ^ (b & c))) | 0;
      k = g; g = f; f = e; e = (d + t1) | 0; d = c; c = b; b = a; a = (t1 + t2) | 0;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += k;
  }

  // Returns the digest as 64 lower-case hex digits.
  hex() {
    const bits = this.length * 8;
    const pad = new Uint8Array(((this.used < 56 ? 56 : 120) - this.used) + 8);
    pad[0] = 0x80;
    const hi = Math.floor(bits / 0x100000000), lo = bits >>> 0;
    const n = pad.length;
    pad[n - 8] = hi >>> 24; pad[n - 7] = hi >>> 16; pad[n - 6] = hi >>> 8; pad[n - 5] = hi;
    pad[n - 4] = lo >>> 24; pad[n - 3] = lo >>> 16; pad[n - 2] = lo >>> 8; pad[n - 1] = lo;
    this.update(pad);
    return Array.from(this.h, (x) => x.toString(16).padStart(8, "0")).join("");
  }
}

// Hashes bytes (Uint8Array) in slices, yielding to the page between them so
// it stays responsive; onProgress(fraction) is called as it goes.
export async function sha256Hex(bytes, onProgress) {
  const s = new Sha256();
  const step = 1 << 20;
  for (let pos = 0; pos < bytes.length; pos += step) {
    s.update(bytes.subarray(pos, pos + step));
    if (onProgress) onProgress(Math.min(1, (pos + step) / bytes.length));
    await new Promise((r) => setTimeout(r, 0));
  }
  return s.hex();
}
