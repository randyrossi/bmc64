/*
 * midi-unix-drv.c - *nix MIDI emulation.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *  Dirk Jagdmann <doj@cubic.org>
 *
 * Based on code by
 *  Andre Fachat <a.fachat@physik.tu-chemnitz.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#if defined(UNIX_COMPILE) && !defined(MACOSX_SUPPORT)

#ifdef HAVE_MIDI

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

/* <sys/select.h> is required for select(2) and fd_set */
#if defined(HAVE_SYS_SELECT_H) || \
    defined(OPENSERVER6_COMPILE) || \
    (defined(__QNX__) && !defined(__QNXNTO__))
#include <sys/select.h>
#endif

#ifdef USE_ALSA
#include <alsa/asoundlib.h>
#endif

#if defined(OPENSTEP_COMPILE) || defined(NEXTSTEP_COMPILE)
#define ssize_t int
#endif

#include "archdep.h"
#include "cmdline.h"
#include "lib.h"
#include "log.h"
#include "mididrv.h"
#include "resources.h"
#include "types.h"
#include "util.h"

/* ------------------------------------------------------------------------- */

static char *midi_in_dev = NULL;
static char *midi_out_dev = NULL;
static int fd_in = -1;
static int fd_out = -1;

#define MIDI_DRIVER_OSS  0
#define MIDI_DRIVER_ALSA 1
static int midi_driver_num = MIDI_DRIVER_OSS;

static log_t mididrv_log = LOG_ERR;

/* ------------------------------------------------------------------------- */
/* OSS driver */

/* opens a MIDI-In device, returns handle */
static int mididrv_oss_in_open(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "oss_in_open");
#endif
    if (fd_in >= 0) {
        mididrv_in_close();
    }

    if (midi_in_dev == NULL) {
        return -1;
    }

    fd_in = open(midi_in_dev, O_RDONLY);
    if (fd_in < 0) {
        log_error(mididrv_log, "Cannot open file \"%s\": %s", midi_in_dev, strerror(errno));
        return -1;
    }

    return fd_in;
}

/* opens a MIDI-Out device, returns handle */
static int mididrv_oss_out_open(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "oss_out_open");
#endif
    if (fd_out >= 0) {
        mididrv_out_close();
    }

    if (midi_out_dev == NULL) {
        return -1;
    }

    fd_out = open(midi_out_dev, O_WRONLY);
    if (fd_out < 0) {
        log_error(mididrv_log, "Cannot open file \"%s\": %s", midi_out_dev, strerror(errno));
        return -1;
    }

    return fd_out;
}

/* closes the MIDI-In device*/
static void mididrv_oss_in_close(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "oss_in_close");
#endif
    if (fd_in < 0) {
        log_error(mididrv_log, "Attempt to close invalid fd %d.", fd_in);
        return;
    }
    close(fd_in);
    fd_in = -1;
}

/* closes the MIDI-Out device*/
static void mididrv_oss_out_close(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "oss_out_close");
#endif
    if (fd_out < 0) {
        log_error(mididrv_log, "Attempt to close invalid fd %d.", fd_out);
        return;
    }
    close(fd_out);
    fd_out = -1;
}

/* sends a byte to MIDI-Out */
static void mididrv_oss_out(uint8_t b)
{
    ssize_t n;

#ifdef DEBUG
    log_message(mididrv_log, "oss_out %02x", b);
#endif
    if (fd_out < 0) {
        log_error(mididrv_log, "Attempt to write to invalid fd %d.", fd_out);
        return;
    }

    do {
        n = write(fd_out, &b, 1);
        if (n < 0) {
            log_error(mididrv_log, "Error writing: %s.", strerror(errno));
        }
    } while (n != 1);

    return;
}

/* gets a byte from MIDI-In, returns !=0 if byte received, byte in *b. */
static int mididrv_oss_in(uint8_t *b)
{
    int ret;
    size_t n;
    fd_set rdset;
    struct timeval ti;

    if (fd_in < 0) {
        log_error(mididrv_log, "Attempt to read from invalid fd %d.", fd_in);
        return -1;
    }

    FD_ZERO(&rdset);
    FD_SET(fd_in, &rdset);
    ti.tv_sec = ti.tv_usec = 0;

    ret = select(fd_in + 1, &rdset, NULL, NULL, &ti);

    if (ret && (FD_ISSET(fd_in, &rdset))) {
        n = read(fd_in, b, 1);
        if (n) {
#ifdef DEBUG
            log_message(mididrv_log, "oss_in got %02x", *b);
#endif
            return 1;
        }
    }
    return 0;
}

