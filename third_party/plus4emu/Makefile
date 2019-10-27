include ../circle-stdlib/Config.mk

SUBDIRS := resid src plus4lib

.PHONY: all clean $(SUBDIRS)

%.o : %.c
	$(CC_FOR_TARGET) $(INCLUDES) -c -o $@ $< $(CFLAGS_FOR_TARGET)

all:
	for dir in $(SUBDIRS); do \
	$(MAKE) -C $$dir -f Makefile $@; \
	done

clean:
	for dir in $(SUBDIRS); do \
	$(MAKE) -C $$dir -f Makefile $@; \
	done
	rm -f *.o plus4emu

plus4emu: main.o plus4lib/libplus4api.a src/libplus4.a resid/libresid.a
	$(CXX) -o plus4emu main.o plus4lib/libplus4api.a src/libplus4.a \
		resid/libresid.a
