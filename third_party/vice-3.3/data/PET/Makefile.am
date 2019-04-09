petdir = $(VICEDIR)/PET

roms = \
	basic1 \
	basic2 \
	basic4 \
	kernal1 \
	kernal2 \
	kernal4 \
	edit1g \
	edit2g \
	edit2b \
	edit4g40 \
	edit4b40 \
	edit4b80 \
	chargen \
	chargen.de \
	characters.901640-01.bin \
	waterloo-a000.901898-01.bin \
	waterloo-b000.901898-02.bin \
	waterloo-c000.901898-03.bin \
	waterloo-d000.901898-04.bin \
	waterloo-e000.901897-01.bin \
	waterloo-f000.901898-05.bin \
	hre-9000.324992-02.bin \
	hre-a000.324993-02.bin

romsets = \
	rom1g.vrs \
	rom2b.vrs \
	rom2g.vrs \
	rom4b40.vrs \
	rom4b80.vrs \
	rom4g40.vrs \
	romsuperpet.vrs \
	hre.vrs

palettes = \
	amber.vpl \
	green.vpl \
	white.vpl

gtk3_keymaps = \
	gtk3_sym.vkm \
	gtk3_bude_pos.vkm \
	gtk3_bude_pos_de.vkm \
	gtk3_bude_sym.vkm \
	gtk3_bude_sym_de.vkm \
	gtk3_grus_pos.vkm \
	gtk3_grus_pos_de.vkm \
	gtk3_grus_sym.vkm \
	gtk3_grus_sym_de.vkm \
	gtk3_buuk_pos.vkm \
	gtk3_buuk_pos_de.vkm \
	gtk3_buuk_sym.vkm \
	gtk3_buuk_sym_de.vkm

sdl_keymaps = \
	sdl_hotkeys.vkm \
	sdl_sym.vkm \
	sdl_buuk_sym.vkm \
	sdl_grus_sym.vkm

if SDL_COMPILE
pet_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps)

EXTRA_DIST = $(pet_DATA) $(gtk3_keymaps)
else
pet_DATA = $(roms) $(romsets) $(palettes) $(gtk3_keymaps)

EXTRA_DIST = $(pet_DATA) $(sdl_keymaps)
endif
