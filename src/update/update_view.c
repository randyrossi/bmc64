// The update view. It draws its own 40x25 character screen and runs its own
// input loop through update_host.h; it does not use the BMC64 menu.

#include "update_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "update_apply.h"
#include "update_fs.h"
#include "update_host.h"
#include "update_manifest.h"
#include "update_plan.h"
#include "update_zip.h"

#define COLS 40
#define ROWS 25
#define LIST_TOP 4       // first list row on screen
#define LIST_ROWS 17     // rows 4..20
#define CHAR 8

enum { ROW_GROUP, ROW_FILE, ROW_BLANK, ROW_APPLY, ROW_LATER, ROW_SKIP };

typedef struct {
  int type;
  int index;             // group number or item index
} row;

// A message screen: a few lines of text and up to three buttons.
typedef struct {
  const char *title;
  char lines[6][COLS + 1];
  int nlines;
  const char *buttons[3];
  int nbuttons;
  int cursor;
} message;

static struct uh_colors col;
static int scr_left, scr_top;

// ---- drawing ----

static int begin(void) {
  if (!uh_begin_frame(&scr_left, &scr_top)) {
    return 0;
  }
  uh_draw_rect(scr_left, scr_top, COLS * CHAR, ROWS * CHAR, col.bg, 1);
  uh_draw_rect(scr_left - 1, scr_top - 1, COLS * CHAR + 2, ROWS * CHAR + 2,
               col.border, 0);
  return 1;
}

static void text_at(int x, int y, const char *s, int color) {
  uh_draw_text(s, scr_left + x * CHAR, scr_top + y * CHAR, color);
}

static void highlight(int y) {
  uh_draw_rect(scr_left, scr_top + y * CHAR, COLS * CHAR, CHAR, col.hilite, 1);
}

static void divider(int y) {
  uh_draw_rect(scr_left, scr_top + y * CHAR + 3, COLS * CHAR, 2, col.border, 1);
}

// Copies s into out, at most width characters; long text keeps its end,
// which is the useful part of a path.
static void fit(char *out, const char *s, int width) {
  int len = (int)strlen(s);
  if (len <= width) {
    strcpy(out, s);
  } else {
    out[0] = '.';
    out[1] = '.';
    strcpy(out + 2, s + len - (width - 2));
  }
}

static void centered(int y, const char *s, int color) {
  int len = (int)strlen(s);
  text_at(len < COLS ? (COLS - len) / 2 : 0, y, s, color);
}

// ---- message screens ----

static void draw_message(const message *m) {
  if (!begin()) {
    return;
  }
  centered(1, m->title, col.fg);
  divider(2);
  for (int i = 0; i < m->nlines; i++) {
    text_at(1, 4 + i, m->lines[i], col.fg);
  }
  for (int i = 0; i < m->nbuttons; i++) {
    int y = 12 + i * 2;
    if (i == m->cursor) {
      highlight(y);
    }
    centered(y, m->buttons[i], col.fg);
  }
  uh_end_frame();
}

// Word-wraps text into the message's lines.
static void message_text(message *m, const char *text) {
  const char *p = text;
  while (*p && m->nlines < 6) {
    int width = COLS - 2;
    int len = (int)strlen(p);
    int take = len;
    if (len > width) {
      take = width;
      while (take > 0 && p[take] != ' ') take--;
      if (take == 0) take = width;
    }
    memcpy(m->lines[m->nlines], p, (size_t)take);
    m->lines[m->nlines][take] = '\0';
    m->nlines++;
    p += take;
    while (*p == ' ') p++;
  }
}

// Shows the message until a button is chosen; returns its index.
static int run_message(message *m) {
  for (;;) {
    draw_message(m);
    uh_poll();
    int key, pressed;
    while (uh_read_key(&key, &pressed)) {
      if (!pressed) {
        continue;
      }
      if (key == UH_KEY_UP && m->cursor > 0) {
        m->cursor--;
      } else if (key == UH_KEY_DOWN && m->cursor < m->nbuttons - 1) {
        m->cursor++;
      } else if (key == UH_KEY_SELECT) {
        return m->cursor;
      }
    }
  }
}

static int ask(const char *title, const char *text, const char *b0,
               const char *b1, const char *b2) {
  message m;
  memset(&m, 0, sizeof(m));
  m.title = title;
  message_text(&m, text);
  m.buttons[m.nbuttons++] = b0;
  if (b1) m.buttons[m.nbuttons++] = b1;
  if (b2) m.buttons[m.nbuttons++] = b2;
  return run_message(&m);
}