static void mididrv_oss_init(void)
{
}

static void mididrv_oss_shutdown(void)
{
    if (fd_in >= 0) {
        mididrv_oss_in_close();
    }

    if (fd_out >= 0) {
        mididrv_oss_out_close();
    }
}

/* ------------------------------------------------------------------------- */
/* ALSA driver */

#ifdef USE_ALSA

/** the ALSA sequencer object which handles MIDI */
static snd_seq_t *seq = NULL;

/** identifier of the ALSA MIDI port */
static int port;

/** the MIDI event parser is used to create ALSA MIDI events from bytes outputted by the AIC */
static snd_midi_event_t *midi_event_parser = NULL;

/** size in bytes of buffers used by ALSA MIDI driver */
#define RINGBUFFER_SIZE 1024

/** this function is called when the AIC has been initialized for MIDI
    transmission.

    @return file descriptor for MIDI out
*/
static int mididrv_alsa_out_open(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "alsa_out_open");
#endif
    if (midi_event_parser) {
        snd_midi_event_reset_encode(midi_event_parser);
    }
    fd_out = 1;
    return fd_out;
}

/** this function is called when the AIC can no longer transmit
    MIDI. */
static void mididrv_alsa_out_close(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "alsa_out_close");
#endif
    /* clear output MIDI queue */
    if (seq) {
        snd_seq_drop_output(seq);
    }
    fd_out = -1;
}

/** this function is called when one MIDI byte needs to be transmitted.
    @param b MIDI byte
 */
static void mididrv_alsa_out(uint8_t b)
{
    snd_seq_event_t ev;

#ifdef DEBUG
    log_message(mididrv_log, "alsa_out %02x", b);
#endif

    /* if ALSA MIDI has not been initialized, we skip transmission */
    if (!seq) {
        return;
    }
    snd_seq_ev_clear(&ev);        /* setup sequencer event */

    /* add the MIDI byte to the event parser. */
    if (snd_midi_event_encode_byte(midi_event_parser, b, &ev) > 0) {
        /* the MIDI event is complete, fill out remaining details on the event */
        snd_seq_ev_set_source(&ev, port);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);

        /* give event to ALSA */
        snd_seq_event_output(seq, &ev);

        /* tell ALSA to transmit event immiediately */
        snd_seq_drain_output(seq);
    }
}

static uint8_t buf[RINGBUFFER_SIZE]; /* a received MIDI event is rendered into this buffer */
static int bufI = -1;             /* index of next byte to read from buf */
static int eventSize = -1;        /* size of event in buf */

/** this function is called when the AIC has been initialized for MIDI
    receiption.

    @return file descriptor for MIDI in
*/
static int mididrv_alsa_in_open(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "alsa_in_open");
#endif
    /* clear any old MIDI data not processed yet */
    if (seq) {
        snd_seq_drop_input(seq);
    }
    if (midi_event_parser) {
        snd_midi_event_reset_decode(midi_event_parser);
    }
    bufI = eventSize = -1;
    fd_in = 1;
    return fd_in;
}

/** this function is called when the AIC can no longer receive
    MIDI. */
static void mididrv_alsa_in_close(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "alsa_in_close");
#endif
    fd_in = -1;
}

