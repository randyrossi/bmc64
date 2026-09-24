# Updating BMC64

BMC64 can update itself from a release zip. It never checks for, downloads
or installs anything on its own: you start every update.

Updating needs a BMC64 version that includes the updater. It can install any
official release zip, including releases from before the updater (see
[Older versions](#older-versions)).

## Two ways to start an update

### With the Web UI (C64 and C128)

1. Open the Web UI and choose **Update**. It shows the installed version and the
   latest stable and pre-release versions on GitHub.
2. Click **Download** next to the one you want. Your browser saves the zip.
3. Drop the zip onto the page (or click the box and choose it). The page checks
   that it is a complete, official BMC64 release that matches GitHub, and that
   it fits on the card, then copies it to the card as `bmc64-update.zip`.
4. Click **Reboot to update**, or reboot whenever it suits you.

The page refuses any zip it can't check against GitHub, for example when
GitHub can't be reached. You can still use such a zip by copying it to the
card yourself (below), at your own risk.

### By copying the file yourself (any machine)

Copy the release zip to the top folder of the SD card and rename it to
`bmc64-update.zip`. Then start BMC64.

## The update view

When BMC64 starts and finds `bmc64-update.zip`, it shows the **BMC64 UPDATE**
screen (or **DOWNGRADE** for an older version, **REPAIR** for the same
version). It lists every file that differs from the new version:

| Label | Meaning | Default |
| --- | --- | --- |
| **Update** | A file from an earlier release that you haven't changed | Replace |
| **New** | A file the new version adds | Add |
| **Changed** | A file you changed (it matches no release) | Keep |

Use Up/Down (keyboard or joystick) to move and Return, Space or Fire to switch
a file between Replace and Keep. A group heading switches the whole group.
`start.elf` and `fixup.dat` always switch together. Replacing `config.txt` or
`cmdline.txt` resets the machine and video mode to the release's defaults.

Then choose:

- **Update now and reboot**: after a confirmation, the chosen files are
  unpacked and checked, the originals are moved to `/backup`, the new files
  are moved into place, and BMC64 restarts on the new version.
- **Not now**: nothing changes; the screen appears again next time.
- **Skip this update**: renames the file to `bmc64-update.zip.skipped` so it
  isn't offered again.

Your own files (`settings*.txt`, `vice.ini`, `wpa_supplicant.conf`, ROMs,
disks, tapes, snapshots, ...) are never touched.

## Older versions

Release zips from before the updater have no `bmc64-manifest.txt`. BMC64
recognises them from the card's own `bmc64-manifest.txt`, which lists every
earlier release, so you can go back to any official release the same way:
download it from the [GitHub releases page](https://github.com/randyrossi/bmc64/releases)
and use it as `bmc64-update.zip`. The card needs its `bmc64-manifest.txt`,
which every install or update with the updater puts there.

These versions don't include the updater, so after installing one you can't
use the updater again: to update, copy a newer release to the card by hand, as
described in [INSTALLATION.md](INSTALLATION.md). The Web UI warns about this
when you drop such a zip and after it is uploaded, and the update view asks
you to **Continue** before it lists the files.

## Backups

- Original kernels are kept in `/backup/kernel/v<old version>/`, for every
  version you update from.
- Every other file that was replaced is kept in `/backup/v<old version>/`.
  Only the most recent of these folders is kept.
- `/backup/v<old version>/update-report.txt` lists what was replaced, added and
  kept. The Web UI's Update page shows it too.

### Going back

Put the card in a computer and follow `/backup/v<old version>/RESTORE.txt`:
copy that folder's files, and the matching `/backup/kernel/v<old version>/`
kernels, back to the top of the card.

## If the power goes off

- While the screen says *Checking* or *Extracting*, nothing on the card has
  changed yet. The update screen comes back next time.
- While it says *Installing*, BMC64 finishes the update on the next start and
  then restarts.
- Don't switch off during an update. If the card no longer starts, restore the
  backup by hand as described above.
