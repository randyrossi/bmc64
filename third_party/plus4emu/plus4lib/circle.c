#include "circle.h"

// TODO: Temporary until joy moves to common.
struct joydev_config joydevs[MAX_JOY_PORTS];


int emu_add_button_values(int dev, unsigned button_value) {
  return 0;
}

int emu_button_function(int device, int button_num, unsigned buttons,
                        int* btn_assignment, int* is_press) {
  return 0;
}

void emu_mouse_move(int x, int y) { }
void emu_mouse_button_left(int pressed) { }
void emu_mouse_button_right(int pressed) { }
void emu_mouse_button_middle(int pressed) { }
void emu_mouse_wheel_up(int pressed) { }
void emu_mouse_wheel_down(int pressed) { }

void emu_joy_interrupt_abs(int port, int device,
                           int js_up,
                           int js_down,
                           int js_left,
                           int js_right,
                           int js_fire,
                           int pot_x,
                           int pot_y) {
}

void emu_quick_func_interrupt(int button_assignment) {
}

int emu_get_gpio_config(void) {
  return 0;
}

void emu_set_joy_usb_interrupt(unsigned device, int value) {
}

long emu_get_key_binding(int index) {
  return 0;
}

void emu_key_pressed(long key) {
}

void emu_key_released(long key) {
}

int emu_get_num_joysticks(void) {
  return 0;
}

void emu_set_demo_mode(int is_demo) {
}

int emu_is_ui_activated(void) {
  return 0;
}

void emu_ui_key_interrupt(long key, int pressed) {
}

void emu_get_usb_pref(int device, int *usb_pref, int *x_axis, int *y_axis,
                      float *x_thresh, float *y_thresh) {
}

void emu_set_gamepad_info(int num_pads,
                          int num_buttons[2],
                          int axes[2],
                          int hats[2]) {
}

int emu_wants_raw_usb(void) {
  return 0;
}

void emu_set_raw_usb(int device,
                     unsigned buttons,
                     const int hats[6],
                     const int axes[16]) {
}
