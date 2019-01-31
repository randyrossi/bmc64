                                VICE
                                ~~~~

                     MS-DOS specific information

                       official VICE home page:
                   http://vice-emu.sourceforge.net/



IMPORTANT NOTICE
================

There is no "real" documentation for the MS-DOS version of VICE at the
moment.  Please refer to the Unix manual, available from the home
page, for the main concepts.  This file is only supposed to get you
started, and is incomplete.

If you feel like helping us to maintain the MS-DOS version of the
documentation, you are of course welcome.  Please contact the VICE
mailing list <vice-emu-mail@lists.sourceforge.net>.


OVERVIEW
========

VICE is a set of advanced emulators for the Commodore C64, C128, VIC20,
PET, PLUS4 and CBM-II.  It runs on Unix, MS-DOS, MS-Windows, OS/2 and
BeOS systems.

VICE is Free Software, released under the GNU General Public License,
and you must read the License before using this program.  The License
is contained in the `COPYING' file which should be included in the
distribution.  If it is not, please tell us.

For more information about the Free Software movement, please consult
the GNU home page at http://www.gnu.org, or the Open Source (tm) home
page at http://www.opensource.org.


SYSTEM REQUIREMENTS
===================

This has not been tested thoroughly, but the minimum requirements
should be:

 - a Pentium-class machine or better;

 - 8 Mbytes of RAM;

 - a register-compatible VGA graphics card (better if it is also SVGA
   and VESA-compliant);

 - DOS 3 or later (or Windows 95/98/ME).

If you want to run VICE under plain MS-DOS (and you should), also the
CWSDPMI DPMI server is needed.  It's available in several places on
the Internet, and is also used by many other 32-bit MS-DOS program.
For example, you can download it from the following site:

  ftp://ftp.simtel.net/pub/simtelnet/gnu/djgpp/v2misc/csdpmi3b.zip

Alternatively, you can search for `csdpmi3b.zip' on

  http://ftpsearch.unit.no/ftpsearch/

or other FTP search engines.

VICE also supports standard PC joysticks and SoundBlaster 1.0/Pro/16
compatible sound cards, although they are not mandatory.

  Important notice: due to license incompatibilities with the MIDAS
  audio system, binaries of the MS-DOS version of VICE do not use the
  MIDAS audio system anymore.  As a consequence, we are now using an
  Allegro-based driver that only supports SoundBlaster cards.

  If somebody persuades the MIDAS people to release their code under
  the GNU Library General Public License or under another license
  compatible with the GNU General Public License used by VICE, there
  are chances MIDAS support will be included again.


VICE also supports hardware based SID cards:

- CatWeasel MK3/MK4     : A PCI 2.0c+ compatible BIOS is needed.
- ISA HardSID (Quattro) : Nothing extra needed.
- PCI HardSID (Quattro) : A PCI 2.0c+ compatible BIOS is needed.
- SSI-2001              : Nothing extra needed.
- ParSID                : LPT1/2/3 port needed.


VICE has been tested and verified to run properly on the following DOS (based) systems:

- DRDOS 3.40
- DRDOS 3.41
- DRDOS 5.0
- DRDOS 6.0 (with memory manager disabled)
- DRDOS 7.02 (with memory manager disabled)
- DRDOS 7.03 (with memory manager disabled)
- DRDOS 8.0 (with memory manager disabled)
- FREEDOS 1.1 (with xmgr memory manager)
- MSDOS 4.01
- MSDOS 5.0 (with himem.sys disabled)
- MSDOS 6.0
- MSDOS 6.20
- MSDOS 6.21
- MSDOS 6.22
- Novell DOS 7 (with memory manager disabled)
- PCDOS 3.0
- PCDOS 3.10
- PCDOS 3.30
- PCDOS 4.00
- PCDOS 5.0 (with himem.sys disabled)
- PCDOS 5.02
- PCDOS 6.3
- PCDOS 7.0

INSTALLATION
============

There is no particular installation procedure for the MS-DOS version
of VICE.  Just unzip the archive by keeping the directory structure
intact (eg. you need to use `pkunzip -d' if you use PkZip).

If you have troubles getting sound to work, make sure that the
`BLASTER' environment variable is set properly (you should have
received information about this together with your soundcard).


