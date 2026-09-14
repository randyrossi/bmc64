// main.c - keyrah_test
//
// A TinyUSB mock of the Individual Computers Keyrah V3's USB behaviour: a
// composite device presenting 2 boot-protocol keyboard interfaces, 2
// joystick-shaped generic-HID interfaces, and 2 more generic-HID interfaces
// (Consumer Control / System Control usages), used to exercise a host's USB
// keyboard/gamepad handling without needing the real hardware.
//
// Its Shift-Lock model:
//   - a ground-truth "locked" flag stands in for the C64's mechanically
//     locking Shift-Lock key
//   - a lock transition sends a synthesized Caps-Lock (0x39) TAP (make then
//     break), never a held key
//   - it watches the host's returned Caps-Lock LED bit; if it doesn't match
//     the lock state, it waits 100ms and sends another tap, with no retry
//     limit, until the LED matches
//   - optional contact-bounce simulation adds a few spurious extra events
//     (Left Shift and/or Caps-Lock) around the transition
//
// Controls:
//   - GPIO button on PIN_BTN_TYPE: send one keypress (letter 'a') on ITF_KBD1
//   - GPIO button on PIN_BTN_LOCK: toggle Shift-Lock (press once to engage,
//     again to disengage)
//   - USB CDC console (a second serial port after flashing) - type 'h' for
//     a full command list: change the LED response mode, toggle contact-
//     bounce simulation, run an unattended soak test, adjust the retry
//     interval, and check status.
//
// bInterval for every HID endpoint is set at build time via
// KEYRAH_POLL_INTERVAL_MS (see CMakeLists.txt / usb_descriptors.h) and is not
// touched at runtime.

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "pico/stdlib.h"
#include "bsp/board_api.h"   // older TinyUSB: "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// Pins
//--------------------------------------------------------------------+

#define PIN_BTN_TYPE   2   // simulate typing a letter
#define PIN_BTN_LOCK   3   // simulate the physical C64 Shift-Lock key

//--------------------------------------------------------------------+
// Console (CDC) helpers
//--------------------------------------------------------------------+

static void console_printf(const char *fmt, ...) {
  char buf[160];
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  if (n <= 0) return;
  if ((size_t) n >= sizeof(buf)) n = sizeof(buf) - 1;

  if (tud_cdc_connected()) {
    tud_cdc_write(buf, (uint32_t) n);
    tud_cdc_write_flush();
  }
  // Always echo to the UART too (see CMakeLists.txt: pico_enable_stdio_uart).
  printf("%s", buf);
}

//--------------------------------------------------------------------+
// Keyboard report helpers
//--------------------------------------------------------------------+

// tud_hid_n_ready() is false whenever the endpoint's previous report hasn't
// been picked up by the host yet - momentary and expected, not an error by
// itself. Wait for it (servicing tud_task() while we do) instead of dropping
// the report, and only give up - loudly - after a generous timeout.
static bool hid_wait_ready(uint8_t itf, uint32_t timeout_ms) {
  uint32_t start = to_ms_since_boot(get_absolute_time());
  while (!tud_hid_n_ready(itf)) {
    tud_task();
    if (to_ms_since_boot(get_absolute_time()) - start >= timeout_ms) return false;
  }
  return true;
}

static void kbd_press(uint8_t itf, uint8_t modifier, uint8_t keycode) {
  if (!hid_wait_ready(itf, 50)) {
    console_printf("[key] WARNING: itf=%u still not ready after 50ms, press DROPPED\r\n", itf);
    return;
  }
  uint8_t keycodes[6] = {0};
  if (keycode) keycodes[0] = keycode;
  tud_hid_n_keyboard_report(itf, 0, modifier, keycodes);
}

static void kbd_release(uint8_t itf) {
  if (!hid_wait_ready(itf, 50)) {
    console_printf("[key] WARNING: itf=%u still not ready after 50ms, release DROPPED\r\n", itf);
    return;
  }
  tud_hid_n_keyboard_report(itf, 0, 0, NULL);
}

