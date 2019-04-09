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

gtk3_keymaps = \
	gtk3_pos.vkm \
	gtk3_sym.vkm

sdl_keymaps = \
	sdl_hotkeys.vkm \
	sdl_pos.vkm \
	sdl_pos_fi.vkm

if SDL_COMPILE
vic20_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps)

EXTRA_DIST = $(vic20_DATA) $(gtk3_keymaps)
else
vic20_DATA = $(roms) $(romsets) $(palettes) $(gtk3_keymaps)

EXTRA_DIST = $(vic20_DATA) $(sdl_keymaps)
endif
