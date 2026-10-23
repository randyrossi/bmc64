#!/usr/bin/env python3
#
# modem_command_probe.py
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

"""TCP peer for manually verifying BMC64 Hayes/ZiModem command sessions."""

import argparse
import socket
import sys


def describe(data):
    text = data.decode("ascii", errors="backslashreplace")
    return "{} ({})".format(repr(text), data.hex())


def serve_connection(listener, number, timeout):
    listener.settimeout(timeout)
    connection, address = listener.accept()
    print("connection {} opened from {}:{}".format(number, *address))
    with connection:
        connection.sendall(
            "BMC64 command probe connection {}\r\n".format(number).encode("ascii")
        )
        connection.settimeout(timeout)
        while True:
            data = connection.recv(256)
            if not data:
                print("connection {} closed by modem".format(number))
                return
            print("connection {} received {}".format(number, describe(data)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="0.0.0.0",
                        help="LAN address to bind (default: all interfaces)")
    parser.add_argument("--port", type=int, default=6503,
                        help="TCP port to listen on (default: 6503)")
    parser.add_argument("--connections", type=int, default=2,
                        help="number of dial/hangup sessions to verify (default: 2)")
    parser.add_argument("--timeout", type=float, default=60.0,
                        help="seconds to wait for each connection or activity")
    arguments = parser.parse_args()

    if arguments.connections < 1:
        parser.error("--connections must be at least 1")

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((arguments.host, arguments.port))
    listener.listen(1)
    print("listening on {}:{}".format(arguments.host, arguments.port))
    try:
        for number in range(1, arguments.connections + 1):
            serve_connection(listener, number, arguments.timeout)
    finally:
        listener.close()
    print("command probe complete")


if __name__ == "__main__":
    try:
        main()
    except (OSError, socket.timeout) as error:
        print("command probe failed: {}".format(error), file=sys.stderr)
        sys.exit(1)