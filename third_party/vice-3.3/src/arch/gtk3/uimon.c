/** \file   uimon.c
 * \brief   Native GTK3 UI monitor stuff
 *
 * \author  Fabrizio Gennari <fabrizio.ge@tiscali.it>
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
 *
 */

#include "vice.h"

#include <stdlib.h>
#include <string.h>

#include "novte/novte.h"

#define vte_terminal_new novte_terminal_new
#define vte_terminal_feed novte_terminal_feed
#define vte_terminal_get_column_count novte_terminal_get_column_count
#define vte_terminal_copy_clipboard novte_terminal_copy_clipboard
#define vte_terminal_get_row_count novte_terminal_get_row_count
#define vte_terminal_set_scrollback_lines novte_terminal_set_scrollback_lines
#define vte_terminal_set_scroll_on_output novte_terminal_set_scroll_on_output
#define vte_terminal_get_char_width novte_terminal_get_char_width
#define vte_terminal_get_char_height novte_terminal_get_char_height

#define VTE_TERMINAL(x) NOVTE_TERMINAL(x)
#define VteTerminal NoVteTerminal

#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#if (defined(sun) || defined(__sun)) && (defined(__SVR4) || defined(__svr4__))
#include <sys/stat.h>
#endif

#include "console.h"
#include "debug_gtk3.h"
#include "monitor.h"
#include "resources.h"
#include "lib.h"
#include "ui.h"
#include "linenoise.h"
#include "uimon.h"
#include "uimonarch.h"
#include "uimon-fallback.h"
#include "mon_command.h"
#include "vsync.h"
#include "uidata.h"

struct console_private_s {
    GtkWidget *window;
    GtkWidget *term;
    char *input_buffer;
} fixed = {NULL, NULL, NULL};

static console_t vte_console;
static linenoiseCompletions command_lc = {0, NULL};
static linenoiseCompletions need_filename_lc = {0, NULL};

/* FIXME: this should perhaps be done using some function from archdep */
static int is_dir(struct dirent *de)
{
#if 0 /* FIXME: mingw */

#if (defined(sun) || defined(__sun)) && (defined(__SVR4) || defined(__svr4__))
    struct stat t;

    stat(de->d_name, &t);
    if ((t.st_mode & S_IFMT) == S_IFDIR) {
        return 1;
    }
#else
    if (de->d_type == DT_DIR) {
        return 1;
    }
#endif

#endif
    return 0;
}

static int native_monitor(void)
{
    int res = 0;
    resources_get_int("NativeMonitor", &res);
    return res;
}

void uimon_write_to_terminal(struct console_private_s *t,
                       const char *data,
                       glong length)
{
    if(t->term) {
        vte_terminal_feed(VTE_TERMINAL(t->term), data, length);
    }
}


int uimon_get_columns(struct console_private_s *t)
{
    if(t->term) {
        return vte_terminal_get_column_count(VTE_TERMINAL(t->term));
    }
    return 80;
}

static char* append_char_to_input_buffer(char *old_input_buffer, char new_char)
{
    char* new_input_buffer = lib_msprintf("%s%c",
        old_input_buffer ? old_input_buffer : "",
        new_char);
    lib_free(old_input_buffer);
    return new_input_buffer;
}

