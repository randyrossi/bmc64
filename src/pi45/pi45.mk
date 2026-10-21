#
# pi45.mk: the Raspberry Pi 4 and 5 builds (64-bit), included from Makefile.
# The VideoCore's services (dispmanx, OpenGL ES and VCHIQ sound) aren't
# there: FrameBufferLayer and ViceSound have their own versions on BMC64's
# display, GPU and sound drivers (src/av/, src/kms/, src/v3d/), and
# src/pi45/compat stands in for Circle's VCHIQ header.
#
# This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
# See LICENSE for licensing details.
#

include $(CIRCLEHOME)/Config.mk

ifneq ($(filter 4 5,$(RASPPI)),)

NEWLIBDIR = third_party/circle-stdlib/install/aarch64-none-circle

# Found before Circle's own include paths (Rules.mk adds them after).
INCLUDE := -I src/pi45/compat $(INCLUDE)

OBJS := $(filter-out src/fbl.o src/crt_pi_idx.o src/crt_pi_rgb.o src/vicesound.o \
                     src/vicesoundbasedevice.o,$(OBJS))
OBJS += src/pi45/board.o \
        src/av/fbl.o src/av/screen.o src/av/vicesound.o src/av/sound.o \
        src/av/hdmisound.o \
        src/kms/common/filter.o src/kms/common/modes.o \
        src/v3d/gpu.o src/v3d/draw.o src/v3d/crt.o src/v3d/shaders.o

# The direct display driver: the Pi 4's (src/kms/pi4/) or the Pi 5's
# (src/kms/pi5/), with shared filter kernels and mode calculations.
ifeq ($(RASPPI),4)
OBJS += src/kms/pi4/dlist.o src/kms/pi4/hvs.o src/kms/pi4/phy.o \
        src/kms/pi4/modeset.o
else
OBJS += src/kms/pi5/dlist.o src/kms/pi5/hvs.o src/kms/pi5/phy.o \
        src/kms/pi5/modeset.o
endif

EXTRACLEAN += src/pi45/*.o src/pi45/*.d src/av/*.o src/av/*.d \
              src/kms/common/*.o src/kms/common/*.d \
              src/kms/pi4/*.o src/kms/pi4/*.d \
              src/kms/pi5/*.o src/kms/pi5/*.d src/v3d/*.o src/v3d/*.d

endif
