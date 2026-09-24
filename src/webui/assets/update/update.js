// The Update page: shows the latest releases, checks a downloaded release zip
// and uploads it to the card as /bmc64-update.zip. BMC64 applies it on the
// next boot, where the user picks Replace or Keep for each file in BMC64's
// update view.
//
//   logic.js   checks that don't need the page (tested with Node)
//   zip.js     zip directory and entry reading
//   sha256.js  SHA-256 (crypto.subtle needs https, the web UI is http)

import * as api from "../js/api.js";
import { fmtBytes } from "../js/util.js";
import { sha256Hex } from "./sha256.js";
import { readZipDirectory, readEntry } from "./zip.js";
import {
  RELEASES_URL, MANIFEST, UPDATE_PATH, compareVersions, latestReleases,
  zipAsset, findOfficial, parseManifest, checkPackage, checkOlderPackage,
  olderReleaseWarning,
} from "./logic.js";

let root;
let vol = "SD";
let installed = "";
let releasesPromise = null;
let pending = null;   // { file, target, direction, older } once a zip passed the checks

const el = (id) => root.querySelector("#" + id);

function h(tag, props = {}, ...children) {
  const e = document.createElement(tag);
  Object.assign(e, props);
  for (const c of children) e.append(c);
  return e;
}

// GitHub's release list, fetched once per page load.
function getReleases() {
  if (!releasesPromise) {
    releasesPromise = fetch(RELEASES_URL, { cache: "no-store" }).then((r) => {
      if (!r.ok) throw new Error("GitHub answered " + r.status);
      return r.json();
    });
    releasesPromise.catch(() => { releasesPromise = null; });
  }
  return releasesPromise;
}

const fmtDate = (iso) => iso ? new Date(iso).toLocaleDateString() : "";

// ---- latest releases ----

function releaseRow(label, release) {
  const row = h("div", { className: "up-rel" });
  row.append(h("span", { className: "up-rel-label" }, label));
  if (!release) {
    row.append(h("span", { className: "up-rel-ver" }, "—"));
    return row;
  }
  const asset = zipAsset(release);
  const cmp = installed ? compareVersions(release.tag_name, installed) : 1;
  row.append(
    h("span", { className: "up-rel-ver" }, release.tag_name),
    h("span", { className: "up-rel-date" }, fmtDate(release.published_at)),
    h("a", { className: "btn", href: release.html_url, target: "_blank", rel: "noopener",
             textContent: "Release notes" }),
    h("a", { className: "btn" + (cmp > 0 ? " btn-primary" : ""), href: asset.browser_download_url,
             textContent: "Download" }),
    h("span", { className: "up-rel-tag" + (cmp > 0 ? " up-new" : ""),
                textContent: cmp > 0 ? "newer" : cmp === 0 ? "installed" : "older" }),
  );
  return row;
}

async function showReleases() {
  const box = el("up-releases");
  box.textContent = "Checking GitHub…";
  try {
    const { stable, pre } = latestReleases(await getReleases());
    box.replaceChildren(releaseRow("Latest stable", stable), releaseRow("Latest pre-release", pre));
  } catch (err) {
    box.replaceChildren(h("p", { className: "msg err",
      textContent: "Couldn't reach GitHub (" + err.message + "). Updates can't be checked right now." }));
  }
}

// ---- update waiting on the card, last report ----

async function showCardState() {
  try {
    const data = await api.listDir("/");
    vol = data.vol || vol;
    const zip = (data.entries || []).find((e) => !e.dir && e.name.toLowerCase() === "bmc64-update.zip");
    el("up-waiting").hidden = !zip;
    if (zip) el("up-waiting-size").textContent = fmtBytes(zip.size);
  } catch (err) {
    el("up-waiting").hidden = true;
  }

  // The report of the last update is in the one /backup/v<old> folder.
  el("up-last").hidden = true;
  try {
    const dirs = ((await api.listDir("/backup")).entries || [])
      .filter((e) => e.dir && /^v\d/.test(e.name))
      .sort((a, b) => compareVersions(b.name, a.name));
    for (const d of dirs) {
      try {
        const text = new TextDecoder().decode(
          await api.readFile(vol, "/backup/" + d.name + "/update-report.txt"));
        el("up-report").textContent = text;
        el("up-last").hidden = false;
        break;
      } catch (err) { /* not an updater backup */ }
    }
  } catch (err) { /* no /backup folder yet */ }
}