/** get a byte from the input MIDI stream.

    @param[out] b if a byte was available, place it in b
    @return 1 if a byte was received and b was set, 0 if no byte available now, -1 upon error
*/
static int mididrv_alsa_in(uint8_t *b)
{
    snd_seq_event_t *ev = NULL;
    int alsa_err;

    if (!seq) {
        return -1;
    }

    /* check if we have a byte in the buffer from the last ALSA MIDI event */
    if (bufI < eventSize) {
        *b = buf[bufI++];
        return 1;
    }

    /* reset buf */
    bufI = eventSize = -1;

    /* check for new ALSA MIDI event */
    snd_seq_event_input(seq, &ev);
    if (!ev) {
        return 0;
    }

    /* ignore some events */
    switch (ev->type) {
        /* these are all ALSA internal events, which don't produce MIDI bytes */
        case SND_SEQ_EVENT_OSS:
        case SND_SEQ_EVENT_CLIENT_START:
        case SND_SEQ_EVENT_CLIENT_EXIT:
        case SND_SEQ_EVENT_CLIENT_CHANGE:
        case SND_SEQ_EVENT_PORT_START:
        case SND_SEQ_EVENT_PORT_EXIT:
        case SND_SEQ_EVENT_PORT_CHANGE:
        case SND_SEQ_EVENT_PORT_SUBSCRIBED:
        case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
        case SND_SEQ_EVENT_USR0:
        case SND_SEQ_EVENT_USR1:
        case SND_SEQ_EVENT_USR2:
        case SND_SEQ_EVENT_USR3:
        case SND_SEQ_EVENT_USR4:
        case SND_SEQ_EVENT_USR5:
        case SND_SEQ_EVENT_USR6:
        case SND_SEQ_EVENT_USR7:
        case SND_SEQ_EVENT_USR8:
        case SND_SEQ_EVENT_USR9:
        case SND_SEQ_EVENT_BOUNCE:
        case SND_SEQ_EVENT_USR_VAR0:
        case SND_SEQ_EVENT_USR_VAR1:
        case SND_SEQ_EVENT_USR_VAR2:
        case SND_SEQ_EVENT_USR_VAR3:
        case SND_SEQ_EVENT_USR_VAR4:
        case SND_SEQ_EVENT_NONE:
            return 0;
    }

    /* Decode Alsa event into raw bytes */
    snd_midi_event_reset_decode(midi_event_parser);
    alsa_err = snd_midi_event_decode(midi_event_parser, buf, sizeof(buf), ev);
    if (alsa_err < 0) {
        log_error(mididrv_log, "could not decode midi event: %s\n", snd_strerror(alsa_err));
        return -1;
    }

    /* upon success, we return the first byte */
    if (alsa_err > 0) {
        eventSize = alsa_err;
        bufI = 1;
        *b = buf[0];
        return 1;
    }

    /* no MIDI available now */
    return 0;
}

/** a function to destroy ALSA MIDI objects */
static void mididrv_alsa_shutdown(void)
{
#ifdef DEBUG
    log_message(mididrv_log, "alsa_shutdown");
#endif

    if (fd_in >= 0) {
        mididrv_alsa_in_close();
    }

    if (fd_out >= 0) {
        mididrv_alsa_out_close();
    }

    if (midi_event_parser) {
        snd_midi_event_free(midi_event_parser);
        midi_event_parser = NULL;
    }

    if (seq) {
        snd_seq_close(seq);
        seq = NULL;
    }
}

static void mididrv_alsa_init(void)
{
    int alsa_err;

#ifdef DEBUG
    log_message(mididrv_log, "alsa_init");
#endif

    /* if we have already been initialized, just return */
    if (seq) {
        return;
    }

    /* create ALSA sequencer object */
    if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK) < 0) {
        log_error(mididrv_log, "could not init ALSA sequencer");
        seq = 0;
        return;
    }

    /* set application name */
    snd_seq_set_client_name(seq, "VICE");

    /* create one MIDI port */
    port = snd_seq_create_simple_port(seq, "MIDI in/out",
                                      SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                                      SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
    if (port < 0) {
        log_error(mididrv_log, "could not create ALSA sequencer port");
        return;
    }

    /* create event parser */
    if ((alsa_err = snd_midi_event_new(RINGBUFFER_SIZE, &midi_event_parser)) < 0) {
        log_error(mididrv_log, "could not create midi_event_parser: %s", snd_strerror(alsa_err));
        return;
    }
    snd_midi_event_no_status(midi_event_parser, 1);
}

#endif /* USE_ALSA */

/* ------------------------------------------------------------------------- */
/* external interface */

typedef struct midi_driver_s {
    void (*init)(void);
    void (*shutdown)(void);
    int (*in)(uint8_t *b);
    void (*out)(uint8_t b);
    int (*in_open)(void);
    void (*in_close)(void);
    int (*out_open)(void);
    void (*out_close)(void);
} midi_driver_t;

