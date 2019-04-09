cbm2dir = $(VICEDIR)/CBM-II

roms = \
	basic.128 \
	basic.256 \
	basic.500 \
	chargen.500 \
	chargen.600 \
	chargen.700 \
	kernal \
	kernal.500

romsets = \
	rom500.vrs \
	rom128l.vrs \
	rom128h.vrs \
	rom256l.vrs \
	rom256h.vrs

palettes = \
	c64hq.vpl \
	c64s.vpl \
	ccs64.vpl \
	frodo.vpl \
	godot.vpl \
	pc64.vpl \
	pepto-ntsc-sony.vpl \
	pepto-ntsc.vpl \
	pepto-pal.vpl \
	pepto-palold.vpl \
	community-colors.vpl \
	colodore.vpl \
	ptoing.vpl \
	deekay.vpl \
	cjam.vpl \
	rgb.vpl \
	vice.vpl \
	amber.vpl \
	green.vpl \
	white.vpl

gtk3_keymaps = \
	gtk3_sym.vkm \
	gtk3_sym_de.vkm

sdl_keymaps = \
	sdl_hotkeys.vkm \
	sdl_sym.vkm

if SDL_COMPILE
cbm2_DATA = $(roms) $(romsets) $(palettes) $(sdl_keymaps)

EXTRA_DIST = $(cbm2_DATA) $(gtk3_keymaps)
else
cbm2_DATA = $(roms) $(romsets) $(palettes) $(gtk3_keymaps)

EXTRA_DIST = $(cbm2_DATA) $(sdl_keymaps)
endif