// ---- checking a dropped zip ----

// One line of the check list; ends as ok (✓) or fail (✗) with a note.
function step(text) {
  const note = h("span", { className: "up-note" });
  const li = h("li", { className: "up-step" }, text, note);
  el("up-steps").append(li);
  const end = (cls, t) => { li.classList.add(cls); note.textContent = t ? " — " + t : ""; };
  return {
    ok: (t) => end("ok", t),
    fail: (t) => end("fail", t),
    note: (t) => { note.textContent = " " + t; },
  };
}

async function checkFile(file) {
  pending = null;
  el("up-steps").replaceChildren();
  el("up-go").hidden = true;
  el("up-msg").textContent = "";
  el("up-msg").className = "msg";

  let s = step("Reading " + file.name + " (" + fmtBytes(file.size) + ")");
  let bytes, entries, manifest, info;
  try {
    bytes = new Uint8Array(await file.arrayBuffer());
    entries = readZipDirectory(bytes);
    s.ok();
  } catch (err) { s.fail(err.message); return; }

  // A zip without a manifest is a release from before the updater: BMC64
  // recognises it from the card's own manifest. Its version comes from GitHub.
  let older = false;
  s = step("Update package");
  try {
    const entry = entries.find((e) => e.name.toLowerCase() === MANIFEST);
    if (entry) {
      manifest = parseManifest(new TextDecoder().decode(await readEntry(bytes, entry)));
      info = checkPackage(entries, manifest);
      s.ok("BMC64 " + info.target + ", " + manifest.files.size + " files");
    } else {
      info = checkOlderPackage(entries);
      older = true;
      const root = (await api.listDir("/")).entries || [];
      if (!root.some((e) => !e.dir && e.name.toLowerCase() === MANIFEST)) {
        throw new Error("it is a release from before the updater, which BMC64 recognises " +
                        "from the card's " + MANIFEST + ", and the card has none");
      }
      s.ok("a release from before the updater");
    }
  } catch (err) { s.fail(err.message); return; }

  s = step("Official release");
  try {
    const releases = await getReleases();
    const sha = await sha256Hex(bytes, (f) => { s.note(Math.round(f * 100) + "%"); });
    const match = findOfficial(releases, sha);
    if (!match) throw new Error("it doesn't match any official BMC64 release on GitHub");
    if (older) {
      info.target = match.release.tag_name;
    } else if (match.release.tag_name !== info.target) {
      throw new Error("it is " + match.release.tag_name + " on GitHub but says " + info.target);
    }
    s.ok("matches " + match.release.tag_name + (match.release.prerelease ? " (pre-release)" : "") +
         " on GitHub");
  } catch (err) {
    s.fail(err instanceof TypeError || /GitHub answered/.test(err.message)
      ? "couldn't check it against GitHub (" + err.message + ")"
      : err.message);
    el("up-msg").textContent = "Only files that can be checked against GitHub are uploaded here. " +
      "You can still copy a zip to the card as bmc64-update.zip yourself, at your own risk.";
    return;
  }

  const direction = compareVersions(info.target, installed);
  s = step("Version");
  s.ok(direction > 0 ? "update from v" + installed + " to " + info.target
     : direction === 0 ? info.target + " is already installed; this repairs missing or damaged files"
     : "downgrade from v" + installed + " to " + info.target);

  s = step("Space on the SD card");
  try {
    const sd = ((await api.getVolumes()).volumes || [])[0];
    const need = file.size + info.unpackedSize;
    if (sd && sd.free_kb * 1024 < need) {
      throw new Error("needs " + fmtBytes(need) + ", " + fmtBytes(sd.free_kb * 1024) + " free");
    }
    s.ok();
  } catch (err) { s.fail(err.message); return; }

  pending = { file, target: info.target, direction, older };
  if (older) {
    el("up-msg").className = "msg up-warn";
    el("up-msg").textContent = olderReleaseWarning(info.target);
  }
  el("up-go").textContent = direction < 0 ? "Upload downgrade to BMC64" : "Upload to BMC64";
  el("up-go").hidden = false;
}

