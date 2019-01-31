/*
 * uidialogs.c - GTK only, various simple dialogs
 *
 * Written by
 *  Ettore Perazzoli
 *  Oliver Schaertel
 *  pottendo <pottendo@gmx.net>
 *  groepaz <groepaz@gmx.net>
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

/* #define DEBUG_X11UI */

#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

#include "vice.h"

#include <string.h>

#include "fullscreenarch.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "ui.h"
#include "uiarch.h"
#include "util.h"
#include "vsync.h"
#ifdef USE_UI_THREADS
#include "videoarch.h"
#include "ui-threads.h"
#endif

#include "uicommands.h"

/* ------------------------------------------------------------------------- */

/* Button callbacks.  */

/* appears to be unused */
#if 0
#define DEFINE_BUTTON_CALLBACK(button)                             \
    static void cb_##button(GtkWidget *w, ui_callback_data_t client_data) \
    {                                                              \
        *((ui_button_t *)client_data) = button;                    \
    }

    DEFINE_BUTTON_CALLBACK(UI_BUTTON_OK)
    DEFINE_BUTTON_CALLBACK(UI_BUTTON_CANCEL)
    DEFINE_BUTTON_CALLBACK(UI_BUTTON_YES)
    DEFINE_BUTTON_CALLBACK(UI_BUTTON_NO)
    DEFINE_BUTTON_CALLBACK(UI_BUTTON_CLOSE)
    DEFINE_BUTTON_CALLBACK(UI_BUTTON_MON)
    DEFINE_BUTTON_CALLBACK(UI_BUTTON_DEBUG)
    DEFINE_BUTTON_CALLBACK(UI_BUTTON_RESET)
    DEFINE_BUTTON_CALLBACK(UI_BUTTON_HARDRESET)
    DEFINE_BUTTON_CALLBACK(UI_BUTTON_CONTENTS)
    DEFINE_BUTTON_CALLBACK(UI_BUTTON_AUTOSTART)
#endif


static void text_response(GtkDialog *dialog, gint arg1, gpointer user_data)
{
    GtkWidget *show_text = user_data;

    /* whatever response we get, we want to close / destroy the window */
    if (show_text) {
        gtk_widget_destroy(show_text);
    }
}

static GtkWidget* build_show_text(const gchar *text, int width, int height)
{
    GtkWidget *show_text, *textw, *scrollw;
    GtkTextBuffer *tb;
    gchar *utf8_text;
    GError *error = NULL;

    /*
        convert text to UTF-8 for GTK+. Try first using the locale, since that
        allows to display text translated by the translation system. Use
        ISO-8859-1 as a fallback if that fails.
     */
    utf8_text = g_locale_to_utf8(text, strlen(text), NULL, NULL, &error);
    if (utf8_text == NULL) {
        log_warning(LOG_DEFAULT, "Can not convert text to UTF-8 using locale, using ISO-8859-1 as fallback.");
        g_error_free(error);
        utf8_text = g_convert(text, strlen(text), "UTF-8", "ISO-8859-1", NULL, NULL, &error);
        if (utf8_text == NULL) {
            log_error(LOG_ERR, "Can not convert text to UTF-8.");
            g_error_free(error);
            util_string_set(&utf8_text, _("Text cannot be displayed.\n"));
        }
    }

    show_text = gtk_dialog_new_with_buttons("", NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
    gtk_widget_set_size_request(show_text, width, height);

    scrollw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    textw = gtk_text_view_new();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(textw), GTK_WRAP_WORD);
    tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textw));
    gtk_text_buffer_set_text(tb, utf8_text, -1);
    gtk_container_add(GTK_CONTAINER(scrollw), textw);
    gtk_widget_show(textw);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(show_text))), scrollw, TRUE, TRUE, 0);
    gtk_widget_show(scrollw);
    g_free(utf8_text);
    return show_text;
}

