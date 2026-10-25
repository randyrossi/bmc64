# Supplied mock HID firmware

[mock_hid.uf2](mock_hid.uf2) is the ready-to-flash firmware for a
**Raspberry Pi Pico (RP2040)**. It exposes a USB keyboard, mouse and gamepad
controlled through UART0 on GP0/GP1 at 115200 baud, 8N1.

See the [fixture instructions](../README.md) for wiring, manual BOOTSEL
flashing, source builds, UART protocol and native tests. The UF2 is not for
a Pico 2 or a BMC64 Raspberry Pi board.

## Build information

| Setting | Value |
| --- | --- |
| Board / platform | `pico` / `rp2040` |
| Configuration | Release |
| Pico SDK | 2.3.1 |
| Arm GCC | 9.2.1 |
| CMake | 3.28.3 |
| picotool | 2.3.1 |
| UF2 size | 66,560 bytes |
| Protocol | Version 1 |

SHA-256 of the supplied UF2:

```text
d9ae30244bbac4ddd2cd0cb0378c16961f49a0a1fcddb5ff204e068115a91214
```

From this directory, verify it with:

```sh
sha256sum mock_hid.uf2
```

The UF2 was rebuilt from the accompanying sources and is byte-for-byte
identical to the image used for the validated Pi3A+ C64 PAL smoke test.
Different SDK/toolchain versions are not guaranteed to produce identical
bytes.

## Updating the supplied image

Follow [the source build instructions](../README.md#build-firmware-from-source).
From `mock_hid/`, after rebuilding and running `make test all`:

```sh
cp build/mock_hid.uf2 bin/mock_hid.uf2
sha256sum bin/mock_hid.uf2
```

Update the checksum, size and tool versions above together with the UF2.
Keep ELF/debug files, native executables and CMake/SDK build products in
the ignored build directories, not here.