async function uploadPending() {
  if (!pending) return;
  const { file, target, direction, older } = pending;
  if (direction < 0 && !confirm("Install the older version " + target + "?" +
                                (older ? "\n\n" + olderReleaseWarning(target) : ""))) {
    return;
  }
  el("up-go").hidden = true;
  const msg = el("up-msg");
  msg.className = "msg";
  try {
    await api.upload(vol, UPDATE_PATH, file, {
      overwrite: true,
      onProgress: (f) => { msg.textContent = "Uploading… " + Math.round(f * 100) + "%"; },
    });
    const data = await api.listDir("/");
    const zip = (data.entries || []).find((e) => e.name.toLowerCase() === "bmc64-update.zip");
    if (!zip || zip.size !== file.size) throw new Error("the file on the card has the wrong size");
    msg.textContent = target + " is on the card. It is installed on the next boot.";
    if (older) {
      msg.className = "msg up-warn";
      msg.textContent += " " + olderReleaseWarning(target);
    }
    pending = null;
    showCardState();
  } catch (err) {
    msg.className = "msg err";
    msg.textContent = "Upload failed: " + err.message;
    el("up-go").hidden = false;
  }
}

async function rebootToUpdate() {
  if (!confirm("Reboot BMC64 now? After the reboot, BMC64 shows its update view, where you choose which files to replace.")) {
    return;
  }
  try {
    await api.reboot();
    el("up-waiting-msg").textContent = "Rebooting… continue in BMC64's update view.";
  } catch (err) {
    el("up-waiting-msg").textContent = "Reboot failed: " + err.message;
  }
}

async function removeWaiting() {
  if (!confirm("Delete bmc64-update.zip from the card?")) return;
  try {
    await api.deleteFile(vol, UPDATE_PATH);
  } catch (err) {
    el("up-waiting-msg").textContent = "Delete failed: " + err.message;
  }
  showCardState();
}

// ---- page ----

const TEMPLATE = `
  <div class="card">
    <h2><span class="c-ico">⇪</span>Update BMC64</h2>
    <dl class="kv"><dt>Installed</dt><dd id="up-installed">—</dd></dl>
    <div class="up-releases" id="up-releases"></div>
    <p class="fb-note">Download a release, then drop the zip below. It is checked, copied to the
      card as bmc64-update.zip and installed on the next boot, where you choose which files to
      replace. Files you changed are kept unless you choose otherwise, and the originals are
      saved in /backup.</p>
  </div>

  <div class="card up-waiting" id="up-waiting" hidden>
    <h2><span class="c-ico">⟳</span>Update waiting</h2>
    <p>bmc64-update.zip (<span id="up-waiting-size"></span>) is on the card and is installed on
      the next boot.</p>
    <div class="up-buttons">
      <button class="btn btn-reboot" id="up-reboot">Reboot to update</button>
      <button class="btn" id="up-remove">Remove</button>
    </div>
    <p class="msg" id="up-waiting-msg"></p>
  </div>

  <div class="card">
    <h2><span class="c-ico">▤</span>Install a downloaded release</h2>
    <label class="up-drop" id="up-drop">
      <input type="file" id="up-file" accept=".zip" hidden>
      <span>Drop the downloaded zip here, or click to choose it</span>
    </label>
    <ul class="up-steps" id="up-steps"></ul>
    <button class="btn btn-primary" id="up-go" hidden>Upload to BMC64</button>
    <p class="msg" id="up-msg"></p>
  </div>

  <div class="card" id="up-last" hidden>
    <h2><span class="c-ico">ⓘ</span>Last update</h2>
    <pre class="up-report" id="up-report"></pre>
  </div>
`;

export function initUpdate(container) {
  root = container;
  root.innerHTML = TEMPLATE;
  const drop = el("up-drop");
  el("up-file").addEventListener("change", (e) => {
    if (e.target.files[0]) checkFile(e.target.files[0]);
    e.target.value = "";
  });
  drop.addEventListener("dragover", (e) => { e.preventDefault(); drop.classList.add("over"); });
  drop.addEventListener("dragleave", () => drop.classList.remove("over"));
  drop.addEventListener("drop", (e) => {
    e.preventDefault();
    drop.classList.remove("over");
    if (e.dataTransfer.files[0]) checkFile(e.dataTransfer.files[0]);
  });
  el("up-go").addEventListener("click", uploadPending);
  el("up-reboot").addEventListener("click", rebootToUpdate);
  el("up-remove").addEventListener("click", removeWaiting);
}

export async function showUpdate() {
  try {
    installed = (await api.getStatus()).version || "";
  } catch (err) {
    installed = "";
  }
  el("up-installed").textContent = installed ? "BMC64 v" + installed : "—";
  showReleases();
  showCardState();
}
