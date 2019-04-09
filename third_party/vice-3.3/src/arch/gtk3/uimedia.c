/** \file   uimedia.c
 * \brief   Media recording dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SoundRecordDeviceArg        all
 * $VICERES SoundRecordDeviceName       all
 * $VICERES DoodleOversizeHandling      -vsid
 * $VICERES DoodleUndersizeHandling     -vsid
 * $VICERES DoodleMultiColorHandling    -vsid
 * $VICERES DoodleTEDLumHandling        -vsid
 * $VICERES DoodleCRTCTextColor         -vsid
 * $VICERES KoalaOversizeHandling       -vsid
 * $VICERES KoalaUndersizeHandling      -vsid
 * $VICERES KoalaTEDLumHandling         -vsid
 * $VICERES KoalaCRTCTextColor          -vsid
 */

/*
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
 */

#include "vice.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "debug_gtk3.h"
#include "lib.h"
#include "resources.h"
#include "machine.h"
#include "sound.h"
#include "screenshot.h"
#include "widgethelpers.h"
#include "basewidgets.h"
#include "basedialogs.h"
#include "openfiledialog.h"
#include "savefiledialog.h"
#include "selectdirectorydialog.h"
#include "ui.h"
#include "gfxoutput.h"

#ifdef HAVE_FFMPEG
#include "ffmpegwidget.h"
#endif

#include "uimedia.h"


/** \brief  Custom response IDs for the dialog
 */
enum {
    RESPONSE_SAVE = 1   /**< Save button clicked */
};


/** \brief  Name of the stack child for screenshots
 */
#define CHILD_SCREENSHOT    "Screenshot"

/** \brief  Name of the stack child for sound recording
 */
#define CHILD_SOUND         "Sound"

/** \brief  Name of the stack child for video recording
*/
#define CHILD_VIDEO         "Video"


/** \brief  Struct to hold information on available video recording drivers
 */
typedef struct video_driver_info_s {
    const char *display;    /**< display string (used in the UI) */
    const char *name;       /**< driver name */
    const char *ext;        /**< default file extension */
} video_driver_info_t;


/** \brief  Struct to hold information on available audio recording drivers
 */
typedef struct audio_driver_info_s {
    const char *display;    /**< display name (used in the UI) */
    const char *name;       /**< driver name */
    const char *ext;        /**< default file extension */
} audio_driver_info_t;


/** \brief  List of available video drivers, gotten during runtime
 */
static video_driver_info_t *video_driver_list = NULL;


/** \brief  Number of available video drivers
 */
static int video_driver_count = 0;


/** \brief  Index of currently selected video driver
 */
static int video_driver_index = 0;


/** \brief  Index of currently selected audio driver
 */
static int audio_driver_index = 0;


/** \brief  List of available audio recording drivers
 *
 * This list is dependent on compile-time options
 */
static audio_driver_info_t audio_driver_list[] = {
    { "WAV", "wav", "wav" },
    { "AIFF", "aiff", "aif" },
    { "VOC", "voc", "voc" },
    { "IFF", "iff", "iff" },
#ifdef USE_LAMEMP3
    { "MP3", "mp3", "mp3" },
#endif
#ifdef USE_FLAC
    { "FLAC", "flac", "flac" },
#endif
#ifdef USE_VORBIS
    { "Ogg/Vorbis", "ogg", "ogg" },
#endif
    { NULL, NULL, NULL }
};


/** \brief  List of 'oversize modes' for some screenshot output drivers
 */
static vice_gtk3_combo_entry_int_t oversize_modes[] = {
    { "scale down", 0 },
    { "crop left top", 1, },
    { "crop center top", 2 },
    { "crop right top", 3 },
    { "crop left center", 4 },
    { "crop center", 5 },
    { "crop right center", 6 },
    { "crop left bottom (huhu)", 7 },
    { "crop center bottom", 8 },
    { "crop right bottom", 9 },
    { NULL, -1 }
};


/** \brief  List of 'undersize modes' for some screenshot output drivers
 */