HOW TO RUN THE EMULATORS
========================

There are several executables for the emulators:

      x64.exe, the C64 emulator
      x64dtv.exe, the C64DTV emulator
      x128.exe, the C128 emulator
      xvic.exe, the VIC20 emulator
      xpet.exe, the PET emulator
      xplus4.exe, the PLUS4 emulator
      xcbm2.exe, the CBM-II emulator (cbm6x0/cbm7x0)
      xcbm5x0.exe, the CBM5x0 emulator

To run them, just type the corresponding name at the DOS prompt.

`xpet.exe' actually emulates all the PET models (2001, 3008, 3016,
3032B, 4016, 4032, 4032B, 8032, 8096, 8296, SuperPET).  You can
select which machine you want by using the `-model' option.  So you
basically have to do:

      xpet -model 3032 for the PET 3032 emulator
      xpet -model 4032 for the PET 4032 emulator
      xpet -model 8032 for the PET 8032 emulator (80 column)

and so on.

As soon as the emulator is started, you should get the boot-up screen
of the emulated machine on your screen.


KEYBOARD LAYOUT
===============


C64/C128/VIC20 Keyboard layout
------------------------------

The keyboard layout is pretty much the same as the one of the real
thing.


       !   "   #   $   %   &   '   (   )               INST
   <-  1   2   3   4   5   6   7   8   9   0   +   -   DEL


   CTRL  Q   W   E   R   T   Y   U   I   O   P   @   *   =

   RUN                                         [   ]
   STOP    A   S   D   F   G   H   J   K   L   :   ;  RETURN

                                         <   >   ?
   SHIFT     Z   X   C   V   B   N   M   ,   .   /   SHIFT


   CBM                  SPACE


The other keys are mapped as follows:

    Delete	  Arrow Up/PI
    Insert	  Pound
    Home	  CLR/HOME
    Page Up	  RESTORE
    Arrow keys    CRSR

The additional C128 keys are mapped as follows:

    F1            ESC
    F2            TAB
    F3            ALT
    F5            HELP
    F6            LINE FEED
    F7            40/80 COLUMN
    F8            NO SCROLL
    F9            F1
    F10           F3
    F11           F5
    F12           F7

    End           CRSR Up/Down
    PgDown        CRSR Left/Right

    Numpad        Numpad
    Grey +        Numpad -
    Grey -        Numpad +
    Grey Enter    Numpad ENTER

Moreover, unlike the C64, the arrow keys emulate the "unshifted"
cursor keys in the upper part of the keyboard instead of the CRSR
ones.


PET keyboard layout
-------------------

The PET keyboard layout is a bit more complicate and varies according
to the model.


  PET "graphics" keyboard (40-column models)
  ..........................................


     <-  !   "   #   $   %   '   &   \   (   )   <-      DEL

           Q   W   E   R   T   Y   U   I   O   P  UpArrow

     STOP    A   S   D   F   G   H   J   K   L   :    RETURN

     SHIFT     Z   X   C   V   B   N   M   ,   ;   ?   SHIFT

     RVS                 SPACE                          STOP


  The other keys are mapped as follows:

      Home            [
      PgUp            ]
      End             <
      PgDown          >
      Ins             @

      Numpad          Numpad
      Grey *          *
      Grey /          /
      Grey +          +
      Grey -          -
      Grey Enter      =
      NumLock         CLR/HOME

      Arrow keys      CRSR


  PET "business" keyboard (80-column models)
  ..........................................


       !   "   #   $   %   &   '   (   )               INST
   <-  1   2   3   4   5   6   7   8   9   0   :   -   DEL


   TAB   Q   W   E   R   T   Y   U   I   O   P   [   ]   \

                                               :
   ESC     A   S   D   F   G   H   J   K   L   ;   @  RETURN

                                         <   >   ?
   SHIFT     Z   X   C   V   B   N   M   ,   .   /    SHIFT


   RVS                  SPACE                           RPT


  The other keys are mapped as follows:

      Home            CLR/HOME
      Ins             RUN/STOP
      Del             Up Arrow

      Numpad          Numpad

      Arrow Keys      CRSR


