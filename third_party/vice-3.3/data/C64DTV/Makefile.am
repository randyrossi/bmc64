c64dtvdir = $(VICEDIR)/C64DTV

roms = \
	basic \
	chargen \
	kernal \
	dtvrom.bin

romsets = \
	default.vrs

palettes = \
	spiff.vpl

gtk3_keymaps = \
	gtk3_keyrah.vkm \
	gtk3_keyrah_de.vkm \
	gtk3_pos.vkm \
	gtk3_pos_de.vkm \
	gtk3_sym.vkm \
	gtk3_sym_de.vkm \
	gtk3_sym_nl.vkm

sdl_keymaps = \
	sdl_hotkeys.vkm \
	sdl_keyrah.vkm \
	sdl_keyrah_de.vkm \
	sdl_keyrah_retropie.vkm \
	sdl_keyrah_retropie_de.vkm \
	sdl_keyrah_combian.vkm \
	sdl_pos.vkm \
	sdl_pos_da.vkm \
	sdl_pos_de.vkm \
	sdl_pos_fi.vkm \
	sdl_pos_no.vkm \
	sdl_sym.vkm \
	sdl_sym_de.vkm \
	sdl_sym_nl.vkm

sym_file = \
	c64mem.sym

if SDL_COMPILE
c64dtv_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps) $(sym_file)

EXTRA_DIST = $(c64dtv_DATA) $(gtk3_keymaps)
else
c64dtv_DATA = $(roms) $(romsets) $(palettes) $(gtk3_keymaps) $(sym_file)

EXTRA_DIST = $(c64dtv_DATA) $(sdl_keymaps)
endif
