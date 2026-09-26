# BMC64 Web UI

BMC64 can serve a web page over the local network for checking the
machine's status and managing files on the SD card. It is **off by default**,
is available on **C64 and C128 only** (it shares the network stack described in
[NETWORKING.md](NETWORKING.md)), and can optionally be protected with a
[PIN](#pin).

The server runs on the Raspberry Pi's networking core, not the core emulation core, 
but it is still recommended to have it disabled if not in use. See
[Optimising BMC64](OPTIMISING.md) for its effect on performance.

## What it does

### Dashboard

- Machine and Raspberry Pi model, firmware version, hostname.
- Network status and IP address, uptime.
- SoC temperature.
- Raspberry Pi power / throttling health: under-voltage (now and since boot)
  and CPU throttling (now and since boot).
- SD-card free space.

### Reboot

A button that restarts BMC64, equivalent to a power cycle of the Raspberry Pi.
Any unsaved emulator state is lost.

### Hard Reset

Resets the emulated machine only. BMC64 itself
keeps running. 

### Disable Web UI

Stops the server immediately, **without a reboot**. It starts again on the next
boot unless you also turn off `Web UI (reboot)` in
`Network -> Web UI Settings`.

### Files

Browse the SD card. Click a folder to open it. Every file and folder has an
**Actions** button that opens a menu of what applies to it: **Autostart**, **Edit…**,
**Edit listing…**, **Download**, **Rename…** and **Delete**. Clicking a file's name does its main
action: it runs a disk, tape or program, edits a config file, and does nothing
for other files. On a phone the size and date are shown under the name so the
Actions button always fits.

- **Download** any file.
- **Upload…** files into the current folder. If a file of the same name already
  exists you are asked to confirm before it is overwritten. Uploaded files
  keep their original modified date and time from your PC.
- **Delete** a file, or a folder together with everything in it, after a
  confirmation prompt. Deleting a folder removes all of its contents, so
  detach any disk image inside it first. 
- **New folder…** creates a folder in the current folder.
- **Rename…** renames a file or folder in place. Names must be plain ASCII
  letters, digits and punctuation, without `/ \ : * ? " < > |`, and must not
  start with a space or end with a space or a dot. Changing only the case of a
  name is allowed.
- **Edit…** BMC64's configuration files in the browser: click **Edit…** (or the
  file name) on `vice.ini`, `settings*.txt`, `cmdline.txt`, `config.txt`,
  `machines.txt` or `wpa_supplicant.conf` in the top folder of the card, or on
  any keyboard mapping file (`.vkm`) in any folder, such as the `rpi_*.vkm` files
  in the machine folders (`C64/`, `C128/`, ...). The file opens in an overlay
  with three buttons:
  - **Cancel** closes it without saving (you are asked first if you have
    unsaved changes).
  - **Save** writes the file and closes the overlay.
  - **Save & Reboot** saves, then reboots BMC64 (after a confirmation).

  Changes take effect after the next reboot, because BMC64 only reads these
  files at startup. Saving from the emulator's own menus before you reboot
  rewrites `settings*.txt`, `vice.ini` and `wpa_supplicant.conf` from what is
  running, so it overwrites your edits. The previous version of a file is kept
  next to it as `<name>.bak` (for example `settings.txt.bak`).

  A mistake in `cmdline.txt`, `config.txt` or `machines.txt` can stop BMC64
  from booting, and a mistake in `wpa_supplicant.conf` or the web UI settings
  in `settings.txt` can lock you out of the web UI; fixing either needs the SD
  card in a computer (the `.bak` file makes that easy). A mistake in a `.vkm`
  keymap can make keys type the wrong character or stop working; the `.bak` file
  restores it. Only plain UTF-8 text files up to 256 KB can be edited.
- **Autostart** a disk image (`.d64`, `.d71`, `.d81`, `.d82`, `.g64`, `.x64`),
  tape (`.t64`, `.tap`) or program (`.prg`, `.p00`): click its name or its
  **Autostart** action and BMC64 starts it, the same as the menu's *Autostart* item.
  This resets the emulated machine. If the file can't be started, the reason
  is only written to the log.
