#!/usr/bin/env python3
#
# BMC64 - A bare metal Commodore emulator for Raspberry Pi
# Copyright (C) 2026 aminch <https://github.com/aminch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

from .base import BootTypingTest
from .layouts import Layout


class Test(BootTypingTest):
    name = "pet-boot-typing"
    description = "Pet: boot into BASIC and verify USB typing"
    machine = "Pet"
    order = 60
    capture_size = (1920, 1080)
    # Use the 40-column 3016 profile: 80-column glyphs exceed the capture's
    # resolving power. Its graphics keyboard boots with the first (uppercase) set.
    layout = Layout(
        columns=40, rows=25, input_row=5, channel=1, inverted=False,
        font_path="/PET/chargen", font_offset=0,
        screen=(
            (0, "### COMMODORE BASIC ###"),
            (2, " 15359 BYTES FREE"),
            (4, "READY."),
        ),
        # No pane edge: the PAL entry centres 320x200 text in a 376x270 frame.
        native_frame_height=270,
    )

    def before_test(self) -> None:
        super().before_test()
        self.prepare_screen_reader(self.layout, self.capture_size, token_length=8)
        self.start_profile(fresh=True)
        self.open_native_capture()

    def run_test(self) -> None:
        self.assert_boot_screen()
        self.wait_for_keyboard()
        self.type_usb_token()
        self.assert_typed_token()
        self.assert_profile_unchanged()

    def after_test(self) -> None:
        super().after_test()
