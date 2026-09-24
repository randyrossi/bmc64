// The update view: a full-screen screen of its own (not built from the BMC64
// menu) where the user chooses Replace or Keep for each changed file.

#ifndef BMC64_UPDATE_VIEW_H
#define BMC64_UPDATE_VIEW_H

// Reads /bmc64-update.zip, shows the view and applies the update if the user
// asks for it (then reboots). Returns if the user chose not to update now.
void uv_run(void);

#endif
