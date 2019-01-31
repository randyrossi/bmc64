/*
 * uifileentry.c - file entry widget.
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "ui.h"
#include "uiapi.h"
#include "uiarch.h"
#include "uifileentry.h"
#include "uilib.h"
#include "util.h"

typedef struct uilib_file_filter_s {
    const char* filter_string;
    unsigned char filter_string_is_case_sensitive;
} uilib_file_filter_t;

static uilib_file_filter_t all_files_filter[] = { { "*", 0 }, { NULL } };
static uilib_file_filter_t palette_filter[] = { { "*.vpl", 0 }, { NULL } };
static uilib_file_filter_t snapshot_filter[] = { { "*.vsf", 0 }, { NULL } };

static uilib_file_filter_t disk_image_filter[] = {
    { "*.d64", 0 },
    { "*.d67", 0 },
    { "*.d71", 0 },
    { "*.d80", 0 },
    { "*.d81", 0 },
    { "*.d82", 0 },
    { "*.d1m", 0 },
    { "*.d2m", 0 },
    { "*.d4m", 0 },
    { "*.g64", 0 },
    { "*.g71", 0 },
    { "*.g41", 0 },
    { "*.p64", 0 },
    { "*.x64", 0 },
    { NULL }
};

static uilib_file_filter_t tape_image_filter[] = { { "*.t64", 0 }, { "*.tap", 0 }, { NULL } };
static uilib_file_filter_t cartridge_image_filter[] = { { "*.crt", 0 }, { "*.bin", 0 }, { NULL } };
static uilib_file_filter_t crt_image_filter[] = { { "*.crt", 0 }, { NULL } };
static uilib_file_filter_t flip_list_filter[] = { { "*.vfl", 0 }, { NULL } };
static uilib_file_filter_t romset_filter[] = { { "*.vrs", 0 }, { NULL } };
static uilib_file_filter_t romset_archives_filter[] = { { "*.vra", 0 }, { NULL } };
static uilib_file_filter_t keymap_filter[] = { { "*.vkm", 0 }, { NULL } };
static uilib_file_filter_t emulator_filter[] = { { "*.prg", 0 }, { "*.p00", 0 }, { NULL } };
static uilib_file_filter_t wav_filter[] = { { "*.wav", 0 }, { NULL } };
static uilib_file_filter_t voc_filter[] = { { "*.voc", 0 }, { NULL } };
static uilib_file_filter_t iff_filter[] = { { "*.iff", 0 }, { NULL } };
static uilib_file_filter_t aiff_filter[] = { { "*.aiff", 0 }, { NULL } };
static uilib_file_filter_t mp3_filter[] = { { "*.mp3", 0 }, { NULL } };
static uilib_file_filter_t flac_filter[] = { { "*.flac", 0 }, { NULL } };
static uilib_file_filter_t vorbis_filter[] = { { "*.ogg", 0 }, { NULL } };
static uilib_file_filter_t serial_filter[] = { { "ttyS*", 1 }, { NULL } };
static uilib_file_filter_t vic20cart_filter[] = { { "*.prg", 0 }, { "*.bin", 0 }, { NULL } };
static uilib_file_filter_t sid_filter[] = { { "*.psid", 0 }, { "*.sid", 0 }, { NULL } };
static uilib_file_filter_t dtvrom_filter[] = { { "*.bin", 0 }, { NULL } };
static uilib_file_filter_t compressed_filter[] = {
    { "*.zip", 0 },
    { "*.gz", 0 },
    { "*.bz2", 0 },
    { NULL }
};
static uilib_file_filter_t eth_filter[] = { { "eth*", 1 }, { NULL } };
static uilib_file_filter_t midi_filter[] = { { "mi*", 1 }, { NULL } }; /* FIXME */
static uilib_file_filter_t hd_image_filter[] = {
    { "*.hdd", 0 },
    { "*.iso", 0 },
    { "*.fdd", 0 },
    { "*.cfa", 0 },
    { NULL }
};

