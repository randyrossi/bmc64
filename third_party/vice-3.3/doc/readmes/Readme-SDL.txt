SDL port of VICE
================


    Contents
    --------

    1. General info
    2. Usage
    3. Video output


1. General info
===============

1.1 Goal

The SDL port is meant to be an easily portable version of VICE
that is fully usable with just a joystick (with at least 2 buttons)
or a keyboard. OS/arch-specific features (such as copy/paste support)
may be omitted for portability. Everything is configurable via the UI.


1.2 Features

Some new features that are missing from (some of) the native versions:
 - Free scaling using OpenGL (from the GTK port) with fixed aspect ratio options
 - Virtual keyboard (adapted from the GP2X port)
 - Hotkey mapping to any menu item
 - (Host) joystick event mapping to (machine) joystick, keyboard or menu item
 - No mouse or keyboard required, but both are supported

Some missing features that are available in (some of) the native versions:
 - (none at the moment)


1.3 Ports

The SDL port has been tested to work on:
 - AIX / rs6000
 - Amiga OS 4.x / ppc
 - AROS / amd64/x86_64, ppc, x86
 - BeOS / ppc, x86
 - BSDi / x86
 - Dingoo / mipsel
 - DragonflyBSD / x86
 - FreeBSD / alpha, amd64/x86_64, ia64, ppc, sparc64, x86
 - GP2X / arm (joystick unusable)
 - Hurd / x86
 - Interix 3.5 (aka SFU) / x86
 - Interix 5.2 (aka win2003 SUA) / x86
 - Interix 6.0 (aka vista SUA) / x86
 - Linux / alpha, amd64/x86_64, arm, armeb, avr32, hppa, ia64, m68k, mips,
           mipsel, mips64, mips64el, ppc, ppc64, s390, s390x, sh3, sh4, sparc,
           sparc64, x86
 - Mac OS X / ppc, amd64/x86_64, x86
 - Minix 3.x / x86
 - MorphOS / ppc
 - NetBSD / alpha, amd64/x86_64, arm, hppa, m68010 (libaa only), m68k, mipseb,
            mipsel, ns32k, ppc, sh3eb, sh3le, sparc, sparc64, vax, x86
 - OpenBSD / alpha, amd64/x86_64, arm, hppa, ppc, sh4, sparc64, x86
 - OpenServer / x86
 - QNX 6.x / armle, mipsle, ppcbe, shle, x86
 - SkyOS / x86
 - Solaris / amd64/x86_64, sparc, sparc64, x86
 - SunOS / m68k, sparc
 - Syllable / x86
 - Tru64 4.x / alpha
 - uClinux / m68000
 - Ultrix / mipsel, vax
 - UnixWare / x86
 - Uwin / x86
 - Win32 / mips (confirmed to work on NT 4)
 - Win32 / x86 (confirmed to work on 9x, ME, NT 3.5 (special SDL.dll),
                NT 3.51 (other special SDL.dll), NT 4, 2000, XP, 2003,
                HS, Vista, HS, 7, 8 and 10)
 - Win64 / ia64, x64
 - Zaurus (qt) / arm
 - ...


2. Usage
========

2.1 The menu

The menu is used with the following commands:
(default keys/joymap shown)

--------------------------------------------------
Command  | Key   | Joy  | Function
--------------------------------------------------
Activate | F12   | btn2 | Activate the menu
Up       | up    |  u   | Move cursor up
Down     | down  |  d   | Move cursor down
Left     | left  |  l   | Cancel/Page up
Right    | right |  r   | Enter menu/Page down
Select   | enter | fire | Select the item
Cancel   | <-    | btn2 | Return to previous menu
Exit     | ESC   | N/A  | Exit the menu
Map      | m     | btn3 | Map a hotkey (see 2.2)
--------------------------------------------------

The keys and joystick events can be configured via the menu.

Left/Right work as Page up/down on the file selector, otherwise
left does Cancel and right enters the selected submenu.

The joystick command Activate behaves as Cancel while in the menu.


2.2 Hotkeys

By default, the SDL port doesn't have any hotkeys defined.
Mapping a hotkey is simple:

1. Navigate the menu to an item
2. Issue the Map command (default: 'm', button 3)
3. Press the desired key(-combo) or joystick direction/button

The keycombo can use multiple modifiers, for example Ctrl+q and
Ctrl+Shift+q can be mapped to different menu entries. Note that
the "left" and "right" versions of a modifier are regarded as the
same key in the context of hotkeys.

Hotkeys can be unmapped by mapping the hotkey to an empty menu item.

Hotkeys do not work while using the menu or virtual keyboard.


2.3 Virtual keyboard

The menu commands are also used in the virtual keyboard:
(default keys/joymap shown)

--------------------------------------------------
Command  | Key   | Joy  | Function
--------------------------------------------------
Up       | up    |  u   | Move cursor up
Down     | down  |  d   | Move cursor down
Left     | left  |  l   | Move cursor left
Right    | right |  r   | Move cursor right
Select   | enter | fire | Press/release the key
Cancel   | <-    | btn2 | Press/release with shift
Exit     | ESC   | N/A  | Close the virtual kbd
Map      | m     | btn3 | Map a key/button
--------------------------------------------------

