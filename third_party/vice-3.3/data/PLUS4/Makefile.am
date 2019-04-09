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

gtk3_keymaps = \
	gtk3_pos.vkm \
	gtk3_sym.vkm \
	gtk3_sym_de.vkm

sdl_keymaps = \
	sdl_hotkeys.vkm \
	sdl_pos.vkm

if SDL_COMPILE
plus4_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps)

EXTRA_DIST = $(plus4_DATA) $(gtk3_keymaps)
else
plus4_DATA = $(roms) $(romsets) $(palettes) $(gtk3_keymaps)

EXTRA_DIST = $(plus4_DATA) $(sdl_keymaps)
endif