static vice_gtk3_combo_entry_int_t undersize_modes[] = {
    { "scale up", 0 },
    { "border size", 1 },
    { NULL, -1 }
};


/** \brief  List of multi color modes for some screenshot output drivers
 */
static const vice_gtk3_combo_entry_int_t multicolor_modes[] = {
    { "B&W", 0 },
    { "2 colors", 1 },
    { "4 colors", 2 },
    { "Grey scale", 3 },
    { "Best cell colors", 4 },
    { NULL, -1 }
};


/** \brief  TED output driver Luma modes
 */
static const vice_gtk3_combo_entry_int_t ted_luma_modes[] = {
    { "ignore", 0 },
    { "dither", 1 },
    { NULL, -1 },
};


/** \brief  List of available colors to use for CRTC screenshots
 */
static const vice_gtk3_combo_entry_int_t crtc_colors[] = {
    { "White", 0 },
    { "Amber", 1 },
    { "Green", 2 },
    { NULL, -1 }
};


/** \brief  Reference to the GtkStack containing the media types
 *
 * Used in the dialog response callback to determine recording mode and params
 */
static GtkWidget *stack;

/* references to widgets, used from various event handlers */
static GtkWidget *screenshot_options_grid = NULL;
static GtkWidget *oversize_widget = NULL;
static GtkWidget *undersize_widget = NULL;
static GtkWidget *multicolor_widget = NULL;
static GtkWidget *ted_luma_widget = NULL;
static GtkWidget *crtc_textcolor_widget = NULL;
static GtkWidget *video_driver_options_grid = NULL;


/* forward declarations of helper functions */
static GtkWidget *create_screenshot_param_widget(const char *prefix);
static void save_screenshot_handler(void);
static void save_audio_recording_handler(void);
static void save_video_recording_handler(void);


/*****************************************************************************
 *                              Event handlers                               *
 ****************************************************************************/

/** \brief  Handler for the "destroy" event of the dialog
 *
 * \param[in]   widget  dialog
 * \param[in]   data    extra event data (unused)
 */
static void on_dialog_destroy(GtkWidget *widget, gpointer data)
{
    if (machine_class != VICE_MACHINE_VSID) {
        debug_gtk3("called: cleaning up driver list.");
        lib_free(video_driver_list);
    }
    ui_pause_emulation(FALSE);
}


/** \brief  Set new widget for the screenshot options, replacing the old one
 *
 * Destroys any widget present before setting the \a new widget.
 *
 * \param[in]   new     new widget
 */
static void update_screenshot_options_grid(GtkWidget *new)
{
    GtkWidget *old = gtk_grid_get_child_at(GTK_GRID(screenshot_options_grid),
            0, 1);
    if (old != NULL) {
        gtk_widget_destroy(old);
    }
    gtk_grid_attach(GTK_GRID(screenshot_options_grid), new, 0, 1, 1, 1);
}


/** \brief  Handler for the "response" event of the \a dialog
 *
 * \param[in,out]   dialog      dialog triggering the event
 * \param[in]       response_id response ID
 * \param[in]       data        extra event data (unused)
 */
static void on_response(GtkDialog *dialog, gint response_id, gpointer data)
{
    const gchar *child_name;

    debug_gtk3("got response ID %d.", response_id);

    switch (response_id) {
        case GTK_RESPONSE_DELETE_EVENT:
            debug_gtk3("destroying dialog.");
            gtk_widget_destroy(GTK_WIDGET(dialog));
            break;

        case RESPONSE_SAVE:

            if (machine_class != VICE_MACHINE_VSID) {
                /* stack child name determines what to do next */
                child_name = gtk_stack_get_visible_child_name(GTK_STACK(stack));
                debug_gtk3("Saving media, tab '%s' selected.", child_name);

                if (strcmp(child_name, CHILD_SCREENSHOT) == 0) {
                    debug_gtk3("Screenshot requested, driver %d.",
                            video_driver_index);
                    save_screenshot_handler();
                } else if (strcmp(child_name, CHILD_SOUND) == 0) {
                    debug_gtk3("Audio recording requested, driver %d.",
                            audio_driver_index);
                    save_audio_recording_handler();
                } else if (strcmp(child_name, CHILD_VIDEO) == 0) {
                    debug_gtk3("Video recording requested, driver %d.",
                            video_driver_index);
                    save_video_recording_handler();
                }
            } else {
                debug_gtk3("Audio recording requested, driver %d.",
                        audio_driver_index);
                save_audio_recording_handler();
            }
            break;


        default:
            break;
    }
}


