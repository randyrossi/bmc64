include ../circle-stdlib/Config.mk
CIRCLE_STDLIB_HOME = ../circle-stdlib
include Config.mk

OBJ = font.o keycodes.o text.o

INCLUDES = -I $(CIRCLE_STDLIB_HOME)/install/arm-none-circle/include -I $(CIRCLE_STDLIB_HOME)/libs/circle/addon/fatfs

all: libbmc64common.a

%.o : %.cpp
	$(CXX_FOR_TARGET) $(INCLUDES) -c -o $@ $< $(CFLAGS_FOR_TARGET) $(CPPFLAGS_FOR_TARGET)

%.o : %.c
	$(CC_FOR_TARGET) $(INCLUDES) -c -o $@ $< $(CFLAGS_FOR_TARGET)

libbmc64common.a: $(OBJ)
	$(AR_FOR_TARGET) cr libbmc64common.a $(OBJ)
	$(RANLIB_FOR_TARGET) libbmc64common.a

clean:
	rm -rf *.o *.a