Special keys
------------

The following keys perform special actions on *all emulators*:

    Esc           Enter the emulation menu
    Alt-F1        Attach next disk image from flip list
    Alt-F2        Attach previous disk image from flip list
    Alt-F3        Add current disk image to flip list
    Alt-F4        Remove current disk image from flip list
    Alt-F5        Toggle the statusbar that displays drive + speed infos
    Alt-F12       Soft reset
    Alt-Ctrl-F4   Datasette Record Key
    Alt-Ctrl-F5   Datasette Play Key
    Alt-Ctrl-F6   Datasette Rewind Key
    Alt-Ctrl-F7   Datasette FF Key
    Alt-Ctrl-F8   Datasette Stop Key
    Alt-Ctrl-F12  Hard reset (wipes out RAM)
    ScrollLock    Toggle warp mode (it makes the emulator run as fast as
                  possible and turns sound off)
    Alt-Pause     Freeze (emulates the Action Replay freeze button
                  when an Action Replay image is loaded in the C64
                  emulator)


THE EMULATOR MENU
=================

You can access the emulator's main menu by pressing the `Esc' key.
You can then select a menu item with the arrow keys and activate it by
pressing Enter.  Moreover, every menu item has an highlighted
character which you can press to activate it; the items of the main
menu can also be activate by pressing Alt + that character from within
the emulator screen.

Many menu items open other submenus; you can leave the submenu by
pressing the `Esc' key.  To leave the menu system and return to the
emulation screen, you can either press `Esc' at the toplevel menu or
press `Tab' from the main menu or any submenu.  You can also navigate
through menu levels using `Left' and `Right' cursor keys.

Here is a short list of what the various items do.  (For more
information, refer to the Unix documentation.)


Attach images
-------------

This menu is used to "attach" a disk, tape or cartridge image for use.
"Attaching an image" means that the virtual disk, tape or cartridge is
inserted in the virtual disk drive, tape recorder or cartridge slot
and you can use it.

If you select any of the devices in the menu, a simple file selector
will be shown; press `SPACE' to preview the contents of the image or
`RETURN' to attach it.

In preview mode, you can browse the contents of the image and
automagically start one of the programs, by selecting it with the
arrow keys and pressing `RETURN'.  The contents are displayed in a
special C64-lookalike font, and you can switch between the uppercase
and lowercase character sets by pressing backspace.

When a program is autostarted, the 1541 automatically switches to the
fast emulation mode (see below) before loading the file, and the
user-specified emulation mode is then restored before the program is
executed via the "RUN" command.  This procedure does not work for all
the programs, though.  In such cases, you have to load manually
(e.g. `LOAD"NAME",8,1' or `LOAD"*",8,1' for disk images and
`LOAD"NAME"' or simply `LOAD' for tape images).


Detach image
------------

This menu is used to "detach" disk, tape or cartridge images.  This
means that the virtual disk, tape or cartridge is removed from the
virtual disk drive, tape recorder or cartridge slot.

Notice that, when a T64 tape image is attached, X64 emulates a pressed
PLAY button on the cassette recorder.  This should not do any harm in
most cases, but some programs do not like it; if you experience any
weird problems, try to detach the tape image after you have loaded it.


Joystick settings
-----------------

This allows you to select in which port the emulated joysticks should
be plugged in.

You can specify a different device for the two joystick ports (the
VIC20 has only one); available devices are the PC keyboard and the
PC's real joysticks.  The PC keyboard can also be configured for any
joystick layout you like.


Video settings
--------------

This is a sub-menu from which you can specify the desired refresh
rate, enable/disable sprite collisions (on the C64) and select the VGA
mode you want.  The following resolutions are supported:

     320x200 (linear)
     640x480 (linear, requires a VESA-compliant SVGA card)
     360x240, 360x270, 376x282, 400x300 (planar)

  *Warning*: the planar resolutions might not work on your equipment!
  Some monitors do not like to be fed with strange frequencies, and
  although we have never heard of any damages resulting from these
  resolutions, we cannot give any warranties about that.

