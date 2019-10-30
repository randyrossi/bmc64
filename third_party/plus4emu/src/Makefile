include ../../circle-stdlib/Config.mk
CIRCLE_STDLIB_HOME = ../../circle-stdlib
include ../Config.mk

TOP=..

OBJ = acia6551.o bplist.o cfg_db.o charconv.o cia8520.o compress.o comprlib.o cpu.o cpuoptbl.o d64image.o decompm2.o disasm.o display.o emucfg.o fileio.o iecdrive.o joystick.o memory.o mps801.o plus4vm.o pngwrite.o render.o riot6532.o script.o snd_conv.o soundio.o system.o tape.o ted_api.o ted_init.o ted_main.o ted_read.o ted_snd.o ted_write.o vc1526.o vc1541.o vc1551.o vc1581.o via6522.o videorec.o vm.o wd177x.o dotconf.o sndfile.o fileno_wrapper.o

UNSUPPORTED = fldisp.o gldisp.o guicolor.o sndio_pa.o

INCLUDES = -I $(CIRCLE_STDLIB_HOME)/install/arm-none-circle/include -I $(CIRCLE_STDLIB_HOME)/libs/circle/addon/fatfs -I $(TOP)/src -I $(TOP)

all: libplus4.a

%.o : %.cpp
	$(CXX_FOR_TARGET) $(INCLUDES) -c -O2 -o $@ $< $(CFLAGS_FOR_TARGET) $(CPPFLAGS_FOR_TARGET)

%.o : %.c
	$(CC_FOR_TARGET) $(INCLUDES) -c -O2 -o $@ $< $(CFLAGS_FOR_TARGET)

libplus4.a: $(OBJ)
	$(AR_FOR_TARGET) cr libplus4.a $(OBJ)
	$(RANLIB_FOR_TARGET) libplus4.a

clean:
	rm -rf *.o *.a
