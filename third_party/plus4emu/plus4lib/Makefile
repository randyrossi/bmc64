include ../../circle-stdlib/Config.mk
CIRCLE_STDLIB_HOME = ../../circle-stdlib
include ../Config.mk

TOP=..

OBJ = plus4api.o circle.o virtual_keyboard_plus4.o

INCLUDES = -I $(CIRCLE_STDLIB_HOME)/install/arm-none-circle/include -I $(CIRCLE_STDLIB_HOME)/libs/circle/addon/fatfs -I $(TOP)/src -I $(TOP) -I ../../common

all: libplus4api.a

%.o : %.cpp
	$(CXX_FOR_TARGET) $(INCLUDES) -c -O2 -o $@ $< $(CFLAGS_FOR_TARGET) $(CPPFLAGS_FOR_TARGET)

%.o : %.c
	$(CC_FOR_TARGET) $(INCLUDES) -c -O2 -o $@ $< $(CFLAGS_FOR_TARGET)

libplus4api.a: $(OBJ)
	$(AR_FOR_TARGET) cr libplus4api.a $(OBJ)
	$(RANLIB_FOR_TARGET) libplus4api.a

clean:
	rm -rf *.o *.a
