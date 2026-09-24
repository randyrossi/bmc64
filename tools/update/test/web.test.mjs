// Tests for the Web UI Update page's modules (src/webui/assets/update/).
// Run by run_tests.py:  node web.test.mjs <package.zip>

import { createHash, randomBytes } from "node:crypto";
import { readFileSync } from "node:fs";

const base = new URL("../../../src/webui/assets/update/", import.meta.url);
const { Sha256, sha256Hex } = await import(new URL("sha256.js", base));
const { readZipDirectory, readEntry } = await import(new URL("zip.js", base));
const logic = await import(new URL("logic.js", base));

let failed = 0;
const check = (cond, what) => { if (!cond) { failed++; console.log("  FAIL: " + what); } };
const nodeSha = (b) => createHash("sha256").update(b).digest("hex");

// SHA-256 against Node's, across block boundaries and in odd-sized pieces.
for (const size of [0, 1, 55, 56, 63, 64, 65, 119, 120, 1000, 1048579]) {
  const data = new Uint8Array(randomBytes(size));
  const s = new Sha256();
  for (let pos = 0; pos < size; pos += 777) s.update(data.subarray(pos, pos + 777));
  check(s.hex() === nodeSha(data), "sha256 of " + size + " bytes");
}
{
  const data = new Uint8Array(randomBytes(3 << 20));
  check(await sha256Hex(data) === nodeSha(data), "sha256Hex over 3 MB");
}

// Version compare.
check(logic.compareVersions("v5.1.10", "5.1.9") > 0, "5.1.10 > 5.1.9");
check(logic.compareVersions("5.1.0", "v5.1.0") === 0, "5.1.0 == v5.1.0");
check(logic.compareVersions("v4.2", "5.0.0") < 0, "4.2 < 5.0.0");

// Latest stable / pre-release, and matching a digest.
const asset = (v, sha) => ({ name: "bmc64-" + v + ".files.zip", digest: "sha256:" + sha });
const releases = [
  { tag_name: "v5.1.10", prerelease: true, assets: [asset("v5.1.10", "aa")] },
  { tag_name: "v5.1.9", prerelease: true, assets: [asset("v5.1.9", "bb")] },
  { tag_name: "v6.0.0", draft: true, prerelease: false, assets: [asset("v6.0.0", "cc")] },
  { tag_name: "v5.1.0", prerelease: false, assets: [asset("v5.1.0", "dd")] },
  { tag_name: "v5.0.0", prerelease: false, assets: [asset("v5.0.0", "ee")] },
  { tag_name: "v4.2", prerelease: false, assets: [] },
];
const latest = logic.latestReleases(releases);
check(latest.stable.tag_name === "v5.1.0", "latest stable skips drafts");
check(latest.pre.tag_name === "v5.1.10", "latest pre-release by version");
check(logic.findOfficial(releases, "bb").release.tag_name === "v5.1.9", "digest match");
check(logic.findOfficial(releases, "cc") !== null, "drafts can still match (by digest)");
check(logic.findOfficial(releases, "zz") === null, "no digest match");

// A real package from run_tests.py: directory, manifest, every file's hash.
const zipPath = process.argv[2];
if (zipPath) {
  const bytes = new Uint8Array(readFileSync(zipPath));
  const entries = readZipDirectory(bytes);
  const manifestEntry = entries.find((e) => e.name === logic.MANIFEST);
  check(!!manifestEntry, "manifest entry found");
  const text = new TextDecoder().decode(await readEntry(bytes, manifestEntry));
  const manifest = logic.parseManifest(text);
  const info = logic.checkPackage(entries, manifest);
  check(info.target === manifest.target && info.unpackedSize > 0, "package checks pass");

  const shas = new Map();
  for (const line of text.split("\n")) {
    const w = line.split(" ");
    if (w.length === 4 && w[0] === manifest.target) shas.set(w[3], w[1]);
  }
  for (const e of entries) {
    if (e.isDir || e.name === logic.MANIFEST) continue;
    const data = await readEntry(bytes, e);
    check(nodeSha(data) === shas.get(e.name), "entry " + e.name + " unpacks to its manifest hash");
  }

  const missing = entries.filter((e) => e.name !== "kernel7.img");
  let threw = "";
  try { logic.checkPackage(missing, manifest); } catch (err) { threw = err.message; }
  check(/kernel7\.img/.test(threw), "a missing file is reported");

  let bad = "";
  try { logic.parseManifest("format 2\ntarget v1\n"); } catch (err) { bad = err.message; }
  check(/format/.test(bad), "unknown manifest format is refused");

  let notZip = "";
  try { readZipDirectory(new Uint8Array(100)); } catch (err) { notZip = err.message; }
  check(/not a zip/.test(notZip), "a non-zip is refused");
}

console.log(failed ? failed + " web test(s) failed" : "web tests passed");
process.exit(failed ? 1 : 0);