The mode-X resolutions are _much_ slower than the linear ones, so the
performance will be worse with them.

The VGA mode in the PET emulator is hard-coded: you will get 320x200
for the 40-column models (2001, 3032, 4032) and 640x480 for the
80-column models (8032, 8096, 8296).

From this menu you can also enable or disable a video cache which
*might* make things faster when little graphics activity is going on.
This feature was originally intended for Unix system running their
window systems across networks; on MS-DOS, it usually does not help
much (if at all).  As it can even slow things down, you probably want
to keep it turned off.

Another important feature is "triple buffering".  With triple
buffering, animations will be much smoother, provided that your system
is fast enough to support them.  Triple buffering overrides the video
cache (which is always turned off), uses some additional processor
power and does not work at 640x480.  Moreover, it turns 320x200 into a
Mode-X planar resolution, and thus that mode will become as slow as
the other planar ones.

Important notice: triple buffering might not work correctly under
Windows; and even if it works, it might cause serious performance
loss.  You should definitely run the emulator under plain DOS if you
want to use this feature.


Drive settings
-----------------------------------

This sub-menu specifies the parameters for the hardware-level
emulation of the 1541 disk drive.  If the programs you are running do
not need accurate emulation of the disk drive, keep the "true" 1541
emulation turned off as it is much faster this way.  On the other
hand, if you run games that require turboloaders or other features
that are not supported by the fast disk-drive emulation you should
keep it turned on.

The 1541 "idle method" selects the way the 1541 CPU is emulated.  With
the "Skip Cycles" method, each time the serial line is accessed by the
computer the drive executes all the cycles since the last time it was
running; if the number of elapsed cycles is larger than a certain
value, the drive discards part of them.  With the "Trap Idle" method,
the disk drive is still emulated upon serial line accesses as with the
previous option, but it is also always emulated at least at the end of
each screen frame.  Moreover, if the drive gets into the DOS idle
loop, only pending interrupts are emulated to save time.

The first option ("Skip cycles") is usually better for
performance, as the drive is emulated as little as possible; on the
other hand, you can notice sudden slowdowns (when the drive
executes several cycles at once).  Moreover, if the drive tries to
get in sync with the computer in some weird way and the computer
does not access the serial line for a long time, it is possible
that some cycles are discarded and the sync is lost.  Notice that
this hack can have no effect on performance if one program
continuously reads the CIA ports, as the drive will have to be
emulated in any case.

The second option ("Trap idle") is usually a bit slower, as at least
interrupts are always emulated, but always keeps the 1541 and the
computer in sync.  On the other hand, if a program installs a
non-standard idle loop in the drive, the 1541 CPU has to be emulated
even when not necessary and the global emulation speed is then much
slower.

If the "Trap Idle" method is enabled, the Scroll Lock LED of the
keyboard will emulate the 1541 drive LED.

The "Enable parallel cable" option can be used to enable/disable the
emulation of a SpeedDOS-compatible parallel cable.


Sound settings
--------------

This menu is used to specify the parameters for the sound emulation.
Sound playback only works if you have specified a 100% speed limit,
and is turned off with an error message otherwise.  Moreover, it eats
some CPU speed.

The "sound buffer size" setting is used to specify the amount of data
the audio playback buffer can hold; data is actually played through
the speaker when the buffer is full.  Because of this, large values
will result in delayed sound output; slow machines have to use larger
values than fast ones, though, because this helps to avoid "clicking".

SID filters are a feature of the original audio chip of the C64; if
you turn them off, some sound effects will not be played correctly.
But on the other hand, keeping them enabled makes VICE a bit slower.


Other settings
--------------

These settings include:

- the 1750 512K RAM emulation;

- 1351 mouse emulation;

- IEEE488 interface emulation;

- custom emulation speed;

- "warp mode" (makes the emulator run as fast as possible);

- usage of the keyboard LEDs for the 1541 emulation and warp mode (try
  to turn it off if your machine hangs without any apparent reason);

- memory configuration (PET and VIC20 only);

On the PET emulator, this also lets you choose the model of PET being
emulated.  Notice that switching from one model to another causes the
emulated machine to reset itself.


