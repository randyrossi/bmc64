// BMC64 updater: applies /bmc64-update.zip. See docs/UPDATING.md for what the
// user sees.
//
// This is the only header the rest of BMC64 includes.

#ifndef BMC64_UPDATE_H
#define BMC64_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

// Early boot, after the SD card is mounted and before any BMC64 file is read.
// Finishes an update that was interrupted while files were being moved into
// place (and reboots if it did), then remembers whether /bmc64-update.zip is
// waiting.
void update_boot_check(void);

// Once boot has completed, on the emulator's main loop. If an update is
// waiting, shows the update view. Returns when the user chooses not to
// update now; if the update is applied, BMC64 reboots instead.
void update_run_if_pending(void);

#ifdef __cplusplus
}
#endif

#endif
