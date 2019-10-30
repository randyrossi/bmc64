include ../../circle-stdlib/Config.mk
include ../Config.mk

OBJ = dac.o envelope.o extfilt.o filter.o pot.o sid.o version.o voice.o wave.o

INCLUDES = -I ../src

all: libresid.a

%.o : %.cpp
	$(CXX_FOR_TARGET) $(INCLUDES) -c -O2 -o $@ $< $(CFLAGS_FOR_TARGET) $(CXXFLAGS_FOR_TARGET)

libresid.a: $(OBJ)
	$(AR_FOR_TARGET) cr libresid.a $(OBJ)
	$(RANLIB_FOR_TARGET) libresid.a

clean:
	rm -rf *.o *.a
