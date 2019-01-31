/*
 * about.c - Info about the VICE project, including the GPL.
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

#include "info.h"
#include "lib.h"
#include "uiarch.h"
#include "util.h"
#include "version.h"
#include "vicefeatures.h"

#include <string.h>

#ifdef USE_SVN_REVISION
#include "svnversion.h"
#endif

GtkWidget *about = NULL;
gchar **authors = NULL;
gchar *transl = NULL;

static void license_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    ui_show_text(_("VICE is FREE software!"), info_license_text, 500, 300);
}

static void warranty_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    ui_show_text(_("No warranty!"), info_warranty_text, 500, 300);
}

static void contrib_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    ui_show_text(_("Contributors to the VICE project"), info_contrib_text, 500, 300);
}


static void destroyed_cb(GtkWidget *self, GtkWidget** widget_pointer)
{
    gchar **str;


    printf("DESTROY CALLED\n");
    abort();
    gtk_widget_destroyed(self, widget_pointer);
    lib_free(transl);
    str = authors;
    while (*str != NULL) {
        g_free(*str);
        ++str;
    }
    printf("FREE THE AUTHORS!\n");
    lib_free(authors);
}

static char *get_compiletime_features(void)
{
    feature_list_t *list;
    char *str, *lstr;
    unsigned int len = 0;

    list = vice_get_feature_list();
    while (list->symbol) {
        len += strlen(list->descr) + strlen(list->symbol) + (15);
        ++list;
    }
    str = lib_malloc(len);
    lstr = str;
    list = vice_get_feature_list();
    while (list->symbol) {
        sprintf(lstr, "%4s\t%s (%s)\n", list->isdefined ? "yes " : "no  ", list->descr, list->symbol);
        lstr += strlen(lstr);
        ++list;
    }
    return str;
}

static void features_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    char *features = NULL;
    features = get_compiletime_features();
    ui_show_text(_("Compile time features"), features, 500, 300);
    lib_free(features);
}

static void response_cb(GtkWidget *w, gint id, gpointer data)
{
    if ((id == GTK_RESPONSE_CANCEL) || (id == GTK_RESPONSE_DELETE_EVENT)) {
        gtk_widget_hide(GTK_WIDGET(data));
        ui_unblock_shells();
    }
}

static gchar *convert_text(char *text)
{
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
            util_string_set(&utf8_text, _("Text cannot be displayed."));
        }
    }
    return utf8_text;
}


/* FIXME: needs serious cleanup: either use stdlib, VICE's lib.c or GLib */
static gchar **get_team(void)
{
    vice_team_t *list;
    gchar **str, **lstr;
    char *text;
    unsigned int len = 0;
    gchar *headline =
#ifdef __GNUC__
    _("The VICE Team:");
#else
    "The VICE Team:";
#endif
    gchar *footer1 =
#ifdef __GNUC__
    _("Official VICE homepage:");
#else
    "Official VICE homepage:";
#endif
    gchar *footer2 = "http://vice-emu.sourceforge.net/";

    list = core_team;
    while (list->name) {
        ++len;
        ++list;
    }
    len += 6; /* 5 additional lines plus one end pointer */
    str = lib_malloc(len * sizeof(gchar*));

    lstr = str;

    *lstr = lib_stralloc(headline); lstr++;
    *lstr = lib_stralloc(""); lstr++;

    list = core_team;
    while (list->name) {
        len = strlen(list->name) + strlen(list->years) + (15);
        text = lib_malloc(len);
        sprintf(text, "Copyright @ %s %s", list->years, list->name);
        *lstr = convert_text(text);
        lib_free(text);
        ++lstr;
        ++list;
    }
    *lstr = lib_stralloc(""); lstr++;
    *lstr = lib_stralloc(footer1); lstr++;
    *lstr = lib_stralloc(footer2); lstr++;
    *lstr = NULL;
    return str;
}

static gchar *get_translators(void)
{
    vice_trans_t *list;
    gchar *str, *lstr, *utf8txt;
    unsigned int len = 0;
    char *text;

    list = trans_team;
    while (list->name) {
        len += strlen(list->name) + strlen(list->language) + (15);
        ++list;
    }
    str = lib_malloc(len);

    lstr = str;

    list = trans_team;
    while (list->name) {
        len = strlen(list->name) + strlen(list->language) + (15);
        text = lib_malloc(len);
        sprintf(text, "%s - %s\n", list->name, list->language);
        utf8txt = convert_text(text);
        strcpy(lstr, utf8txt);
        lstr += strlen(utf8txt);
        lib_free(text);
        g_free(utf8txt);
        ++list;
    }
    return str;
}

void ui_about(gpointer data)
{
    GtkWidget *button;

    if (!authors) {
        authors = get_team();
    }
    if (!transl) {
        transl = get_translators();
    }

    if (!about) {
        /* GdkPixbuf *logo = gdk_pixbuf_new_from_file ("logo.png", NULL); */
        about = g_object_new(GTK_TYPE_ABOUT_DIALOG,
                             "name", "V I C E",
#ifdef USE_SVN_REVISION
                             "version", VERSION " r" VICE_SVN_REV_STRING " (GTK+)",
#else
                             "version", VERSION " (GTK+)",
#endif
                             "copyright", _("(c) 1998 - 2018 The VICE Team"),
                             "comments", "Versatile Commodore Emulator",
                             "authors", authors,
                             "documenters", doc_team,
                             "translator-credits", transl,
                             /* "logo", logo, */
                             NULL);
        g_signal_connect(G_OBJECT(about), "destroy", G_CALLBACK(destroyed_cb), &about);
        button = gtk_dialog_add_button(GTK_DIALOG(about), _("License"), GTK_RESPONSE_OK);
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(license_cb), NULL);
        button = gtk_dialog_add_button(GTK_DIALOG(about), _("Warranty"), GTK_RESPONSE_OK);
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(warranty_cb), NULL);
        button = gtk_dialog_add_button(GTK_DIALOG(about), _("Contributors"), GTK_RESPONSE_OK);
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(contrib_cb), NULL);
        button = gtk_dialog_add_button(GTK_DIALOG(about), _("Features"), GTK_RESPONSE_OK);
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(features_cb), NULL);
        g_signal_connect(G_OBJECT(about), "response", G_CALLBACK(response_cb), about);
    } else {
        gdk_window_show(gtk_widget_get_window(about));
        gdk_window_raise(gtk_widget_get_window(about));
    }

    gtk_window_set_modal(GTK_WINDOW(about), TRUE);
    ui_make_window_transient(get_active_toplevel(), about);
    gtk_widget_show(about);
    ui_block_shells();
}
