// tusb_config.h - keyrah_test
//
// Minimal TinyUSB config for a composite device: 6 HID interfaces (2 boot
// keyboards + 2 joystick-shaped generic HID + 2 spare HID, matching the
// Individual Computers Keyrah V3's descriptor shape) plus one CDC interface
// used as an interactive test console.

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUSB_MCU              OPT_MCU_RP2040
#define CFG_TUSB_OS               OPT_OS_PICO
#define CFG_TUSB_RHPORT0_MODE     (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN         __attribute__((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE     64
#endif

//------------- CLASS -------------//
#define CFG_TUD_CDC                1
#define CFG_TUD_MSC                0
#define CFG_TUD_HID                6   // ITF_KBD1/2, ITF_JOY1/2, ITF_CONSUMER, ITF_SYSCTRL - see usb_descriptors.h
#define CFG_TUD_MIDI               0
#define CFG_TUD_VENDOR             0

// CDC FIFO sizes (the interactive test console)
#define CFG_TUD_CDC_RX_BUFSIZE     64
#define CFG_TUD_CDC_TX_BUFSIZE     64

#define CFG_TUD_HID_EP_BUFSIZE     48

#ifdef __cplusplus
}
#endif

#endif
