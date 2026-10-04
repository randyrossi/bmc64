#!/usr/bin/env bash
set -euo pipefail

OPTIONS=()
if [[ $# -eq 1 && "$1" == --mouse-interaction ]]; then
    OPTIONS=(--mouse-interaction)
elif [[ $# -gt 0 ]]; then
    echo "Usage: $0 [--mouse-interaction] (one Logitech Bolt 046d:c548 receiver)" >&2
    exit 1
fi
for tool in cc lsusb sudo tee; do
    if ! command -v "${tool}" >/dev/null 2>&1; then
        echo "Required tool not found: ${tool}" >&2
        exit 1
    fi
done

DEVICES="$(lsusb -d 046d:c548 || true)"
COUNT="$(awk 'NF { n++ } END { print n+0 }' <<<"${DEVICES}")"
if [[ "${COUNT}" -ne 1 ]]; then
    echo "Expected exactly one Bolt 046d:c548 receiver; found ${COUNT}." >&2
    exit 1
fi
BUS="$(awk '{print $2}' <<<"${DEVICES}")"
DEVICE="$(awk '{gsub(/:/, "", $4); print $4}' <<<"${DEVICES}")"
USB_PATH="/dev/bus/usb/${BUS}/${DEVICE}"
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$(mktemp -d)"
cleanup() {
    rm -f -- "${BUILD_DIR}/probe"
    rmdir -- "${BUILD_DIR}"
}
trap cleanup EXIT
cc -std=c11 -O2 -Wall -Wextra -Werror \
    "${SCRIPT_DIR}/keyboard/bolt_boot_probe.c" -o "${BUILD_DIR}/probe"

LOG="${HOME}/keyboard-046d-c548-boot-$(date +%Y%m%d-%H%M%S).txt"
echo "Receiver: ${USB_PATH}"
echo "Output: ${LOG}"
if [[ ${#OPTIONS[@]} -gt 0 ]]; then
    echo "This temporarily disconnects the receiver's keyboard AND mouse interfaces from Linux."
    echo "Two additional 20-second phases test mouse report mode and keyboard boot restoration."
else
    echo "This temporarily disconnects only the receiver's keyboard interface from Linux."
fi
echo "Use a different keyboard for terminal input; select the Bolt channel on the test keyboard."
echo "Three 20-second phases: report mode, boot mode, then Circle-sized boot reads."
echo "Repeat K, L, F12, A, Shift+A, Up and Caps Lock during EACH phase. Do not type passwords."
echo "On exit it restores the original protocol and reconnects the Linux driver."
echo "If interrupted by power loss or forced kill, unplug/replug the receiver."
read -r -p "Press Enter on the other keyboard when ready..."
sudo -v
sudo "${BUILD_DIR}/probe" "${USB_PATH}" "${OPTIONS[@]}" 2>&1 | tee "${LOG}"
