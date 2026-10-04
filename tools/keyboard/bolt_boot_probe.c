#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <linux/usbdevice_fs.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t interrupted;

struct probe {
    int fd;
    int pending;
    struct usbdevfs_urb urb;
    unsigned char buffer[64];
};

static void handle_signal(int signal_number)
{
    (void)signal_number;
    interrupted = 1;
}

static int control(int fd, unsigned interface, uint8_t type, uint8_t request, uint16_t value,
                   void *data, uint16_t length)
{
    struct usbdevfs_ctrltransfer transfer = {
        .bRequestType = type,
        .bRequest = request,
        .wValue = value,
        .wIndex = interface,
        .wLength = length,
        .timeout = 2000,
        .data = data
    };
    return ioctl(fd, USBDEVFS_CONTROL, &transfer);
}

static int get_protocol(int fd, unsigned interface, unsigned char *protocol)
{
    int result = control(fd, interface, 0xa1, 0x03, 0, protocol, 1);
    if (result != 1) {
        if (result < 0)
            fprintf(stderr, "GET_PROTOCOL interface %u: %s\n", interface, strerror(errno));
        else
            fprintf(stderr, "GET_PROTOCOL returned %d bytes, expected 1\n", result);
        return -1;
    }
    printf("GET_PROTOCOL interface %u: %u (%s)\n", interface, *protocol,
           *protocol == 0 ? "boot" : *protocol == 1 ? "report" : "invalid");
    if (*protocol > 1) {
        fprintf(stderr, "Invalid protocol value; refusing to change the device\n");
        return -1;
    }
    return 0;
}

static int set_protocol(int fd, unsigned interface, unsigned char protocol)
{
    if (control(fd, interface, 0x21, 0x0b, protocol, NULL, 0) < 0) {
        fprintf(stderr, "SET_PROTOCOL interface %u: %s\n", interface, strerror(errno));
        return -1;
    }
    printf("SET_PROTOCOL interface %u = %u: accepted\n", interface, protocol);
    return 0;
}

static int cancel_request(struct probe *probe)
{
    void *completed = NULL;
    if (!probe->pending)
        return 0;
    if (ioctl(probe->fd, USBDEVFS_DISCARDURB, &probe->urb) < 0 && errno != EINVAL) {
        perror("Cancel interrupt request");
        return -1;
    }
    int result;
    do {
        result = ioctl(probe->fd, USBDEVFS_REAPURB, &completed);
    } while (result < 0 && errno == EINTR);
    if (result < 0) {
        perror("Reap cancelled interrupt request");
        return -1;
    }
    probe->pending = 0;
    return 0;
}