static GtkWidget *build_confirm_dialog(GtkWidget **confirm_dialog_message)
{
    GtkWidget *confirm_dialog;

    confirm_dialog = gtk_dialog_new_with_buttons("", NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_NO,
                                                 GTK_RESPONSE_NO, GTK_STOCK_YES, GTK_RESPONSE_YES, NULL);
    *confirm_dialog_message = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(confirm_dialog))), *confirm_dialog_message,TRUE,TRUE,0);
    gtk_widget_show(*confirm_dialog_message);
    gtk_widget_show(confirm_dialog);
    gtk_widget_set_size_request(confirm_dialog, (gtk_widget_get_allocated_width(gtk_dialog_get_action_area(GTK_DIALOG(confirm_dialog))) * 40) / 30, 
                                    gtk_widget_get_allocated_height(gtk_dialog_get_action_area(GTK_DIALOG(confirm_dialog))) * 3);

    return confirm_dialog;
}

static GtkWidget *build_yesno_dialog(GtkWidget **yesno_dialog_message)
{
    GtkWidget *yesno_dialog;

    yesno_dialog = gtk_dialog_new_with_buttons("", NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_NO,
                                                 GTK_RESPONSE_NO, GTK_STOCK_YES, GTK_RESPONSE_YES, NULL);
    *yesno_dialog_message = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(yesno_dialog))), *yesno_dialog_message,TRUE,TRUE,0);
    gtk_widget_show(*yesno_dialog_message);
    gtk_widget_show(yesno_dialog);
    gtk_widget_set_size_request(yesno_dialog, gtk_widget_get_allocated_width(gtk_dialog_get_action_area(GTK_DIALOG(yesno_dialog))) * 2, 
                                    gtk_widget_get_allocated_height(gtk_dialog_get_action_area(GTK_DIALOG(yesno_dialog))) * 3);
    return yesno_dialog;
}

/* Ask for a string.  The user can confirm or cancel. */
ui_button_t ui_input_string(const char *title, const char *prompt, char *buf, unsigned int buflen)
{
    GtkWidget *input_dialog, *entry, *label;
    gint res;
    ui_button_t ret;

    vsync_suspend_speed_eval();
    input_dialog = gtk_dialog_new_with_buttons(title, NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
    g_signal_connect(G_OBJECT(input_dialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &input_dialog);

    entry = gtk_entry_new();

    label = gtk_label_new(prompt);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(input_dialog))), label, FALSE, FALSE, 0);
    gtk_widget_show(label);

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(input_dialog))), entry, FALSE, FALSE, 0);
    gtk_entry_set_text(GTK_ENTRY(GTK_ENTRY(entry)), buf);
    gtk_widget_show(entry);

    gtk_dialog_set_default_response(GTK_DIALOG(input_dialog), GTK_RESPONSE_ACCEPT);
    ui_popup(input_dialog, title, FALSE);
    res = gtk_dialog_run(GTK_DIALOG(input_dialog));
    ui_popdown(input_dialog);

    if ((res == GTK_RESPONSE_ACCEPT) && input_dialog) {
        strncpy(buf, gtk_entry_get_text(GTK_ENTRY(entry)), buflen);
        ret = UI_BUTTON_OK;
    } else {
        ret = UI_BUTTON_CANCEL;
    }

    if (input_dialog) {
        gtk_widget_destroy(input_dialog);
    }

    return ret;
}

/* Display a text to the user. */
void ui_show_text(const char *title, const char *text, int width, int height)
{
    GtkWidget *show_text;

    vsync_suspend_speed_eval();
    show_text = build_show_text((const gchar*)text, width, height);
    g_signal_connect(G_OBJECT(show_text), "destroy", G_CALLBACK(gtk_widget_destroyed), &show_text);
    g_signal_connect(G_OBJECT(show_text), "response", G_CALLBACK(text_response), show_text);

    ui_make_window_transient(get_active_toplevel(), show_text);
    gtk_window_set_modal(GTK_WINDOW(show_text), TRUE);
    gtk_window_set_title(GTK_WINDOW(show_text), title);
    gtk_widget_show(show_text);
}

