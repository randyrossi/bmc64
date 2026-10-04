#include "keyboard_feature.h"

#if BMC64_NEW_KEYBOARD_INPUT

#include <assert.h>

#include "circle.h"
#include "emux_api.h"
#include "joy.h"
#include "kbd.h"
#include "keycodes.h"
#include "menu_switch.h"
#include "ui.h"
#include "keyboard_router.h"

enum { HOTKEY_COUNT = 8, HOTKEYS_PER_MODIFIER = 4, PHYSICAL_KEYS = 0x108 };

static struct {
	long emitted_key;
	int held;
} physical_keys[2][PHYSICAL_KEYS];
static unsigned output_holds[PHYSICAL_KEYS];
static unsigned char output_to_ui[PHYSICAL_KEYS];

static struct {
	long key;
	int action;
	int armed;
	int held;
} hotkeys[HOTKEY_COUNT];

static int commodore_down;
static int control_down;
static int host_modifiers;
static int safe_reset_armed;
static unsigned long video_reset_start;


static int host_modifier(long key) {
	switch (key) {
	case KEYCODE_LeftShift: return EMUX_KEY_MOD_LSHIFT;
	case KEYCODE_RightShift: return EMUX_KEY_MOD_RSHIFT;
	case KEYCODE_LeftControl: return EMUX_KEY_MOD_LCTRL;
	case KEYCODE_RightControl: return EMUX_KEY_MOD_RCTRL;
	case KEYCODE_LeftAlt: return EMUX_KEY_MOD_LALT;
	case KEYCODE_RightAlt: return EMUX_KEY_MOD_RALT;
	default: return 0;
	}
}

void kbd_set_hotkey_function(unsigned int slot, long key, int function) {
	if (slot >= HOTKEY_COUNT) {
		return;
	}
	hotkeys[slot].key = key;
	hotkeys[slot].action = function;
	hotkeys[slot].armed = 0;
	hotkeys[slot].held = 0;
}

static int joystick_key(long key, int pressed) {
	for (unsigned port = 0; port < 2; port++) {
		int device = joydevs[port].device;
		if (device == JOYDEV_NUMS_1 || device == JOYDEV_NUMS_2 ||
				device == JOYDEV_CURS_SP || device == JOYDEV_CURS_LC ||
				device == JOYDEV_KEYSET1 || device == JOYDEV_KEYSET2) {
			if (pressed ? joy_key_down(port, key) : joy_key_up(port, key)) {
				return 1;
			}
		}
	}
	return 0;
}

static int hotkey_press(long key) {
	for (unsigned slot = 0; slot < HOTKEY_COUNT; slot++) {
		int modifier_down = slot < HOTKEYS_PER_MODIFIER ? commodore_down : control_down;
		if (modifier_down && hotkeys[slot].key == key) {
			hotkeys[slot].armed = 1;
			hotkeys[slot].held = 1;
			return 1;
		}
	}
	return 0;
}

static int hotkey_release(long key) {
	for (unsigned slot = 0; slot < HOTKEY_COUNT; slot++) {
		if (!hotkeys[slot].held || hotkeys[slot].key != key) {
			continue;
		}
		hotkeys[slot].held = 0;
		if (!hotkeys[slot].armed) {
			return 1;
		}
		switch (hotkeys[slot].action) {
		case BTN_ASSIGN_WARP:
		case BTN_ASSIGN_SWAP_PORTS:
		case BTN_ASSIGN_STATUS_TOGGLE:
		case BTN_ASSIGN_CART_FREEZE:
		case BTN_ASSIGN_ACTIVE_DISPLAY:
		case BTN_ASSIGN_PIP_LOCATION:
		case BTN_ASSIGN_PIP_SWAP:
		case BTN_ASSIGN_40_80_COLUMN:
		case BTN_ASSIGN_FLUSH_DISK:
			hotkeys[slot].armed = 0;
			emu_quick_func_interrupt(hotkeys[slot].action);
			break;
		default:
			break;
		}
		return 1;
	}
	return 0;
}

