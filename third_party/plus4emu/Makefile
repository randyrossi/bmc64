include ../circle-stdlib/Config.mk
include Config.mk

SUBDIRS := resid src plus4lib

.PHONY: all clean $(SUBDIRS)

OBJS = main.o

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
