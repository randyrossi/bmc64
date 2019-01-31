#! /usr/bin/make -f

.SILENT:

VICESRC=../src

PROGS= \
	$(VICESRC)/petcat \
	$(VICESRC)/cartconv \
	$(VICESRC)/c1541 \
	$(VICESRC)/vsid \
	$(VICESRC)/x64 \
	$(VICESRC)/x64sc \
	$(VICESRC)/x64dtv \
	$(VICESRC)/x128 \
	$(VICESRC)/xcbm2 \
	$(VICESRC)/xvic \
	$(VICESRC)/xplus4 \
	$(VICESRC)/xpet \
	$(VICESRC)/xscpu64

VICEDUMPOPT=-console -default -dumpconfig vice.rc.tmp -limitcycles 10 2>&1 > /dev/null

help:
	echo -ne "check vice.texi\n\n"
	echo -ne "usage: ./checkdoc.mak [full | opt | res | clean | update]\n\n"
	echo -ne "options:\n"
	echo -ne "full\tdo all checks\n"
	echo -ne "opt\tcheck command-line options\n"
	echo -ne "res\tcheck resources\n"
	echo -ne "listopt\tlist all command-line options\n"
	echo -ne "listres\tlist all resources\n"
	echo -ne "fixme\tshow FIXMEs\n"
	echo -ne "nodes\tshow nodes marked FIXME\n"
	echo -ne "clean\tremove temp files\n"
	echo -ne "update\tgenerate documentation\n"

vice.opts.tmp: $(PROGS) checkdoc.mak
	echo "creating vice.opts.tmp"
	echo "[C64]" > vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/x64 -help >> vice.opts.tmp
	echo "[C64SC]" >> vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/x64sc -help >> vice.opts.tmp
	echo "[C64DTV]" >> vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/x64dtv -help >> vice.opts.tmp
	echo "[C128]" >> vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/x128 -help >> vice.opts.tmp
	echo "[VSID]" >> vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/vsid -help >> vice.opts.tmp
	echo "[CBM-II]" >> vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/xcbm2 -help >> vice.opts.tmp
	echo "[CBM-II-5x0]" >> vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/xcbm2 -help >> vice.opts.tmp
	echo "[VIC20]" >> vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/xvic -help >> vice.opts.tmp
	echo "[PLUS4]" >> vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/xplus4 -help >> vice.opts.tmp
	echo "[PET]" >> vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/xpet -help >> vice.opts.tmp
	echo "[SCPU64]" >> vice.opts.tmp
	LANGUAGE="en" $(VICESRC)/xscpu64 -help >> vice.opts.tmp
	echo "[petcat]" >> vice.opts.tmp
	-LANGUAGE="en" $(VICESRC)/petcat -help | tr '[]' '()' >> vice.opts.tmp
	echo "[cartconv]" >> vice.opts.tmp
	-LANGUAGE="en" $(VICESRC)/cartconv -help >> vice.opts.tmp

vice.rc.tmp: $(PROGS) checkdoc.mak
	echo "creating vice.rc.tmp"
	rm -f vice.rc.tmp vice.rcall.tmp
	-$(VICESRC)/x64     $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	-$(VICESRC)/x64sc   $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	-$(VICESRC)/x64dtv  $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	-$(VICESRC)/x128    $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	-$(VICESRC)/vsid    $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	-$(VICESRC)/xcbm5x0 $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	-$(VICESRC)/xcbm2   $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	-$(VICESRC)/xvic    $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	-$(VICESRC)/xplus4  $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	-$(VICESRC)/xpet    $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	-$(VICESRC)/xscpu64 $(VICEDUMPOPT) ; cat vice.rc.tmp >> vice.rcall.tmp
	mv vice.rcall.tmp vice.rc.tmp
	
checkdoc: checkdoc.c
	echo "creating checkdoc"
	gcc -Wall -o checkdoc checkdoc.c

full: checkdoc vice.opts.tmp vice.rc.tmp fixme nodes
	./checkdoc -all vice.texi vice.rc.tmp vice.opts.tmp

res: checkdoc vice.opts.tmp vice.rc.tmp
	./checkdoc -res vice.texi vice.rc.tmp vice.opts.tmp

listres: checkdoc vice.opts.tmp vice.rc.tmp
	./checkdoc -listres vice.texi vice.rc.tmp vice.opts.tmp

opt: checkdoc vice.opts.tmp vice.rc.tmp
	./checkdoc -opt vice.texi vice.rc.tmp vice.opts.tmp

listopt: checkdoc vice.opts.tmp vice.rc.tmp
	./checkdoc -listopt vice.texi vice.rc.tmp vice.opts.tmp
	
update: vice.texi
	make --silent

all: full

.PHONY: fixme todo nodes clean

fixme:
	echo -ne "list of FIXMEs ("
	echo -ne `grep -an "@c " vice.texi | grep -v "\-\-\-" | grep -v "@node" | grep -i "fixme" | wc -l`
	echo -ne "):\n"
	grep -an "@c " vice.texi | grep -v "\-\-\-" | grep -v "@node" | grep -i "fixme"
	echo -ne "\n"

todo: fixme

nodes:
	echo -ne "nodes that need fixing ("
	echo -ne `grep -an "@c " vice.texi | grep -v "\-\-\-" | grep -i "fixme" | grep "@node" | wc -l`
	echo -ne "):\n"
	grep -an "@c " vice.texi | grep -v "\-\-\-" | grep -i "fixme" | grep "@node"
	echo -ne "\n"

clean:
	rm -f ./checkdoc
	rm -f vice.opts.tmp
	rm -f vice.rc.tmp
	rm -f vice.rcall.tmp
