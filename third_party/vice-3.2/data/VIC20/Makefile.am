vic20dir = $(VICEDIR)/VIC20

roms = \
	basic \
	chargen \
	kernal

romsets = \
	default.vrs

palettes = \
	vice.vpl \
	mike-pal.vpl \
	mike-ntsc.vpl \
	colodore_vic.vpl

x11_keymaps = \
	x11_pos.vkm \
	x11_sym.vkm \
	\
	gtk3_pos.vkm \
	gtk3_sym.vkm

sdl_keymaps = \
	sdl_hotkeys.vkm \
	sdl_pos.vkm \
	sdl_pos_fi.vkm

extra_keymaps = \
	amiga_sym.vkm \
	amiga_pos.vkm \
	beos_sym.vkm \
	beos_pos.vkm \
	dos_pos.vkm \
	dos_sym.vkm \
	osx_pos.vkm \
	osx_sym.vkm \
	win_sym.vkm \
	win_sym_US.vkm \
	win_pos.vkm

win32_files = \
	win_hotkeys.vhk \
	win_shortcuts.vsc

if SDL_COMPILE
vic20_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps)

EXTRA_DIST = $(vic20_DATA) $(extra_keymaps) $(x11_keymaps) $(win32_files)
else
vic20_DATA = $(roms) $(romsets) $(palettes) $(x11_keymaps)

EXTRA_DIST = $(vic20_DATA) $(extra_keymaps) $(sdl_keymaps) $(win32_files)
endif
