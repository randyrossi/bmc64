/*
 * midi-osx-drv.c - MIDI emulation for Mac OS X.
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * Based on Code by
 *  Daniel Kahlin <daniel@kahlin.net>
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

#ifdef MACOSX_SUPPORT

#ifdef HAVE_MIDI

#include <CoreMIDI/MIDIServices.h>
#include <CoreFoundation/CFRunLoop.h>
#include <stdio.h>

#ifdef USE_COREAUDIO
#include <CoreAudio/HostTime.h>
#endif

#include "cmdline.h"
#include "lib.h"
#include "log.h"
#include "mididrv.h"
#include "resources.h"
#include "types.h"
#include "util.h"

static log_t mididrv_log = LOG_ERR;

/* ----- FIFO Buffer ----- */
#define OUT_BUF_LEN 3

static int out_index = 0;
static uint8_t out_buf[OUT_BUF_LEN];

#define IN_BUF_LEN 1024
static volatile unsigned int in_wi = 0;
static volatile unsigned int in_ri = 0;
static uint8_t in_buf[IN_BUF_LEN];

/* ----- MIDI Vars ----- */
static int midi_client_usage = 0;
static int midi_destination_usage = 0;
static int midi_source_usage = 0;
static MIDIClientRef midi_client;
static MIDIEndpointRef midi_destination;
static MIDIEndpointRef midi_source;

/* ----- Resources ----- */

static char *midi_name = NULL;
static char *midi_in_name = NULL;
static char *midi_out_name = NULL;

static int set_midi_name(const char *val, void *param)
{
    util_string_set(&midi_name, val);
    return 0;
}

static int set_midi_in_name(const char *val, void *param)
{
    util_string_set(&midi_in_name, val);
    return 0;
}

static int set_midi_out_name(const char *val, void *param)
{
    util_string_set(&midi_out_name, val);
    return 0;
}

static const resource_string_t resources_string[] = {
    { "MIDIName", "VICE", RES_EVENT_NO, NULL,
      &midi_name, set_midi_name, NULL },
    { "MIDIInName", "VICEInput", RES_EVENT_NO, NULL,
      &midi_in_name, set_midi_in_name, NULL },
    { "MIDIOutName", "VICEOutput", RES_EVENT_NO, NULL,
      &midi_out_name, set_midi_out_name, NULL },
    RESOURCE_STRING_LIST_END
};

int mididrv_resources_init(void)
{
    return resources_register_string(resources_string);
}

void mididrv_resources_shutdown(void)
{
    lib_free(midi_name);
    lib_free(midi_in_name);
    lib_free(midi_out_name);
}

static const cmdline_option_t cmdline_options[] =
{
    { "-midiname", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS | CMDLINE_ATTRIB_NEED_BRACKETS,
      NULL, NULL, "MIDIName", NULL,
      "<Name>", "Name of MIDI Client" },
    { "-midiinname", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS | CMDLINE_ATTRIB_NEED_BRACKETS,
      NULL, NULL, "MIDIInName", NULL,
      "<Name>", "Name of MIDI-In Port" },
    { "-midioutname", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS | CMDLINE_ATTRIB_NEED_BRACKETS,
      NULL, NULL, "MIDIOutName", NULL,
      "<Name>", "Name of MIDI-Out Port" },
    CMDLINE_LIST_END
};

int mididrv_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ----- FIFO ----- */

static void reset_fifo(void)
{
    in_wi = 0;
    in_ri = 0;
}

static int write_fifo(uint8_t data)
{
    if (((in_wi - in_ri) % IN_BUF_LEN) == (IN_BUF_LEN - 1)) {
        return 1;
    }

    in_buf[in_wi] = data;
    in_wi = (in_wi + 1) % IN_BUF_LEN;
    return 0;
}

static int read_fifo(uint8_t *data)
{
    if (((in_wi - in_ri) % IN_BUF_LEN) != 0) {
        *data = in_buf[in_ri];
        in_ri = (in_ri + 1) % IN_BUF_LEN;
        return 1;
    }
    return 0;
}

