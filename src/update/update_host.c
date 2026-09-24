// The updater's only connection to the rest of BMC64 (see update_host.h).

#include "update_host.h"

#include <stdio.h>

#include "circle.h"
#include "emux_api.h"
#include "keycodes.h"
#include "menu.h"
#include "ui.h"

extern void reboot(void);

const char *uh_version(void) { return bmc64_version_string(); }

const char *uh_board_kernel(void) {
  // Same mapping as menu_switch.c's apply_config().
  switch (circle_get_model()) {
  case 2:
    return "kernel7.img";
  case 3:
    return "kernel8-32.img";
  default:
    return "kernel.img";
  }
}

void uh_take_screen(int active) { ui_set_external_owner(active); }

int uh_begin_frame(int *left, int *top) {
  return ui_external_begin_frame(left, top);
}

void uh_draw_text(const char *text, int x, int y, int color) {
  ui_draw_text(text, x, y, color);
}

void uh_draw_rect(int x, int y, int w, int h, int color, int fill) {
  ui_draw_rect(x, y, w, h, color, fill);
}

void uh_end_frame(void) { ui_external_end_frame(); }

void uh_colors(struct uh_colors *c) {
  struct ui_external_colors u;
  ui_external_colors(&u);
  c->bg = u.bg;
  c->fg = u.fg;
  c->hilite = u.hilite;
  c->border = u.border;
  c->dim = u.dim;
  c->note = u.note;
}

int uh_read_key(int *key, int *pressed) {
  long code;
  while (ui_read_key_event(&code, pressed)) {
    switch (code) {
    case KEYCODE_Up:
      *key = UH_KEY_UP;
      return 1;
    case KEYCODE_Down:
      *key = UH_KEY_DOWN;
      return 1;
    case KEYCODE_Left:
      *key = UH_KEY_LEFT;
      return 1;
    case KEYCODE_Right:
      *key = UH_KEY_RIGHT;
      return 1;
    case KEYCODE_Return:
    case KEYCODE_Space:
      *key = UH_KEY_SELECT;
      return 1;
    case KEYCODE_Escape:
    case KEYCODE_F12:
      *key = UH_KEY_BACK;
      return 1;
    case KEYCODE_PageUp:
      *key = UH_KEY_PAGE_UP;
      return 1;
    case KEYCODE_PageDown:
      *key = UH_KEY_PAGE_DOWN;
      return 1;
    default:
      break; // Ignore everything else.
    }
  }
  return 0;
}

void uh_poll(void) {
  circle_check_gpio();
  emux_ensure_video();
  circle_yield();
}

void uh_reboot(void) { reboot(); }

void uh_log(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  printf("update: ");
  vprintf(fmt, ap);
  printf("\n");
  va_end(ap);
}