/** \brief  Handler for the "toggled" event of the screenshot driver radios
 *
 * \param[in]       widget      radio button riggering the event
 * \param[in]       data        extra event data (unused)
 */
static void on_screenshot_driver_toggled(GtkWidget *widget, gpointer data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        int index = GPOINTER_TO_INT(data);
        debug_gtk3("screenshot driver %d (%s) selected.",
                index, video_driver_list[index].name);
        video_driver_index = index;
        update_screenshot_options_grid(
                create_screenshot_param_widget(video_driver_list[index].name));
    }
}


/** \brief  Handler for the "toggled" event of the audio driver radios
 *
 * \param[in]       widget      radio button riggering the event
 * \param[in]       data        extra event data (unused)
 */
static void on_audio_driver_toggled(GtkWidget *widget, gpointer data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        int index = GPOINTER_TO_INT(data);
        debug_gtk3("audio driver %d (%s) selected.",
                index, audio_driver_list[index].name);
        audio_driver_index = index;
    }
}



/*****************************************************************************
 *                              Helpers functions                            *
 ****************************************************************************/

/** \brief  Create a string in the format 'yyyymmddHHMM' of the current time
 *
 * \return  string owned by GLib, free with g_free()
 */
static gchar *create_datetime_string(void)
{
    GDateTime *d;
    gchar *s;

    d = g_date_time_new_now_local();
    s = g_date_time_format(d, "%Y%m%d%H%M");
    g_date_time_unref(d);
    return s;
}


/** \brief  Create a filename based on the current datetime and \a ext
 *
 * \param[in]   ext file extension (without the dot)
 *
 * \return  heap-allocated string, owned by VICE, free with lib_free()
 */
static char *create_proposed_screenshot_name(const char *ext)
{
    char *date;
    char *filename;

    date = create_datetime_string();
    filename = lib_msprintf("vice-screen-%s.%s", date, ext);
    g_free(date);
    return filename;
}



/** \brief  Create a filename based on the current datetime and \a ext
 *
 * \param[in]   ext file extension (without the dot)
 *
 * \return  heap-allocated string, owned by VICE, free with lib_free()
 */
static char *create_proposed_video_recording_name(const char *ext)
{
    gchar *date;
    char *filename;

    date = create_datetime_string();
    filename = lib_msprintf("vice-video-%s.%s", date, ext);
    g_free(date);
    return filename;
}



/** \brief  Create a filename based on the current datetime and \a ext
 *
 * \param[in]   ext file extension (without the dot)
 *
 * \return  heap-allocated string, owned by VICE, free with lib_free()
 */
static char *create_proposed_audio_recording_name(const char *ext)
{
    gchar *date;
    char *filename;

    date = create_datetime_string();
    filename = lib_msprintf("vice-audio-%s.%s", date, ext);
    g_free(date);
    return filename;
}



/** \brief  Save a screenshot
 *
 * Pops up a save-file dialog with a proposed filename (ie
 * 'screenshot-197411151210.png'.
 */
static void save_screenshot_handler(void)
{
    const char *display;
    const char *name;
    const char *ext;
    gchar *filename;
    char *title;
    char *proposed;

    display = video_driver_list[video_driver_index].display;
    name = video_driver_list[video_driver_index].name;
    ext = video_driver_list[video_driver_index].ext;

    title = lib_msprintf("Save %s file", display);
    proposed = create_proposed_screenshot_name(ext);

    filename = vice_gtk3_save_file_dialog(title, proposed, TRUE, NULL);
    if (filename != NULL) {
        /* TODO: add extension if not present? */
        if (screenshot_save(name, filename, ui_get_active_canvas()) < 0) {
            vice_gtk3_message_error("VICE Error",
                    "Failed to write screenshot file '%s'", filename);
        } else {
            vice_gtk3_message_info("VICE Info",
                    "Saved screenshot as '%s'", filename);
        }
        g_free(filename);
    }
    lib_free(proposed);
    lib_free(title);
}