static double monotonic_seconds(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
        perror("Read monotonic clock");
        return -1;
    }
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static int read_reports(struct probe *probe, const char *phase, int request_size)
{
    unsigned reports = 0;
    double start = monotonic_seconds();
    if (start < 0)
        return -1;
    printf("\n%s: 20 seconds, endpoint 0x81, receive buffer %d bytes.\n",
           phase, request_size);
    printf("On the Bolt keyboard: press/release K, L, F12, A, Shift+A, Up, Caps Lock.\n");
    printf("Do not type passwords. These keys will not reach Linux applications.\n");

    while (!interrupted) {
        double now = monotonic_seconds();
        if (now < 0)
            return -1;
        if (now - start >= 20.0)
            break;
        if (!probe->pending) {
            memset(&probe->urb, 0, sizeof(probe->urb));
            probe->urb.type = USBDEVFS_URB_TYPE_INTERRUPT;
            probe->urb.endpoint = 0x81;
            probe->urb.buffer = probe->buffer;
            probe->urb.buffer_length = request_size;
            if (ioctl(probe->fd, USBDEVFS_SUBMITURB, &probe->urb) < 0) {
                perror("Submit keyboard interrupt request");
                return -1;
            }
            probe->pending = 1;
        }
        struct pollfd poll_fd = {.fd = probe->fd, .events = POLLOUT};
        int result = poll(&poll_fd, 1, 250);
        if (result < 0) {
            if (errno == EINTR)
                continue;
            perror("Poll keyboard interrupt request");
            return -1;
        }
        if (result == 0)
            continue;
        if (poll_fd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            fprintf(stderr, "USB device disconnected or polling failed (0x%x)\n",
                    poll_fd.revents);
            return -1;
        }
        void *completed = NULL;
        if (ioctl(probe->fd, USBDEVFS_REAPURBNDELAY, &completed) < 0) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            perror("Reap keyboard interrupt request");
            return -1;
        }
        probe->pending = 0;
        if (completed != &probe->urb) {
            fprintf(stderr, "Unexpected completed USB request\n");
            return -1;
        }
        printf("%s status=%d length=%d data=", phase, probe->urb.status,
               probe->urb.actual_length);
        if (probe->urb.actual_length < 0 || probe->urb.actual_length > request_size) {
            fprintf(stderr, "Invalid USB report length\n");
            return -1;
        }
        for (int i = 0; i < probe->urb.actual_length; i++)
            printf("%02x", probe->buffer[i]);
        putchar('\n');
        if (probe->urb.status != 0) {
            fprintf(stderr, "Interrupt transfer failed: %s\n",
                    strerror(-probe->urb.status));
            return -1;
        }
        reports++;
    }
    printf("%s: %u completed reports%s\n", phase, reports,
           reports == 0 ? " -- no input observed; this phase is inconclusive" : "");
    return cancel_request(probe);
}