static char* append_string_to_input_buffer(char *old_input_buffer, GtkWidget *terminal, GdkAtom clipboard_to_use)
{
    GtkClipboard *clipboard = gtk_widget_get_clipboard(terminal, clipboard_to_use);
    gchar *new_string = gtk_clipboard_wait_for_text(clipboard);

    if (new_string != NULL) {
        char *new_input_buffer = lib_realloc(old_input_buffer, strlen(old_input_buffer) + strlen(new_string) + 1);
        char *char_in, *char_out = new_input_buffer + strlen(new_input_buffer);

        for (char_in = new_string; *char_in; char_in++) {
#if CHAR_MIN < 0
            if (*char_in < 0 || *char_in >= 32) {
#else
            /* char is unsigned on raspberry Pi 2B with GCC */
            if (*char_in >= 32) {
#endif
                *char_out++ = *char_in;
            }
        }
        *char_out = 0;
        g_free(new_string);

        return new_input_buffer;
    }
    return old_input_buffer;
}

static gboolean plain_key_pressed(char **input_buffer, guint keyval)
{
    switch (keyval) {
        default:
            if(keyval >= GDK_KEY_space && keyval <= GDK_KEY_ydiaeresis) {
                *input_buffer = append_char_to_input_buffer(*input_buffer, (char)keyval);
                return TRUE;
            }
            return FALSE;
        case GDK_KEY_Return:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 13);
            return TRUE;
        case GDK_KEY_BackSpace:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 127);
            return TRUE;
        case GDK_KEY_Left:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 2);
            return TRUE;
        case GDK_KEY_Right:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 6);
            return TRUE;
        case GDK_KEY_Up:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 16);
            return TRUE;
        case GDK_KEY_Down:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 14);
            return TRUE;
        case GDK_KEY_Tab:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 9);
            return TRUE;
        case GDK_KEY_Delete:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 4);
            return TRUE;
        case GDK_KEY_Home:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 1);
            return TRUE;
        case GDK_KEY_End:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 5);
            return TRUE;
        case GDK_KEY_dead_tilde:
            *input_buffer =
                append_char_to_input_buffer(*input_buffer, GDK_KEY_asciitilde);
            return TRUE;
    }
}

static gboolean ctrl_plus_key_pressed(char **input_buffer, guint keyval, GtkWidget *terminal)
{
    switch (keyval) {
        default:
            return FALSE;
        case GDK_KEY_h:
        case GDK_KEY_H:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 127);
            return TRUE;
        case GDK_KEY_b:
        case GDK_KEY_B:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 2);
            return TRUE;
        case GDK_KEY_f:
        case GDK_KEY_F:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 6);
            return TRUE;
        case GDK_KEY_p:
        case GDK_KEY_P:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 16);
            return TRUE;
        case GDK_KEY_n:
        case GDK_KEY_N:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 14);
            return TRUE;
        case GDK_KEY_t:
        case GDK_KEY_T:
            *input_buffer = append_char_to_input_buffer(*input_buffer, 20);
            return TRUE;
        case GDK_KEY_d:
        case GDK_KEY_D:
            /* ctrl-d, remove char at right of cursor */
            *input_buffer = append_char_to_input_buffer(*input_buffer, 4);
            return TRUE;
        case GDK_KEY_u:
        case GDK_KEY_U:
            /* Ctrl+u, delete the whole line. */
            *input_buffer = append_char_to_input_buffer(*input_buffer, 21);
            return TRUE;
        case GDK_KEY_k:
        case GDK_KEY_K:
            /* Ctrl+k, delete from current to end of line. */
            *input_buffer = append_char_to_input_buffer(*input_buffer, 11);
            return TRUE;
        case GDK_KEY_a:
        case GDK_KEY_A:
            /* Ctrl+a, go to the start of the line */
            *input_buffer = append_char_to_input_buffer(*input_buffer, 1);
            return TRUE;
        case GDK_KEY_e:
        case GDK_KEY_E:
            /* ctrl+e, go to the end of the line */
            *input_buffer = append_char_to_input_buffer(*input_buffer, 5);
            return TRUE;
        case GDK_KEY_c:
        case GDK_KEY_C:
            vte_terminal_copy_clipboard(VTE_TERMINAL(terminal));
            /* _format only exists in bleeding edge VTE 0.50 */
            /* vte_terminal_copy_clipboard_format(VTE_TERMINAL(terminal), VTE_FORMAT_TEXT); */
            return TRUE;
        case GDK_KEY_v:
        case GDK_KEY_V:
            *input_buffer = append_string_to_input_buffer(*input_buffer, terminal, GDK_SELECTION_CLIPBOARD);
            return TRUE;
    }
}

