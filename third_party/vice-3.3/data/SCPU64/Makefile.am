scpu64dir = $(VICEDIR)/SCPU64

roms = \
	scpu64 \
	chargen \
	jpchrgen

romsets = \
	default.vrs

palettes = \
	c64hq.vpl \
	c64s.vpl \
	ccs64.vpl \
	frodo.vpl \
	godot.vpl \
	pc64.vpl \
	vice.vpl \
	pepto-ntsc-sony.vpl \
	pepto-ntsc.vpl \
	pepto-pal.vpl \
	pepto-palold.vpl \
	community-colors.vpl \
	colodore.vpl \
	ptoing.vpl \
	deekay.vpl \
	cjam.vpl \
	rgb.vpl

gtk3_keymaps = \
	gtk3_sym.vkm \
	gtk3_pos.vkm \
	gtk3_pos_de.vkm \
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
	scpu64mem.sym

if SDL_COMPILE
scpu64_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps) $(sym_file)

EXTRA_DIST = $(scpu64_DATA) $(gtk3_keymaps)
else
scpu64_DATA = $(roms) $(romsets) $(palettes) $(gtk3_keymaps) $(sym_file)

EXTRA_DIST = $(scpu64_DATA) $(sdl_keymaps)
endif
