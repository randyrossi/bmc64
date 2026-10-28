A pretend SD card for previewing the web UI's Profiles page without a Pi:

    python3 tools/webui_dev_server.py --root tools/webui_test/sdcard --running geos

It has a profile for every machine, with startup disks, IEC folders,
expansions and control ports set, so every part of a profile card shows.
The disk and cartridge files they name don't exist.