/** \brief  Start an audio recording
 *
 * Pops up a save-file dialog with a proposed filename (ie
 * 'audio-recording-197411151210.png'.
 */
static void save_audio_recording_handler(void)
{
    const char *display;
    const char *name;
    const char *ext;
    gchar *filename;
    char *title;
    char *proposed;

    display = audio_driver_list[audio_driver_index].display;
    name = audio_driver_list[audio_driver_index].name;
    ext = audio_driver_list[audio_driver_index].ext;

    title = lib_msprintf("Save %s file", display);
    proposed = create_proposed_audio_recording_name(ext);

    filename = vice_gtk3_save_file_dialog(title, proposed, TRUE, NULL);
    if (filename != NULL) {
        /* XXX: setting resources doesn't exactly help with catching errors */
        resources_set_string("SoundRecordDeviceArg", filename);
        resources_set_string("SoundRecordDeviceName", name);
        g_free(filename);
    }
}


/** \brief  Start recording a video
 *
 * Pops up a save-file dialog with a proposed filename (ie
 * 'video-recording-197411151210.png'.
 */
static void save_video_recording_handler(void)
{
    /* these may be useful once QuickTime is supported */
#if 0
    const char *display;
    const char *name;
#endif
    const char *ext;
    gchar *filename;
    char *title;
    char *proposed;

    debug_gtk3("video driver index = %d.", video_driver_index);

#if 0
    display = video_driver_list[video_driver_index].display;
    name = video_driver_list[video_driver_index].name;
#endif
    /* we dont' have a format->extension mapping, so the format name itself is 
       better than `video_driver_list[video_driver_index].ext' */
    resources_get_string("FFMPEGFormat", &ext);

    title = lib_msprintf("Save %s file", "FFMPEG");
    proposed = create_proposed_video_recording_name(ext);

    filename = vice_gtk3_save_file_dialog(title, proposed, TRUE, NULL);
    if (filename != NULL) {

        const char *driver;
        int ac;
        int vc;
        int ab;
        int vb;

        resources_get_string("FFMPEGFormat", &driver);
        resources_get_int("FFMPEGVideoCodec", &vc);
        resources_get_int("FFMPEGVideoBitrate", &vb);
        resources_get_int("FFMPEGAudioCodec", &ac);
        resources_get_int("FFMPEGAudioBitrate", &ab);

        debug_gtk3("Format = '%s'.", driver);
        debug_gtk3("Video = %d, bitrate %d.", vc, vb);
        debug_gtk3("Audio = %d, bitrate %d.", ac, ab);


        ui_pause_emulation(FALSE);

        /* TODO: add extension if not present? */
        if (screenshot_save("FFMPEG", filename, ui_get_active_canvas()) < 0) {
            vice_gtk3_message_error("VICE Error",
                    "Failed to write video file '%s'", filename);
        }
        g_free(filename);
    }
    lib_free(proposed);
    lib_free(title);
}


/** \brief  Create heap-allocated list of available video drivers
 *
 * Queries the gfxoutputdrv subsystem and builds a list of currently available
 * drivers and stores it in `driver_list`. Must be freed with lib_free() after
 * use.
 */
static void create_video_driver_list(void)
{
    gfxoutputdrv_t *driver;
    int index;

    video_driver_count = gfxoutput_num_drivers();
    debug_gtk3("got %d output drivers.", video_driver_count);

    video_driver_list = lib_malloc((size_t)(video_driver_count + 1) *
            sizeof *video_driver_list);

    index = 0;

    if (video_driver_count > 0) {
        driver = gfxoutput_drivers_iter_init();
        while (driver != NULL) {
            debug_gtk3(".. adding driver '%s'. ext: %s.",
			    driver->name,
			    driver->default_extension);
            video_driver_list[index].display = driver->displayname;
            video_driver_list[index].name = driver->name;
            video_driver_list[index].ext = driver->default_extension;
            index++;
            driver = gfxoutput_drivers_iter_next();
        }
    }
    video_driver_list[index].display = NULL;
    video_driver_list[index].name = NULL;
    video_driver_list[index].ext = NULL;
}