static void finish_hotkeys(void) {
	for (unsigned slot = 0; slot < HOTKEY_COUNT; slot++) {
		if (!hotkeys[slot].armed) {
			continue;
		}
		hotkeys[slot].armed = 0;
		switch (hotkeys[slot].action) {
		case BTN_ASSIGN_MENU:
			circle_lock_acquire();
			ui_toggle_pending = 2;
			circle_lock_release();
			break;
		case BTN_ASSIGN_RESET_MENU:
		case BTN_ASSIGN_RESET_HARD:
		case BTN_ASSIGN_RESET_SOFT:
		case BTN_ASSIGN_TAPE_MENU:
		case BTN_ASSIGN_CART_MENU:
			emu_quick_func_interrupt(hotkeys[slot].action);
			break;
		default:
			break;
		}
	}
}

static void release_key(long key, int to_ui);

void keyboard_router_physical_key(keyboard_source_t source,
																	unsigned physical_id, long keycode,
																	int pressed) {
	if ((source != KEYBOARD_SOURCE_USB && source != KEYBOARD_SOURCE_GPIO) ||
			physical_id >= PHYSICAL_KEYS ||
			(pressed && (keycode < 0 || keycode >= PHYSICAL_KEYS))) {
		assert(0);
		return;
	}

	if (pressed) {
		if (physical_keys[source][physical_id].held) {
			return;
		}
		physical_keys[source][physical_id].emitted_key = keycode;
		physical_keys[source][physical_id].held = 1;
		if (output_holds[keycode]++ == 0) {
			output_to_ui[keycode] = ui_enabled != 0;
			emu_key_pressed(keycode);
		}
	} else if (physical_keys[source][physical_id].held) {
		long emitted_key = physical_keys[source][physical_id].emitted_key;
		physical_keys[source][physical_id].held = 0;
		assert(output_holds[emitted_key] > 0);
		if (--output_holds[emitted_key] == 0) {
			release_key(emitted_key, output_to_ui[emitted_key]);
		}
	}
}

void emu_key_pressed(long key) {
	if (raw_keycode_func) {
		return;
	}
	host_modifiers |= host_modifier(key);

	if (key == commodore_key_sym) {
		commodore_down = 1;
	} else if (key == ctrl_key_sym) {
		control_down = 1;
	} else if (key == KEYCODE_F7 && commodore_down) {
		safe_reset_armed = 1;
		video_reset_start = circle_get_ticks();
	}

	if (joystick_key(key, 1) || hotkey_press(key)) {
		return;
	}

	if (ui_enabled) {
		emu_ui_key_interrupt(key, 1);
	} else {
		emux_key_interrupt_mod(key, 1, host_modifiers);
	}
}

static void release_key(long key, int to_ui) {
	if (raw_keycode_func) {
		raw_keycode_func(key);
		return;
	}
	host_modifiers &= ~host_modifier(key);

	if (key == commodore_key_sym) {
		commodore_down = 0;
	} else if (key == ctrl_key_sym) {
		control_down = 0;
	} else if (key == KEYCODE_F7 && commodore_down && safe_reset_armed &&
						 circle_get_ticks() - video_reset_start >= 5000000UL) {
		// Safe mode writes files; do it on the main loop, not in the interrupt.
		emu_safe_mode_interrupt();
	}

	if (key == KEYCODE_F7) {
		safe_reset_armed = 0;
	}

	if (key != KEYCODE_F12 && (joystick_key(key, 0) || hotkey_release(key))) {
		return;
	}

	if (to_ui) {
		emu_ui_key_interrupt(key, 0);
	} else {
		emux_key_interrupt_mod(key, 0, host_modifiers);
	}

	if (key == KEYCODE_F12 && !to_ui) {
		circle_lock_acquire();
		ui_toggle_pending = 2;
		circle_lock_release();
	}
	if (key == commodore_key_sym || key == ctrl_key_sym) {
		finish_hotkeys();
	}
}

void emu_key_released(long key) {
	release_key(key, ui_enabled);
}

#endif