// The Update page's checks, kept free of the page so they can be tested with
// Node (tools/update/test/). The Pi repeats its own checks when it applies
// the update (src/update/).

export const REPO = "randyrossi/bmc64";
export const RELEASES_URL = "https://api.github.com/repos/" + REPO + "/releases?per_page=100";
export const MANIFEST = "bmc64-manifest.txt";
export const UPDATE_PATH = "/bmc64-update.zip";

// Files every release has; a zip without them is not a BMC64 release.
const CORE_FILES = ["kernel.img", "kernel7.img", "kernel8-32.img", "bootcode.bin",
                    "start.elf", "fixup.dat", "config.txt", "cmdline.txt"];

// Compares "v5.1.10" and "5.1.9" numerically: < 0, 0 or > 0.
export function compareVersions(a, b) {
  const pa = String(a).replace(/^v/i, "").split(".").map((x) => parseInt(x, 10) || 0);
  const pb = String(b).replace(/^v/i, "").split(".").map((x) => parseInt(x, 10) || 0);
  for (let i = 0; i < Math.max(pa.length, pb.length); i++) {
    const d = (pa[i] || 0) - (pb[i] || 0);
    if (d) return d < 0 ? -1 : 1;
  }
  return 0;
}

export function zipAsset(release) {
  return (release.assets || []).find((a) => a.name.endsWith(".files.zip")) || null;
}

// The newest stable and newest pre-release (by version number) that have a
// release zip. Drafts are never offered.
export function latestReleases(releases) {
  let stable = null;
  let pre = null;
  for (const r of releases) {
    if (r.draft || !zipAsset(r)) continue;
    if (r.prerelease) {
      if (!pre || compareVersions(r.tag_name, pre.tag_name) > 0) pre = r;
    } else if (!stable || compareVersions(r.tag_name, stable.tag_name) > 0) {
      stable = r;
    }
  }
  return { stable, pre };
}

// The release whose zip has this SHA-256, or null.
export function findOfficial(releases, sha) {
  for (const r of releases) {
    const a = zipAsset(r);
    if (a && a.digest === "sha256:" + sha) return { release: r, asset: a };
  }
  return null;
}

// Parses bmc64-manifest.txt; returns { target, files: Map(path -> size) }
// for the version the zip installs.
export function parseManifest(text) {
  let format = 0;
  let target = "";
  const lines = [];
  for (const raw of text.split("\n")) {
    const line = raw.trim();
    if (!line || line.startsWith("#")) continue;
    const w = line.split(/\s+/);
    if (w[0] === "format") format = parseInt(w[1], 10);
    else if (w[0] === "target") target = w[1] || "";
    else if (w.length === 4) lines.push(w);
    else throw new Error("Its " + MANIFEST + " is damaged.");
  }
  if (format !== 1) throw new Error("Its " + MANIFEST + " has a format this version can't read.");
  if (!target) throw new Error("Its " + MANIFEST + " is damaged.");
  const files = new Map();
  for (const [version, , size, path] of lines) {
    if (version === target) files.set(path, parseInt(size, 10));
  }
  if (!files.size) throw new Error("Its " + MANIFEST + " lists no files.");
  return { target, files };
}

// Checks the zip's directory against its manifest. Throws an Error with a
// message for the user; returns { target, unpackedSize }.
export function checkPackage(entries, manifest) {
  const byName = new Map(entries.filter((e) => !e.isDir).map((e) => [e.name.toLowerCase(), e]));
  for (const f of CORE_FILES) {
    if (!byName.has(f)) throw new Error("This is not a BMC64 release zip (no " + f + ").");
  }
  let unpackedSize = 0;
  for (const [path, size] of manifest.files) {
    const e = byName.get(path.toLowerCase());
    if (!e || e.size !== size) throw new Error("The zip is incomplete (" + path + ").");
    if (e.flags & 1) throw new Error("Encrypted zips are not supported.");
    unpackedSize += size;
  }
  return { target: manifest.target, unpackedSize };
}