static void status_screen(const char *title, const char *line1,
                          const char *line2) {
  if (!begin()) {
    return;
  }
  centered(1, title, col.fg);
  divider(2);
  char buf[COLS + 1];
  fit(buf, line1, COLS - 2);
  text_at(1, 10, buf, col.fg);
  fit(buf, line2, COLS - 2);
  text_at(1, 12, buf, col.note);
  text_at(1, 22, "Please wait, do not switch off.", col.note);
  uh_end_frame();
  uh_poll();
}

static void check_progress(void *ctx, const char *name, int done, int total) {
  (void)ctx;
  char line[COLS + 1];
  snprintf(line, sizeof(line), "Checking files %d/%d", done + 1, total);
  status_screen("BMC64 UPDATE", line, name);
}

static void apply_progress(void *ctx, const char *stage, const char *name,
                           int done, int total) {
  (void)ctx;
  char line[COLS + 1];
  if (total > 1) {
    snprintf(line, sizeof(line), "%s %d/%d", stage, done + 1, total);
  } else {
    snprintf(line, sizeof(line), "%s...", stage);
  }
  status_screen("BMC64 UPDATE", line, name);
}

// ---- the list ----

static const char *const group_names[UP_GROUPS] = {
    "Kernels", "Raspberry Pi firmware", "Configuration and other files"};

typedef struct {
  up_plan *plan;
  row *rows;
  int nrows;
  int cursor;
  int top;
} list;

static int is_row_selectable(const row *r) { return r->type != ROW_BLANK; }

static void build_rows(list *l) {
  up_plan *p = l->plan;
  l->rows = calloc((size_t)(p->count + UP_GROUPS * 2 + 4), sizeof(row));
  l->nrows = 0;
  for (int g = 0; g < UP_GROUPS; g++) {
    int any = 0;
    for (int i = 0; i < p->count; i++) {
      if (p->items[i].group != g) {
        continue;
      }
      if (!any) {
        l->rows[l->nrows++] = (row){ROW_GROUP, g};
        any = 1;
      }
      l->rows[l->nrows++] = (row){ROW_FILE, i};
    }
    if (any) {
      l->rows[l->nrows++] = (row){ROW_BLANK, 0};
    }
  }
  l->rows[l->nrows++] = (row){ROW_APPLY, 0};
  l->rows[l->nrows++] = (row){ROW_LATER, 0};
  l->rows[l->nrows++] = (row){ROW_SKIP, 0};
  l->cursor = 0;
  l->top = 0;
}

// 1 = all replace, 0 = all keep, -1 = mixed.
static int group_state(const up_plan *p, int g) {
  int on = 0, off = 0;
  for (int i = 0; i < p->count; i++) {
    if (p->items[i].group == g) {
      if (p->items[i].replace) on++; else off++;
    }
  }
  return off == 0 ? 1 : on == 0 ? 0 : -1;
}

static int group_count(const up_plan *p, int g) {
  int n = 0;
  for (int i = 0; i < p->count; i++) {
    n += p->items[i].group == g;
  }
  return n;
}

static int is_pair(const char *path) {
  return strcasecmp(path, "start.elf") == 0 || strcasecmp(path, "fixup.dat") == 0;
}

static void set_item(up_plan *p, int index, int replace) {
  p->items[index].replace = replace;
  // start.elf and fixup.dat must always match each other.
  if (is_pair(p->items[index].rec->path)) {
    for (int i = 0; i < p->count; i++) {
      if (is_pair(p->items[i].rec->path)) {
        p->items[i].replace = replace;
      }
    }
  }
}

static const char *status_label(const up_item *it) {
  switch (it->status) {
  case UP_NEW:
    return "New";
  case UP_CHANGED:
    return "Changed";
  default:
    return "Update";
  }
}

static const char *choice_label(const up_item *it) {
  if (it->status == UP_NEW) {
    return it->replace ? "Add" : "Skip";
  }
  return it->replace ? "Replace" : "Keep";
}

// A one-line explanation of the selected row, shown at the bottom.
static const char *row_help(const list *l, const row *r) {
  const up_plan *p = l->plan;
  if (r->type == ROW_FILE) {
    const up_item *it = &p->items[r->index];
    if (strcasecmp(it->rec->path, "config.txt") == 0 ||
        strcasecmp(it->rec->path, "cmdline.txt") == 0) {
      return "Replacing resets machine & video mode";
    }
    if (it->status == UP_CHANGED) {
      return "You changed this. Keep leaves it as is";
    }
    if (it->this_board) {
      return "A kernel for this Raspberry Pi";
    }
    if (is_pair(it->rec->path)) {
      return "start.elf and fixup.dat go together";
    }
    return "Fire/Return: Replace or Keep";
  }
  switch (r->type) {
  case ROW_GROUP:
    return "Fire/Return: change the whole group";
  case ROW_APPLY:
    return "Originals are saved in /backup";
  case ROW_LATER:
    return "Leave bmc64-update.zip; ask next boot";
  case ROW_SKIP:
    return "Rename it so it is not offered again";
  default:
    return "";
  }
}