static gboolean key_press_event (GtkWidget   *widget,
                                 GdkEventKey *event,
                                 gpointer     user_data)
{
    char **input_buffer = (char **)user_data;
    GdkModifierType state = 0;

    gdk_event_get_state((GdkEvent*)event, &state);

    if (*input_buffer && event->type == GDK_KEY_PRESS) {
        switch (state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) {
            case 0:
            case GDK_SHIFT_MASK:
                return plain_key_pressed(input_buffer, event->keyval);
            case GDK_CONTROL_MASK:
                return ctrl_plus_key_pressed(input_buffer, event->keyval, widget);
            default:
                return FALSE;
        }
    }
    return FALSE;
}


static gboolean button_press_event(GtkWidget *widget,
                            GdkEvent  *event,
                            gpointer   user_data)
{
    char **input_buffer = (char **)user_data;
    GdkEventButton *button_event = (GdkEventButton*)event;

    if (button_event->button != 2
     || button_event->type   != GDK_BUTTON_PRESS) {
        return FALSE;
    }

    *input_buffer = append_string_to_input_buffer(*input_buffer, widget, GDK_SELECTION_PRIMARY);
    return TRUE;
}

static gboolean close_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    char **input_buffer = (char **)user_data;
    lib_free(*input_buffer);
    *input_buffer = NULL;
    return gtk_widget_hide_on_delete(widget);
}


int uimon_get_string(struct console_private_s *t, char* string, int string_len)
{
    int retval=0;
    while(retval<string_len) {
        int i;

        gtk_main_iteration();
        if (!t->input_buffer) {
            return -1;
        }
        for (i = 0; i < strlen(t->input_buffer) && retval < string_len; i++, retval++) {
            string[retval]=t->input_buffer[i];
        }
        memmove(t->input_buffer, t->input_buffer + i, strlen(t->input_buffer) + 1 - i);
    }
    return retval;
}

static void get_terminal_size_in_chars(VteTerminal *terminal,
                           glong *width,
                           glong *height)
{
    *width = vte_terminal_get_column_count(terminal);
    *height = vte_terminal_get_row_count(terminal);
}

static void screen_resize_window_cb (VteTerminal *terminal,
                         gpointer* window)
{
    glong width, height;
    get_terminal_size_in_chars(terminal, &width, &height);
    vte_console.console_xres = (unsigned int)width;
    vte_console.console_yres = (unsigned int)height;
}

/* resize the terminal when the window is resized */
static void screen_resize_window_cb2 (VteTerminal *terminal,
                         gpointer* window)
{
    int width, height;
    int cwidth, cheight;
    int newwidth, newheight;

    gtk_window_get_size (GTK_WINDOW(fixed.window), &width, &height);
    cwidth = vte_terminal_get_char_width (VTE_TERMINAL(fixed.term));
    cheight = vte_terminal_get_char_height (VTE_TERMINAL(fixed.term));

    newwidth = width / cwidth;
    newheight = height / cheight;
    if (newwidth < 1) {
        newwidth = 1;
    }
    if (newheight < 1) {
        newheight = 1;
    }

    vte_terminal_set_size(VTE_TERMINAL(fixed.term), newwidth, newheight);
}

/** \brief  Create an icon by loading it from the vice.gresource file
 *
 * \return  Standard C= icon ripped from the internet (but at least scalable)
 *          Which ofcourse sucks on Windows for some reason, *sigh*
 */
static GdkPixbuf *get_default_icon(void)
{
    return uidata_get_pixbuf("CBM_Logo.svg");
}

console_t *uimonfb_window_open(void);

