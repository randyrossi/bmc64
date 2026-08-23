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
	      -I$(CIRCLEHOME)/addon/fatfs

ifeq ($(MACHINE_CLASS),RASPI_PLUS4EMU)
	APP_INCLUDES += -I "third_party/plus4emu/src"
endif

EXTRAINCLUDE += $(APP_INCLUDES)

OBJS	= src/main.o src/kernel.o src/new_io.o src/vicesound.o src/vicesoundbasedevice.o src/bmcmodem.o \
		  src/viceoptions.o src/viceapp.o src/vice_network.o src/network_time_sync.o src/fbl.o src/crt_pi_idx.o src/crt_pi_rgb.o

ifeq ($(MACHINE_CLASS),RASPI_PLUS4EMU)
OBJS	+= src/plus4emulatorcore.o
else
OBJS	+= src/viceemulatorcore.o
endif

include $(CIRCLEHOME)/Rules.mk

CFLAGS += $(APP_INCLUDES) -D $(MACHINE_CLASS)
CPPFLAGS += $(APP_INCLUDES) -D $(MACHINE_CLASS) -fno-exceptions -fno-rtti

FILTERED_CIRCLE_NEWLIB = libcirclenewlib-bmc64.a

$(FILTERED_CIRCLE_NEWLIB): $(NEWLIBDIR)/lib/libcirclenewlib.a
	@cp $< $@
	@$(AR) d $@ io.o

EXTRACLEAN += $(FILTERED_CIRCLE_NEWLIB)
EXTRACLEAN += $(OBJS) $(DEPS)

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