static void draw_list(const list *l) {
  const up_plan *p = l->plan;
  if (!begin()) {
    return;
  }
  char buf[COLS + 1];
  const char *title = p->direction < 0   ? "BMC64 UPDATE"
                      : p->direction > 0 ? "BMC64 DOWNGRADE"
                                         : "BMC64 REPAIR";
  centered(0, title, col.fg);
  snprintf(buf, sizeof(buf), "Installed v%s -> %s", p->running, p->target);
  centered(1, buf, col.fg);
  snprintf(buf, sizeof(buf), "%d file%s already up to date", p->up_to_date,
           p->up_to_date == 1 ? "" : "s");
  centered(2, buf, col.note);
  divider(3);

  for (int n = 0; n < LIST_ROWS && l->top + n < l->nrows; n++) {
    int index = l->top + n;
    const row *r = &l->rows[index];
    int y = LIST_TOP + n;
    if (index == l->cursor) {
      highlight(y);
    }
    char name[COLS + 1];
    switch (r->type) {
    case ROW_GROUP: {
      int state = group_state(p, r->index);
      snprintf(buf, sizeof(buf), "%s (%d)", group_names[r->index],
               group_count(p, r->index));
      fit(name, buf, 30);
      text_at(0, y, name, col.fg);
      text_at(32, y, state > 0 ? "All" : state == 0 ? "None" : "Some",
              col.fg);
      break;
    }
    case ROW_FILE: {
      const up_item *it = &p->items[r->index];
      fit(name, it->rec->path, 22);
      text_at(2, y, name, it->replace ? col.fg : col.dim);
      text_at(25, y, status_label(it),
              it->status == UP_CHANGED ? col.note : col.fg);
      text_at(33, y, choice_label(it), it->replace ? col.fg : col.dim);
      break;
    }
    case ROW_APPLY:
      text_at(0, y, p->direction > 0 ? "Downgrade now and reboot"
                                     : "Update now and reboot", col.fg);
      break;
    case ROW_LATER:
      text_at(0, y, "Not now", col.fg);
      break;
    case ROW_SKIP:
      text_at(0, y, "Skip this update", col.fg);
      break;
    default:
      break;
    }
  }
  if (l->top > 0) {
    text_at(39, LIST_TOP, "^", col.note);
  }
  if (l->top + LIST_ROWS < l->nrows) {
    text_at(39, LIST_TOP + LIST_ROWS - 1, "v", col.note);
  }

  divider(21);
  fit(buf, row_help(l, &l->rows[l->cursor]), COLS - 1);
  text_at(0, 22, buf, col.note);
  text_at(0, 24, "Up/Down: move  Fire/Return: choose", col.dim);
  uh_end_frame();
}

// Moves the cursor by delta selectable rows (stops at either end).
static void move_cursor(list *l, int delta) {
  int step = delta > 0 ? 1 : -1;
  for (int n = delta > 0 ? delta : -delta; n > 0; n--) {
    int c = l->cursor + step;
    while (c >= 0 && c < l->nrows && !is_row_selectable(&l->rows[c])) {
      c += step;
    }
    if (c < 0 || c >= l->nrows) {
      break;
    }
    l->cursor = c;
  }
  if (l->cursor < l->top) {
    l->top = l->cursor;
  } else if (l->cursor >= l->top + LIST_ROWS) {
    l->top = l->cursor - LIST_ROWS + 1;
  }
}

enum { CHOICE_APPLY, CHOICE_LATER, CHOICE_SKIP };

static int run_list(list *l) {
  for (;;) {
    draw_list(l);
    uh_poll();
    int key, pressed;
    while (uh_read_key(&key, &pressed)) {
      if (!pressed) {
        continue;
      }
      row *r = &l->rows[l->cursor];
      switch (key) {
      case UH_KEY_UP:
        move_cursor(l, -1);
        break;
      case UH_KEY_DOWN:
        move_cursor(l, 1);
        break;
      case UH_KEY_PAGE_UP:
        move_cursor(l, -LIST_ROWS);
        break;
      case UH_KEY_PAGE_DOWN:
        move_cursor(l, LIST_ROWS);
        break;
      case UH_KEY_LEFT:
      case UH_KEY_RIGHT:
      case UH_KEY_SELECT:
        if (r->type == ROW_FILE) {
          set_item(l->plan, r->index, !l->plan->items[r->index].replace);
        } else if (r->type == ROW_GROUP) {
          int state = group_state(l->plan, r->index);
          for (int i = 0; i < l->plan->count; i++) {
            if (l->plan->items[i].group == r->index) {
              set_item(l->plan, i, state != 1);
            }
          }
        } else if (key == UH_KEY_SELECT) {
          if (r->type == ROW_APPLY) return CHOICE_APPLY;
          if (r->type == ROW_LATER) return CHOICE_LATER;
          if (r->type == ROW_SKIP) return CHOICE_SKIP;
        }
        break;
      default:
        break;
      }
    }
  }
}