console_t *uimon_window_open(void)
{
    GtkWidget *scrollbar, *horizontal_container;
    GdkGeometry hints;
    GdkPixbuf *icon;

    if (native_monitor()) {
        return uimonfb_window_open();
    }

    if (fixed.window == NULL) {
        fixed.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(fixed.window), "VICE monitor");
        gtk_window_set_position(GTK_WINDOW(fixed.window), GTK_WIN_POS_CENTER);
        gtk_widget_set_app_paintable(fixed.window, TRUE);
        gtk_window_set_deletable(GTK_WINDOW(fixed.window), TRUE);

        /* set a default C= icon for now */
        icon = get_default_icon();
        if (icon != NULL) {
            gtk_window_set_icon(GTK_WINDOW(fixed.window), icon);
        }

        fixed.term = vte_terminal_new();
        vte_terminal_set_scrollback_lines (VTE_TERMINAL(fixed.term), 1000);
        vte_terminal_set_scroll_on_output (VTE_TERMINAL(fixed.term), TRUE);

        /* allowed window widths are base_width + width_inc * N
         * allowed window heights are base_height + height_inc * N
         */
        hints.width_inc = vte_terminal_get_char_width (VTE_TERMINAL(fixed.term));
        hints.height_inc = vte_terminal_get_char_height (VTE_TERMINAL(fixed.term));
        /* min size should be multiple of .._inc, else we get funky effects */
        hints.min_width = hints.width_inc;
        hints.min_height = hints.height_inc;
        /* base size should be multiple of .._inc, else we get funky effects */
        hints.base_width = hints.width_inc;
        hints.base_height = hints.height_inc;
        gtk_window_set_geometry_hints (GTK_WINDOW (fixed.window),
                                     fixed.term,
                                     &hints,
                                     GDK_HINT_RESIZE_INC |
                                     GDK_HINT_MIN_SIZE |
                                     GDK_HINT_BASE_SIZE);
        scrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,
        gtk_scrollable_get_vadjustment (GTK_SCROLLABLE(fixed.term)));

        horizontal_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_container_add(GTK_CONTAINER(fixed.window), horizontal_container);
        gtk_container_add(GTK_CONTAINER(horizontal_container), fixed.term);
        gtk_container_add(GTK_CONTAINER(horizontal_container), scrollbar);

        g_signal_connect(G_OBJECT(fixed.window), "delete-event",
            G_CALLBACK(close_window), &fixed.input_buffer);

        g_signal_connect(G_OBJECT(fixed.term), "key-press-event",
            G_CALLBACK(key_press_event), &fixed.input_buffer);

        g_signal_connect(G_OBJECT(fixed.term), "button-press-event",
            G_CALLBACK(button_press_event), &fixed.input_buffer);

        g_signal_connect(G_OBJECT(fixed.term), "text-modified",
            G_CALLBACK (screen_resize_window_cb), NULL);

        g_signal_connect(G_OBJECT(fixed.window), "configure-event",
            G_CALLBACK (screen_resize_window_cb2), NULL);

        vte_console.console_can_stay_open = 1;
    }
    return uimon_window_resume();
}

console_t *uimon_window_resume(void)
{
    if (native_monitor()) {
        return uimonfb_window_resume();
    }

    gtk_widget_show_all(fixed.window);
    screen_resize_window_cb (VTE_TERMINAL(fixed.term), NULL);
    gtk_window_present(GTK_WINDOW(fixed.window));
    ui_dispatch_events();
    return &vte_console;
}

void uimon_window_suspend(void)
{
    if (native_monitor()) {
        uimonfb_window_suspend();
        return;
    }
}

int uimon_out(const char *buffer)
{
    const char *c;
    if (native_monitor()) {
        return uimonfb_out(buffer);
    }
    for(c = buffer; *c; c++) {
        if(*c == '\n') {
            uimon_write_to_terminal(&fixed, "\r", 1);
        }
        uimon_write_to_terminal(&fixed, c, 1);
    }
    return 0;
}

void uimon_window_close(void)
{
    if (native_monitor()) {
        uimonfb_window_close();
        return;
    }

    /* only close window if there is one: this avoids a GTK_CRITICAL warning
     * when using a remote monitor */
    if (fixed.window != NULL) {
        gtk_widget_hide(fixed.window);
    }
}