Reset
-----

This is used to reset the emulated machine.  The are two types of
reset: the "hard" one which wipes out the RAM as if the machine had
been turned off and on, and the "soft" one which simply emulates a
standard reset button and preserves the memory contents.  The soft
reset can be performed from the emulation screen by pressing
`Alt-F12'; the hard one by pressing `Alt-Ctrl-F12'.  "Soft" reset is
sometimes not enough to bring the machine to the startup sequence.


Monitor
-------

Enters the built-in machine-language monitor.  Type `x' to go back to
the emulation.  A complete list of the commands is in the UNIX
documentation, available from the VICE home page.  Otherwise, you can
use the `help' command.

The monitor is still under construction, so use with care.


VICE info
---------

Displays some information about the authors, the copyright and the GNU
General Public License.


Quit
----

Exits the emulator.


USING GZIP-COMPRESSED FILES
===========================

The VICE emulators are able to handle files compressed with GNU zip
automagically, without the need of unzipping them first; this can be
used for both D64 and T64 images.  In order to allow this, the file's
extension must end with `Z': for instance, compressed D64 files must
have a `D6Z' extension while compressed T64 files must have a `T6Z'
extension.

A binary of GNU zip is available through the DJGPP distribution, so
you can find it in any of the DJGPP mirror sites.  For example, you
can find it at the following places:

  ftp://ftp.simtel.net/pub/simtelnet/gnu/djgpp/
  ftp://x2ftp.oulu.fi/pub/msdos/programming/djgpp/v2/

Of course, GNU Zip is free software released under the GNU General
Public License (just like VICE is), and its use is highly recommended
as T64 and D64 usually waste much of space and can be highly
compressed.

One drawback is that when an image is opened in read/write mode, VICE
has to re-compress it when the image is detached to update the original
in case something has changed.  This makes using of GNU zipped images
somewhat more harmful (and slower) than using plain uncompressed ones,
as it relies on the zipping process to always work correctly.  If you
are concerned about destroying important images, you should protect
them from being written to with the DOS command `ATTRIB':

  attrib +r diskimage.d6z

This is a good idea even if you are not using GNU zipped images, as it
prevents emulated programs from writing to the disk.  (Of course, some
programs explicitly require the disk not be write-protected instead.)


CONTACTS
========

You can always find the latest news about VICE at the official VICE
home page:

  http://vice-emu.sourceforge.net/

Also the public patches and beta releases will be available there, so
you might want to have a look at it periodically.  If you are going to
report a bug, please check those pages first; it is possible that the
problem you encountered has already been fixed in a more recent
version.  Also notice that, since VICE is released under the GNU
General Public License, the sources are available from the home page
too.

If you want to report bugs, make suggestions or contribute to the
project, write to the VICE developer's mailing list:

    vice-emu-mail@lists.sourceforge.net


COPYRIGHT
=========

  Copyright C 1998-2007 Andreas Boose
  Copyright C 1998-2007 Dag Lem
  Copyright C 1998-2007 Tibor Biczo
  Copyright C 1999-2007 Andreas Dehmel
  Copyright C 1999-2007 Andreas Matthies
  Copyright C 1999-2007 Martin Pottendorfer
  Copyright C 2000-2007 Spiro Trikaliotis
  Copyright C 2005-2007 Marco van den Heuvel
  Copyright C 1999-2005 Thomas Bretz
  Copyright C 2003-2005 David Hansel
  Copyright C 2000-2004 Markus Brenner

  Copyright C 1997-2001 Daniel Sladic
  Copyright C 1996-1999 Ettore Perazzoli
  Copyright C 1996-1999 André Fachat
  Copyright C 1993-1994, 1997-1999 Teemu Rantanen
  Copyright C 1993-1996 Jouko Valta
  Copyright C 1993-1994 Jarkko Sonninen

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307  USA

The MS-DOS version of VICE was made possible by a great library:

  - Allegro, a Game Programming Library by Shawn Hargreaves
    (http://www.talula.demon.co.uk/allegro).

There have also been contributions from several people around the
world: look at the Unix documentation or at the "VICE Info" command
for a complete list.