// A plain sleep_ms() here would stop tud_task() from running for the whole
// wait - long enough (a Shift-Lock action does several of these back to
// back) to matter for USB responsiveness. Keep servicing it while we wait
// instead. Does not touch the UART hardware directly - stdio_init_all()
// already owns it via pico-sdk's own interrupt-driven stdio_uart driver, and
// reading the same peripheral through two APIs at once is unsafe.
static void delay_ms_servicing_io(uint32_t ms) {
  uint32_t start = to_ms_since_boot(get_absolute_time());
  while (to_ms_since_boot(get_absolute_time()) - start < ms) {
    tud_task();
  }
}

// One visible "key was pressed and released" event. The hold time (between
// press and release) needs to be longer than one host video frame period
// (~16.7-20ms) - a host that only applies queued key events to its emulated
// keyboard matrix once per frame can otherwise drain both the press and the
// release in the same pass, with no emulated time between them, and never
// see the key as having been down at all.
static void kbd_tap(uint8_t itf, uint8_t modifier, uint8_t keycode) {
  kbd_press(itf, modifier, keycode);
  delay_ms_servicing_io(50);
  kbd_release(itf);
  delay_ms_servicing_io(15);
}

//--------------------------------------------------------------------+
// Shift-Lock state machine
//--------------------------------------------------------------------+

typedef enum { LED_FAST, LED_SLOW, LED_SILENT } led_mode_t;

static volatile led_mode_t led_mode        = LED_FAST;
static volatile uint32_t   led_slow_ms     = 200;    // LED_SLOW delay
static volatile uint32_t   led_silent_ms   = 5000;   // LED_SILENT delay (0xFFFFFFFF = until reset)

static bool     bounce_sim      = false;   // simulate contact bounce around a lock transition
static bool     soak_mode       = false;   // auto-toggle Shift-Lock unattended
static uint32_t soak_period_ms  = 2000;

static bool     turbo_mode      = false;   // fast-typist stress test, cycling A-Z
static uint32_t turbo_period_ms = 100;      // ~600 chars/min between key starts

// Whether the Shift-Lock tap goes out on just ukbd1, or on both ukbd1 and
// ukbd2. The real Keyrah sends it on one keyboard interface only - the
// second interface is used for extra simultaneous keys once more are held
// than fit in one boot-protocol report (5), not for duplicating Shift-Lock
// taps. TAP_KBD1_ONLY is the default; TAP_BOTH_KBD is kept as a switchable
// option for testing how a host reacts to a duplicated tap on both
// interfaces.
typedef enum { TAP_KBD1_ONLY, TAP_BOTH_KBD } tap_mode_t;
static tap_mode_t tap_mode = TAP_KBD1_ONLY;

static bool     shift_locked    = false;   // ground truth, mirrors the mechanical latch
static bool     host_caps_led   = false;   // last Caps-Lock bit the host reported back
static bool     pending_sync    = false;   // we're still waiting for host_caps_led == shift_locked
static uint32_t retry_interval_ms = 100;   // matches the real Keyrah's retry interval
static uint32_t last_attempt_ms = 0;
static uint32_t sync_attempts   = 0;       // taps sent so far for the current lock transition (1-based)

// One Caps-Lock tap, i.e. one attempt at telling the host the current
// shift_locked state. Called by both the initial action and every retry -
// from the host's point of view they're indistinguishable. sync_attempts is
// incremented before sending (not after) so it reads correctly as "attempt
// #N" even when tud_hid_set_report_cb() reports back before this returns.
static void send_capslock_tap(void) {
  sync_attempts++;
  last_attempt_ms = to_ms_since_boot(get_absolute_time());

  if (bounce_sim) {
    // A couple of spurious Left-Shift blips before settling, simulating
    // contact bounce around the Shift-Lock transition.
    kbd_press(ITF_KBD1, KEYBOARD_MODIFIER_LEFTSHIFT, 0);   // spurious left-shift blip
    delay_ms_servicing_io(4);
    kbd_release(ITF_KBD1);
    delay_ms_servicing_io(4);
  }

  kbd_tap(ITF_KBD1, 0, HID_KEY_CAPS_LOCK);
  if (tap_mode == TAP_BOTH_KBD) {
    kbd_tap(ITF_KBD2, 0, HID_KEY_CAPS_LOCK);
  }

  if (bounce_sim) {
    kbd_press(ITF_KBD1, KEYBOARD_MODIFIER_LEFTSHIFT, 0);   // spurious left-shift blip on release too
    delay_ms_servicing_io(4);
    kbd_release(ITF_KBD1);
  }
}

