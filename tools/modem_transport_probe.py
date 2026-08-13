#!/usr/bin/env python3
"""Deterministic TCP peer for diagnosing the BMC64 Hayes modem path."""

import argparse
import socket
import struct
import sys
import time


DELETE = b"\x14"
WIDTH_40 = b"4"
BURST_SIZE = 2048
FRAGMENT_SIZES = (1, 3, 3, 237)


def receive_exact(connection, expected, timeout):
    connection.settimeout(timeout)
    received = connection.recv(1)
    print("received: {}".format(received.hex()))
    if received != expected:
        raise RuntimeError(
            "expected {}, got {}".format(expected.hex(), received.hex())
        )


def send_all(connection, data):
    connection.sendall(data)
    print("sent: {} bytes".format(len(data)))


def send_fragmented(connection, data):
    offset = 0
    for size in FRAGMENT_SIZES:
        send_all(connection, data[offset:offset + size])
        offset += size
        time.sleep(0.12)
    send_all(connection, data[offset:])


def print_tcp_info(connection, label):
    info = connection.getsockopt(socket.IPPROTO_TCP, socket.TCP_INFO, 44)
    values = struct.unpack("8B9I", info)
    print("tcp {}: state={} unacked={} retrans={}".format(
        label, values[0], values[12], values[15]))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="0.0.0.0",
                        help="LAN address to bind (default: all interfaces)")
    parser.add_argument("--port", type=int, default=6502,
                        help="TCP port to listen on (default: 6502)")
    parser.add_argument("--timeout", type=float, default=30.0,
                        help="seconds to wait for each C64 reply (default: 30)")
    parser.add_argument("--hold-open", type=float, default=15.0,
                        help="seconds to keep TCP connected after the burst "
                             "(default: 15)")
    parser.add_argument("--require-space", action="store_true",
                        help="wait for Space after the burst before replying")
    parser.add_argument("--fragmented-burst", action="store_true",
                        help="send the burst as small fragments before the "
                             "remaining payload")
    parser.add_argument("--tcp-nodelay", action="store_true",
                        help="disable Nagle on the accepted TCP connection")
    arguments = parser.parse_args()

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((arguments.host, arguments.port))
    listener.listen(1)
    print("listening on {}:{}".format(arguments.host, arguments.port))

    connection, address = listener.accept()
    print("connected: {}:{}".format(*address))
    with connection:
        if arguments.tcp_nodelay:
            connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            print("TCP_NODELAY enabled")
        send_all(connection, b"BMC64 transport probe\r\nPRESS DEL: ")
        receive_exact(connection, DELETE, arguments.timeout)

        send_all(connection, b"\r\nSELECT 40 COLUMNS: ")
        receive_exact(connection, WIDTH_40, arguments.timeout)

        # Printable PETSCII-safe payload makes every dropped byte visible.
        payload = bytes(65 + index % 26 for index in range(BURST_SIZE))
        burst = b"\r\nBEGIN BURST\r\n" + payload + b"\r\nEND BURST\r\n"
        if arguments.fragmented_burst:
            send_fragmented(connection, burst)
        else:
            send_all(connection, burst)
        print_tcp_info(connection, "after burst")
        time.sleep(2)
        if arguments.require_space:
            send_all(connection, b"PRESS SPACE TO CONTINUE: ")
            receive_exact(connection, b" ", arguments.timeout)
            send_all(connection, b"\r\nSPACE RECEIVED\r\n")
        else:
            send_all(connection, b"TRANSPORT PROBE COMPLETE\r\n")
        print("holding connection open for {:.1f} seconds".format(
            arguments.hold_open))
        time.sleep(arguments.hold_open)
        print_tcp_info(connection, "before close")

    listener.close()
    print("complete")


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError) as error:
        print("probe failed: {}".format(error), file=sys.stderr)
        sys.exit(1)