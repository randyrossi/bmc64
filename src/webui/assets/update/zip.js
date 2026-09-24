// Reads a zip that is already in memory: its directory, and single entries
// (stored or deflate, unpacked with the browser's DecompressionStream).

export function readZipDirectory(bytes) {
  const dv = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const stop = Math.max(0, bytes.length - 22 - 65535);
  let eocd = -1;
  for (let i = bytes.length - 22; i >= stop; i--) {
    if (dv.getUint32(i, true) === 0x06054b50) { eocd = i; break; }
  }
  if (eocd < 0) throw new Error("This is not a zip file.");
  const count = dv.getUint16(eocd + 10, true);
  const cdOffset = dv.getUint32(eocd + 16, true);
  if (count === 0xffff || cdOffset === 0xffffffff) throw new Error("ZIP64 zips are not supported.");

  const latin1 = new TextDecoder("latin1");
  const entries = [];
  let pos = cdOffset;
  for (let i = 0; i < count; i++) {
    if (pos + 46 > bytes.length || dv.getUint32(pos, true) !== 0x02014b50) {
      throw new Error("The zip file is damaged.");
    }
    const nameLen = dv.getUint16(pos + 28, true);
    const name = latin1.decode(bytes.subarray(pos + 46, pos + 46 + nameLen));
    entries.push({
      name,
      isDir: name.endsWith("/"),
      flags: dv.getUint16(pos + 8, true),
      method: dv.getUint16(pos + 10, true),
      crc: dv.getUint32(pos + 16, true),
      compressedSize: dv.getUint32(pos + 20, true),
      size: dv.getUint32(pos + 24, true),
      localOffset: dv.getUint32(pos + 42, true),
    });
    pos += 46 + nameLen + dv.getUint16(pos + 30, true) + dv.getUint16(pos + 32, true);
  }
  return entries;
}

// The entry's uncompressed bytes, as a Uint8Array.
export async function readEntry(bytes, entry) {
  const dv = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const at = entry.localOffset;
  if (dv.getUint32(at, true) !== 0x04034b50) throw new Error("The zip file is damaged.");
  const start = at + 30 + dv.getUint16(at + 26, true) + dv.getUint16(at + 28, true);
  const data = bytes.subarray(start, start + entry.compressedSize);
  if (entry.method === 0) return data.slice();
  if (entry.method !== 8) throw new Error("Unsupported compression in the zip.");
  if (typeof DecompressionStream === "undefined") {
    throw new Error("This browser is too old to read zip files. Try a current browser.");
  }
  const stream = new Blob([data]).stream().pipeThrough(new DecompressionStream("deflate-raw"));
  return new Uint8Array(await new Response(stream).arrayBuffer());
}