/** \brief  Determine if driver \a name is a video driver
 *
 * \param[in]   name    driver name
 *
 * \return  bool
 *
 * \todo    There has to be a better, more reliable way than this
 */
static int driver_is_video(const char *name)
{
    return strcmp(name, "FFMPEG") == 0 || strcmp(name, "QuickTime") == 0;
}


/** \brief  Create widget to control screenshot parameters
 *
 * This widget exposes controls to alter screenshot parameters, depending on
 * machine class and output file format
 *
 * \param[in]   prefix  resource prefix (either "Doodle", "Koala", or ""/NULL)
 */
static GtkWidget *create_screenshot_param_widget(const char *prefix)
{
    GtkWidget *grid;
    GtkWidget *label;
    int row;
    int doodle = 0;
    int koala = 0;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    /* according to the standard, doing a strcmp() with one or more `NULL`
     * arguments is implementation-defined, so better safe than sorry */
    if (prefix == NULL) {
        debug_gtk3("some idiot passed NULL.");
        return grid;
    }

    if ((strcmp(prefix, "DOODLE") == 0)
            || (strcmp(prefix, "DOODLE_COMPRESSED") == 0)) {
        prefix = "Doodle";  /* XXX: not strictly required since resource names
                                    seem to be case insensitive, but better
                                    safe than sorry */
        doodle = 1;
    } else if ((strcmp(prefix, "KOALA") == 0)
            || (strcmp(prefix, "KOALA_COMPRESSED") == 0)) {
        prefix = "Koala";
        koala = 1;
    }

    if (!koala && !doodle) {
        label = gtk_label_new("No parameters required");
        g_object_set(label, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
        gtk_widget_show_all(grid);
        return grid;
    }

    /* ${FORMAT}OversizeHandling */
    label = gtk_label_new("Oversize handling");
    g_object_set(label, "margin-left", 16, NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    oversize_widget = vice_gtk3_resource_combo_box_int_new_sprintf(
            "%sOversizeHandling", oversize_modes, prefix);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), oversize_widget, 1, 0, 1, 1);

    /* ${FORMAT}UndersizeHandling */
    label = gtk_label_new("Undersize handling");
    g_object_set(label, "margin-left", 16, NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    undersize_widget = vice_gtk3_resource_combo_box_int_new_sprintf(
            "%sUndersizeHandling", undersize_modes, prefix);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), undersize_widget, 1, 1, 1, 1);

    /* ${FORMAT}MultiColorHandling */

    row = 2;    /* from now on, the widgets depend on machine and image type */

    /* DoodleMultiColorHandling */
    if (doodle) {
        label = gtk_label_new("Multi color handling");
        g_object_set(label, "margin-left", 16, NULL);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        multicolor_widget = vice_gtk3_resource_combo_box_int_new_sprintf(
                "%sMultiColorHandling", multicolor_modes, prefix);
        gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), multicolor_widget, 1, row, 1, 1);
        row++;
    }

    /* ${FORMAT}TEDLumaHandling */
    if (machine_class == VICE_MACHINE_PLUS4) {
        label = gtk_label_new("TED luma handling");
        g_object_set(label, "margin-left", 16, NULL);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        ted_luma_widget = vice_gtk3_resource_combo_box_int_new_sprintf(
                "%sTEDLumHandling", ted_luma_modes, prefix);
        gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), ted_luma_widget, 1, row, 1, 1);
        row++;
    }

    /* ${FORMAT}CRTCTextColor */
    if (machine_class == VICE_MACHINE_PET
            || machine_class == VICE_MACHINE_CBM6x0) {
        label = gtk_label_new("CRTC text color");
        g_object_set(label, "margin-left", 16, NULL);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        crtc_textcolor_widget = vice_gtk3_resource_combo_box_int_new_sprintf(
                "%sCRTCTextColor", crtc_colors, prefix);
        gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), crtc_textcolor_widget, 1, row, 1, 1);
        row++;
    }

    gtk_widget_show_all(grid);
    return grid;
}