// Called when the physical Shift-Lock button is pressed - toggles the
// ground-truth state and starts (or restarts) the reconciliation sequence.
static void shiftlock_action(void) {
  shift_locked = !shift_locked;
  pending_sync = true;
  sync_attempts = 0;

  console_printf("[lock] switch now %s, sending CapsLock tap\r\n", shift_locked ? "LOCKED" : "unlocked");
  send_capslock_tap();
}

// Call every loop iteration. Waits retry_interval_ms; if still not in sync,
// sends another tap, with no retry limit.
static void retry_task(void) {
  if (!pending_sync) return;

  uint32_t now = to_ms_since_boot(get_absolute_time());
  if (now - last_attempt_ms < retry_interval_ms) return;

  console_printf("[lock] retry #%lu: host LED=%d, switch=%d - still out of sync, resending\r\n",
                 (unsigned long) sync_attempts, host_caps_led, shift_locked);
  send_capslock_tap();
}

//--------------------------------------------------------------------+
// HID callbacks
//--------------------------------------------------------------------+

// GET_REPORT is not used by a boot keyboard/gamepad in this test - stub it.
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type,
                                uint8_t *buffer, uint16_t reqlen) {
  (void) itf; (void) report_id; (void) report_type; (void) buffer; (void) reqlen;
  return 0;
}

// SET_REPORT - this is the LED write. Circle's CUSBKeyboardDevice::SetLEDs()
// sends exactly this: a 1-byte OUTPUT report, bit1 = Caps Lock.
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type,
                            uint8_t const *buffer, uint16_t bufsize) {
  (void) report_id;
  if (report_type != HID_REPORT_TYPE_OUTPUT || bufsize < 1) return;

  uint8_t led_byte = buffer[0];

  switch (led_mode) {
    case LED_FAST:
      break;
    case LED_SLOW:
      sleep_ms(led_slow_ms);
      break;
    case LED_SILENT:
      // Blocks the control transfer's STATUS stage from completing, to
      // simulate an unresponsive device. UINT32_MAX never returns short of a
      // reset.
      if (led_silent_ms == UINT32_MAX) {
        while (1) tight_loop_contents();
      }
      sleep_ms(led_silent_ms);
      break;
  }

  host_caps_led = (led_byte & 0x02) != 0;
  gpio_put(PICO_DEFAULT_LED_PIN, host_caps_led);   // mirror it on the onboard LED

  console_printf("[led] itf=%u byte=0x%02X (caps=%d) after %s\r\n",
                 itf, led_byte, host_caps_led,
                 led_mode == LED_FAST ? "fast" : led_mode == LED_SLOW ? "slow" : "silent");

  if (pending_sync && host_caps_led == shift_locked) {
    pending_sync = false;
    console_printf("[lock] in sync after %lu attempt(s)\r\n", (unsigned long) sync_attempts);
  }
}

//--------------------------------------------------------------------+
// Buttons
//--------------------------------------------------------------------+

static void buttons_init(void) {
  gpio_init(PIN_BTN_TYPE); gpio_set_dir(PIN_BTN_TYPE, GPIO_IN); gpio_pull_up(PIN_BTN_TYPE);
  gpio_init(PIN_BTN_LOCK); gpio_set_dir(PIN_BTN_LOCK, GPIO_IN); gpio_pull_up(PIN_BTN_LOCK);
}

