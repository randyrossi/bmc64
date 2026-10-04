#!/usr/bin/env bash

set -euo pipefail

BUS="1"
USB_ID="2dc8:5201"
DEVICE_NAME="8bitdo-keyboard"
usage() {
    echo "Usage: $0 [--usb-id vendor:product] [usb-bus-number]"
    echo "Default: 8BitDo keyboard 2dc8:5201 on bus 1."
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --usb-id)
            if [[ $# -lt 2 ]]; then
                usage >&2
                exit 1
            fi
            USB_ID="${2,,}"
            DEVICE_NAME="keyboard-${USB_ID/:/-}"
            shift
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        -*)
            usage >&2
            exit 1
            ;;
        *)
            BUS="$1"
            ;;
    esac
    shift
done

if ! [[ "${BUS}" =~ ^[0-9]+$ && "${USB_ID}" =~ ^[0-9a-f]{4}:[0-9a-f]{4}$ ]]; then
    usage >&2
    exit 1
fi

STAMP="$(date +%Y%m%d-%H%M%S)"
CAPTURE_NAME="${DEVICE_NAME}-${STAMP}"
TEMP_CAPTURE="/tmp/${CAPTURE_NAME}.pcapng"
OUTPUT_CAPTURE="${HOME}/${CAPTURE_NAME}.pcapng"
OUTPUT_TEXT="${HOME}/${CAPTURE_NAME}.txt"
OUTPUT_DESCRIPTORS="${HOME}/${CAPTURE_NAME}-lsusb.txt"
OUTPUT_HID="${HOME}/${CAPTURE_NAME}-hid.txt"
OUTPUT_REPORTS="${HOME}/${CAPTURE_NAME}-reports.tsv"
OUTPUT_CAPTURE_LOG="${HOME}/${CAPTURE_NAME}-dumpcap.txt"
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

trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

for command in dumpcap tshark lsusb; do
    if ! command -v "${command}" >/dev/null 2>&1; then
        echo "Missing ${command}. Install the USB capture tools first:" >&2
        echo "  sudo apt install wireshark usbutils" >&2
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
echo "The keyboard/receiver ${USB_ID} must be unplugged before capture begins."
echo "Use a different keyboard for these prompts. Capture only test keys, not passwords."
echo "For a wireless keyboard, select its paired USB receiver channel, not Bluetooth."
read -r -p "Press Enter when it is unplugged..."

echo "Starting capture on usbmon${BUS}."
sudo dumpcap -i "usbmon${BUS}" -w "${TEMP_CAPTURE}" >"${OUTPUT_CAPTURE_LOG}" 2>&1 &
CAPTURE_PID="$!"

sleep 1
if ! kill -0 "${CAPTURE_PID}" 2>/dev/null; then
    echo "USB capture failed to start. See ${OUTPUT_CAPTURE_LOG}" >&2
    exit 1
fi
echo
read -r -p "Plug in the keyboard/receiver, then press Enter..."
read -r -p "Wait three seconds for it to enumerate, then press Enter..."
read -r -p "Press and release A on the test keyboard, then press Enter on the other keyboard..."
read -r -p "Press and release Shift+A, then press Enter..."
read -r -p "Press and release Ctrl+A, then press Enter..."
read -r -p "Press and release Up, then press Enter..."
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

DEVICE_LIST="$(lsusb -d "${USB_ID}" || true)"
MATCHES="$(awk -v bus="${BUS}" '$2 + 0 == bus + 0 { print int($4) }' <<<"${DEVICE_LIST}")"
if [[ "$(grep -c '[0-9]' <<<"${MATCHES}")" -gt 1 ]]; then
    echo "Multiple ${USB_ID} devices on bus ${BUS}; keeping the unfiltered capture." >&2
else
    USB_DEVICE_ADDRESS="${MATCHES}"
fi
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
if [[ -n "${USB_DEVICE_ADDRESS}" ]]; then
    if ! sudo lsusb -s "${BUS}:${USB_DEVICE_ADDRESS}" -v > "${OUTPUT_DESCRIPTORS}" 2>&1; then
        echo "USB descriptor read failed. See ${OUTPUT_DESCRIPTORS}" >&2
    fi
else
    echo "No unique device address; USB descriptor snapshot unavailable." > "${OUTPUT_DESCRIPTORS}"
fi

echo "Writing Linux HID report descriptors and driver bindings."
HID_COUNT=0
{
    for device in /sys/bus/hid/devices/0003:*; do
        [[ -d "${device}" ]] || continue
        if ! grep -qi "^HID_ID=0003:0000${USB_ID%:*}:0000${USB_ID#*:}$" "${device}/uevent"; then
            continue
        fi
        if [[ -z "${USB_DEVICE_ADDRESS}" ]]; then
            continue
        fi
        USB_PATH="$(readlink -f "${device}")"
        USB_PATH="${USB_PATH%/*}"
        USB_PATH="${USB_PATH%/*}"
        if [[ "$(cat "${USB_PATH}/busnum")" -ne "${BUS}" ||
              "$(cat "${USB_PATH}/devnum")" -ne "${USB_DEVICE_ADDRESS}" ]]; then
            continue
        fi
        HID_COUNT=$((HID_COUNT + 1))
        echo "${device}"
        grep -E '^(DRIVER|HID_ID|HID_NAME|HID_PHYS)=' "${device}/uevent"
        if [[ -r "${device}/report_descriptor" ]]; then
            od -An -tx1 "${device}/report_descriptor"
        else
            echo "HID report descriptor is not readable."
        fi
        echo
    done
    if [[ "${HID_COUNT}" -eq 0 ]]; then
        echo "No matching USB HID device found; keep the USB descriptor/capture files."
    fi
} > "${OUTPUT_HID}"

echo "Writing interrupt-IN report summary."
tshark --disable-protocol usbhid -r "${OUTPUT_CAPTURE}" \
    -Y 'usb.transfer_type == 0x01 && usb.endpoint_address.direction == 1 && usb.data_len > 0' \
    -T fields -E header=y -E separator=/t \
    -e frame.number -e frame.time_relative -e usb.device_address \
    -e usb.endpoint_address -e usb.data_len -e usb.capdata > "${OUTPUT_REPORTS}"

echo
echo "Capture complete:"
echo "  ${OUTPUT_CAPTURE}"
echo "  ${OUTPUT_TEXT}"
echo "  ${OUTPUT_DESCRIPTORS}"
echo "  ${OUTPUT_HID}"
echo "  ${OUTPUT_REPORTS}"
echo "  ${OUTPUT_CAPTURE_LOG}"
echo
echo "Compare SET_PROTOCOL (0x0b), SET_IDLE (0x0a), SET_REPORT (0x09), and interrupt-IN data."
echo "Linux normally uses report protocol; Circle requests boot protocol for keyboards."
echo "Send the descriptor files, reports.tsv and relevant .txt packets for comparison."
