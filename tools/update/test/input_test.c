//
// input_test.c
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Exercises the updater's input through src/update/update_host.h, for
// run_tests.py.
//

#include <assert.h>
#include <stdio.h>

#include "circle.h"
#include "keycodes.h"
#include "ui.h"
#include "update_host.h"

static unsigned long now;
static int event_ready, event_pressed;
static long event_key;

unsigned long circle_get_ticks() { return now; }
void ui_set_external_owner(int active) { (void)active; }
int ui_read_key_event(long *key, int *pressed) {
  if (!event_ready) return 0;
  *key = event_key;
  *pressed = event_pressed;
  event_ready = 0;
  return 1;
}

static void event(long key, int pressed) {
  assert(!event_ready);
  event_key = key;
  event_pressed = pressed;
  event_ready = 1;
}

static void expect(int expected_key, int expected_pressed) {
  int key, pressed;
  assert(uh_read_key(&key, &pressed));
  assert(key == expected_key && pressed == expected_pressed);
}

static void expect_none(void) {
  int key, pressed;
  assert(!uh_read_key(&key, &pressed));
}

int main(void) {
  uh_take_screen(1);
  event(KEYCODE_Down, 1);
  expect(UH_KEY_DOWN, 1);
  now = 399000;
  expect_none();
  now = 401000;
  expect(UH_KEY_DOWN, 1);
  expect_none();
  now = 460000;
  expect_none();
  now = 462000;
  expect(UH_KEY_DOWN, 1);
  now = 800000;
  expect(UH_KEY_DOWN, 1);
  expect_none();
  event(KEYCODE_Down, 0);
  expect(UH_KEY_DOWN, 0);
  now = 1000000;
  expect_none();

  event(KEYCODE_Up, 1);
  expect(UH_KEY_UP, 1);
  now = 1401000;
  expect(UH_KEY_UP, 1);
  now = 1500000;
  event(KEYCODE_Up, 0);
  expect(UH_KEY_UP, 0);
  expect_none();

  // Joystick and keyboard both holding Down: repeat until both release.
  now = 2000000;
  event(KEYCODE_Down, 1);
  expect(UH_KEY_DOWN, 1);
  event(KEYCODE_Down, 1);
  expect(UH_KEY_DOWN, 1);
  event(KEYCODE_Down, 0);
  expect(UH_KEY_DOWN, 0);
  now = 2401000;
  expect(UH_KEY_DOWN, 1);
  event(KEYCODE_Down, 0);
  expect(UH_KEY_DOWN, 0);
  now = 3000000;
  expect_none();

  // A stray release never underflows.
  event(KEYCODE_Up, 0);
  expect(UH_KEY_UP, 0);
  event(KEYCODE_Up, 1);
  expect(UH_KEY_UP, 1);
  event(KEYCODE_Up, 0);
  expect(UH_KEY_UP, 0);
  now = 4000000;
  expect_none();

  // Other keys don't repeat.
  event(KEYCODE_Return, 1);
  expect(UH_KEY_SELECT, 1);
  event(KEYCODE_Left, 1);
  expect(UH_KEY_LEFT, 1);
  now = 5000000;
  expect_none();
  event(KEYCODE_Return, 0);
  expect(UH_KEY_SELECT, 0);
  event(KEYCODE_Left, 0);
  expect(UH_KEY_LEFT, 0);

  // Taking or leaving the screen forgets held directions.
  event(KEYCODE_Down, 1);
  expect(UH_KEY_DOWN, 1);
  uh_take_screen(1);
  now = 6000000;
  expect_none();
  event(KEYCODE_Down, 1);
  expect(UH_KEY_DOWN, 1);
  uh_take_screen(0);
  now = 7000000;
  expect_none();

  puts("updater input tests passed");
  return 0;
}
