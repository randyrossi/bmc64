# Store PNGs in the repository as well. If some wants to update ICO files,
# they should say 'make' in this directory. Since it's quite unlikely these
# changes a lot over the time, IMHO it's not problem to store them in two
# versions inside the repository.

ICOS	= $(wildcard *.ico)
PNGS	= $(ICOS:.ico=.png)

all:	$(PNGS)

%.png:	%.ico
	convert $< $@