- **Write or edit a BASIC program** without leaving the browser (C64 BASIC V2):
  - **Create BASIC PRG…** opens an editor where you type or paste a program
    listing (`10 PRINT "HELLO"`), give it a file name and press **Save PRG**. The
    listing is converted to a real tokenised `.prg` in the current folder, ready
    to Autostart. `.prg` is added to the name if it is missing.
  - **Edit listing…** (in a `.prg` file's Actions menu) shows the program as a
    listing, which you can change and save again. It works on any BASIC program
    that loads at `$0801`; for other files it tells you they are not BASIC.
    Change the file name before saving to keep the original and save a copy.

  Keywords can be typed in either case, and letters of either case give the
  C64's default upper case, so `print "hello"` shows HELLO on the C64. Anything
  that isn't a plain character is written in braces: `{clr}`, `{home}`,
  `{down}`, `{red}`, `{f1}` and so on (the names VICE's `petcat` uses), or
  `{$xx}` for any byte value, for example `{$c1}` for a shifted A. A count
  repeats a code: `{right*39}`. `?` is
  stored as `PRINT`. Saving a line that is just a number deletes that line, and
  lines are stored in number order.

  If a program has data after its BASIC part (typically machine code that a
  `SYS` line calls), that data is kept, but it isn't shown, so don't change the
  length of the BASIC part unless the program allows for it. A program that
  wasn't made by a normal tokeniser can change slightly when saved; the editor
  warns you when it sees this.
- **Attach a cartridge**: a `.crt` file has an **Attach cartridge** action
  (clicking the name does the same). It attaches the cartridge like the menu's
  *Attach CRT* item and, by default, hard resets the machine so it starts
  (VICE's *reset on cartridge change* setting). Only `.crt` files work; raw
  8K/16K/Ultimax images need their type chosen in the emulator's menu. There
  is no detach here yet, so use the menu's *Detach cartridge*.

### Update

Shows the installed version and the latest stable and pre-release versions on
GitHub, each with a **Download** link. Drop a downloaded release zip on the
page: it is checked (a complete release, the same file GitHub published, room
on the card) and uploaded as
`/bmc64-update.zip`, which BMC64 installs on the next boot. The page also shows
an update waiting on the card (**Reboot to update** / **Remove**) and the
report of the last update. A release from before the updater is accepted too,
with a warning that the updater can't be used after installing it. A zip that
doesn't match a GitHub release can still be uploaded after a warning that it
is at your own risk. Releases deleted from GitHub, and any release while
GitHub can't be reached, are still recognised from a list of official
releases built into the Web UI. See
[UPDATING.md](UPDATING.md).

Only the page itself talks to GitHub (your browser fetches the release list);
BMC64 never connects to GitHub.

BMC64's own configuration files (`settings*.txt`,
`wpa_supplicant.conf`, `cmdline.txt`, `config.txt`, `machines.txt`,
`bmc64.log`) and the `/firmware` folder are protected: they cannot be uploaded
to, deleted, renamed or renamed onto from the web UI, and their rows have no
Rename or Delete action. The configuration files can still be changed
with the editor above (`bmc64.log` and `/firmware` cannot be changed at all).
`vice.ini` can be edited and also uploaded.

> [!WARNING]
> Do not upload to, delete or rename a disk image that the emulator currently
> has attached, or a folder that contains one. Detach it first.

## Requirements

- `Network Device` set to `Ethernet` or `WiFi` and connected, so the `Network`
  menu shows an `IP Address`. See [NETWORKING.md](NETWORKING.md) for network
  setup.
- A phone or computer on the same local network with a web browser.

## Enable it

1. Open `Network` and set `Network Device` to `Ethernet` or `WiFi` if you have
   not already. The `Web UI Settings` folder is greyed out until a network
   device is selected.
2. Open `Network -> Web UI Settings` and set `Web UI (reboot)` to on.
3. Accept the reboot prompt, or save the settings and reboot.

The setting is stored as `webui_enabled=1` in `settings.txt`
(`settings-c128.txt` on C128).

## PIN

The `Web UI PIN` field in `Network -> Web UI Settings` sets an access PIN. Leave
it blank for no PIN (anyone on the network can use the page).

With a PIN set, the browser shows its standard login prompt the first time you
open the page. It uses the username **and** a password box for
HTTP Basic authentication. BMC64 ignores the username, so type anything there and enter the PIN as the password. The PIN is remembered for the rest of the browser session.

Changing the PIN needs a reboot to take effect (you are prompted). The PIN is
stored in clear text as `webui_pin=` in `settings.txt`, so do not reuse a
password you use elsewhere.

Wrong-PIN attempts are answered after a short delay to slow guessing, but a
short numeric PIN is still weak; treat this as "keep the household out", not
real security.

## Open it

After BMC64 has rebooted and connected, browse to:

```text
http://<bmc64-ip>/
```

`<bmc64-ip>` is the address shown as `IP Address` in the `Network` menu, for
example `http://192.168.1.42/`. The server listens on port `80`, or port `8080`
if port `80` is not available (`http://<bmc64-ip>:8080/`).

## Security

> [!WARNING]
> Without a [PIN](#pin) the web UI has **no authentication** and anyone who can
> reach BMC64 on the network can view its status, browse / download / upload /
> delete files on the SD card, edit its configuration files (including
> `cmdline.txt`, `config.txt` and the Wi-Fi settings, and the PIN in
> `settings.txt`), and reboot the machine. Even with a PIN, traffic is plain
> HTTP on the LAN, and the PIN and Wi-Fi password are shown in clear text in
> the editor.
>
> Only enable it on a network you trust. Set a PIN. Turn it off
> (`Network -> Web UI Settings -> Web UI (reboot)` off, then reboot) when you
> are done, or use the **Disable Web UI** button to stop it until the next
> reboot.

---

# Developing the Web UI

The front end is a plain single-page app — no framework, no build step. The
source files live in `src/webui/assets/`:

| File | Purpose |
| --- | --- |
| `index.html` | page structure, including the editor overlay |
| `style.css` | styling |
| `js/app.js` | entry point: wires the modules together and switches between views |
| `js/api.js` | every call to a `/api/*` endpoint; other modules never use `fetch` directly |
| `js/util.js` | DOM lookup, number formatting, path helpers |
| `js/dashboard.js` | dashboard, status polling, hardware / storage meters, reboot / reset / disable |
| `js/files.js` | file browser: list, upload, download, delete, run |
| `js/editor.js` | overlay editor for the config files and BASIC listings |
| `js/basic.js` | C64 BASIC V2 tokeniser and detokeniser (listing <-> PRG); tested in `tools/webui_test/` |
| `update/update.js` | the Update page: release list, zip checks, upload of `bmc64-update.zip` |
| `update/logic.js` | the Update page's checks (version compare, manifest, package); tested in `tools/update/test/` |
| `update/zip.js`, `update/sha256.js` | zip reading and SHA-256 for the Update page |
| `update/settings.js` | the updater settings (Update page on or off, GitHub repository); **generated** from `updater.cfg` by `tools/update/updater_cfg.py` (the build and the preview server refresh it) |
| `update/official_releases.js` | the official releases known to this build; **generated** from `release/release_digests.txt` by `tools/update/gen_update_manifest.py` (the build refreshes it) |
| `update/update.css` | the Update page's styles |
| `logo.png`, `title.png` | images |

The scripts are ES modules (`<script type="module">`), so imports use paths
such as `./api.js`. `index.html` lists every module in a `modulepreload` link
so the browser fetches them in parallel rather than one import level at a
time, which matters because the device serves one connection at a time. Add a
line there when you add a module. Subfolders are supported; the file's path
under `assets/` is its URL.

These assets are **embedded into the kernel image** as a generated C source,
`src/webui/webui_assets.c`, produced by `tools/gen_webui_assets.py`. The build
regenerates it automatically. 

After editing an asset you can regenerate it by hand with:

```sh
python3 tools/gen_webui_assets.py
```
## Local preview server

`tools/webui_dev_server.py` serves `src/webui/assets/` exactly the way the
on-device server does (root paths like `/style.css` and `/js/app.js` resolve) and
**mocks every `/api/*` endpoint**, so the whole UI — dashboard, file browser,
upload, delete, edit, reboot, disable — works on your PC with no Raspberry Pi.

```sh
python3 tools/webui_dev_server.py
# then open http://localhost:8000/
```

With **live reload** on (the default), the browser refreshes automatically
whenever you save a file in `src/webui/assets/`. Put your editor and the
browser side by side.

Before it starts serving, it runs the tests in `tools/webui_test/` (needs
Node.js 22.12+ on `PATH`; `source get_gnu_toolchain.sh` if you don't have one)
and refuses to start if they fail, printing what failed. Pass `--skip-tests`
to start anyway; if no Node.js can be found at all, that only prints a
warning and does not stop the server starting.

### What the mocks do

| Endpoint | Mock behaviour |
| --- | --- |
| `GET /api/status` | fake but plausible C64 / Pi 3 status; the `throttled` value comes from `--throttled` |
| `GET /api/volumes` | real free / total space of the browsed folder |
| `GET /api/fs/list` | lists a **real** local directory (see `--root`) |
| `GET /api/fs/download` | streams the real local file |
| `POST /api/fs/upload` | writes a real file into `--root` (same `.part`-then-rename, protected-name, `overwrite=1` and `mtime=` rules as the device) |
| `POST /api/fs/save` | rewrites an editable config file in `--root` (same allowlist, 256 KB limit, `.part` / `.bak` handling and required `X-BMC64-Web` header as the device); `GET /api/fs/list` marks editable files with `"edit": true` |
| `POST /api/fs/delete` | removes the real file / empty directory (same protected-name rules); with `recursive=1` (and the required `X-BMC64-Web` header) a whole folder tree |
| `POST /api/fs/mkdir` | creates a real folder (same name rules, protected-name rules and required `X-BMC64-Web` header as the device) |
| `POST /api/fs/rename` | renames a real file or folder in place (same rules as `mkdir`; `to=` is the new bare name); `GET /api/fs/list` marks the entries the device would refuse to rename or delete with `"protected": true` |
| `POST /api/reboot` | logs and does nothing |
| `POST /api/reset` | logs and does nothing |
| `POST /api/webui/disable` | actually stops the dev server, like the device |

### Options

| Flag | Effect |
| --- | --- |
| `--root <dir>` | folder the file browser reads and writes (default: the repo root). Upload / download / delete act on real files here — point it at a copy of your SD-card contents. |
| `--throttled <hex>` | value returned as `throttled`, to exercise the power / throttling styling, e.g. `0x1` (under-voltage now), `0x8` (partial throttle), `0x50000` (under-voltage + throttled since boot) |
| `--pin <pin>` | require this PIN via HTTP Basic Auth, like the device |
| `--port <n>` | listen port (default `8000`) |
| `--no-watch` | disable live reload |
| `--skip-tests` | start even if `tools/webui_test/` fails or can't run |

### Tests

The BASIC tokeniser (`src/webui/assets/js/basic.js`) has tests in
`tools/webui_test/`. They run under Node.js 22.12 or newer:

```sh
source get_gnu_toolchain.sh               # also puts a suitable Node.js on PATH
node tools/webui_test/run_tests.mjs       # exit status 0 = all passed
```

**The build requires them to pass.** `make_all.sh` runs them right after setting
up the toolchain, and the `Makefile` runs them again before it embeds the web UI
assets (only when `basic.js` or the tests have changed since they last passed),
so a failing test stops the build.

The build also requires Node.js. `get_gnu_toolchain.sh` uses a Node.js already on
your `PATH` if it is new enough, and otherwise downloads a pinned release
(checksum verified, needs `curl` or `wget`) into the same cache as the Arm
toolchain. If it can't get one, the build stops.

The tests do not need VICE: the expected results are stored in
`basic_vectors.js`. That file was made with VICE's `petcat` and the C64 BASIC
ROM's keyword table by `gen_basic_vectors.py`, which is only needed to add test
cases.

The Update page's modules (`update/logic.js`, `zip.js`, `sha256.js`) are tested
together with the on-device updater by `tools/update/test/run_tests.py`.
`make_all.sh` runs them too and stops if they fail. Besides Python 3 they need
the PC's C compiler (`cc`, or set `HOST_CC`) with AddressSanitizer, as gcc and
clang on Linux have.

## Folding changes back into the image

Run the generator (or just build):

```sh
python3 tools/gen_webui_assets.py
./make_all.sh <pi0|pi2|pi3>          # or build_sdcard.sh ...
```

Then test on real hardware: the on-device server and FatFs behave slightly
differently from the mock (CP850 file names, the 8080 port fallback, Wi-Fi
multicast timing).