// ---- the whole flow ----

static void rename_zip(const char *suffix) {
  char to[64];
  snprintf(to, sizeof(to), UA_ZIP "%s", suffix);
  uf_unlink(to);
  if (uf_rename(UA_ZIP, to) != 0) {
    uh_log("cannot rename " UA_ZIP " to %s", to);
  }
}

static void not_a_package(const char *reason) {
  char text[256];
  snprintf(text, sizeof(text), "bmc64-update.zip can't be used. %s", reason);
  int b = ask("BMC64 UPDATE", text, "Rename it to .bad and continue",
              "Leave it and continue", NULL);
  if (b == 0) {
    rename_zip(".bad");
  }
}

void uv_run(void) {
  uh_colors(&col);
  char err[200] = "";
  uz_zip z;
  um_manifest m;
  up_plan p;
  memset(&m, 0, sizeof(m));
  memset(&p, 0, sizeof(p));

  status_screen("BMC64 UPDATE", "Reading bmc64-update.zip", "");
  if (uz_open(&z, UA_ZIP, err, sizeof(err)) != 0) {
    not_a_package(err);
    return;
  }
  if (up_load_manifests(&z, &m, err, sizeof(err)) != 0 ||
      up_build(&p, &z, &m, uh_version(), uh_board_kernel(), check_progress,
               NULL, err, sizeof(err)) != 0) {
    uh_log("%s", err);
    not_a_package(err);
    um_free(&m);
    uz_close(&z);
    return;
  }

  if (p.count == 0) {
    char text[160];
    snprintf(text, sizeof(text),
             "Every file already matches %s. There is nothing to update.",
             p.target);
    if (ask("BMC64 UPDATE", text, "Delete bmc64-update.zip and continue",
            "Leave it and continue", NULL) == 0) {
      uf_unlink(UA_ZIP);
    }
    up_free(&p);
    um_free(&m);
    uz_close(&z);
    return;
  }

  list l;
  memset(&l, 0, sizeof(l));
  l.plan = &p;
  build_rows(&l);

  for (;;) {
    int choice = run_list(&l);
    if (choice == CHOICE_LATER) {
      break;
    }
    if (choice == CHOICE_SKIP) {
      if (ask("SKIP THIS UPDATE",
              "bmc64-update.zip will be renamed to bmc64-update.zip.skipped "
              "and not offered again.",
              "Skip it", "Back", NULL) == 0) {
        rename_zip(".skipped");
        break;
      }
      continue;
    }

    int replace = 0, keep = 0;
    for (int i = 0; i < p.count; i++) {
      if (p.items[i].replace) replace++; else keep++;
    }
    if (replace == 0) {
      ask("BMC64 UPDATE", "No files are selected. Choose Replace for at least "
          "one file, or choose Not now.", "Back", NULL, NULL);
      continue;
    }
    char text[200];
    char bdir[64];
    ua_backup_dir(p.running, bdir, sizeof(bdir));
    snprintf(text, sizeof(text),
             "Replace or add %d file%s and keep %d. The originals are saved "
             "in %s and /backup/kernel. BMC64 then reboots.",
             replace, replace == 1 ? "" : "s", keep, bdir);
    if (ask(p.direction > 0 ? "DOWNGRADE NOW?" : "UPDATE NOW?", text,
            "Yes, go ahead", "Back", NULL) != 0) {
      continue;
    }

    int rc = ua_apply(&p, &z, apply_progress, NULL, err, sizeof(err));
    if (rc < 0) {
      // Nothing on the card was changed.
      uh_log("update stopped: %s", err);
      char msg[260];
      snprintf(msg, sizeof(msg), "The update stopped and nothing was "
               "changed. %s", err);
      if (ask("UPDATE STOPPED", msg, "Back to the list", "Continue to BMC64",
              NULL) == 0) {
        continue;
      }
      break;
    }
    uz_close(&z);
    char msg[260];
    if (rc == 0) {
      snprintf(msg, sizeof(msg), "BMC64 is now %s. A report is in %s.",
               p.target, bdir);
    } else {
      snprintf(msg, sizeof(msg), "Finished with problems: %s. To go back, "
               "see RESTORE.txt in %s.", err, bdir);
    }
    uh_log("%s", msg);
    ask(rc == 0 ? "UPDATE COMPLETE" : "UPDATE FINISHED", msg, "Reboot now",
        NULL, NULL);
    uh_reboot();
    return;
  }

  free(l.rows);
  up_free(&p);
  um_free(&m);
  uz_close(&z);
}
