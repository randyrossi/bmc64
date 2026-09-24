// Drives the updater's planner and apply code on a PC folder, for
// run_tests.py. The folder is the "card"; its /bmc64-update.zip is used.
//
//   update_cli plan    <running> <board-kernel>
//   update_cli apply   <running> <board-kernel> [keep|replace <path>]...
//   update_cli resume
//   update_cli extract <zip> <entry> <out-file>
//
// The card folder is given in UPDATE_TEST_ROOT.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "update_apply.h"
#include "update_fs.h"
#include "update_manifest.h"
#include "update_plan.h"
#include "update_zip.h"

static const char *const status_names[] = {"new", "update", "changed"};
static const char *const group_names[] = {"kernel", "firmware", "config"};

static int load(uz_zip *z, um_manifest *m, up_plan *p, const char *running,
                const char *board) {
  char err[200];
  if (uz_open(z, UA_ZIP, err, sizeof(err)) != 0) {
    printf("error %s\n", err);
    return -1;
  }
  if (up_load_manifests(z, m, err, sizeof(err)) != 0 ||
      up_build(p, z, m, running, board, NULL, NULL, err, sizeof(err)) != 0) {
    printf("error %s\n", err);
    um_free(m);
    uz_close(z);
    return -1;
  }
  return 0;
}

static int write_file(void *ctx, const uint8_t *buf, unsigned len) {
  return fwrite(buf, 1, len, (FILE *)ctx) == len ? 0 : -1;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: see update_cli.c\n");
    return 2;
  }
  const char *cmd = argv[1];
  setvbuf(stdout, NULL, _IONBF, 0);

  if (strcmp(cmd, "resume") == 0) {
    printf("resumed %d\n", ua_resume());
    return 0;
  }

  if (strcmp(cmd, "extract") == 0 && argc == 5) {
    // The zip path here is relative to UPDATE_TEST_ROOT like everything else.
    uz_zip z;
    char err[200];
    if (uz_open(&z, argv[2], err, sizeof(err)) != 0) {
      printf("error %s\n", err);
      return 1;
    }
    int index = uz_find(&z, argv[3]);
    FILE *out = fopen(argv[4], "wb");
    int rc = (index < 0 || out == NULL) ? -1 : uz_extract(&z, index, write_file, out);
    if (out) fclose(out);
    uz_close(&z);
    printf(rc == 0 ? "ok\n" : "error extract\n");
    return rc == 0 ? 0 : 1;
  }

  if ((strcmp(cmd, "plan") == 0 || strcmp(cmd, "apply") == 0) && argc >= 4) {
    uz_zip z;
    um_manifest m;
    up_plan p;
    memset(&m, 0, sizeof(m));
    if (load(&z, &m, &p, argv[2], argv[3]) != 0) {
      return 1;
    }
    for (int a = 4; a + 1 < argc; a += 2) {
      for (int i = 0; i < p.count; i++) {
        if (strcasecmp(p.items[i].rec->path, argv[a + 1]) == 0) {
          p.items[i].replace = strcmp(argv[a], "replace") == 0;
        }
      }
    }
    printf("target %s direction %d up_to_date %d legacy %d\n", p.target,
           p.direction, p.up_to_date, p.legacy);
    for (int i = 0; i < p.count; i++) {
      const up_item *it = &p.items[i];
      printf("item %s %s %s %d %d %s\n", group_names[it->group],
             status_names[it->status], it->replace ? "replace" : "keep",
             it->this_board, it->order, it->rec->path);
    }
    int rc = 0;
    if (strcmp(cmd, "apply") == 0) {
      char err[200] = "";
      rc = ua_apply(&p, &z, NULL, NULL, err, sizeof(err));
      printf("apply %d %s\n", rc, err);
    }
    up_free(&p);
    um_free(&m);
    uz_close(&z);
    return rc < 0 ? 1 : 0;
  }

  fprintf(stderr, "bad arguments\n");
  return 2;
}
