include ../circle-stdlib/Config.mk
include Config.mk

SUBDIRS := resid src plus4lib

.PHONY: all clean $(SUBDIRS)

OBJS = main.o

ifeq ($(HOST_BUILD),Y)
CFLAGS_FOR_TARGET += -DHOST_BUILD
endif

%.o : %.c
	$(CC_FOR_TARGET) $(INCLUDES) -c -O2 -o $@ $< $(CFLAGS_FOR_TARGET)

all: $(OBJS)
	for dir in $(SUBDIRS); do \
	$(MAKE) -C $$dir -f Makefile $@; \
	done

clean:
	for dir in $(SUBDIRS); do \
	$(MAKE) -C $$dir -f Makefile $@; \
	done
	rm -f *.o plus4emu

# Host build only
plus4emu: all main.o plus4lib/libplus4api.a src/libplus4.a resid/libresid.a
	$(CXX) -o plus4emu main.o plus4lib/libplus4api.a \
	src/libplus4.a resid/libresid.a
