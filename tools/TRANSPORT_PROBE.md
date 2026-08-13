# BMC modem transport probe

On a development machine connected to the same LAN as BMC64, determine its LAN
address, then run:

	hostname -I
	python3 tools/modem_transport_probe.py --host <development-machine-lan-ip>

In CCGMS, dial that address and port:

	ATDT<development-machine-lan-ip>:6502

The probe asks for DEL, then `4`, then sends a 2048-byte printable burst and
keeps the TCP connection open for 15 seconds. Its terminal must report
`received: 14`, `received: 34`, and two successful send counts. BMC64 must
report zero receive drops, drain the queue, and retain carrier until the probe
exits. Re-run it several times before testing live BBS servers again.

To test a BBS-style pager response after the burst, add `--require-space`:

	python3 tools/modem_transport_probe.py --host <development-machine-lan-ip> --require-space

After `PRESS SPACE TO CONTINUE:` appears in CCGMS, press Space. The probe must
report `received: 20` and send `SPACE RECEIVED`; BMC64 must log `sent byte: 20`.

To reproduce the live BBS's small initial receive fragments, add
`--fragmented-burst`. The probe sends 1, 3, 3, and 237 bytes with 120 ms gaps,
then the remaining payload. BMC64 must receive all fragments and the remaining
payload without disconnecting.

To distinguish a TCP Nagle interaction from a BMC64 receive failure, repeat
the fragmented test with `--tcp-nodelay`. If this passes while the default
fragmented test fails, retain both logs and investigate Circle's ACK handling;
do not change CCGMS or ACIA code.

The probe reports Linux TCP `unacked` and `retrans` counts after the burst and
before closing. Retain these lines with the BMC64 log: nonzero `unacked` or
`retrans` indicates a missing acknowledgement; zeros indicate data was
acknowledged but did not reach the modem receive queue.

A good test is to connect to:

	python3 tools/modem_transport_probe.py --host <development-machine-lan-ip> --fragmented-burst --tcp-nodelay