static void buttons_task(void) {
  static bool prev_type = true, prev_lock = true;   // pulled-up idle = true

  bool type_now = gpio_get(PIN_BTN_TYPE);
  bool lock_now = gpio_get(PIN_BTN_LOCK);

  if (prev_type && !type_now) {
    console_printf("[key] tap 'a' on ukbd1\r\n");
    kbd_tap(ITF_KBD1, 0, HID_KEY_A);
  }

  if (prev_lock && !lock_now) {
    delay_ms_servicing_io(20);   // crude debounce
    if (!gpio_get(PIN_BTN_LOCK)) shiftlock_action();
  }

  prev_type = type_now;
  prev_lock = lock_now;
}

//--------------------------------------------------------------------+
// Soak test - unattended, repeated Shift-Lock cycling to try to catch
// something that only shows up after many repetitions.
//--------------------------------------------------------------------+

static void soak_task(void) {
  static uint32_t next_ms = 0;
  static bool do_shiftlock = true;
  if (!soak_mode) return;

  uint32_t now = to_ms_since_boot(get_absolute_time());
  if (now < next_ms) return;
  next_ms = now + soak_period_ms;

  if (do_shiftlock) {
    shiftlock_action();
  } else {
    kbd_tap(ITF_KBD1, KEYBOARD_MODIFIER_LEFTSHIFT, 0);
  }
  do_shiftlock = !do_shiftlock;
}

// Turbo test - unattended, fast cycling through A-Z plus modifiers and
// Shift-Lock (like a fast typist), instead of just one key or Shift-Lock
// alone, to see whether transfer error counts climb under sustained varied
// use. Every 8th action is something other than a plain letter, rotating
// through: proper Left Shift, Right Shift, the malformed Left Shift the
// real Keyrah sends (duplicate 0xE1 keycode), and a Shift-Lock toggle.
static void turbo_task(void) {
  static uint32_t next_ms = 0;
  static uint8_t letter = 0;
  static uint8_t action = 0;
  if (!turbo_mode) return;

  uint32_t now = to_ms_since_boot(get_absolute_time());
  if (now < next_ms) return;
  next_ms = now + turbo_period_ms;

  if (action % 8 == 7) {
    switch ((action / 8) % 4) {
      case 0: kbd_tap(ITF_KBD1, KEYBOARD_MODIFIER_LEFTSHIFT, 0);                     break;
      case 1: kbd_tap(ITF_KBD1, KEYBOARD_MODIFIER_RIGHTSHIFT, 0);                    break;
      case 2: kbd_tap(ITF_KBD1, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_SHIFT_LEFT);    break;
      case 3: shiftlock_action();                                                   break;
    }
  } else {
    kbd_tap(ITF_KBD1, 0, HID_KEY_A + (letter % 26));
    letter++;
  }
  action++;
}

//--------------------------------------------------------------------+
// Console commands
//--------------------------------------------------------------------+

static void print_status(void) {
  console_printf("--- status ---\r\n");
  console_printf("  shift_locked   : %d\r\n", shift_locked);
  console_printf("  host_caps_led  : %d\r\n", host_caps_led);
  console_printf("  pending_sync   : %d (attempts so far: %lu)\r\n", pending_sync, (unsigned long) sync_attempts);
  console_printf("  led_mode       : %s\r\n", led_mode == LED_FAST ? "fast" : led_mode == LED_SLOW ? "slow" : "silent");
  console_printf("  led_slow_ms    : %lu\r\n", (unsigned long) led_slow_ms);
  console_printf("  led_silent_ms  : %lu%s\r\n", (unsigned long) led_silent_ms,
                 led_silent_ms == UINT32_MAX ? " (forever, needs reset)" : "");
  console_printf("  retry_interval : %lu ms\r\n", (unsigned long) retry_interval_ms);
  console_printf("  tap_mode       : %s\r\n", tap_mode == TAP_BOTH_KBD ? "both (ukbd1+ukbd2)" : "ukbd1 only");
  console_printf("  bounce_sim     : %d\r\n", bounce_sim);
  console_printf("  soak_mode      : %d (period %lu ms)\r\n", soak_mode, (unsigned long) soak_period_ms);
  console_printf("  turbo_mode     : %d (period %lu ms)\r\n", turbo_mode, (unsigned long) turbo_period_ms);
  console_printf("  bInterval      : %d ms (all HID endpoints, set at build time)\r\n", KEYRAH_POLL_INTERVAL_MS);
}

