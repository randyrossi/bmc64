# Builds the C64 performance workloads used by the BMC64 performance test
# suite (see README.md).
#
# Needs the xa assembler (xa65). On Debian/Ubuntu: sudo apt install xa65
#
# Copy the resulting .prg files onto the SD card and start one from the
# menu's file browser or the web UI (click the PRG). Each runs forever.

XA ?= xa

PRGS = perf_idle.prg perf_vic.prg perf_sid.prg perf_sid2.prg perf_stress.prg

all: $(PRGS)

perf_idle.prg: perf_workload.asm
	$(XA) -o $@ $<

perf_vic.prg: perf_workload.asm
	$(XA) -DVIC -o $@ $<

perf_sid.prg: perf_workload.asm
	$(XA) -DSID -o $@ $<

perf_sid2.prg: perf_workload.asm
	$(XA) -DSID -DDUAL -o $@ $<

perf_stress.prg: perf_workload.asm
	$(XA) -DVIC -DSID -o $@ $<

clean:
	rm -f $(PRGS)

.PHONY: all clean
