/*
 * uimon.c - Implementation of the terminal window for the GTK+ UI,
 * based on the VTE terminal widget.
 *
 * Written by
 *  Fabrizio Gennari <fabrizio.ge@tiscali.it>
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
#include "uiarch.h"

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_VTE
#include <vte/vte.h>
#endif
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
#include "lib.h"
#ifdef HAVE_VTE
#include "linenoise.h"
#endif
#include "uimon.h"
#include "mon_command.h"

#ifdef HAVE_VTE

struct console_private_s {
    GtkWidget *window;
    GtkWidget *term;
    char *input_buffer;
} fixed = {NULL, NULL, NULL};

static console_t vte_console;
static linenoiseCompletions command_lc = {0, NULL};
static linenoiseCompletions need_filename_lc = {0, NULL};

static int is_dir(struct dirent *de)
{
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
    return 0;
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
            if(keyval >= GDK_KEY(space) && keyval <= GDK_KEY(ydiaeresis)) {
                *input_buffer = append_char_to_input_buffer(*input_buffer, (char)keyval);
                return TRUE;
            }
            return FALSE;
        case GDK_KEY(Return):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 13);
            return TRUE;
        case GDK_KEY(BackSpace):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 127);
            return TRUE;
        case GDK_KEY(Left):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 2);
            return TRUE;
        case GDK_KEY(Right):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 6);
            return TRUE;
        case GDK_KEY(Up):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 16);
            return TRUE;
        case GDK_KEY(Down):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 14);
            return TRUE;
        case GDK_KEY(Tab):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 9);
            return TRUE;
        case GDK_KEY(Delete):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 4);
            return TRUE;
        case GDK_KEY(Home):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 1);
            return TRUE;
        case GDK_KEY(End):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 5);
            return TRUE;
        case GDK_KEY(dead_tilde):
            *input_buffer =
                append_char_to_input_buffer(*input_buffer, GDK_KEY(asciitilde));
            return TRUE;
    }
}

static gboolean ctrl_plus_key_pressed(char **input_buffer, guint keyval, GtkWidget *terminal)
{
    switch (keyval) {
        default:
            return FALSE;
        case GDK_KEY(h):
        case GDK_KEY(H):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 127);
            return TRUE;
        case GDK_KEY(b):
        case GDK_KEY(B):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 2);
            return TRUE;
        case GDK_KEY(f):
        case GDK_KEY(F):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 6);
            return TRUE;
        case GDK_KEY(p):
        case GDK_KEY(P):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 16);
            return TRUE;
        case GDK_KEY(n):
        case GDK_KEY(N):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 14);
            return TRUE;
        case GDK_KEY(t):
        case GDK_KEY(T):
            *input_buffer = append_char_to_input_buffer(*input_buffer, 20);
            return TRUE;
        case GDK_KEY(d):
        case GDK_KEY(D):
            /* ctrl-d, remove char at right of cursor */
            *input_buffer = append_char_to_input_buffer(*input_buffer, 4);
            return TRUE;
        case GDK_KEY(u):
        case GDK_KEY(U):
            /* Ctrl+u, delete the whole line. */
            *input_buffer = append_char_to_input_buffer(*input_buffer, 21);
            return TRUE;
        case GDK_KEY(k):
        case GDK_KEY(K):
            /* Ctrl+k, delete from current to end of line. */
            *input_buffer = append_char_to_input_buffer(*input_buffer, 11);
            return TRUE;
        case GDK_KEY(a):
        case GDK_KEY(A):
            /* Ctrl+a, go to the start of the line */
            *input_buffer = append_char_to_input_buffer(*input_buffer, 1);
            return TRUE;
        case GDK_KEY(e):
        case GDK_KEY(E):
            /* ctrl+e, go to the end of the line */
            *input_buffer = append_char_to_input_buffer(*input_buffer, 5);
            return TRUE;
        case GDK_KEY(c):
        case GDK_KEY(C):
            vte_terminal_copy_clipboard(VTE_TERMINAL(terminal));
            return TRUE;
        case GDK_KEY(v):
        case GDK_KEY(V):
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

console_t *uimon_window_open(void)
{
    GtkWidget *scrollbar, *horizontal_container;
    GdkGeometry hints;

    if (fixed.window == NULL) {
        fixed.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(fixed.window), "VICE monitor");
        gtk_window_set_position(GTK_WINDOW(fixed.window), GTK_WIN_POS_CENTER);
        gtk_widget_set_app_paintable(fixed.window, TRUE);
        gtk_window_set_deletable(GTK_WINDOW(fixed.window), TRUE);
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
#if GTK_CHECK_VERSION (2, 91, 1)
        {
            glong width, height;
            get_terminal_size_in_chars(VTE_TERMINAL(fixed.term), &width, &height);
            gtk_window_resize_to_geometry (GTK_WINDOW (fixed.window), width, height);
        }
#endif
#if GTK_CHECK_VERSION (3, 0, 0)
        scrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,
            gtk_scrollable_get_vadjustment (GTK_SCROLLABLE(fixed.term)));
#else
        scrollbar = gtk_vscrollbar_new(vte_terminal_get_adjustment (VTE_TERMINAL(fixed.term)));
#endif

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

        g_signal_connect (fixed.term, "text-modified",
            G_CALLBACK (screen_resize_window_cb), NULL);

        vte_console.console_can_stay_open = 1;
    }
    return uimon_window_resume();
}

