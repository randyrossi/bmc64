c64dir = $(VICEDIR)/C64

roms = \
	basic \
	chargen \
	jpchrgen \
	kernal \
	sxkernal \
	edkernal \
	gskernal \
	jpkernal

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
	gtk3_sym_nl.vkm \
	gtk3_keyrah.vkm \
	gtk3_keyrah_de.vkm

sdl_keymaps = \
	sdl_joymap_ps3.vjm \
	sdl_hotkeys.vkm \
	sdl_hotkeys_vsid.vkm \
	sdl_pos.vkm \
	sdl_pos_da.vkm \
	sdl_pos_de.vkm \
	sdl_pos_fi.vkm \
	sdl_pos_no.vkm \
	sdl_keyrah.vkm \
	sdl_keyrah_de.vkm \
	sdl_keyrah_retropie.vkm \
	sdl_keyrah_retropie_de.vkm \
	sdl_keyrah_combian.vkm \
	sdl_sym.vkm \
	sdl_sym_de.vkm \
	sdl_sym_nl.vkm

sym_file = \
	c64mem.sym

if SDL_COMPILE
c64_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps) $(sym_file)

EXTRA_DIST = $(c64_DATA) $(gtk3_keymaps)
else
c64_DATA = $(roms) $(romsets) $(palettes) $(gtk3_keymaps) $(sym_file)

EXTRA_DIST = $(c64_DATA) $(sdl_keymaps)
endif
