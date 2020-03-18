#
# Makefile for a machine kernel image
#

CIRCLEHOME = third_party/circle-stdlib/libs/circle
NEWLIBDIR = third_party/circle-stdlib/install/arm-none-circle

OBJS	= main.o kernel.o vicesound.o vicesoundbasedevice.o \
          viceoptions.o viceapp.o fbl.o crt_pi_idx.o crt_pi_rgb.o

ifeq ($(MACHINE_CLASS),RASPI_PLUS4EMU)
OBJS	+= plus4emulatorcore.o
else
OBJS	+= viceemulatorcore.o
endif

include $(CIRCLEHOME)/Rules.mk

ifeq ($(MACHINE_CLASS),RASPI_PLUS4EMU)
CFLAGS  += -I "third_party/plus4emu/src"
endif

CFLAGS += -I "$(NEWLIBDIR)/include" -I $(STDDEF_INCPATH) \
          -I third_party/circle-stdlib/include \
          -I third_party/vice-3.3/src \
          -I $(CIRCLEHOME)/addon/fatfs \
          -D $(MACHINE_CLASS)

LIBS := $(VICELIBS) \
        third_party/common/libbmc64common.a \
        $(NEWLIBDIR)/lib/libm.a \
	$(NEWLIBDIR)/lib/libc.a \
	$(NEWLIBDIR)/lib/libcirclenewlib.a \
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
  	$(CIRCLEHOME)/lib/sched/libsched.a \
  	$(CIRCLEHOME)/lib/libcircle.a

