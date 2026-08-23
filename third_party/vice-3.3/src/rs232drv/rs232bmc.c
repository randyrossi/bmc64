#include "vice.h"

#ifdef HAVE_RS232BMC

#include "machine.h"
#include "rs232bmc.h"
#include "rs232.h"

unsigned int rs232bmc_get_acia_capabilities(int device)
{
    (void) device;

    /* Add the ACIA behavior each machine's modem software needs beyond
       normal RS-232 transport handling. See rs232_acia_capability enum in
       rs232drv.h for more details */
    switch (machine_class) {
        case VICE_MACHINE_C64:
            return RS232_ACIA_OPEN_ON_RESET
                 | RS232_ACIA_SET_2400_ON_RESET
                 | RS232_ACIA_KEEP_DTR_ASSERTED
                 | RS232_ACIA_EXACT_RX_TIMING
                 | RS232_ACIA_NOTIFY_CARRIER_CHANGE;
        case VICE_MACHINE_C128:
            return RS232_ACIA_ASSERT_READY_WHEN_DISCONNECTED
                 | RS232_ACIA_EXACT_RX_TIMING
                 | RS232_ACIA_NOTIFY_CARRIER_CHANGE;
        default:
            return 0;
    }
}

void rs232bmc_note_acia_data_write(int device, uint8_t byte)
{
    (void) device;
    bmcmodem_note_acia_tx(byte);
}

void rs232bmc_init(void)
{
    bmcmodem_init();
}

void rs232bmc_reset(void)
{
    bmcmodem_reset();
}

int rs232bmc_open(int device)
{
    return bmcmodem_open(device);
}

void rs232bmc_close(int fd)
{
    bmcmodem_close(fd);
}

int rs232bmc_putc(int fd, uint8_t byte)
{
    return bmcmodem_putc(fd, byte);
}

int rs232bmc_getc(int fd, uint8_t *byte)
{
    return bmcmodem_getc(fd, byte);
}

int rs232bmc_set_status(int fd, enum rs232handshake_out status)
{
    (void) fd;
    bmcmodem_set_status(status);
    return 0;
}

enum rs232handshake_in rs232bmc_get_status(int fd)
{
    int carrier = bmcmodem_has_carrier();
    int ready = carrier || (rs232bmc_get_acia_capabilities(fd)
                            & RS232_ACIA_ASSERT_READY_WHEN_DISCONNECTED);
    enum rs232handshake_in status =
        (ready ? RS232_HSI_CTS : 0) | (ready ? RS232_HSI_DSR : 0);

    (void) fd;
    return status;
}

void rs232bmc_set_bps(int fd, unsigned int bps)
{
    (void) fd;
    bmcmodem_set_bps(bps);
}

#endif