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
	rgb.vpl

x11_keymaps = \
	x11_sym.vkm \
	x11_pos.vkm \
	x11_pos_de.vkm \
	x11_sym_de.vkm \
	x11_keyrah.vkm \
	x11_keyrah_de.vkm \
	\
	gtk3_sym.vkm \
	gtk3_pos.vkm \
	gtk3_pos_de.vkm \
	gtk3_sym_de.vkm \
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
	sdl_sym_de.vkm

extra_keymaps = \
	amiga_sym.vkm \
	amiga_pos.vkm \
	beos_sym.vkm \
	beos_pos.vkm \
	dos_sym.vkm \
	dos_pos.vkm \
	osx_pos.vkm \
	osx_sym_de.vkm \
	osx_sym.vkm \
	win_sym_de.vkm \
	win_sym_it.vkm \
	win_sym.vkm \
	win_pos.vkm \
	win_pos_de.vkm

win32_files = \
	win_hotkeys.vhk \
	win_shortcuts.vsc

sym_file = \
	c64mem.sym

if SDL_COMPILE
c64_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps) $(sym_file)

EXTRA_DIST = $(c64_DATA) $(extra_keymaps) $(x11_keymaps) $(win32_files)
else
c64_DATA = $(roms) $(romsets) $(palettes) $(x11_keymaps) $(sym_file)

EXTRA_DIST = $(c64_DATA) $(extra_keymaps) $(sdl_keymaps) $(win32_files)
endif