static void print_help(void) {
  console_printf("\r\nkeyrah_test console\r\n");
  console_printf("  a       tap letter 'a' on ukbd1\r\n");
  console_printf("  w       tap letter 'a' on ukbd2\r\n");
  console_printf("  z       tap Left Shift on ukbd1\r\n");
  console_printf("  x       tap Left Shift on ukbd1 (with duplicate 0xE1 keycode, like the real Keyrah)\r\n");
  console_printf("  j       tap gamepad button A on upad1\r\n");
  console_printf("  l       toggle Shift-Lock (same as the physical button)\r\n");
  console_printf("  1/2/3   LED response mode: fast / slow / silent\r\n");
  console_printf("  +/-     adjust retry interval by 10ms\r\n");
  console_printf("  k       toggle Shift-Lock tap: both keyboard interfaces / ukbd1 only\r\n");
  console_printf("  b       toggle contact-bounce simulation\r\n");
  console_printf("  s       toggle unattended soak test (auto Shift-Lock every %lu ms)\r\n", (unsigned long) soak_period_ms);
  console_printf("  t       toggle turbo test (fast typing + shifts + Shift-Lock every %lu ms)\r\n", (unsigned long) turbo_period_ms);
  console_printf("  i       print status\r\n");
  console_printf("  h / ?   this help\r\n\r\n");
}

// Shared by both input paths - see console_task() below.
static void handle_console_char(int c) {
  // Case-insensitive on purpose: this mock toggles the host's global,
  // system-wide Caps-Lock state, which affects every keyboard attached to
  // the host, including whichever one is typing these console commands. If
  // Caps Lock is engaged on the host, letters typed here arrive uppercase.
  if (c >= 'A' && c <= 'Z') {
    c += ('a' - 'A');
  }

  switch (c) {
    case 'a':
      console_printf("[key] tap 'a' on ukbd1\r\n");
      kbd_tap(ITF_KBD1, 0, HID_KEY_A);
      break;

    // Same as 'a' but on ukbd2, for exercising that interface directly.
    case 'w':
      console_printf("[key] tap 'a' on ukbd2\r\n");
      kbd_tap(ITF_KBD2, 0, HID_KEY_A);
      break;

    case 'z':
      console_printf("[key] tap Left Shift on ukbd1\r\n");
      kbd_tap(ITF_KBD1, KEYBOARD_MODIFIER_LEFTSHIFT, 0);
      break;

    // The real Keyrah also duplicates Left Shift into the keycode array
    // (mod=0x02, keys=E1 00...) instead of signalling it via the modifier
    // byte alone. HID_KEY_SHIFT_LEFT (0xE1) is out of range for a plain
    // keycode consumer sized for the 0x00-0x7F usage range.
    case 'x':
      console_printf("[key] tap Left Shift on ukbd1 (with duplicate 0xE1 keycode)\r\n");
      kbd_tap(ITF_KBD1, KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_SHIFT_LEFT);
      break;

    case 'j': {
      console_printf("[joy] tap button A on upad1\r\n");
      hid_gamepad_report_t report = {0};
      report.buttons = GAMEPAD_BUTTON_A;
      if (hid_wait_ready(ITF_JOY1, 50)) {
        tud_hid_n_report(ITF_JOY1, 0, &report, sizeof(report));
      } else {
        console_printf("[joy] WARNING: itf=%u still not ready after 50ms, press DROPPED\r\n", ITF_JOY1);
      }
      delay_ms_servicing_io(15);
      memset(&report, 0, sizeof(report));
      if (hid_wait_ready(ITF_JOY1, 50)) {
        tud_hid_n_report(ITF_JOY1, 0, &report, sizeof(report));
      } else {
        console_printf("[joy] WARNING: itf=%u still not ready after 50ms, release DROPPED\r\n", ITF_JOY1);
      }
      break;
    }

    case 'l':
      shiftlock_action();
      break;

    case '1': led_mode = LED_FAST;   console_printf("[led] mode = fast\r\n");   break;
    case '2': led_mode = LED_SLOW;   console_printf("[led] mode = slow (%lu ms)\r\n", (unsigned long) led_slow_ms); break;
    case '3': led_mode = LED_SILENT; console_printf("[led] mode = silent (%lu ms)\r\n", (unsigned long) led_silent_ms); break;

    case '+':
      retry_interval_ms += 10;
      console_printf("[lock] retry interval = %lu ms\r\n", (unsigned long) retry_interval_ms);
      break;
    case '-':
      if (retry_interval_ms > 10) retry_interval_ms -= 10;
      console_printf("[lock] retry interval = %lu ms\r\n", (unsigned long) retry_interval_ms);
      break;

    case 'k':
      tap_mode = (tap_mode == TAP_BOTH_KBD) ? TAP_KBD1_ONLY : TAP_BOTH_KBD;
      console_printf("[cfg] Shift-Lock tap now sent on: %s\r\n",
                     tap_mode == TAP_BOTH_KBD ? "both ukbd1+ukbd2" : "ukbd1 only");
      break;

    case 'b':
      bounce_sim = !bounce_sim;
      console_printf("[cfg] contact-bounce simulation = %d\r\n", bounce_sim);
      break;

    case 's':
      soak_mode = !soak_mode;
      console_printf("[cfg] soak test = %d\r\n", soak_mode);
      break;

    case 't':
      turbo_mode = !turbo_mode;
      console_printf("[cfg] turbo test = %d\r\n", turbo_mode);
      break;

    case 'i':
      print_status();
      break;

    case 'h':
    case '?':
      print_help();
      break;

    default:
      break;
  }
}

