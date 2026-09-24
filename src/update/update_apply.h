// Applies an update plan: extract and verify the chosen files, then move the
// originals into /backup and the new files into place, following a journal
// so an interrupted update can be finished on the next boot.

#ifndef BMC64_UPDATE_APPLY_H
#define BMC64_UPDATE_APPLY_H

#include "update_plan.h"
#include "update_zip.h"

#define UA_ZIP "/bmc64-update.zip"
#define UA_TMP "/update_tmp"
#define UA_JOURNAL UA_TMP "/journal.txt"
#define UA_BACKUP "/backup"
#define UA_MARKER ".bmc64-backup"

typedef void (*ua_progress_fn)(void *ctx, const char *stage, const char *name,
                               int done, int total);

// Returns 0 when everything was applied. Returns < 0 if it stopped before
// any file on the card was changed (err says why; z is still open). Returns
// > 0, the number of files that could not be moved, if the commit ran with
// problems. z is closed once files start moving: the zip is deleted at the
// end.
int ua_apply(const up_plan *p, uz_zip *z, ua_progress_fn progress, void *ctx,
             char *err, unsigned errlen);

// Early boot: if a complete journal is waiting, finishes it and returns 1.
// Returns 0 if there was nothing to finish.
int ua_resume(void);

// Deletes a file or a folder with everything in it. Returns 0 on success.
int ua_rmtree(const char *path);

// The folder old files are saved in, e.g. "/backup/v5.1.10".
void ua_backup_dir(const char *running, char *out, unsigned size);

#endif
