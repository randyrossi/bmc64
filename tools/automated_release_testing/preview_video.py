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

"""Open the configured HDMI video input in FFplay without controlling BMC64."""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

from run_smoke import HERE, load_setup, parser


def preview_command(video: str) -> list[str]:
    return [
        "ffplay", "-window_title", "BMC64 HDMI video test",
        "-f", "v4l2", "-input_format", "mjpeg",
        "-video_size", "1280x720", "-framerate", "30",
        "-fflags", "nobuffer", "-flags", "low_delay", "-framedrop", "-an",
        "-i", video,
    ]


def main(argv: list[str] | None = None) -> int:
    arguments = argparse.ArgumentParser(description=__doc__)
    arguments.add_argument("--config", type=Path, default=HERE / "test_setup.json")
    arguments.add_argument("--video", help="override video device without loading saved setup")
    args = arguments.parse_args(argv)
    try:
        if args.video:
            video = args.video
        else:
            settings = load_setup(args.config)
            video = settings.get("video", parser().get_default("video"))
        if not isinstance(video, str) or not Path(video).is_absolute():
            raise ValueError("video device must be an absolute path")
        if shutil.which("ffplay") is None:
            raise RuntimeError("ffplay is required but was not found on PATH")
        print(f"Opening HDMI video: {video}", flush=True)
        print("You may now power on the Pi and check its display.", flush=True)
        print("Press Q or close the preview BEFORE running run_tests.", flush=True)
        print("Video only: no UART, HTTP, USB input or audio capture.", flush=True)
        return subprocess.run(preview_command(video), check=False).returncode
    except KeyboardInterrupt:
        return 130
    except (OSError, ValueError, RuntimeError) as error:
        print(f"Video preview failed: {error}", file=sys.stderr)
        print("Run configure_test_setup.py or supply --video /dev/VIDEO_DEVICE.",
              file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
