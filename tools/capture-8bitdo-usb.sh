#!/usr/bin/env bash

set -euo pipefail

BUS="1"
DOCK_CYCLE=0
M30=0
XBOX360=0
XBOX360_CLONE=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        --dock-cycle)
            DOCK_CYCLE=1
            ;;
        --m30)
            M30=1
            ;;
        --xbox360)
            XBOX360=1
            ;;
        --xbox360-clone)
            XBOX360=1
            XBOX360_CLONE=1
            ;;
        *)
            BUS="$1"
            ;;
    esac
    shift
done
STAMP="$(date +%Y%m%d-%H%M%S)"
DEVICE_NAME="8bitdo-310b"
USB_ID="2dc8:"
if [[ "${XBOX360}" -eq 1 ]]; then
    DEVICE_NAME="xbox360-wireless-receiver"
    USB_ID="045e:0719"
    if [[ "${XBOX360_CLONE}" -eq 1 ]]; then
        DEVICE_NAME="xbox360-wireless-receiver-clone"
        USB_ID="045e:02a9"
    fi
elif [[ "${M30}" -eq 1 ]]; then
    DEVICE_NAME="8bitdo-m30-2.4g"
    USB_ID="0ca3:0024"
fi
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
    echo "Usage: $0 [--dock-cycle] [--m30|--xbox360|--xbox360-clone] [usb-bus-number]" >&2
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
echo "The receiver must be unplugged before capture begins."
read -r -p "Press Enter when it is unplugged..."

echo "Starting capture on usbmon${BUS}."
sudo sh -c "exec dumpcap -i usbmon${BUS} -w '${TEMP_CAPTURE}'" >/dev/null 2>&1 &
CAPTURE_PID="$!"

sleep 1
echo
if [[ "${DOCK_CYCLE}" -eq 1 ]]; then
    read -r -p "Plug in the receiver with the controller docked, then press Enter..."
    read -r -p "Lift the controller from the dock and wait for it to connect, then press Enter..."
    read -r -p "Put the controller back on the dock and wait for it to disconnect, then press Enter..."
    read -r -p "Lift the controller again and wait ten seconds, then press Enter..."
else
    read -r -p "Plug in the receiver with the controller OFF, then press Enter..."
    read -r -p "Wait three seconds, turn the controller ON, then press Enter..."
    if [[ "${M30}" -eq 1 ]]; then
        read -r -p "Press A once, then press Enter..."
        read -r -p "Press B once, then press Enter..."
        read -r -p "Press C once, then press Enter..."
        read -r -p "Press X once, then press Enter..."
        read -r -p "Press Y once, then press Enter..."
        read -r -p "Press Z once, then press Enter..."
        read -r -p "Press D-pad Up once, then press Enter..."
        read -r -p "Press Start once, then press Enter..."
        read -r -p "Press Mode once, then press Enter..."
    else
        read -r -p "Press A once, then press Enter..."
        read -r -p "Press D-pad Up once, then press Enter..."
        read -r -p "Press Start once, then press Enter..."
        read -r -p "Move the left stick fully left and release it, then press Enter..."
    fi
fi

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
    echo "Cannot identify the receiver USB address; keeping the unfiltered capture." >&2
    mv "${TEMP_CAPTURE}" "${OUTPUT_CAPTURE}"
else
    echo "Filtering capture for USB device address ${USB_DEVICE_ADDRESS}."
    tshark -r "${TEMP_CAPTURE}" -Y "usb.device_address == ${USB_DEVICE_ADDRESS}" -w "${OUTPUT_CAPTURE}"
    rm "${TEMP_CAPTURE}"
fi

echo "Writing verbose USB text export."
tshark -r "${OUTPUT_CAPTURE}" -Y usb -V > "${OUTPUT_TEXT}"

echo "Writing receiver USB descriptor snapshot."
sudo lsusb -d "${USB_ID}" -v > "${OUTPUT_DESCRIPTORS}" 2>&1 || true

echo
echo "Capture complete:"
echo "  ${OUTPUT_CAPTURE}"
echo "  ${OUTPUT_TEXT}"
echo "  ${OUTPUT_DESCRIPTORS}"
echo
echo "Attach the .pcapng and -lsusb.txt files, or send the USB packets around the receiver connection."