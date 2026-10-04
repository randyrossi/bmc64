# BMC64 Profiles

> **Status: in development.** Profiles work as described on this page.
> *Drives → Auto-attach options* is already in the menu but doesn't do
> anything yet.

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
- Main can have an autostart too, one for each machine. Setting one is the
  only thing that creates the `profiles` folder without making a profile.
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
- the machine and video mode chosen with **Switch machine** (`config.txt`,
  `cmdline.txt`)

---

## Profiles and machines

Each profile belongs to the machine it was made on (C64, C128, VIC-20, Plus/4
or PET). For now a profile only starts on that machine: all profiles are
listed, but choosing one for another machine shows a message instead. Switch
to that machine first (**Switch machine**), then choose the profile.

---

## If something goes wrong

- **A profile can't be read**, or it's for another machine: BMC64 starts Main
  instead and shows a message once it's active.
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
  main/               Main's autostart, one file per machine
    c64.txt
  jiffy-reu/          one folder per profile
    profile.txt       the profile's name, machine and autostart
    vice.ini          the profile's emulator settings
    settings.txt      the profile's BMC64 settings
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

The Web UI's file editor can open and save `active.txt`, Main's files in
`main/`, and each profile's `profile.txt`, `settings.txt` and `vice.ini`. As with the other config files,
the previous version is kept as `<name>.bak`. Changes take effect at the next
restart; saving from the menus before then overwrites edits to the active
profile's files.

### `active.txt`

| Key | Meaning |
|---|---|
| `profile` | The id of the profile used at power-on, or `main` for Main. If the file or key is missing, Main is used. |
| `once` | Optional: the id of a profile to use for the next start only. BMC64 removes it once that profile is active. |

```ini
profile=jiffy-reu
once=stock-c64
```

### `profile.txt`

| Key | Required | Meaning |
|---|---|---|
| `name` | yes | The name shown in the menu (up to 32 characters) |
| `machine` | yes | The machine: `C64`, `C128`, `VIC20`, `Plus4`, `Plus4Emu` or `Pet` |
| `category` | no | Groups profiles into folders in *Select profile*, e.g. `Games` |
| `start` | no | `switch` (the default) or `once`: which choice *Select profile* offers first |
| `autostart` | no | The program or disk image to start at power-on, with its volume, e.g. `SD:/games/elite.d64` |

```ini
name=Elite
machine=C64
category=Games
autostart=SD:/games/elite.d64
```

### `main/<machine>.txt`

Main's autostart for one machine: `c64.txt`, `c128.txt`, `vic20.txt`,
`plus4.txt`, `plus4emu.txt` or `pet.txt`. It only holds the `autostart` key.

`vice.ini` and `settings.txt` have the same format as the files at the root of
the SD card. BMC64 writes them when you press **Save settings** in that
profile.
