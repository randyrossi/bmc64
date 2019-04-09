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
	chargno \
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
	cjam.vpl \
	rgb.vpl

gtk3_keymaps = \
	gtk3_sym.vkm \
	gtk3_pos.vkm

sdl_keymaps = \
	sdl_hotkeys.vkm \
	sdl_pos.vkm

if SDL_COMPILE
c128_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps)

EXTRA_DIST = $(c128_DATA) $(gtk3_keymaps)
else
c128_DATA = $(roms) $(romsets) $(palettes) $(gtk3_keymaps)

EXTRA_DIST = $(c128_DATA) $(sdl_keymaps)
endif
