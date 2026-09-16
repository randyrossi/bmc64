#!/usr/bin/env bash

set -euo pipefail

BUS="1"
while [[ $# -gt 0 ]]; do
    case "$1" in
        *)
            BUS="$1"
            ;;
    esac
    shift
done

STAMP="$(date +%Y%m%d-%H%M%S)"
DEVICE_NAME="8bitdo-keyboard"
USB_ID="2dc8:5201"
CAPTURE_NAME="${DEVICE_NAME}-${STAMP}"
TEMP_CAPTURE="/tmp/${CAPTURE_NAME}.pcapng"
OUTPUT_CAPTURE="${HOME}/${CAPTURE_NAME}.pcapng"
OUTPUT_TEXT="${HOME}/${CAPTURE_NAME}.txt"
OUTPUT_DESCRIPTORS="${HOME}/${CAPTURE_NAME}-lsusb.txt"
CAPTURE_PID=""
USB_DEVICE_ADDRESS=""

stop_capture() {
    if [[ -n "${CAPTURE_PID}" ]] && kill -0 "${CAPTURE_PID}" 2>/dev/null; then
        kill -INT "${CAPTURE_PID}" 2>/dev/null || true
        wait "${CAPTURE_PID}" 2>/dev/null || true
    fi
}

cleanup() {
    stop_capture
}

trap cleanup EXIT INT TERM

if ! [[ "${BUS}" =~ ^[0-9]+$ ]]; then
    echo "Usage: $0 [usb-bus-number]" >&2
    exit 1
fi

for command in dumpcap tshark; do
    if ! command -v "${command}" >/dev/null 2>&1; then
        echo "Missing ${command}. Install Wireshark first:" >&2
        echo "  sudo apt install wireshark" >&2
        exit 1
    fi
done

echo "Preparing USB monitor for bus ${BUS}."
sudo -v
sudo modprobe usbmon

if ! sudo dumpcap -D | grep -q "usbmon${BUS}"; then
    echo "usbmon${BUS} is unavailable. Available USB monitor interfaces:" >&2
    sudo dumpcap -D | grep -i usbmon || true
    exit 1
fi

echo
echo "The 8BitDo keyboard must be unplugged before capture begins."
read -r -p "Press Enter when it is unplugged..."

echo "Starting capture on usbmon${BUS}."
sudo sh -c "exec dumpcap -i usbmon${BUS} -w '${TEMP_CAPTURE}'" >/dev/null 2>&1 &
CAPTURE_PID="$!"

sleep 1
echo
read -r -p "Plug in the keyboard, then press Enter..."
read -r -p "Wait three seconds for it to enumerate, then press Enter..."
read -r -p "Press Caps Lock ONCE to turn it ON (wait for the LED, if any), then press Enter..."
read -r -p "Press Caps Lock ONCE to turn it OFF, then press Enter..."
read -r -p "Press Caps Lock ON then OFF one more time, then press Enter..."

echo "Stopping capture."
stop_capture
CAPTURE_PID=""

if [[ ! -s "${TEMP_CAPTURE}" ]]; then
    echo "Capture file was not created or is empty: ${TEMP_CAPTURE}" >&2
    exit 1
fi

sudo chown "$(id -u):$(id -g)" "${TEMP_CAPTURE}"

USB_DEVICE_ADDRESS="$(sudo lsusb -d "${USB_ID}" | awk 'NR == 1 { print int($4) }')"
if [[ -z "${USB_DEVICE_ADDRESS}" ]]; then
    echo "Cannot identify the keyboard USB address; keeping the unfiltered capture." >&2
    mv "${TEMP_CAPTURE}" "${OUTPUT_CAPTURE}"
else
    echo "Filtering capture for USB device address ${USB_DEVICE_ADDRESS}."
    tshark -r "${TEMP_CAPTURE}" -Y "usb.device_address == ${USB_DEVICE_ADDRESS}" -w "${OUTPUT_CAPTURE}"
    rm "${TEMP_CAPTURE}"
fi

echo "Writing verbose USB text export."
tshark -r "${OUTPUT_CAPTURE}" -Y usb -V > "${OUTPUT_TEXT}"

echo "Writing keyboard USB descriptor snapshot."
sudo lsusb -d "${USB_ID}" -v > "${OUTPUT_DESCRIPTORS}" 2>&1 || true

echo
echo "Capture complete:"
echo "  ${OUTPUT_CAPTURE}"
echo "  ${OUTPUT_TEXT}"
echo "  ${OUTPUT_DESCRIPTORS}"
echo
echo "Look in the .txt file for a SET_REPORT (bRequest 0x09) URB - that's the LED write, if the"
echo "host driver sends one at all. Send me that section, or the whole .txt file if unsure."
