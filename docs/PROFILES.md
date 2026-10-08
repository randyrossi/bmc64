# BMC64 Profiles

> **Status: in development.** Profiles work as described on this page.

A **profile** is a complete, saved set of BMC64 settings that you can switch
to from the menu. For example:

- **C64 JiffyDOS + REU:** JiffyDOS kernal and drive ROMs, a 512K REU, a 1541-II
  on drive 8 and a 1581 on drive 9.
- **Stock C64:** the original kernal and a 1541, for games that don't like
  JiffyDOS.
- **Couch:** a USB keyboard with a symbolic keymap, while a "Desk" profile uses
  a real Commodore keyboard.
- **Elite:** set up just for one game: the stock kernal, a 1541 with true drive
  emulation, the joystick in port 2, the 6581 SID, and the game set to
  autostart. Use *Start once* to play it, and the next power-on is back on
  your usual profile.

If you don't create any profiles, BMC64 works exactly as it always has.
Profiles only do anything when BMC64 starts and when you use the *Profiles*
menu.

Profiles only **refer** to your files (ROMs, disk images, cartridges, REU and
IDE64 images). Nothing is copied into a profile, so those files must stay
where they are on the SD card. Profiles are not meant to be moved to another
SD card.

---

## Contents

- [BMC64 Profiles](#bmc64-profiles)
  - [Contents](#contents)
  - [The Main profile](#the-main-profile)
  - [Start with a good Main profile](#start-with-a-good-main-profile)
  - [Using profiles](#using-profiles)
    - [Switch to, or Start once](#switch-to-or-start-once)
    - [Saving settings](#saving-settings)
    - [Creating, renaming and deleting](#creating-renaming-and-deleting)
    - [Auto-attach disks](#auto-attach-disks)
    - [Autostart](#autostart)
  - [What a profile keeps](#what-a-profile-keeps)
  - [Profiles and machines](#profiles-and-machines)
  - [If something goes wrong](#if-something-goes-wrong)
  - [Older BMC64 versions](#older-bmc64-versions)
  - [Profile files](#profile-files)
    - [Editing them in the Web UI](#editing-them-in-the-web-ui)
    - [`active.txt`](#activetxt)
    - [`profile.txt`](#profiletxt)
    - [`main/<machine>.txt`](#mainmachinetxt)

---

## The Main profile

Your existing settings are the **Main** profile.

- Main is always in the list and can't be renamed or deleted, so you can
  always switch back to it.
- Main isn't tied to one machine: it's whatever machine you boot, with that
  machine's usual settings files (`vice.ini`, `settings.txt`,
  `settings-vic20.txt`, …), and **Switch machine** works as before.
- Main is the starting point for your profiles, so keep it set up the way you
  want BMC64 to be (see [Start with a good Main profile](#start-with-a-good-main-profile)).
- If you break your Main settings, fix them in the menu, or start again from a
  fresh SD card or by deleting the settings files.

---

## Start with a good Main profile

Every profile is a **complete copy** of the settings it was made from. Once a
profile exists, changing Main doesn't change it. So the order matters:

1. **Set up Main first,** with everything you want in every profile: video
   and scaling, keyboard, joysticks and gamepads, hotkeys, sound, network and
   Web UI, preferences. Press **Save settings**. In particular, set the
   **Web UI PIN** before you make profiles: each profile keeps the PIN it was
   made with.
2. **Make your profiles from Main:** with Main active, change only what's
   different (a kernal, drives, a REU…) and choose *New profile from current
   settings*. Back in Main, the changes you didn't save are gone.
3. **Make a profile from a profile** when it's closer to what you want, e.g.
   a game profile from your "Stock C64" profile.

If you later change something that every profile should have (a new video
mode, say), you have to change it in each profile, or make the profiles again
from Main.

---

## Using profiles

Everything is in the on-screen menu. While a profile is active, **all actions happen
in that profile** and save things as usual by pressing **Save settings**.

```
(main menu)
  C64 NTSC 60Hz HDMI
  Profile: Jiffy REU                      ← the active profile
  ...
  Profiles...
    Select profile...
    New profile from current settings
    Manage profile
      Rename profile...
      Delete profile...
    Set autostart: elite.d64
      Autostart Prg/Disk...
    Clear autostart
  Save settings (Jiffy REU)               ← saves into the active profile
```

The *Profile:* line appears once you've created a profile. It shows
*Profile: Main (no profile)* when Main is active. Choosing it is a shortcut
to *Profiles → Select profile…*. With Main active, the button is just
*Save settings*.

### Switch to, or Start once

*Profiles → Select profile…* lists every profile with its machine, e.g.
*Jiffy REU (C64)*. The active profile is marked *(\*)*. Pick one, then:

- **Switch to:** BMC64 restarts into that profile, and it becomes the profile
  used at every power-on until you switch again.
- **Start once:** BMC64 restarts into that profile for this session only. The
  next time you power on, you're back on your usual profile. If BMC64 itself
  asks to restart during that session (for example after changing network
  settings), you stay in the same profile.

A hard reset doesn't change profiles, it resets the emulated machine as usual.

### Saving settings

- **Save settings** saves into the active profile; the button shows its name,
  e.g. *Save settings (Jiffy REU)*. In a profile started once, it saves into
  that profile.
- To undo changes you haven't saved, restart BMC64.

### Creating, renaming and deleting

- **New profile from current settings** asks for a name (up to 32
  characters), saves the machine exactly as it is now as a new profile, and
  restarts into it. Start from Main, or from the profile that's closest, change
  what you need, and save it as a new profile.
- **Manage profile → Rename profile…** changes the active profile's name.
- **Manage profile → Delete profile…** deletes a profile. You can't delete Main
  or the profile that's active. Only the profile's own files are deleted,
  never your disk images or other files. If you've put anything else into a
  profile's folder, it isn't deleted and you get an error instead.

### Auto-attach disks

A profile can attach disk images to drives 8 to 11 every time it starts, for
example a GEOS boot disk on drive 8 and a data disk on drive 9:

1. Attach the disks as usual from the *Drives* menu.
2. Choose *Drives → Auto-attach options → Auto-attach current disks at
   boot*. The disks attached now are saved straight away (no need for *Save
   settings*), and a message says which drives they're on.

*Clear auto-attached disks* (in the same folder) removes them. Choosing
*Auto-attach current disks at boot* again replaces the saved disks with the
ones attached now, so a drive with no disk attached is left empty.

- The disks are attached a couple of seconds after power-on, once BMC64 has
  finished starting, and before any autostart. A disk autostart replaces the
  disk on drive 8.
- Only disks attached from the *Drives* menu are saved, not the one an
  autostart attached.
- Main can have auto-attached disks too, one set for each machine.
- If a disk isn't there any more, you get a message and the other drives are
  still attached.
- A C128 checks for a boot disk when it starts, which is before the disks
  are attached. Use a hard reset to boot from the disk, or an autostart.

### Autostart

A profile can start a program or disk image by itself every time it starts:

1. Choose *Profiles → Set autostart → Autostart Prg/Disk…* and pick the file,
   the same way as the main menu's *Autostart Prg/Disk…*.
2. That's saved straight away (no need for *Save settings*), and the menu
   shows it, e.g. *Set autostart: elite.d64*.

*Profiles → Clear autostart* removes it.

- The file is started a couple of seconds after power-on, once BMC64 has
  finished starting, exactly as if you'd picked it from *Autostart
  Prg/Disk…*. Set up the profile so the file runs (drives, kernal, memory…);
  BMC64 doesn't check that.
- Main can have an autostart too, one for each machine. Setting one (or
  auto-attached disks) is the only thing that creates the `profiles` folder
  without making a profile.
- If the file isn't there any more (moved or deleted, for example), you get
  a message and the machine starts normally.
- A hard reset doesn't run the autostart again; only power-on (or a
  restart) does.

---

## What a profile keeps

A profile keeps everything that **Save settings** saves: the emulator settings
(ROMs, drives, cartridge default, REU, IDE64, SID and so on) and BMC64's own
settings (video and scaling, keyboard layout and mapping, joysticks and USB
gamepads, GPIO, hotkeys, network and Web UI options including the Web UI
PIN, volume, preferences). So, for example, one profile can have the network
off and another on.

A few things are the same for every profile, because they live in files of
their own on the SD card:

- the Wi-Fi network and password (`wpa_supplicant.conf`)
- the logging destination (`cmdline.txt`)

The machine isn't a setting either: each profile remembers which machine it
runs on (see below).

---

## Profiles and machines

Each profile belongs to the machine it was made on: the machine, video
standard and output, for example *C64, PAL, HDMI*. All profiles are listed
whatever machine you're on.

- **Choosing a profile for another machine** switches to it for you: BMC64
  applies a matching **Switch machine** entry and restarts into the profile.
- **Main remembers the machine it last ran on.** Going from Main on the C64
  to a VIC-20 profile and back to Main puts you back on the C64.
- **Start once** on another machine: the next power-on is back on your usual
  profile's machine. Restarts BMC64 asks for during that session stay on the
  Start-once profile's machine.
- **Switch machine inside a profile:** the profile moves with you, to
  another video mode or to another machine, and Main is left as it was. So
  you can keep Main on the C64 and make a Plus/4 profile by making a new
  profile, then switching it to the Plus/4. A profile keeps BMC64's settings
  for each machine separately (like Main does), so on a machine that's new to
  it, it starts with that machine's defaults: set it up and **Save
  settings**. Its settings for the other machines are kept.
- **At power-on**, a profile whose machine isn't what booted (for example
  after editing `config.txt` by hand) isn't used: Main starts with a message.

If `machines.txt` has several entries for a machine, standard and output (for
example 720p and 1080p), the first one is used. To pin a profile to one,
set its `machine` to the whole entry (see `profile.txt` below).

---

## If something goes wrong

- **A profile can't be read**, or it's for another machine than the one that
  booted: BMC64 starts Main instead and shows a message once it's active.
- **A profile won't start properly at all:** hold **C=+F7 for 5 seconds**, then
  let go of F7 (safe mode). As well as the usual safe-mode reset of the video
  mode, this makes **Main** the power-on profile again.

---

## Older BMC64 versions

Profiles don't change the files older versions use. Main's settings stay in
the usual files at the root of the SD card. If you go back to an older
release, it starts with your Main settings and ignores the `profiles` folder.
Upgrading again finds your profiles as they were.

---

## Profile files

Profiles are stored in a `profiles` folder at the root of the SD card. You
don't need to touch these files, but you can.

```
/profiles/
  active.txt          which profile starts at power-on
  main/               Main's disks and autostart, one file per machine
    c64.txt
  jiffy-reu/          one folder per profile
    profile.txt       the profile's name, machine and autostart
    vice.ini          the profile's emulator settings
    settings-c64.txt  the profile's BMC64 settings, one file per machine
  stock-c64/
    ...
```

The folder name is the profile's **id**. It's made from the name when the
profile is created: lowercase letters, digits and `-` only (anything else
becomes `-`), up to 32 characters, with `-2`, `-3`, … added if it's already
used. Renaming a profile changes only its name, not its folder.

All profile files are plain text, one `key=value` per line. Lines starting
with `#` are comments.

### Editing them in the Web UI

The Web UI's **Profiles** page lists Main and every profile with its
machine, auto-attached disks and autostart, and marks the profile that's
running, the one used at power-on and any pending *Start once*. Each
profile's buttons open its files in the editor, and its *Actions* menu can
open its folder in *Files*, rename it or delete it, the same as the BMC64
menu (the running profile can't be deleted). Deleting the power-on profile
makes Main the power-on profile.

Each card also has **Switch to** and **Start once** buttons, which work the
same as in *Select profile*: BMC64 switches machine if needed and restarts
into the profile, and the page waits until it's back. The Web UI only runs on
the C64 and C128, so after starting a profile for another machine the page
stays offline until a C64 or C128 profile is running again.

The Web UI's file editor can open and save `active.txt`, Main's files in
`main/`, and each profile's `profile.txt`, settings files and `vice.ini`. As
with the other config files, the previous version is kept as `<name>.bak`.
Changes take effect at the next restart; saving from the menus before then overwrites edits to the active
profile's files.

### `active.txt`

| Key | Meaning |
|---|---|
| `profile` | The id of the profile used at power-on, or `main` for Main. If the file or key is missing, Main is used. |
| `once` | Optional: the id of a profile to use for the next start only. BMC64 removes it once that profile is active. |
| `main_machine` | Written by BMC64: the machine Main last ran on, e.g. `C64/PAL/HDMI`, or the whole `machines.txt` entry if it was chosen with **Switch machine** in Main. |

```ini
profile=jiffy-reu
once=stock-c64
```

### `profile.txt`

| Key | Required | Meaning |
|---|---|---|
| `name` | yes | The name shown in the menu (up to 32 characters) |
| `machine` | yes | The machine, video standard and output, as at the start of a `machines.txt` entry's `[...]` header, e.g. `C64/PAL/HDMI`. The machine is `C64`, `C128`, `VIC20`, `Plus4`, `Plus4Emu` or `Pet`. Just `C64` means any standard and output. A whole header, e.g. `C64/PAL/HDMI/VICE 1080p@50Hz`, picks that entry when switching to the profile. |
| `category` | no | Groups profiles into folders in *Select profile*, e.g. `Games` |
| `start` | no | `switch` (the default) or `once`: which choice *Select profile* offers first |
| `disk_8` … `disk_11` | no | The disk image attached to that drive at power-on, with its volume, e.g. `SD:/disks/geos.d64` |
| `autostart` | no | The program or disk image to start at power-on, with its volume, e.g. `SD:/games/elite.d64` |

```ini
name=Elite
machine=C64/PAL/HDMI
category=Games
disk_9=SD:/games/elite-save.d64
autostart=SD:/games/elite.d64
```

### `main/<machine>.txt`

Main's auto-attached disks and autostart for one machine: `c64.txt`,
`c128.txt`, `vic20.txt`, `plus4.txt`, `plus4emu.txt` or `pet.txt`. It only
holds the `disk_8` … `disk_11` and `autostart` keys.

`vice.ini` and the settings files have the same format as the files at the
root of the SD card. BMC64 writes them when you press **Save settings** in
that profile. `vice.ini` holds a section for each machine, like the one at
the root; BMC64's settings are in `settings-<machine>.txt` (`settings-c64.txt`,
`settings-c128.txt`, `settings-vic20.txt`, `settings-plus4.txt`,
`settings-plus4emu.txt`, `settings-pet.txt`). Profiles made with earlier
versions have a single `settings.txt`; BMC64 renames it for its machine the
next time the profile starts.