/** \brief  Create the main 'screenshot' widget
 *
 * \return  GtkGrid
 */
static GtkWidget *create_screenshot_widget(void)
{
    GtkWidget *grid;
    GtkWidget *drv_grid;
    GtkWidget *radio;
    GtkWidget *last;
    GSList *group = NULL;
    int index;
    int grid_index;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    drv_grid = uihelpers_create_grid_with_label("Driver", 1);
    grid_index = 1;
    last = NULL;
    for (index = 0; video_driver_list[index].name != NULL; index++) {
        const char *display = video_driver_list[index].display;
        const char *name = video_driver_list[index].name;

        if (!driver_is_video(name)) {
            radio = gtk_radio_button_new_with_label(group, display);
            g_object_set(radio, "margin-left", 8, NULL);
            gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio),
                    GTK_RADIO_BUTTON(last));
            gtk_grid_attach(GTK_GRID(drv_grid), radio, 0, grid_index, 1, 1);

            g_signal_connect(radio, "toggled",
                    G_CALLBACK(on_screenshot_driver_toggled),
                    GINT_TO_POINTER(index));

            last = radio;
            grid_index++;
        }
    }

    /* this is where the various options go per screenshot driver (for example
     * Koala or Doodle) */
    screenshot_options_grid = uihelpers_create_grid_with_label(
            "Driver options", 1);

    gtk_grid_attach(GTK_GRID(grid), drv_grid, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), screenshot_options_grid, 1, 0, 1, 1);

    update_screenshot_options_grid(create_screenshot_param_widget(""));

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create the main 'sound recording' widget
 *
 * \return  GtkGrid
 */