void uimon_notify_change(void)
{
    if (native_monitor()) {
        uimonfb_notify_change();
        return;
    }
}

void uimon_set_interface(struct monitor_interface_s **interf, int i)
{
    if (native_monitor()) {
        uimonfb_set_interface(interf, i);
        return;
    }
}

static char* concat_strings(const char *string1, int nchars, const char *string2)
{
    char *ret = lib_malloc(nchars + strlen(string2) + 1);
    memcpy(ret, string1, nchars);
    strcpy(ret + nchars, string2);
    return ret;
}

static void fill_completions(const char *string_so_far, int initial_chars, int token_len, const linenoiseCompletions *possible_lc, linenoiseCompletions *lc)
{
    int word_index;

    lc->len = 0;
    for(word_index = 0; word_index < possible_lc->len; word_index++) {
        int i;
        for(i = 0; i < token_len; i++) {
            if (string_so_far[initial_chars + i] != possible_lc->cvec[word_index][i]) {
                break;
            }
        }
        if (i == token_len && possible_lc->cvec[word_index][token_len] != 0) {
            char *string_to_append = concat_strings(string_so_far, initial_chars, possible_lc->cvec[word_index]);
            linenoiseAddCompletion(lc, string_to_append);
            lib_free(string_to_append);
        }
    }
}

static void find_next_token(const char *string_so_far, int start_of_search, int *start_of_token, int *token_len)
{
    for(*start_of_token = start_of_search; string_so_far[*start_of_token] && isspace((int)(string_so_far[*start_of_token])); (*start_of_token)++);
    for(*token_len = 0; string_so_far[*start_of_token + *token_len] && !isspace((int)(string_so_far[*start_of_token + *token_len])); (*token_len)++);
}

static gboolean is_token_in(const char *string_so_far, int token_len, const linenoiseCompletions *lc)
{
    int i;
    for(i = 0; i < lc->len; i++) {
        if(strlen(lc->cvec[i]) == token_len && !strncmp(string_so_far, lc->cvec[i], token_len)) {
            return TRUE;
        }
    }
    return FALSE;
}

static void monitor_completions(const char *string_so_far, linenoiseCompletions *lc)
{
    int start_of_token, token_len;
    char *help_commands[] = {"help", "?"};
    const linenoiseCompletions help_lc = {
         sizeof(help_commands)/sizeof(*help_commands),
         help_commands
    };

    find_next_token(string_so_far, 0, &start_of_token, &token_len);
    if (!string_so_far[start_of_token + token_len]) {
         fill_completions(string_so_far, start_of_token, token_len, &command_lc, lc);
         return;
    }
    if (is_token_in(string_so_far + start_of_token, token_len, &help_lc)) {
        find_next_token(string_so_far, start_of_token + token_len, &start_of_token, &token_len);
        if (!string_so_far[start_of_token + token_len]){
             fill_completions(string_so_far, start_of_token, token_len, &command_lc, lc);
             return;
        }
    }
    if (is_token_in(string_so_far + start_of_token, token_len, &need_filename_lc)) {
        int start_of_path;
        DIR* dir;
        struct dirent *direntry;
        struct linenoiseCompletions files_lc = {0, NULL};
        int i;

        for (start_of_token += token_len; string_so_far[start_of_token] && isspace((int)(string_so_far[start_of_token])); start_of_token++);
        if (string_so_far[start_of_token] != '"') {
            char *string_to_append = concat_strings(string_so_far, start_of_token, "\"");
            linenoiseAddCompletion(lc, string_to_append);
            lib_free(string_to_append);
            return;
        }
        for (start_of_path = ++start_of_token, token_len = 0; string_so_far[start_of_token + token_len]; token_len++) {
            if(string_so_far[start_of_token + token_len] == '"'
            && string_so_far[start_of_token + token_len - 1] != '\\') {
                return;
            }
            if(string_so_far[start_of_token + token_len] == '/') {
                start_of_token += token_len + 1;
                token_len = -1;
            }
        }
        if (start_of_token == start_of_path) {
            dir = opendir(".");
        } else {
            char *path = concat_strings(string_so_far + start_of_path, start_of_token - start_of_path, "");
            dir = opendir(path);
            lib_free(path);
        }
        if (dir) {
            for (direntry = readdir(dir); direntry; direntry = readdir(dir)) {
                if (strcmp(direntry->d_name, ".") && strcmp(direntry->d_name, "..")) {
                    char *entryname = lib_msprintf("%s%s", direntry->d_name, is_dir(direntry) ? "/" : "\"");
                    linenoiseAddCompletion(&files_lc, entryname);
                    lib_free(entryname);
                }
            }
            fill_completions(string_so_far, start_of_token, token_len, &files_lc, lc);
            for(i = 0; i < files_lc.len; i++) {
                free(files_lc.cvec[i]);
            }
            closedir(dir);
            return;
        }
    }
}

