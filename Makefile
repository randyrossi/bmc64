#
# Makefile for a machine kernel image
#

CIRCLEHOME = third_party/circle-stdlib/libs/circle
NEWLIBDIR = third_party/circle-stdlib/install/arm-none-circle
VPATH = src

APP_INCLUDES = -I"$(NEWLIBDIR)/include" -I$(STDDEF_INCPATH) \
	      -Ithird_party/circle-stdlib/include \
	      -I$(CIRCLEHOME)/include \
	      -I$(CIRCLEHOME)/addon \
	      -Ithird_party/vice-3.3/src \
	      -Ithird_party/common \
	      -I$(CIRCLEHOME)/addon/fatfs

ifeq ($(MACHINE_CLASS),RASPI_PLUS4EMU)
	APP_INCLUDES += -I "third_party/plus4emu/src"
endif

EXTRAINCLUDE += $(APP_INCLUDES)

OBJS	= src/main.o src/kernel.o src/new_io.o src/io_stats_bench.o src/perf_stats_env.o src/vicesound.o src/vicesoundbasedevice.o src/bmcmodem.o \
		  src/viceoptions.o src/viceapp.o src/vice_network.o src/network_time_sync.o src/fbl.o src/crt_pi_idx.o src/crt_pi_rgb.o \
		  src/webui/webui.o src/webui/webui_http.o src/webui/webui_fs.o src/webui/webui_assets.o

ifeq ($(MACHINE_CLASS),RASPI_PLUS4EMU)
OBJS	+= src/plus4emulatorcore.o
else
OBJS	+= src/viceemulatorcore.o
endif

include $(CIRCLEHOME)/Rules.mk

CFLAGS += $(APP_INCLUDES) -D $(MACHINE_CLASS)
CPPFLAGS += $(APP_INCLUDES) -D $(MACHINE_CLASS) -fno-exceptions -fno-rtti

# Opt-in storage I/O instrumentation.
ifeq ($(BMC64_IO_STATS),1)
CFLAGS += -DBMC64_IO_STATS
CPPFLAGS += -DBMC64_IO_STATS
endif

# Opt-in frame/audio performance instrumentation.
ifeq ($(BMC64_PERF_STATS),1)
CFLAGS += -DBMC64_PERF_STATS
CPPFLAGS += -DBMC64_PERF_STATS
endif

# The web UI's static assets are embedded in the image via a generated C
# source. Regenerate it from src/webui/assets/ whenever an asset or the
# generator changes. The generated file is committed, so builds without a
# Python interpreter fall back to that copy.
PYTHON ?= $(shell command -v python3 2>/dev/null || command -v python 2>/dev/null)
WEBUI_ASSET_SRCS = $(wildcard src/webui/assets/*)

src/webui/webui_assets.c: $(WEBUI_ASSET_SRCS) tools/gen_webui_assets.py
ifeq ($(PYTHON),)
	@echo "  WARN  no python interpreter found; using committed $@"
	@touch $@
else
	@echo "  GEN   $@"
	@$(PYTHON) tools/gen_webui_assets.py
endif

FILTERED_CIRCLE_NEWLIB = libcirclenewlib-bmc64.a

$(FILTERED_CIRCLE_NEWLIB): $(NEWLIBDIR)/lib/libcirclenewlib.a
	@cp $< $@
	@$(AR) d $@ io.o

EXTRACLEAN += $(FILTERED_CIRCLE_NEWLIB)
EXTRACLEAN += $(OBJS) $(DEPS)
# make_machines.sh invokes an unqualified "make clean", for which
# MACHINE_CLASS is unset. Remove both mutually exclusive core objects so a
# previous machine build cannot link against a differently configured Circle.
EXTRACLEAN += src/plus4emulatorcore.o src/viceemulatorcore.o

$(TARGET).img: $(FILTERED_CIRCLE_NEWLIB)

LIBS := $(VICELIBS) \
        third_party/common/libbmc64common.a \
        $(NEWLIBDIR)/lib/libm.a \
	$(NEWLIBDIR)/lib/libc.a \
	$(FILTERED_CIRCLE_NEWLIB) \
 	$(CIRCLEHOME)/addon/SDCard/libsdcard.a \
  	$(CIRCLEHOME)/lib/usb/libusb.a \
 	$(CIRCLEHOME)/lib/input/libinput.a \
 	$(CIRCLEHOME)/lib/fs/libfs.a \
  	$(CIRCLEHOME)/lib/net/libnet.a \
  	$(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
	$(CIRCLEHOME)/addon/vc4/interface/bcm_host/libbcm_host.a \
	$(CIRCLEHOME)/addon/vc4/interface/khronos/libkhrn_client.a \
	$(CIRCLEHOME)/addon/vc4/interface/vcos/libvcos.a \
	$(CIRCLEHOME)/addon/vc4/interface/vmcs_host/libvmcs_host.a \
  	$(CIRCLEHOME)/addon/linux/liblinuxemu.a \
	$(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	$(CIRCLEHOME)/addon/wlan/hostap/wpa_supplicant/libwpa_supplicant.a \
	$(CIRCLEHOME)/addon/wlan/libwlan.a \
	$(CIRCLEHOME)/lib/sound/libsound.a \
  	$(CIRCLEHOME)/lib/sched/libsched.a \
  	$(CIRCLEHOME)/lib/libcircle.a
