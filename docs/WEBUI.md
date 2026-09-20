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

Browse the SD card, and:

- **Download** any file.
- **Upload** files into the current folder. If a file of the same name already
  exists you are asked to confirm before it is overwritten.
- **Delete** a file, after a confirmation prompt.
- **Run** a disk image (`.d64`, `.d71`, `.d81`, `.d82`, `.g64`, `.x64`), tape
  (`.t64`, `.tap`) or program (`.prg`, `.p00`): click its name or its **Run**
  button and BMC64 autostarts it, the same as the menu's *Autostart* item.
  This resets the emulated machine. If the file can't be started, the reason
  is only written to the log.

There is no rename yet. BMC64's own configuration files (`settings*.txt`,
`wpa_supplicant.conf`, `cmdline.txt`, `config.txt`, `machines.txt`,
`bmc64.log`) and the `/firmware` folder are protected: they cannot be uploaded
to or deleted from the web UI.

> [!WARNING]
> Do not upload to or delete a disk image that the emulator currently has
> attached. Detach it first.

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
> delete files on the SD card, and reboot the machine. Even with a PIN, traffic
> is plain HTTP on the LAN.
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
| `index.html` | page structure |
| `style.css` | styling |
| `app.js` | all behaviour (status polling, file browser, upload/delete, reboot) |
| `logo.png`, `title.png` | images |

These assets are **embedded into the kernel image** as a generated C source,
`src/webui/webui_assets.c`, produced by `tools/gen_webui_assets.py`. The build
regenerates it automatically. 

After editing an asset you can regenerate it by hand with:

```sh
python3 tools/gen_webui_assets.py
```
## Local preview server

`tools/webui_dev_server.py` serves `src/webui/assets/` exactly the way the
on-device server does (root paths like `/style.css` and `/app.js` resolve) and
**mocks every `/api/*` endpoint**, so the whole UI — dashboard, file browser,
upload, delete, reboot, disable — works on your PC with no Raspberry Pi.

```sh
python3 tools/webui_dev_server.py
# then open http://localhost:8000/
```

With **live reload** on (the default), the browser refreshes automatically
whenever you save a file in `src/webui/assets/`. Put your editor and the
browser side by side.

### What the mocks do

| Endpoint | Mock behaviour |
| --- | --- |
| `GET /api/status` | fake but plausible C64 / Pi 3 status; the `throttled` value comes from `--throttled` |
| `GET /api/volumes` | real free / total space of the browsed folder |
| `GET /api/fs/list` | lists a **real** local directory (see `--root`) |
| `GET /api/fs/download` | streams the real local file |
| `POST /api/fs/upload` | writes a real file into `--root` (same `.part`-then-rename, protected-name, and `overwrite=1` rules as the device) |
| `POST /api/fs/delete` | removes the real file / empty directory (same protected-name rules) |
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

## Folding changes back into the image

Run the generator (or just build):

```sh
python3 tools/gen_webui_assets.py
./make_all.sh <pi0|pi2|pi3>          # or build_sdcard.sh ...
```

Then test on real hardware: the on-device server and FatFs behave slightly
differently from the mock (CP850 file names, the 8080 port fallback, Wi-Fi
multicast timing).
