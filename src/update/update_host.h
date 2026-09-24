// Everything the updater needs from BMC64, in one place. Implemented only in
// update_host.c, so the rest of src/update/ never includes BMC64 headers and
// can be built and tested on a PC (tools/update/test/).

#ifndef BMC64_UPDATE_HOST_H
#define BMC64_UPDATE_HOST_H

#include <stdarg.h>

// Running BMC64 version, e.g. "5.1.10".
const char *uh_version(void);

// Base kernel file name of the board BMC64 is running on: "kernel.img",
// "kernel7.img" or "kernel8-32.img".
const char *uh_board_kernel(void);

// Take (1) or give back (0) the screen and input.
void uh_take_screen(int active);

// Drawing. uh_begin_frame() clears the screen and returns the top left of a
// 40x25 character area (8x8 pixel characters); 0 if nothing can be drawn.
int uh_begin_frame(int *left, int *top);
void uh_draw_text(const char *text, int x, int y, int color);
void uh_draw_rect(int x, int y, int w, int h, int color, int fill);
void uh_end_frame(void);

struct uh_colors {
  int bg, fg, hilite, border, dim, note;
};
void uh_colors(struct uh_colors *c);

// Input. Returns 1 and a key code (UH_KEY_*) if a key was pressed or
// released since the last call.
enum {
  UH_KEY_NONE,
  UH_KEY_UP,
  UH_KEY_DOWN,
  UH_KEY_LEFT,
  UH_KEY_RIGHT,
  UH_KEY_SELECT,   // Return, Space, joystick fire
  UH_KEY_BACK,     // Escape, F12
  UH_KEY_PAGE_UP,
  UH_KEY_PAGE_DOWN,
};
int uh_read_key(int *key, int *pressed);

// Keeps devices serviced while the view runs its own loop (GPIO scanning,
// other tasks, video).
void uh_poll(void);

void uh_reboot(void);

void uh_log(const char *fmt, ...);

#endif