Note that pressing a key and releasing the key generate separate events.
This means that pressing Select on a key, moving to an another key and
releasing Select releases the latter key; with this, multiple keys
can be pressed down at once.

The joystick command Activate behaves as Cancel while using the virtual
keyboard.

The virtual keyboard can be moved by pressing (and holding) Select on an
empty space and moving the cursor.

The virtual keyboard can be closed by pressing the 'X' in the top left corner,
with the command Exit or with the command Cancel when the cursor is at an
empty spot.

Keys and joystick events can be mapped to the keyboard via the virtual keyboard.


2.4 Text input dialog virtual PC keyboard

The text input dialog also has a virtual PC keyboard, which can be activated
with the key F10 or joystick commands Cancel or Map.

When the virtual PC keyboard is active, the following commands are active:
(default keys/joymap shown)

--------------------------------------------------
Command  | Key   | Joy  | Function
--------------------------------------------------
Up       | up    |  u   | Move cursor up
Down     | down  |  d   | Move cursor down
Left     | left  |  l   | Move cursor left
Right    | right |  r   | Move cursor right
Select   | enter | fire | Press the key
Cancel   | <-    | btn2 | Press with shift
Exit     | ESC   | N/A  | Close the virtual PC kbd
Map      | m     | btn3 | Close the virtual PC kbd
--------------------------------------------------

The joystick command Activate behaves as Cancel while using the virtual PC
keyboard.

The virtual PC keyboard uses the US keymap. Simultaneous keypresses are not
possible. The virtual PC keyboard cannot be moved.

The virtual PC keyboard can be closed by pressing the 'X' in the top left corner,
with the commands Exit and Map or with the command Cancel when the cursor is at
an empty spot.

To exit/cancel the text dialog itself, press "esc" on the virtual PC keyboard.

Note that normal text input via keyboard is not possible while the virtual PC
keyboard is active.


2.5 Slider controls

Several settings like volume and colour values are configured using the slider
control. For example, the volume control is represented as follows:

Select Volume:

Step: 1
********************....................
50

The slider value can be decremented/incremented by the Left/Right commands.

The step value indicates how many units the slider will change by for each Left/
Right command.

Step values can be changed by the Up/Down command and will decrement/increment in
units of 10. The maximum step value will always be 1 order of magnitude less than
the maximum setting value e.g. volume ranges between 0 and 100, so valid step
values are 1 and 10. For a larger maximum value e.g. 1000, valid step values
would be 1, 10 and 100.

Note that normal text input via keyboard is not possible while the slider is
displayed. The Map command opens a text input dialog for editing the value
directly.


2.6 Settings

The settings are saved separately into 4 files:
 - main settings (sdl-vicerc, sdl-vice.ini, "Load/Save settings")
 - fliplist (...)
 - hotkey (sdl-hotkey-MACHINE.vkm, "Load/Save hotkeys")
 - joymap (sdl-joymap-MACHINE.vjm, "Load/Save joystick map")

Remember to save the relevant settings file.


2.7 Movie recording

Movie recording options appear in Screenshot->Record movie. Disabling with
--disable-ffmpeg, not having the FFMPEG headers during ./configure, missing or
mismatching FFMPEG library versions (see the top of log/output) and missing
dynlib support on the OS/platform may cause this menu to not exist.

Once the recording has started, any action that leads into the UI menu getting
activated stops the recording. Using hotkeys to f.ex swap joystick ports doesn't
stop it, but f.ex entering the monitor (via hotkey or a breakpoint) does.

The virtual keyboard (if used) and statusbar (if enabled) are visible
in the recording.


3. Video output
===============

3.1 Bit depth

By default, the bit depth for the video output is determined automatically;
in most cases, this means the current desktop bit depth. The detected value
is saved to the main settings file (resource "SDLBitdepth").

There is no menu item for bit depth selection, as runtime switching is not
implemented. To use a specific bit depth, use the command line parameter
"-sdlbitdepth <bpp>". Supported bpp values are 0, 8, 15, 16, 24 and 32.
The value 0 forces autodetection.


3.2 Fullscreen

There are two fullscreen modes: "Automatic" asks SDL for a fullscreen mode
with the resolution of the current window size. "Custom" uses the custom
width/height settings.


3.3 Resolution limit

The resolution limit modes are meant for devices with restricted screen sizes,
f.ex handhelds with 320x240. For most cases, the mode should be set to "Off"
at all times.

"Max" mode caps and "Fixed" forces the window size to the custom width/height.


3.4 OpenGL

OpenGL free scaling works only with bit depths 24 and 32. For fullscreen with
OpenGL, using the "Custom" fullscreen mode is strongly advised. The pixel aspect
ratio can be left as is ("Off"), fixed to a settable constant ("Custom") or
fixed to the true aspect ratio of the emulated video chip/system ("True").
Resolution limiting is disabled when free scaling is enabled.