char *uimon_get_in(char **ppchCommandLine, const char *prompt)
{
    char *p, *ret_string;

    if (native_monitor()) {
        return uimonfb_get_in(ppchCommandLine, prompt);
    }

    fixed.input_buffer = lib_stralloc("");;
    linenoiseSetCompletionCallback(monitor_completions);
    p = linenoise(prompt, &fixed);
    if (p) {
        if (*p) {
            linenoiseHistoryAdd(p);
        }
        ret_string = lib_stralloc(p);
        free(p);
    } else {
        ret_string = lib_stralloc("x");
    }
    lib_free(fixed.input_buffer);
    fixed.input_buffer = NULL;

    return ret_string;
}

int console_init(void)
{
    int i = 0;
    char *full_name;
    char *short_name;
    int takes_filename_as_arg;
    
    if (native_monitor()) {
        return consolefb_init();
    }
    
    while (mon_get_nth_command(i++, (const char **)&full_name, (const char **)&short_name, &takes_filename_as_arg)) {
        if (strlen(full_name)) {
            linenoiseAddCompletion(&command_lc, full_name);
            if (strlen(short_name)) {
                linenoiseAddCompletion(&command_lc, short_name);
            }
            if (takes_filename_as_arg) {
                linenoiseAddCompletion(&need_filename_lc, full_name);
                if (strlen(short_name)) {
                    linenoiseAddCompletion(&need_filename_lc, short_name);
                }
            }
        }
    }
    return 0;
}

int console_close_all(void)
{
    int i;

    if (native_monitor()) {
        return consolefb_close_all();
    }

    for(i = 0; i < command_lc.len; i++) {
        free(command_lc.cvec[i]);
    }
    for(i = 0; i < need_filename_lc.len; i++) {
        free(need_filename_lc.cvec[i]);
    }
    return 0;
}

/** \brief  Callback to activate the ML-monitor
 *
 * \param[in,out]   widget      widget triggering the event
 * \param[in]       user_data   data for the event (unused)
 */
void ui_monitor_activate_callback(GtkWidget *widget, gpointer user_data)
{
    int v;
    int native = 0;

    /*
     * Determine if we use the spawing terminal or the (yet to write) Gtk3
     * base monitor
     */
    if (resources_get_int("NativeMonitor", &native) < 0) {
        debug_gtk3("failed to get value of resource 'NativeMonitor'.");
    }
    debug_gtk3("called, native monitor = %s.", native ? "true" : "false");

    resources_get_int("MonitorServer", &v);

    if (v == 0) {
#ifdef HAVE_FULLSCREEN
        fullscreen_suspend(0);
#endif
        vsync_suspend_speed_eval();
        /* ui_autorepeat_on(); */

#ifdef HAVE_MOUSE
        /* FIXME: restore mouse in case it was grabbed */
        /* ui_restore_mouse(); */
#endif
        if (!ui_emulation_is_paused()) {
            monitor_startup_trap();
        } else {
            monitor_startup(e_default_space);
#ifdef HAVE_FULLSCREEN
            fullscreen_resume();
#endif
        }
    }
}