static GtkWidget *create_sound_widget(void)
{
    GtkWidget *grid;
    GtkWidget *drv_grid;
    GtkWidget *radio;
    GtkWidget *last;
    GSList *group = NULL;
    int index;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    drv_grid = uihelpers_create_grid_with_label("Driver", 1);
    last = NULL;
    for (index = 0; audio_driver_list[index].name != NULL; index++) {
        const char *display = audio_driver_list[index].display;

        radio = gtk_radio_button_new_with_label(group, display);
        g_object_set(radio, "margin-left", 8, NULL);
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio),
                GTK_RADIO_BUTTON(last));
        gtk_grid_attach(GTK_GRID(drv_grid), radio, 0, index + 1, 1, 1);

        g_signal_connect(radio, "toggled",
                G_CALLBACK(on_audio_driver_toggled),
                GINT_TO_POINTER(index));

        last = radio;
    }

    gtk_grid_attach(GTK_GRID(grid), drv_grid, 0, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create the main 'video recording' widget
 *
 * \return  GtkGrid
 */
static GtkWidget *create_video_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *combo;
    int index;

    GtkWidget *selection_grid;
    GtkWidget *options_grid;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("Video driver");
    g_object_set(label, "margin-left", 16, NULL);

    combo = gtk_combo_box_text_new();
    for (index = 0; video_driver_list[index].name != NULL; index++) {
        const char *display = video_driver_list[index].display;
        const char *name = video_driver_list[index].name;

        if (driver_is_video(name)) {
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), name, display);
        }
    }
    gtk_widget_set_hexpand(combo, TRUE);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

    selection_grid = uihelpers_create_grid_with_label("Driver selection", 2);
    gtk_grid_set_column_spacing(GTK_GRID(selection_grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(selection_grid), 8);
    gtk_grid_attach(GTK_GRID(selection_grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(selection_grid), combo, 1, 1, 1, 1);
    gtk_widget_show_all(selection_grid);

    gtk_grid_attach(GTK_GRID(grid), selection_grid, 0, 0, 1, 1);

    /* grid around ffmpeg/quicktime options */
    options_grid = uihelpers_create_grid_with_label("Driver options", 1);
    gtk_grid_set_column_spacing(GTK_GRID(options_grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(options_grid), 8);

/* XXX: this obviously needs a cleaner solution which also handles QuickTime
 *      on MacOS
 */
#ifdef HAVE_FFMPEG
    gtk_grid_attach(GTK_GRID(options_grid), ffmpeg_widget_create(), 0, 1, 1,1);
#endif
    video_driver_options_grid = options_grid;



    gtk_grid_attach(GTK_GRID(grid), options_grid, 0, 1, 1, 1);


    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create the content widget for the dialog
 *
 * Contains a GtkStack and GtkStackSwitcher to switch between 'screenshot',
 * 'sound' and 'video' sub widgets.
 *
 * \return  GtkGrid
 */
static GtkWidget *create_content_widget(void)
{
    GtkWidget *grid;
    GtkWidget *switcher;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    stack = gtk_stack_new();
    gtk_stack_add_titled(GTK_STACK(stack), create_screenshot_widget(),
            CHILD_SCREENSHOT, "Screenshot");
    gtk_stack_add_titled(GTK_STACK(stack), create_sound_widget(),
            CHILD_SOUND, "Sound recording");
    gtk_stack_add_titled(GTK_STACK(stack), create_video_widget(),
            CHILD_VIDEO, "Video recording");
    gtk_stack_set_transition_type(GTK_STACK(stack),
            GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 1000);
    /* avoid resizing the dialog when switching */
    gtk_stack_set_homogeneous(GTK_STACK(stack), TRUE);

    switcher = gtk_stack_switcher_new();
    gtk_widget_set_halign(switcher, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(switcher, TRUE);
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(switcher), GTK_STACK(stack));
    /* make all titles of the switcher the same size */
    gtk_box_set_homogeneous(GTK_BOX(switcher), TRUE);

    gtk_widget_show_all(stack);
    gtk_widget_show_all(switcher);

    gtk_grid_attach(GTK_GRID(grid), switcher, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), stack, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Show dialog to save screenshot, record sound and/or video
 *
 * \param[in]   parent  parent widget (unused)
 * \param[in]   data    extra data (unused)
 */
void uimedia_dialog_show(GtkWidget *parent, gpointer data)
{
    GtkWidget *dialog;
    GtkWidget *content;

    /*
     * Pause emulation, the ui_emulation_is_paused() check is required since
     * the ui_pause_emulation() function is a little weird: when passed FALSE
     * it unpauses, when passed TRUE it toggles the paused state.
     */
    if (!ui_emulation_is_paused()) {
        ui_pause_emulation(TRUE);
    }

    /* create driver list */
    if (machine_class != VICE_MACHINE_VSID) {
        create_video_driver_list();
    }

    dialog = gtk_dialog_new_with_buttons(
            "Record media file",
            ui_get_active_window(),
            GTK_DIALOG_MODAL,
            "Save", RESPONSE_SAVE,
            "Close", GTK_RESPONSE_DELETE_EVENT,
            NULL);

    /* add content widget */
    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    if (machine_class != VICE_MACHINE_VSID) {
        gtk_container_add(GTK_CONTAINER(content), create_content_widget());
    } else {
        gtk_container_add(GTK_CONTAINER(content), create_sound_widget());
    }

    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    g_signal_connect(dialog, "response", G_CALLBACK(on_response), NULL);
    g_signal_connect(dialog, "destroy", G_CALLBACK(on_dialog_destroy), NULL);

    gtk_widget_show_all(dialog);
}


/** \brief  Stop audio or video recording, if active
 *
 * \return  TRUE, so the emulated machine doesn't get the shortcut key
 */
gboolean uimedia_stop_recording(GtkWidget *parent, gpointer data)
{
    debug_gtk3("Stopping media recording.");

    /* stop sound recording, if active */
    if (sound_is_recording()) {
        sound_stop_recording();
    }
    /* stop video recording */
    if (screenshot_is_recording()) {
        screenshot_stop_recording();
    }
    return TRUE;
}
