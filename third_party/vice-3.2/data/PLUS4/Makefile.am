plus4dir = $(VICEDIR)/PLUS4

roms = \
	3plus1hi \
	3plus1lo \
	basic \
	kernal \
	kernal.005 \
	kernal.232 \
	kernal.364 \
	c2lo.364

romsets = \
	default.vrs

palettes = \
	yape-pal.vpl \
	yape-ntsc.vpl \
	colodore_ted.vpl

x11_keymaps = \
	x11_pos.vkm \
	x11_sym.vkm \
	x11_sym_de.vkm \
	\
	gtk3_pos.vkm \
	gtk3_sym.vkm \
	gtk3_sym_de.vkm

sdl_keymaps = \
	sdl_hotkeys.vkm \
	sdl_pos.vkm

extra_keymaps = \
	amiga_sym.vkm \
	amiga_pos.vkm \
	beos_sym.vkm \
	beos_pos.vkm \
	dos_pos.vkm \
	dos_sym.vkm \
	osx_sym.vkm \
	osx_pos.vkm \
	win_sym.vkm \
	win_pos.vkm

win32_files = \
	win_hotkeys.vhk \
	win_shortcuts.vsc

if SDL_COMPILE
plus4_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps)

EXTRA_DIST = $(plus4_DATA) $(extra_keymaps) $(x11_keymaps) $(win32_files)
else
plus4_DATA = $(roms) $(romsets) $(palettes) $(x11_keymaps)

EXTRA_DIST = $(plus4_DATA) $(extra_keymaps) $(sdl_keymaps) $(win32_files)
endif