/* ----- packetizing ----- */

static int message_len(uint8_t msg)
{
    int len = 0;

    switch (msg & 0xf0) {
        case 0x80: /* Note Off */
        case 0x90: /* Note On */
        case 0xa0: /* Polyphonic Aftertouch */
        case 0xb0: /* Control Change */
        case 0xe0: /* Pitch Wheel */
            len = 3;
            break;
        case 0xc0: /* Program Change */
        case 0xd0: /* Channel Aftertouch */
            len = 2;
            break;
        case 0xf0: /* Special */
            switch (msg) {
                case 0xf0: /* Sysex Start (shouldn't happen here) */
                case 0xf7: /* Sysex End (shouldn't happen here) */
                    len = -1;
                    break;
                case 0xf2: /* Song Pointer */
                    len = 3;
                    break;
                case 0xf1: /* Quarter Frame */
                case 0xf3: /* Song Select */
                case 0xf9: /* Measure End */
                    len = 2;
                    break;
                case 0xf6: /* Tuning Request */
                case 0xf8: /* Timing Clock */
                case 0xfa: /* Start */
                case 0xfb: /* Continue */
                case 0xfc: /* Stop */
                case 0xfe: /* Active Sensing */
                case 0xff: /* Reset */
                    len = 1;
                    break;
                default:
                    break;
            }
        default: /* running status */
            len = 2;
            break;
    }
    return len;
}

/* ----- CoreMIDI ----- */

static int create_client(void)
{
    if (midi_client_usage == 0) {
        log_message(mididrv_log, "Opening MIDI client '%s'", midi_name);
        CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, midi_name, kCFStringEncodingUTF8); 
        OSStatus status = MIDIClientCreate(name, NULL, NULL, &midi_client);
        CFRelease(name);
        if (status != noErr) {
            log_error(mididrv_log, "Error creating MIDI client!");
            return -1;
        }
    }
    midi_client_usage++;
    return 0;
}

static void dispose_client(void)
{
    midi_client_usage--;
    if (midi_client_usage == 0) {
        log_message(mididrv_log, "Closing MIDI client");
        OSStatus status = MIDIClientDispose(midi_client);
        if (status != noErr) {
            log_error(mididrv_log, "Error disposing client!");
        }
    }
}

static void midi_read_proc(const MIDIPacketList *pktlist, void *refCon, void *connRefCon)
{
    unsigned int i,j;
    
    const MIDIPacket *packet = pktlist->packet;
    for (i = 0; i < pktlist->numPackets; ++i) {
        for (j = 0; j < packet->length; j++) {
            if (write_fifo(packet->data[j])) {
                log_error(mididrv_log, "MIDI-In overrun!");
            }
        }
        packet = MIDIPacketNext(packet);
    }
}

static void dump_sources(void)
{
    int i,n;
    CFStringRef pname;
    char name[64];
    
    n = MIDIGetNumberOfSources();
    log_message(mididrv_log,"found %d sources", n);
    for (i = 0 ; i < n; ++i) {
        MIDIEndpointRef endpnt = MIDIGetSource(i);
        MIDIObjectGetStringProperty(endpnt, kMIDIPropertyName, &pname);
        CFStringGetCString(pname, name, sizeof(name), 0);
        CFRelease(pname);

        log_message(mididrv_log, "source #%d: %s", i, name);
    }
}

static void dump_destinations(void)
{
    int i,n;
    CFStringRef pname;
    char name[64];
    
    n = MIDIGetNumberOfDestinations();
    log_message(mididrv_log, "found %d destinations", n);
    for (i = 0; i < n; ++i) {
        MIDIEndpointRef endpnt = MIDIGetDestination(i);
        MIDIObjectGetStringProperty(endpnt, kMIDIPropertyName, &pname);
        CFStringGetCString(pname, name, sizeof(name), 0);
        CFRelease(pname);

        log_message(mididrv_log, "destination #%d: %s", i, name);
    }
}

/* ----- midi driver ----- */

