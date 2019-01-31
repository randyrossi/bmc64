c128dir = $(VICEDIR)/C128

roms = \
	basic64 \
	basichi \
	basiclo \
	chargde \
	chargen \
	chargfr \
	chargse \
	chargch \
	kernal \
	kernal64 \
	kernalde \
	kernalfi \
	kernalfr \
	kernalit \
	kernalno \
	kernalse \
	kernalch

romsets = \
	default.vrs

palettes = \
	c64hq.vpl \
	c64s.vpl \
	ccs64.vpl \
	frodo.vpl \
	godot.vpl \
	pc64.vpl \
	vdc_deft.vpl \
	vdc_comp.vpl \
	vice.vpl \
	pepto-ntsc-sony.vpl \
	pepto-ntsc.vpl \
	pepto-pal.vpl \
	pepto-palold.vpl \
	community-colors.vpl \
	colodore.vpl \
	ptoing.vpl \
	deekay.vpl \
	rgb.vpl

x11_keymaps = \
	x11_sym.vkm \
	x11_pos.vkm \
	\
	gtk3_sym.vkm \
	gtk3_pos.vkm

sdl_keymaps = \
	sdl_hotkeys.vkm \
	sdl_pos.vkm

extra_keymaps = \
	amiga_sym.vkm \
	amiga_pos.vkm \
	beos_sym.vkm \
	beos_pos.vkm \
	dos_sym.vkm \
	dos_pos.vkm \
	osx_pos.vkm \
	osx_sym.vkm \
	win_sym.vkm \
	win_pos.vkm

win32_files = \
	win_hotkeys.vhk \
	win_shortcuts.vsc

if SDL_COMPILE
c128_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps)

EXTRA_DIST = $(c128_DATA) $(extra_keymaps) $(x11_keymaps) $(win32_files)
else
c128_DATA = $(roms) $(romsets) $(palettes) $(x11_keymaps)

EXTRA_DIST = $(c128_DATA) $(extra_keymaps) $(sdl_keymaps) $(win32_files)
endif
