include ../../circle-stdlib/Config.mk
CIRCLE_STDLIB_HOME = ../../circle-stdlib

TOP=..

OBJ = plus4api.o

INCLUDES = -I $(CIRCLE_STDLIB_HOME)/install/arm-none-circle/include -I $(CIRCLE_STDLIB_HOME)/libs/circle/addon/fatfs -I $(TOP)/src -I $(TOP)

all: libplus4api.a

%.o : %.cpp
	$(CXX_FOR_TARGET) $(INCLUDES) -c -o $@ $< $(CFLAGS_FOR_TARGET) $(CPPFLAGS_FOR_TARGET)

%.o : %.c
	$(CC_FOR_TARGET) $(INCLUDES) -c -o $@ $< $(CFLAGS_FOR_TARGET)

libplus4api.a: $(OBJ)
	$(AR_FOR_TARGET) cr libplus4api.a $(OBJ)
	$(RANLIB_FOR_TARGET) libplus4api.a

clean:
	rm -rf *.o *.a
