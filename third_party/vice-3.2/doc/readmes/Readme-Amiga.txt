                ._______ _______ __________    .________________
       ________ |       (_______)      ___/____|_     __       /spot
       \       \|       |       |      |/       /     |/      /_ 
.-------\_______________|_______|______________/_______________/-------.
|                                                                      |
| V E R S A T I L E   C O M M O D O R E   8 - B I T   E M U L A T O R  |
|                                                                      |
`----------------------------------------------------------------------'


This is a port of the VICE emulator package to the following AmigaOS flavors:

- AmigaOS4
  Using the AHI sound system, the P96 graphics system, the AmigaInput input
  system and the MUI GUI system.

- Classic AmigaOS
  Using the AHI sound system, the CGX or P96 graphics system, the LowLevel
  input system and the MUI GUI system.

- MorphOS
  Using the AHI or SDL sound system, the CGX graphics system, the LowLevel
  input system and the MUI GUI system.

- AROS
  Using the AHI or SDL sound system, the CGX graphics system, the LowLevel
  input system and the MUI GUI system.

These ports are still missing some dialogs and missing some features, 
however, we are working on implementing them.

Building the binaries using a cross-compiler :

- AmigaOS3:   configure with --host=m68k-amigaos
- AmigaOS4:   configure with --host=ppc-amigaos
- MorphOS:    configure with --host=ppc-morphos
- Intel AROS: configure with --host=i686-aros
- PPC AROS:   configure with --host=powerpc-aros
- AROS64:     configure with --host=x86_64-aros

After the configure script is done:

make
make bindist

After this a binary distribution directory will have been generated, and 
you can use this to transfer to the real machine and use it. If you want 
to generate a distribution archive instead of the directory you can use

make bindistzip

instead of make bindist.

Any comments or questions related to the Amiga ports can be sent to the 
VICE mailing-list at vice-emu-mail@lists.sourceforge.net

The ports are done by Mathias "AmiDog" Roslund and Marco van den Heuvel.