console_t *uimon_window_resume(void)
{
    gtk_widget_show_all(fixed.window);
    gtk_window_present(GTK_WINDOW(fixed.window));
    screen_resize_window_cb (VTE_TERMINAL(fixed.term), NULL);
    ui_dispatch_events();
    gdk_flush();
    return &vte_console;
}

void uimon_window_suspend(void)
{
    /* transfer focus to the main emulator window */
    ui_restore_focus();
}

int uimon_out(const char *buffer)
{
    const char *c;
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
    /* only close window if there is one: this avoids a GTK_CRITICAL warning
     * when using a remote monitor */
    if (fixed.window != NULL) {
        gtk_widget_hide(fixed.window);
        /* transfer focus to the main emulator window */
        ui_restore_focus();
    }
}

void uimon_notify_change(void)
{
}

void uimon_set_interface(struct monitor_interface_s **interf, int i)
{
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
    for(i = 0; i < command_lc.len; i++) {
        free(command_lc.cvec[i]);
    }
    for(i = 0; i < need_filename_lc.len; i++) {
        free(need_filename_lc.cvec[i]);
    }
    return 0;
}

#else

/*******************************************************************************
 Fallback implementation for when the VTE library is not available
 ******************************************************************************/

static console_t *console_log_local = NULL;

#if defined(HAVE_READLINE) && defined(HAVE_READLINE_READLINE_H)
#include <readline/readline.h>
#include <readline/history.h>
#else
static FILE *mon_input, *mon_output;
#endif

int console_init(void)
{
#if defined(HAVE_READLINE) && defined(HAVE_READLINE_READLINE_H) && defined(HAVE_RLNAME)
    rl_readline_name = "VICE";
#endif

    return 0;
}

#if !defined(HAVE_READLINE) || !defined(HAVE_READLINE_READLINE_H)
int console_out(console_t *log, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vfprintf(mon_output, format, ap);
    va_end(ap);

    return 0;
}
#endif

console_t *uimon_window_open(void)
{
#ifdef HAVE_SYS_IOCTL_H
    struct winsize w;
#endif

    if (!isatty(fileno(stdin))) {
        log_error(LOG_DEFAULT, "console_open: stdin is not a tty.");
        console_log_local = NULL;
        return NULL;
    }
    if (!isatty(fileno(stdout))) {
        log_error(LOG_DEFAULT, "console_open: stdout is not a tty.");
        console_log_local = NULL;
        return NULL;
    }
    console_log_local = lib_malloc(sizeof(console_t));
    /* change window title for console identification purposes */
    if (getenv("WINDOWID") == NULL) {
        printf("\033]2;VICE monitor console (%d)\007", (int)getpid());
    }

#if !defined(HAVE_READLINE) || !defined(HAVE_READLINE_READLINE_H)
    mon_input = stdin;
    mon_output = stdout;
#endif

#ifdef HAVE_SYS_IOCTL_H
    if (ioctl(fileno(stdin), TIOCGWINSZ, &w)) {
        console_log_local->console_xres = 80;
        console_log_local->console_yres = 25;
    } else {
        console_log_local->console_xres = w.ws_col >= 40 ? w.ws_col : 40;
        console_log_local->console_yres = w.ws_row >= 22 ? w.ws_row : 22;
    }
#else
    console_log_local->console_xres = 80;
    console_log_local->console_yres = 25;
#endif
    console_log_local->console_can_stay_open = 1;
    console_log_local->console_cannot_output = 0;
#ifdef HAVE_MOUSE
    ui_restore_mouse();
#endif
    ui_focus_monitor();
    return console_log_local;
}

void uimon_window_close(void)
{
    lib_free(console_log_local);
    console_log_local = NULL;

    uimon_window_suspend();
}

void uimon_window_suspend( void )
{
    ui_restore_focus();
#ifdef HAVE_MOUSE
    ui_check_mouse_cursor();
#endif
}

console_t *uimon_window_resume(void)
{
    if (console_log_local) {
#ifdef HAVE_MOUSE
        ui_restore_mouse();
#endif
        ui_focus_monitor();
        return console_log_local;
    }
    log_error(LOG_DEFAULT, "uimon_window_resume: log was not opened.");
    return uimon_window_open();
}

int uimon_out(const char *buffer)
{
    fprintf(stdout, "%s", buffer);
    return 0;
}

#if !defined(HAVE_READLINE) || !defined(HAVE_READLINE_READLINE_H)
static char *readline(const char *prompt)
{
    char *p = lib_malloc(1024);

    console_out(NULL, "%s", prompt);

    fflush(mon_output);
    if (fgets(p, 1024, mon_input) == NULL) {
        /* FIXME: handle error */
    }

    /* Remove trailing newlines.  */
    {
        int len;

        for (len = strlen(p); len > 0 && (p[len - 1] == '\r' || p[len - 1] == '\n'); len--) {
            p[len - 1] = '\0';
        }
    }

    return p;
}
#endif

char *uimon_get_in(char **ppchCommandLine, const char *prompt)
{
    char *p, *ret_sting;

    p = readline(prompt);
#if defined(HAVE_READLINE) && defined(HAVE_READLINE_READLINE_H)
    if (p && *p) {
        add_history(p);
    }
#endif
    ret_sting = lib_stralloc(p);
    free(p);

    return ret_sting;
}

int console_close_all(void)
{
    return 0;
}

void uimon_notify_change( void )
{
}

void uimon_set_interface(struct monitor_interface_s **monitor_interface_init, int count)
{
}

#endif
