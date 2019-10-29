#include "circle.h"

// TODO: Temporary until joy moves to common.
struct joydev_config joydevs[MAX_JOY_PORTS];


int circle_add_button_values(int dev, unsigned button_value) {
  return 0;
}

int circle_add_pot_values(int *value, int potx, int poty) {
  return 0;
}

int circle_button_function(int device, int button_num, unsigned buttons,
                                  int* btn_assignment, int* is_press) {
  return 0;
}

void circle_mouse_move(int x, int y) { }
void circle_mouse_button_left(int pressed) { }
void circle_mouse_button_right(int pressed) { }
void circle_mouse_button_middle(int pressed) { }
void circle_mouse_button_up(int pressed) { }
void circle_mouse_button_down(int pressed) { }

void circle_emu_joy_interrupt(int type, int port, int device, int value) {
}

void circle_emu_quick_func_interrupt(int button_assignment) {
}

int circle_gpio_config(void) {
  return 0;
}

void circle_joy_usb(unsigned device, int value) {
}

long circle_key_binding(int slot) {
  return 0;
}

void circle_key_pressed(long key) {
}

void circle_key_released(long key) {
}

int circle_num_joysticks(void) {
  return 0;
}

void circle_set_demo_mode(int is_demo) {
}

int circle_ui_activated(void) {
  return 0;
}

void circle_ui_key_interrupt(long key, int pressed) {
}

void circle_usb_pref(int device, int *usb_pref, int *x_axis, int *y_axis,
                            float *x_thresh, float *y_thresh) {
}

void joy_set_gamepad_info(int num_pads, int num_buttons[2], int axes[2],
                                 int hats[2]) {
}

void menu_raw_usb(int device, unsigned buttons, const int hats[6],
                         const int axes[16]) {
}

int menu_wants_raw_usb(void) {
  return 0;
}
