/*
 * uivsidcontrol.c - UI controls for VSID
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

#include <stdlib.h>

#include "lib.h"
#include "ui.h"
#include "uiarch.h"
#include "vsiduiunix.h"

static GtkWidget *current_line;
static char *author, *copyright, *name, *vsidsync, *model, *irq, *info;
static int tune, numtunes, deftune;
static char *line;

static void update_line(void)
{
    char *a1, *a2, *a3;
    if (line) {
        lib_free(line);
    }
    a1 = (char*)convert_utf8((unsigned char*)name);
    a2 = (char*)convert_utf8((unsigned char*)author);
    a3 = (char*)convert_utf8((unsigned char*)copyright);
    line = lib_msprintf(_("Name: %s\nAuthor: %s\nCopyright: %s\n\nTune: %d of %d (Default: %d)\n%s\nModel: %s\nIRQ: %s\n\n%s"), 
                        a1, a2, a3, tune, numtunes, deftune, vsidsync, model, irq, info);
    gtk_label_set_text(GTK_LABEL(current_line), line);
    lib_free(a1);
    lib_free(a2);
    lib_free(a3);
}

ui_window_t build_vsid_ctrl_widget(void)
{
    GtkWidget *event_box, *f;

    event_box = gtk_event_box_new();
    f = gtk_frame_new("");
    gtk_frame_set_shadow_type((GtkFrame*)f, GTK_SHADOW_NONE);

    current_line = gtk_label_new("");
    gtk_container_add(GTK_CONTAINER(f), current_line);
    gtk_widget_show(current_line);

    gtk_container_add(GTK_CONTAINER(event_box), f);
    gtk_widget_show(f);

    name = lib_stralloc("-");
    tune = 0;
    numtunes = 0;
    author = lib_stralloc("-");
    copyright = lib_stralloc("-");
    model = lib_stralloc("-");
    vsidsync = lib_stralloc("-");
    irq = lib_stralloc("-");
    info = lib_stralloc("-");

    update_line();
    return event_box;
}

void shutdown_vsid_ctrl_widget(void)
{
    lib_free(name);
    lib_free(author);
    lib_free(copyright);
    lib_free(model);
    lib_free(vsidsync);
    lib_free(irq);
    lib_free(info);
    lib_free(line);
}

void ui_vsid_setpsid(const char *psid)
{
    lib_free(name);
    name = lib_stralloc(psid);
    update_line();
}

void ui_vsid_settune(const int t)
{
    tune = t;
    update_line();
}

void ui_vsid_setdeftune(const int t)
{
    deftune = t;
    update_line();
}

void ui_vsid_setnumtunes(const int t)
{
    numtunes = t;
    update_line();
}

void ui_vsid_setauthor(const char *a)
{
    lib_free(author);
    author = lib_stralloc(a);
    update_line();
}
void ui_vsid_setcopyright(const char *c)
{
    lib_free(copyright);
    copyright = lib_stralloc(c);
    update_line();
}

void ui_vsid_setmodel(const char *c)
{
    lib_free(model);
    model = lib_stralloc(c);
    update_line();
}

void ui_vsid_setsync(const char *c)
{
    lib_free(vsidsync);
    vsidsync = lib_stralloc(c);
    update_line();
}

void ui_vsid_setirq(const char *c)
{
    lib_free(irq);
    irq = lib_stralloc(c);
    update_line();
}

void ui_vsid_setdrv(const char *c)
{
    char *p;
    lib_free(info);
    info = lib_stralloc(c);
    p = info;
    while (*p) {
        if (*p == '=') {
            *p = ':';
        } else if (*p == ',') {
            *p = ' ';
            ++p;
            *p = '\n';
        }
        ++p;
    }
    update_line();
}
