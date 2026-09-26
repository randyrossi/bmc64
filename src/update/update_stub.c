// Built instead of the updater when updater.cfg has updater = off (or there is
// no updater.cfg): the two entry points do nothing, so bmc64-update.zip is
// ignored.

#include "update.h"

void update_boot_check(void) {}

void update_run_if_pending(void) {}