static midi_driver_t midi_drivers[] = {
    { /* OSS driver */
        mididrv_oss_init,
        mididrv_oss_shutdown,
        mididrv_oss_in,
        mididrv_oss_out,
        mididrv_oss_in_open,
        mididrv_oss_in_close,
        mididrv_oss_out_open,
        mididrv_oss_out_close,
    },
#ifdef USE_ALSA
    { /* ALSA driver */
        mididrv_alsa_init,
        mididrv_alsa_shutdown,
        mididrv_alsa_in,
        mididrv_alsa_out,
        mididrv_alsa_in_open,
        mididrv_alsa_in_close,
        mididrv_alsa_out_open,
        mididrv_alsa_out_close,
    },
#endif
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

void mididrv_init(void)
{
    if (mididrv_log == LOG_ERR) {
        mididrv_log = log_open("MIDIdrv");
    }

    midi_drivers[midi_driver_num].init();
}

int mididrv_in(uint8_t *b)
{
    return midi_drivers[midi_driver_num].in(b);
}

void mididrv_out(uint8_t b)
{
    midi_drivers[midi_driver_num].out(b);
}

int mididrv_in_open(void)
{
    return midi_drivers[midi_driver_num].in_open();
}

void mididrv_in_close(void)
{
    midi_drivers[midi_driver_num].in_close();
}

int mididrv_out_open(void)
{
    return midi_drivers[midi_driver_num].out_open();
}

void mididrv_out_close(void)
{
    midi_drivers[midi_driver_num].out_close();
}

/* ------------------------------------------------------------------------- */
/* Resources and cmdline */

static int set_midi_in_dev(const char *val, void *param)
{
    util_string_set(&midi_in_dev, val);
    return 0;
}

static int set_midi_out_dev(const char *val, void *param)
{
    util_string_set(&midi_out_dev, val);
    return 0;
}

static const resource_string_t resources_string[] = {
    { "MIDIInDev", ARCHDEP_MIDI_IN_DEV, RES_EVENT_NO, NULL,
      &midi_in_dev, set_midi_in_dev, NULL },
    { "MIDIOutDev", ARCHDEP_MIDI_OUT_DEV, RES_EVENT_NO, NULL,
      &midi_out_dev, set_midi_out_dev, NULL },
    RESOURCE_STRING_LIST_END
};

#ifdef USE_ALSA
static int set_midi_driver(int val, void *param)
{
    int in_was_open, out_was_open;

    switch (val) {
        case MIDI_DRIVER_OSS:
        case MIDI_DRIVER_ALSA:
            break;
        default:
            return -1;
    }

    if (midi_driver_num == val) {
        return 0;
    }

    in_was_open = (fd_in >= 0) ? 1 : 0;
    out_was_open = (fd_out >= 0) ? 1 : 0;

    midi_drivers[midi_driver_num].shutdown();

    midi_driver_num = val;

    midi_drivers[midi_driver_num].init();

    if (in_was_open) {
        midi_drivers[midi_driver_num].in_open();
    }

    if (out_was_open) {
        midi_drivers[midi_driver_num].out_open();
    }
    return 0;
}

static const resource_int_t resources_int[] = {
    { "MIDIDriver", MIDI_DRIVER_OSS,
      RES_EVENT_SAME, (resource_value_t)MIDI_DRIVER_OSS,
      &midi_driver_num, set_midi_driver, NULL },
    RESOURCE_INT_LIST_END
};
#endif

int mididrv_resources_init(void)
{
#ifdef USE_ALSA
    if (resources_register_int(resources_int) < 0) {
        return -1;
    }
#endif

    return resources_register_string(resources_string);
}

void mididrv_resources_shutdown(void)
{
    /* TODO move somewhere else */
    midi_drivers[midi_driver_num].shutdown();

    lib_free(midi_in_dev);
    lib_free(midi_out_dev);
}

static const cmdline_option_t cmdline_options[] =
{
    { "-midiin", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS | CMDLINE_ATTRIB_NEED_BRACKETS,
      NULL, NULL, "MIDIInDev", NULL,
      "<Name>", "Specify MIDI-In device" },
    { "-midiout", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS | CMDLINE_ATTRIB_NEED_BRACKETS,
      NULL, NULL, "MIDIOutDev", NULL,
      "<Name>", "Specify MIDI-Out device" },
#ifdef USE_ALSA
    { "-mididrv", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS | CMDLINE_ATTRIB_NEED_BRACKETS,
      NULL, NULL, "MIDIDriver", NULL,
      "<Driver>", "Specify MIDI driver (0 = OSS, 1 = ALSA)" },
#endif
    CMDLINE_LIST_END
};

int mididrv_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}
#endif /* HAVE_MIDI */
#endif