/* Ask for a confirmation. */
ui_button_t ui_ask_confirmation(const char *title, const char *text)
{
    static GtkWidget *confirm_dialog, *confirm_dialog_message;
    gint res;

    vsync_suspend_speed_eval();
    if (!confirm_dialog) {
        confirm_dialog = build_confirm_dialog(&confirm_dialog_message);
        g_signal_connect(G_OBJECT(confirm_dialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &confirm_dialog);
    }

    gtk_label_set_text(GTK_LABEL(confirm_dialog_message),text);

    ui_popup(confirm_dialog, title, FALSE);
    res = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
    ui_popdown(confirm_dialog);

    return (res == GTK_RESPONSE_YES) ? UI_BUTTON_YES : (res == GTK_RESPONSE_NO) ? UI_BUTTON_NO : UI_BUTTON_CANCEL;
}

ui_button_t ui_ask_yesno(const char *title, const char *text)
{
    static GtkWidget *yesno_dialog = NULL, *yesno_dialog_message = NULL;
    gint res;

    vsync_suspend_speed_eval();
    if (!yesno_dialog) {
        yesno_dialog = build_yesno_dialog(&yesno_dialog_message);
        g_signal_connect(G_OBJECT(yesno_dialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &yesno_dialog);
    }

    gtk_label_set_text(GTK_LABEL(yesno_dialog_message),text);

    ui_popup(yesno_dialog, title, FALSE);
    res = gtk_dialog_run(GTK_DIALOG(yesno_dialog));
    ui_popdown(yesno_dialog);

    return (res == GTK_RESPONSE_YES) ? UI_BUTTON_YES : UI_BUTTON_NO;
}

ui_button_t ui_change_dir(const char *title, const char *prompt, char *buf, unsigned int buflen)
{
    GtkWidget *fc;
    gint res;
    gchar *fname = NULL;
    ui_button_t r;

    fc = gtk_file_chooser_dialog_new(title, GTK_WINDOW(get_active_toplevel()), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    ui_popup(fc, title, FALSE);
    res = gtk_dialog_run(GTK_DIALOG(fc));
    ui_popdown(fc);

    if ((res == GTK_RESPONSE_ACCEPT) && (fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc)))) {
        strncpy(buf, fname, buflen);
        r = UI_BUTTON_OK;
    } else {
        r = UI_BUTTON_CANCEL;
    }

    g_free(fname);
    return r;
}


/* Report a message to the user, allow different buttons. */
ui_jam_action_t ui_jam_dialog(const char *format, ...)
{
    char str[1024];
    va_list ap;
    static GtkWidget *jam_dialog, *message;
    gint res;

    if (console_mode) {
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);
        exit(0);
    }

    vsync_suspend_speed_eval();
    jam_dialog = gtk_dialog_new_with_buttons("", NULL, GTK_DIALOG_DESTROY_WITH_PARENT, _("Reset"), 0, _("Hard Reset"), 1, _("Monitor"), 2, _("Continue"), 3, NULL);
    g_signal_connect(G_OBJECT(jam_dialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &jam_dialog);

    va_start(ap, format);
    vsprintf(str, format, ap);
    va_end(ap);
    message = gtk_label_new(str);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(jam_dialog))), message, TRUE, TRUE, 0);
    gtk_widget_show(message);
    gtk_dialog_set_default_response(GTK_DIALOG(jam_dialog), 0);

    ui_popup(jam_dialog, "VICE", FALSE);
    res = gtk_dialog_run(GTK_DIALOG(jam_dialog));
    ui_popdown(jam_dialog);
    if (jam_dialog) {
        gtk_widget_destroy(jam_dialog);
    }

    ui_dispatch_events();

    switch (res) {
        case 3:
            return UI_JAM_NONE;
        case 2:
            ui_restore_mouse();
#ifdef HAVE_FULLSCREEN
            fullscreen_suspend(0);
#endif
            return UI_JAM_MONITOR;
        case 1:
            return UI_JAM_HARD_RESET;
        case 0:
        default:
            return UI_JAM_RESET;
    }

    return 0;
}