/* this must be in sync with uilib_file_filter_enum_t (gui/uilib.h)*/
struct {
    const char* filter_name;
    uilib_file_filter_t *filters;
} file_filters[] = {
    { "All files", all_files_filter },
    { "Palette files", palette_filter },
    { "Snapshot files", snapshot_filter },
    { "Disk image files", disk_image_filter },
    { "Tape image files", tape_image_filter },
    { "Cartridge image files", cartridge_image_filter },
    { "CRT cartridge files", crt_image_filter },
    { "Flip list files", flip_list_filter },
    { "Romset files", romset_filter },
    { "Romset archives", romset_archives_filter },
    { "Keymap files", keymap_filter },
    { "Emulator files", emulator_filter },
    { "WAV files", wav_filter },
    { "VOC files", voc_filter },
    { "IFF files", iff_filter },
    { "AIFF files", aiff_filter },
    { "MP3 files", mp3_filter },
    { "FLAC files", flac_filter },
    { "ogg/vorbis files", vorbis_filter },
    { "Serial ports", serial_filter },
    { "VIC20 cartridges", vic20cart_filter },
    { "SID files", sid_filter },
    { "C64DTV ROM images", dtvrom_filter },
    { "Compressed files", compressed_filter },
    { "Network ports", eth_filter },
    { "Midi ports", midi_filter },
    { "HD image files", hd_image_filter },
};

GtkWidget *vice_file_entry(const char *title, GtkWidget* parent_window, const char *default_dir, uilib_file_filter_enum_t* patterns, int num_patterns, ui_filechooser_t action)
{
    GtkWidget *fb;
    int i;
    GtkFileChooserAction a;
    const gchar* accept_button;

    switch (action)
    {
        case UI_FC_LOAD:
            a = GTK_FILE_CHOOSER_ACTION_OPEN;
            accept_button = GTK_STOCK_OPEN;
            break;
        case UI_FC_SAVE:
            a = GTK_FILE_CHOOSER_ACTION_SAVE;
            accept_button = GTK_STOCK_SAVE;
            break;
        default:
            return NULL;
    }


    fb = gtk_file_chooser_dialog_new(title, GTK_WINDOW(parent_window), a, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, accept_button, GTK_RESPONSE_ACCEPT, NULL);

    gtk_window_set_resizable(GTK_WINDOW(fb), TRUE);
    for (i = 0; i < num_patterns; i++) {
        int j;
        GtkFileFilter *ff = gtk_file_filter_new();
        char filter_name[1024];
        int filter_name_len = sizeof(filter_name);

        snprintf(filter_name, filter_name_len, "%s", file_filters[patterns[i]].filter_name);
        filter_name_len = sizeof(filter_name) - strlen(filter_name);
        for (j = 0; file_filters[patterns[i]].filters[j].filter_string != NULL; j++) {
            char filter_made_case_insensitive[1024];
            char *dst = filter_made_case_insensitive;
            char *dstend = filter_made_case_insensitive + sizeof(filter_made_case_insensitive) - 1;
            const char *src = file_filters[patterns[i]].filters[j].filter_string;
            char c;

            strncat(filter_name, j == 0 ? " (" : ";", filter_name_len);
            filter_name_len = sizeof(filter_name) - strlen(filter_name);
            strncat(filter_name, file_filters[patterns[i]].filters[j].filter_string, filter_name_len);
            filter_name_len = sizeof(filter_name) - strlen(filter_name);
            while (*src) {
                if (dst >= dstend) {
                    break;
                }
                c = *src;
                if (!file_filters[patterns[i]].filters[j].filter_string_is_case_sensitive && isalpha((int)c) && islower((int)c)) {
                    *dst++ = '[';
                    if (dst >= dstend) {
                        break;
                    }
                    *dst++ = *src;
                    if (dst >= dstend) {
                        break;
                    }
                    *dst++ = util_toupper(*src++);
                    if (dst >= dstend) {
                        break;
                    }
                    *dst++ = ']';
                } else {
                    *dst++ = *src++;
                }
            }
            *dst = 0;
            gtk_file_filter_add_pattern(ff, filter_made_case_insensitive);
        }
        if (j > 0) {
            strncat(filter_name, ")", filter_name_len);
        }
        gtk_file_filter_set_name(ff, filter_name);
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fb), ff);
    }

    if (default_dir) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fb), default_dir);
    }
    return fb;
}