static int reconnect_interface(int fd, unsigned interface)
{
    int status = 0;
    if (ioctl(fd, USBDEVFS_RELEASEINTERFACE, &interface) < 0) {
        fprintf(stderr, "Release interface %u: %s\n", interface, strerror(errno));
        status = -1;
    }
    struct usbdevfs_ioctl reconnect = {
        .ifno = (int)interface,
        .ioctl_code = USBDEVFS_CONNECT,
        .data = NULL
    };
    if (ioctl(fd, USBDEVFS_IOCTL, &reconnect) < 0) {
        fprintf(stderr, "Reconnect interface %u: %s; unplug/replug receiver if needed\n",
                interface, strerror(errno));
        status = -1;
    } else {
        printf("Linux driver for interface %u reconnected.\n", interface);
    }
    return status;
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    int mouse_interaction = argc == 3 && strcmp(argv[2], "--mouse-interaction") == 0;
    if (argc != 2 && !mouse_interaction) {
        fprintf(stderr, "Usage: %s /dev/bus/usb/BUS/DEVICE [--mouse-interaction]\n", argv[0]);
        return 1;
    }
    struct sigaction action = {.sa_handler = handle_signal};
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) < 0 ||
        sigaction(SIGTERM, &action, NULL) < 0 ||
        sigaction(SIGHUP, &action, NULL) < 0) {
        perror("Install signal handlers");
        return 1;
    }
    struct probe probe = {.fd = open(argv[1], O_RDWR | O_CLOEXEC)};
    if (probe.fd < 0) {
        perror("Open USB device (local sudo is required)");
        return 1;
    }
    int status = 1;
    int claimed = 0;
    int changed = 0;
    int mouse_claimed = 0;
    int mouse_changed = 0;
    unsigned char original_protocol = 0;
    unsigned char original_mouse_protocol = 0;
    unsigned char descriptor[18];
    if (read(probe.fd, descriptor, sizeof(descriptor)) != (ssize_t)sizeof(descriptor)) {
        fprintf(stderr, "Cannot read the USB device descriptor\n");
        goto cleanup;
    }
    if (descriptor[1] != 1 || descriptor[8] != 0x6d || descriptor[9] != 0x04 ||
        descriptor[10] != 0x48 || descriptor[11] != 0xc5) {
        fprintf(stderr, "Not a Logitech Bolt 046d:c548 receiver; refusing to probe\n");
        goto cleanup;
    }
    struct usbdevfs_disconnect_claim claim = {
        .interface = 0,
        .flags = USBDEVFS_DISCONNECT_CLAIM_IF_DRIVER,
        .driver = "usbhid"
    };
    if (ioctl(probe.fd, USBDEVFS_DISCONNECT_CLAIM, &claim) < 0) {
        perror("Temporarily detach usbhid and claim keyboard interface 0");
        goto cleanup;
    }
    claimed = 1;
    printf("Claimed Bolt keyboard interface 0 only; mouse/vendor interfaces untouched.\n");
    if (get_protocol(probe.fd, 0, &original_protocol) < 0)
        goto cleanup;
    if (mouse_interaction) {
        claim.interface = 1;
        if (ioctl(probe.fd, USBDEVFS_DISCONNECT_CLAIM, &claim) < 0) {
            perror("Temporarily detach usbhid and claim mouse interface 1");
            goto cleanup;
        }
        mouse_claimed = 1;
        printf("Also claimed mouse interface 1; vendor interface 2 untouched.\n");
        if (get_protocol(probe.fd, 1, &original_mouse_protocol) < 0)
            goto cleanup;
    }
    changed = 1;
    if (set_protocol(probe.fd, 0, 1) < 0 ||
        read_reports(&probe, "REPORT", 64) < 0 || interrupted)
        goto cleanup;
    if (set_protocol(probe.fd, 0, 0) < 0)
        goto cleanup;
    unsigned char current_protocol;
    if (get_protocol(probe.fd, 0, &current_protocol) < 0)
        goto cleanup;
    if (current_protocol != 0) {
        fprintf(stderr, "Receiver accepted SET_PROTOCOL but did not report boot mode\n");
        goto cleanup;
    }
    unsigned char leds = 0;
    int led_result = control(probe.fd, 0, 0x21, 0x09, 0x0200, &leds, 1);
    if (led_result != 1) {
        if (led_result < 0)
            perror("Circle-style LED SET_REPORT");
        else
            fprintf(stderr, "Circle-style LED SET_REPORT returned %d bytes, expected 1\n",
                    led_result);
        goto cleanup;
    }
    printf("Circle-style LED SET_REPORT: accepted (output ID 0, interface 0, byte 00).\n");
    if (read_reports(&probe, "BOOT-64", 64) < 0 || interrupted ||
        read_reports(&probe, "BOOT-8-CIRCLE", 8) < 0 || interrupted)
        goto cleanup;
    if (mouse_interaction) {
        printf("\nTesting Circle order: keyboard boot, then mouse report protocol.\n");
        mouse_changed = 1;
        if (set_protocol(probe.fd, 1, 1) < 0 ||
            get_protocol(probe.fd, 0, &current_protocol) < 0 ||
            read_reports(&probe, "AFTER-MOUSE-REPORT", 64) < 0 || interrupted)
            goto cleanup;
        printf("\nReasserting keyboard boot protocol after mouse configuration.\n");
        if (set_protocol(probe.fd, 0, 0) < 0 ||
            get_protocol(probe.fd, 0, &current_protocol) < 0 ||
            read_reports(&probe, "REASSERT-BOOT", 64) < 0 || interrupted)
            goto cleanup;
    }
    status = 0;

cleanup:
    if (cancel_request(&probe) < 0)
        status = 1;
    if (mouse_changed && set_protocol(probe.fd, 1, original_mouse_protocol) < 0) {
        fprintf(stderr, "Mouse protocol restoration failed; unplug/replug the receiver.\n");
        status = 1;
    }
    if (changed && set_protocol(probe.fd, 0, original_protocol) < 0) {
        fprintf(stderr, "Protocol restoration failed; unplug/replug the receiver.\n");
        status = 1;
    }
    if (mouse_claimed && reconnect_interface(probe.fd, 1) < 0)
        status = 1;
    if (claimed && reconnect_interface(probe.fd, 0) < 0)
        status = 1;
    if (close(probe.fd) < 0) {
        perror("Close USB device");
        status = 1;
    }
    return interrupted ? 130 : status;
}