int ui_extend_image_dialog(void)
{
    ui_button_t b;

    vsync_suspend_speed_eval();
    /* FIXME: this must be updated to deal with all kinds of images/sizes */
    b = ui_ask_yesno(_("Extend disk image"), (_("Do you want to extend the disk image?")));
    return (b == UI_BUTTON_YES) ? 1 : 0;
}

/* ------------------------------------------------------------------------- */

/* Message Helper */
static void ui_message2(const GtkMessageType type, const char *msg, const char *title)
{
    static GtkWidget* msgdlg;

    vsync_suspend_speed_eval();
    msgdlg = gtk_message_dialog_new(GTK_WINDOW(get_active_toplevel()), GTK_DIALOG_DESTROY_WITH_PARENT, type, GTK_BUTTONS_OK, msg, NULL);

    ui_popup(msgdlg, title, FALSE);
    gtk_dialog_run(GTK_DIALOG(msgdlg));
    ui_unblock_shells();        /* ui_popdown can't be used in message_boxes */
    gtk_widget_destroy(msgdlg);

    /* still needed ? */
    ui_check_mouse_cursor();
    ui_dispatch_events();
}

/* Report a message to the user.  */
void ui_message(const char *format, ...)
{
    va_list ap;
    char str[1024];

    va_start(ap, format);
    vsprintf(str, format, ap);
    va_end(ap);
    ui_message2(GTK_MESSAGE_INFO, str, _("VICE Message"));
}

/* Report an error to the user.  */
void ui_error(const char *format, ...)
{
    va_list ap;
    char str[1024];

    va_start(ap, format);
    vsprintf(str, format, ap);
    va_end(ap);
    ui_message2(GTK_MESSAGE_ERROR, str, _("VICE Error!"));
}

void ui_exit(void)
{
    ui_button_t b;
    int value1, value2;
    char *s = util_concat(_("Exit "), (machine_class != VICE_MACHINE_VSID) ? machine_name : "SID", 
                          _(" emulator"), NULL);

#ifdef HAVE_FULLSCREEN
    fullscreen_suspend(1);
#endif
    resources_get_int("ConfirmOnExit", &value1);
    resources_get_int("SaveResourcesOnExit", &value2);

    b = UI_BUTTON_YES;
    if ((value1) && (!value2)) {
        b = ui_ask_yesno(s, _("Do you really want to exit?"));
    }

    if (b == UI_BUTTON_YES) {
        if (value2) {
            b = UI_BUTTON_YES;
            if (value1) {
                b = ui_ask_confirmation(s, _("Save the current settings?"));
            }
            if (b == UI_BUTTON_YES) {
                if (resources_save(NULL) < 0) {
                    ui_error(_("Cannot save settings."));
                    b = UI_BUTTON_CANCEL;
                }
            }
        }
        if (b != UI_BUTTON_CANCEL) {
            /* ui_autorepeat_on(); */
            ui_restore_mouse();
#ifdef HAVE_FULLSCREEN
            fullscreen_suspend(0);
#endif
            ui_dispatch_events();

            lib_free(s);
#ifdef USE_UI_THREADS
            dthread_shutdown();
#endif
            exit(0);
        };
    }
    lib_free(s);
    vsync_suspend_speed_eval();
}

/* open the manual using gnome desktop mechanism */
int ui_open_manual(const char *path)
{
/* gtk_show_uri exists since 2.14 */
#if GTK_CHECK_VERSION(2, 14, 0)
    GError *error = NULL;
    gboolean res;
    char *uri;
    /* first try opening the pdf */
    uri = util_concat("file://", path, "vice.pdf", NULL);
    res = gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, &error);
    lib_free(uri);
    g_clear_error(&error);
    if (res) {
        return 0;
    }
    /* try opening the html doc */
    uri = util_concat("file://", path, "vice_toc.html", NULL);
    res = gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, &error);
    lib_free(uri);
    g_clear_error(&error);
    if (res) {
        return 0;
    }
#endif
    return -1;
}
