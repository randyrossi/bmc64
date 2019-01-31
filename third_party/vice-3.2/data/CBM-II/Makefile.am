cbm2dir = $(VICEDIR)/CBM-II

roms = \
	basic.128 \
	basic.256 \
	basic.500 \
	chargen.500 \
	chargen.600 \
	chargen.700 \
	kernal \
	kernal.500

romsets = \
	rom500.vrs \
	rom128l.vrs \
	rom128h.vrs \
	rom256l.vrs \
	rom256h.vrs

palettes = \
	c64hq.vpl \
	c64s.vpl \
	ccs64.vpl \
	frodo.vpl \
	godot.vpl \
	pc64.vpl \
	pepto-ntsc-sony.vpl \
	pepto-ntsc.vpl \
	pepto-pal.vpl \
	pepto-palold.vpl \
	community-colors.vpl \
	colodore.vpl \
	ptoing.vpl \
	deekay.vpl \
	rgb.vpl \
	vice.vpl \
	amber.vpl \
	green.vpl \
	white.vpl

x11_keymaps = \
	x11_sym.vkm \
	x11_sym_de.vkm \
	\
	gtk3_sym.vkm \
	gtk3_sym_de.vkm

sdl_keymaps = \
	sdl_hotkeys.vkm \
	sdl_sym.vkm

extra_keymaps = \
	amiga_pos.vkm \
	amiga_sym.vkm \
	beos_bukp.vkm \
	beos_buks.vkm \
	dos_bukp.vkm \
	dos_buks.vkm \
	osx_pos_uk.vkm \
	osx_sym_uk.vkm \
	win_sym.vkm \
	win_pos_uk.vkm \
	win_sym_uk.vkm

win32_files = \
	win_hotkeys.vhk \
	win_shortcuts.vsc

if SDL_COMPILE
cbm2_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps)

EXTRA_DIST = $(cbm2_DATA) $(extra_keymaps) $(x11_keymaps) $(win32_files)
else
cbm2_DATA = $(roms) $(romsets) $(palettes) $(x11_keymaps)

EXTRA_DIST = $(cbm2_DATA) $(extra_keymaps) $(sdl_keymaps) $(win32_files)
endif