// Reads from both the USB CDC console and the physical UART (TX=GP0, RX=GP1,
// plus GND), since only one can be reached depending on where the mock's USB
// is plugged in - see the README.
static void console_task(void) {
  if (tud_cdc_available()) {
    handle_console_char(tud_cdc_read_char());
  }

  int uc = getchar_timeout_us(0);
  if (uc != PICO_ERROR_TIMEOUT) {
    handle_console_char(uc);
  }
}

//--------------------------------------------------------------------+
// UART/console heartbeat - printed every 2s regardless of input, so you can
// confirm the console is alive without needing to type anything or reset
// the board (the startup banner only prints once, at boot).
//--------------------------------------------------------------------+

static void console_heartbeat_task(void) {
  static uint32_t next_ms = 0;
  uint32_t now = to_ms_since_boot(get_absolute_time());
  if (now < next_ms) return;
  next_ms = now + 2000;

  console_printf("[hb] alive, uptime=%lus, shift_locked=%d, host_caps_led=%d, led_mode=%s (type 'h' for help)\r\n",
                 (unsigned long) (now / 1000), shift_locked, host_caps_led,
                 led_mode == LED_FAST ? "fast" : led_mode == LED_SLOW ? "slow" : "silent");
}

//--------------------------------------------------------------------+
// Heartbeat LED (separate from the caps-lock-mirroring use of the same LED -
// only blinks while nothing has driven it via tud_hid_set_report_cb since the
// last update, so it does not fight the LED-state mirroring above).
//--------------------------------------------------------------------+

static void heartbeat_task(void) {
  static uint32_t next_ms = 0;
  static bool state = false;

  uint32_t now = to_ms_since_boot(get_absolute_time());
  if (now < next_ms) return;
  next_ms = now + 500;

  if (!host_caps_led) {   // don't fight the "LED mirrors host Caps-Lock" indicator
    state = !state;
    gpio_put(PICO_DEFAULT_LED_PIN, state);
  }
}

//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+

int main(void) {
  board_init();
  stdio_init_all();
  tusb_init();

  buttons_init();
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  console_printf("\r\nkeyrah_test starting - Keyrah V3 USB mock\r\n");
  print_help();

  while (1) {
    tud_task();

    buttons_task();
    console_task();
    retry_task();
    soak_task();
    turbo_task();
    heartbeat_task();
    console_heartbeat_task();
  }

  return 0;
}