void mididrv_init(void)
{
    if (mididrv_log == LOG_ERR) {
        mididrv_log = log_open("MIDIdrv");
    }
}

/* opens a MIDI-In device, returns handle */
int mididrv_in_open(void)
{
    if (create_client() < 0) {
        return -1;
    }

    if (midi_destination_usage == 0) {
        log_message(mididrv_log, "Opening MIDI-In port '%s'", midi_in_name);

        CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, midi_in_name, kCFStringEncodingUTF8); 
        OSStatus status = MIDIDestinationCreate(midi_client, name, midi_read_proc, NULL, &midi_destination);
        CFRelease(name);
        if (status != noErr) {
            log_error(mididrv_log, "Error creating MIDI-In port!");
            return -1;
        }
    }
    midi_destination_usage++;

    /* reset FIFO */
    reset_fifo();

    dump_destinations();
    return 0;
}

/* opens a MIDI-Out device, returns handle */
int mididrv_out_open(void)
{
    if (create_client() < 0) {
        return -1;
    }
    
    if (midi_source_usage == 0) {
        log_message(mididrv_log, "Opening MIDI-Out port '%s'", midi_out_name);

        CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, midi_out_name, kCFStringEncodingUTF8); 
        OSStatus status = MIDISourceCreate(midi_client, name, &midi_source);
        CFRelease(name);
        if (status != noErr) {
            log_error(mididrv_log, "Error creating MIDI-Out port!");
            return -1;
        }
    }
    midi_source_usage ++;

    /* reset buffer */
    out_index = 0;

    dump_sources();
    return 0;
}

/* closes the MIDI-In device*/
void mididrv_in_close(void)
{
    midi_destination_usage --;
    if (midi_destination_usage == 0) {
        log_message(mididrv_log, "Closing MIDI-In port");

        OSStatus status = MIDIEndpointDispose(midi_destination);
        if (status != noErr) {
            log_error(mididrv_log, "Error disposing MIDI-In port!");
        }
    }

    dispose_client();
}

/* closes the MIDI-Out device*/
void mididrv_out_close(void)
{
    midi_source_usage --;
    if (midi_source_usage == 0) {
        log_message(mididrv_log, "Closing MIDI-Out port");

        OSStatus status = MIDIEndpointDispose(midi_source);
        if (status != noErr) {
            log_error(mididrv_log, "Error disposing MIDI-Out port!");
        }
    }

    dispose_client();
}

/* sends a byte to MIDI-Out */
void mididrv_out(uint8_t b)
{
    int thres;
    MIDIPacketList pktlist;
    MIDIPacket *pkt;

    pkt = MIDIPacketListInit(&pktlist);

#ifdef DEBUG
    log_message(mididrv_log, "out %02x", b);
#endif

    out_buf[out_index] = b;
    out_index++;
    if (out_index >= OUT_BUF_LEN) {
        out_index = 0;
        log_error(mididrv_log, "MIDI-Out overrun.");
    }

    thres = message_len(out_buf[0]);

    /* flush when enough bytes have been queued */
    if (out_index >= thres) {
        out_index = 0;

#ifdef USE_COREAUDIO
        UInt64 timestamp = AudioGetCurrentHostTime();
#else
        UInt64 timestamp = 0;
#endif

        /* add midi packet */
        pkt = MIDIPacketListAdd(&pktlist, 0, pkt, timestamp, thres, out_buf);

        /* send midi packet */
        OSStatus status = MIDIReceived(midi_source, &pktlist);
        if (status != noErr) {
            log_error(mididrv_log, "Failed to output data on MIDI-Out device.");
        }
    }
}

/* gets a byte from MIDI-In, returns != 0 if byte received, byte in *b. */
int mididrv_in(uint8_t *b)
{
    if (read_fifo(b)) {
#ifdef DEBUG
        log_message(mididrv_log, "in got %02x", *b);
#endif
        return 1;
    }
    return 0;
}

#endif /* HAVE_MIDI */
#endif /* MACOSX_SUPPORT */
