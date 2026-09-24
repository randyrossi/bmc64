# gen_update_manifest.py

`gen_update_manifest.py` builds `bmc64-manifest.txt`, the file that drives
the BMC64 updater, and looks after the release history the manifest is made
from.

For how users update, see [docs/UPDATING.md](../../docs/UPDATING.md).

## Why the updater needs a manifest

When BMC64 applies `bmc64-update.zip`, it has to decide what to offer for
each file on the SD card:

- **up to date**: the file is already the new version;
- **Update**: the file is an untouched copy from *some* earlier release, so
  it is safe to replace;
- **Changed**: the file matches no release, so the user edited it (for
  example a keymap or `config.txt`), and it is kept unless the user says
  otherwise.

To tell these apart, the updater needs the SHA-256 of every file of the new
version and of **every release before it**. The manifest carries both, and
is packed inside each release zip.

## The two files

| File | What it is |
| --- | --- |
| `release/manifest_history.txt` | In the repository. Every file of every release so far: version, SHA-256, size and path. It only ever grows. |
| `bmc64-manifest.txt` | In each release zip (and at the root of a card after installing). This release's files marked as the `target`, plus the whole history. |

Both use the same plain-text format, one record per line:

```
# BMC64 update manifest
format 1
target v5.1.11
# <version> <sha256> <size> <path>
v5.1.11 3f9a…c21 4094676 kernel8-32.img
v5.1.11 1a2b…77c 3022848 start.elf
…
v5.1.10 448d…9e5 4094608 kernel8-32.img
…
```

- `format` is the manifest format version. The updater refuses a format it
  doesn't know.
- `target` is the version the zip installs. Every `target` line must match a
  file in the zip with the same size.
- The other lines are history. The history file has no `format` or `target`
  lines.

Paths are relative to the card root, use `/`, and may not contain spaces,
`..`, backslashes or non-ASCII characters. The script stops if a release has
such a path. The manifest never lists itself.

The Pi also reads the card's copy of `bmc64-manifest.txt` (from the version
that is installed) and adds it to the history. That is what lets a downgrade
recognise the newer release's untouched files.

## Commands

Run from the repository root. `--history` defaults to
`release/manifest_history.txt`.

### release: used by make_release.sh

```sh
python3 tools/update/gen_update_manifest.py release \
    --stage stage_dir --version v5.1.11 --update-history
```

This hashes every file in the staged release folder, writes
`stage_dir/bmc64-manifest.txt` (this version plus the history) and, with
`--update-history`, adds the version to the history file. If the history
already has that version, it is replaced.

`make_release.sh` runs this before zipping, and `check_release_files.sh`
requires `bmc64-manifest.txt`. **Commit `release/manifest_history.txt`
with every release**, or the next release will not know about this one.

### add: releases from local zips

```sh
python3 tools/update/gen_update_manifest.py add ~/Desktop/bmc64-v5.0.*.files.zip
```

This adds release zips you have locally to the history. It reads the version
from the file name (`bmc64-v5.0.1.files.zip` → `v5.0.1`); use `--version`
if the name doesn't follow that pattern. Use it for releases that are no
longer on GitHub.

### seed: releases on GitHub

```sh
python3 tools/update/gen_update_manifest.py seed --cache ~/bmc64-release-zips
```

This downloads every non-draft release's `.files.zip` from
`randyrossi/bmc64` (with `gh`, into `--cache`, skipping files already
there). It checks each zip against GitHub's SHA-256 and adds it to the
history. `--repo` picks another repository.

### check: a finished release zip

```sh
python3 tools/update/gen_update_manifest.py check bmc64-v5.1.11.files.zip
```

This confirms that the zip's manifest lists every file in the zip with the
right size and SHA-256, and nothing else.

### package: a test update from your own build

```sh
python3 tools/update/gen_update_manifest.py package \
    --stage build/sdcard --version v5.1.11 --out bmc64-update.zip
```

This makes a `bmc64-update.zip` from any staged SD-card folder, such as
`build_sdcard.sh`'s `build/sdcard`, so the updater can be tried before a
release. It does not change the history file or the stage folder. Put the
zip at the root of a card and boot.

The Web UI's Update page refuses such a zip, because it doesn't match a
release on GitHub. Copy it to the card yourself.

## How `add` and `seed` change the history

Both merge by whole version, and neither ever removes one. Releases can
disappear from GitHub, but the files they put on SD cards are still out
there. For each version they report one of:

- `added`: the version was new;
- `already in the history, identical`: nothing changed;
- `REPLACED (the history had different files)`: the zip differs from what
  was recorded. **Check why before committing.** A published release should
  never change.

Don't edit the history file by hand.

## Tests

`tools/update/test/run_tests.py` uses this script to build synthetic
releases and manifests, then runs the updater against them. `make_all.sh`
runs it. See the docstring at the top of `run_tests.py`.
