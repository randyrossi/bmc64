#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "circle.h"
#include "joy.h"
#include "kbd.h"
#include "keycodes.h"
#include "ui.h"
#include "../../src/keyboard/keyboard_router.h"

struct joydev_config joydevs[MAX_JOY_PORTS];
volatile int ui_enabled;
int ui_toggle_pending;
raw_keycode_func_t raw_keycode_func;
signed long commodore_key_sym = KEYCODE_LeftSuper;
signed long ctrl_key_sym = KEYCODE_LeftControl;

static int emulator_events;
static int menu_events;
static int quick_action;
static int captured_key;
static int joystick_events;
static long last_key;
static int last_pressed;
static unsigned long ticks;
static int safe_video_calls;
static int reboot_calls;

void assertion_failed(const char *expression, const char *file, unsigned line) {
  fprintf(stderr, "%s:%u: %s\n", file, line, expression);
  exit(1);
}

void emux_key_interrupt(long key, int pressed) {
  emulator_events++;
  last_key = key;
  last_pressed = pressed;
}

void emu_ui_key_interrupt(long key, int pressed) {
  menu_events++;
  last_key = key;
  last_pressed = pressed;
}

void emu_quick_func_interrupt(int action) {
  quick_action = action;
}

void circle_lock_acquire(void) {}
void circle_lock_release(void) {}
unsigned long circle_get_ticks(void) { return ticks; }
void switch_safe(void) { safe_video_calls++; }
void reboot(void) { reboot_calls++; }

int joy_key_down(unsigned int port, int key) {
  (void)port;
  if (key != KEYCODE_j) {
    return 0;
  }
  joystick_events++;
  return 1;
}

int joy_key_up(unsigned int port, int key) {
  return joy_key_down(port, key);
}

static void capture_key(long key) {
  captured_key = key;
}

int main(void) {
  emu_key_pressed(KEYCODE_a);
  emu_key_released(KEYCODE_a);
  assert(emulator_events == 2 && last_key == KEYCODE_a && !last_pressed);

  ui_enabled = 1;
  emu_key_pressed(KEYCODE_b);
  emu_key_released(KEYCODE_b);
  assert(menu_events == 2 && last_key == KEYCODE_b && !last_pressed);
  ui_enabled = 0;

  emu_key_pressed(KEYCODE_F12);
  emu_key_released(KEYCODE_F12);
  assert(ui_toggle_pending == 2 && emulator_events == 4);
  ui_toggle_pending = 0;

  kbd_set_hotkey_function(0, KEYCODE_f, BTN_ASSIGN_WARP);
  emu_key_pressed(commodore_key_sym);
  emu_key_pressed(KEYCODE_f);
  emu_key_released(KEYCODE_f);
  emu_key_released(commodore_key_sym);
  assert(quick_action == BTN_ASSIGN_WARP && emulator_events == 6);

  kbd_set_hotkey_function(0, KEYCODE_m, BTN_ASSIGN_MENU);
  emu_key_pressed(commodore_key_sym);
  emu_key_pressed(KEYCODE_m);
  emu_key_released(KEYCODE_m);
  emu_key_released(commodore_key_sym);
  assert(ui_toggle_pending == 2 && emulator_events == 8);

  raw_keycode_func = capture_key;
  emu_key_pressed(KEYCODE_c);
  emu_key_released(KEYCODE_c);
  assert(captured_key == KEYCODE_c && emulator_events == 8);
  raw_keycode_func = 0;

  joydevs[0].device = JOYDEV_KEYSET1;
  emu_key_pressed(KEYCODE_j);
  emu_key_released(KEYCODE_j);
  assert(joystick_events == 2 && emulator_events == 8);

  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_d, KEYCODE_d, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_d, KEYCODE_d, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 12, KEYCODE_e, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_d, KEYCODE_x, 0);
  assert(last_key == KEYCODE_d && !last_pressed);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 12, KEYCODE_y, 0);
  assert(last_key == KEYCODE_e && emulator_events == 12);

  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_t, KEYCODE_t, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 15, KEYCODE_t, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_t, KEYCODE_t, 0);
  assert(emulator_events == 13);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 15, KEYCODE_t, 0);
  assert(emulator_events == 14 && last_key == KEYCODE_t && !last_pressed);

  kbd_set_hotkey_function(0, KEYCODE_n, BTN_ASSIGN_RESET_SOFT);
  emu_key_pressed(commodore_key_sym);
  emu_key_pressed(KEYCODE_n);
  emu_key_released(commodore_key_sym);
  assert(quick_action == BTN_ASSIGN_RESET_SOFT && emulator_events == 16);
  emu_key_released(KEYCODE_n);
  assert(emulator_events == 16);

  emu_key_pressed(commodore_key_sym);
  emu_key_pressed(KEYCODE_F7);
  ticks = 5000001UL;
  emu_key_released(KEYCODE_F7);
  emu_key_released(commodore_key_sym);
  assert(safe_video_calls == 1 && reboot_calls == 1);

  int emulator_before = emulator_events;
  int menu_before = menu_events;
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_v, KEYCODE_v, 1);
  ui_enabled = 1;
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_v, KEYCODE_v, 0);
  assert(emulator_events == emulator_before + 2 && menu_events == menu_before);

  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 22, KEYCODE_w, 1);
  ui_enabled = 0;
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 22, KEYCODE_w, 0);
  assert(menu_events == menu_before + 2 && emulator_events == emulator_before + 2);

  emulator_before = emulator_events;
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_a, KEYCODE_a, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 9, KEYCODE_w, 1);
  assert(emulator_events == emulator_before + 2 && last_key == KEYCODE_w && last_pressed);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_a, KEYCODE_a, 0);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 9, KEYCODE_w, 0);
  assert(emulator_events == emulator_before + 4 && last_key == KEYCODE_w && !last_pressed);

  ui_enabled = 1;
  menu_before = menu_events;
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_Return, KEYCODE_Return, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 8, KEYCODE_3, 1);
  assert(menu_events == menu_before + 2 && last_key == KEYCODE_3 && last_pressed);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_Return, KEYCODE_Return, 0);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 8, KEYCODE_3, 0);
  assert(menu_events == menu_before + 4 && last_key == KEYCODE_3 && !last_pressed);

  ui_enabled = 0;
  emulator_before = emulator_events;
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_LeftShift, KEYCODE_LeftShift, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 15, KEYCODE_LeftShift, 1);
  assert(emulator_events == emulator_before + 1 && last_key == KEYCODE_LeftShift && last_pressed);
  keyboard_router_physical_key(KEYBOARD_SOURCE_USB, KEYCODE_LeftShift, KEYCODE_LeftShift, 0);
  assert(emulator_events == emulator_before + 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 15, KEYCODE_LeftShift, 0);
  assert(emulator_events == emulator_before + 2 && last_key == KEYCODE_LeftShift && !last_pressed);

  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 64, KEYCODE_PageUp, 1);
  keyboard_router_physical_key(KEYBOARD_SOURCE_GPIO, 64, KEYCODE_PageUp, 0);
  assert(emulator_events == emulator_before + 4 && last_key == KEYCODE_PageUp && !last_pressed);

  return 0;
}