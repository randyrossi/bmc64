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
    name = "c128-boot-typing"
    description = "C128: boot into BASIC and verify USB typing"
    machine = "C128"
    order = 20
    capture_size = (1280, 720)
    layout = Layout(
        columns=40, rows=25, input_row=7, channel=1, inverted=False,
        font_path="/C128/chargen", font_offset=0,
        screen=(
            (1, " COMMODORE BASIC V7.0 122365 BYTES FREE"),
            (2, "   (C)1986 COMMODORE ELECTRONICS, LTD."),
            (3, "         (C)1977 MICROSOFT CORP."),
            (4, "           ALL RIGHTS RESERVED"),
            (6, "READY."),
        ),
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
