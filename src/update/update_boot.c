// The two entry points the rest of BMC64 calls (update.h).

#include "update.h"

#include <stddef.h>

#include "update_apply.h"
#include "update_fs.h"
#include "update_host.h"
#include "update_view.h"

static int update_pending;

void update_boot_check(void) {
  if (ua_resume() > 0) {
    // Files were moved into place; start again on the new version.
    uh_log("rebooting into the updated version");
    uh_reboot();
  }
  update_pending = uf_stat(UA_ZIP, NULL, NULL) == 0;
  if (update_pending) {
    uh_log(UA_ZIP " found");
  }
}

void update_run_if_pending(void) {
  if (!update_pending) {
    return;
  }
  update_pending = 0;
  uh_take_screen(1);
  uv_run();
  uh_take_screen(0);
}
