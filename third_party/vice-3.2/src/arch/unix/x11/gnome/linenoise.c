/* linenoise.c -- guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
 *
 * Modified for the VICE project by Fabrizio Gennari,
 * for use in combination with a terminal
 * not represented by a couple of file descrptors
 * (abstracted buy the opaque struct console_private_s)
 *
 * You can find the latest source code at:
 *
 *   http://github.com/antirez/linenoise
 *
 * Does a number of crazy assumptions that happen to be true in 99.9999% of
 * the 2010 UNIX computers around.
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ------------------------------------------------------------------------
 *
 * References:
 * - http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
 * - http://www.3waylabs.com/nw/WWW/products/wizcon/vt220.html
 *
 * Todo list:
 * - Switch to gets() if $TERM is something we can't support.
 * - Filter bogus Ctrl+<char> combinations.
 * - Win32 support
 *
 * Bloat:
 * - Completion?
 * - History search like Ctrl+r in readline?
 *
 * List of escape sequences used by this program, we do everything just
 * with three sequences. In order to be so cheap we may have some
 * flickering effect with some slow terminal, but the lesser sequences
 * the more compatible.
 *
 * CHA (Cursor Horizontal Absolute)
 *    Sequence: ESC [ n G
 *    Effect: moves cursor to column n
 *
 * EL (Erase Line)
 *    Sequence: ESC [ n K
 *    Effect: if n is 0 or missing, clear from cursor to end of line
 *    Effect: if n is 1, clear from beginning of line to cursor
 *    Effect: if n is 2, clear entire line
 *
 * CUF (CUrsor Forward)
 *    Sequence: ESC [ n C
 *    Effect: moves cursor forward of n chars
 *
 * The following are used to clear the screen: ESC [ H ESC [ 2 J
 * This is actually composed of two sequences:
 *
 * cursorhome
 *    Sequence: ESC [ H
 *    Effect: moves the cursor to upper left corner
 *
 * ED2 (Clear entire screen)
 *    Sequence: ESC [ 2 J
 *    Effect: clear the whole screen
 *
 */

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
/* #include <vte/vte.h> */ /* shouldnt be needed here, needs ifdef HAVE_VTE if so */
#include <gtk/gtk.h> /* for gtk_main_iteration() */
#include "linenoise.h"
#include "uimon.h"

#define LINENOISE_DEFAULT_HISTORY_MAX_LEN 100
#define LINENOISE_MAX_LINE 4096
static linenoiseCompletionCallback *completionCallback = NULL;

/* static struct termios orig_termios; */ /* in order to restore at exit */
/* static int rawmode = 0; */ /* for atexit() function to check if restore is needed*/
/* static int atexit_registered = 0; */ /* register atexit just 1 time */
static int history_max_len = LINENOISE_DEFAULT_HISTORY_MAX_LEN;
static int history_len = 0;
char **history = NULL;

/* static void linenoiseAtExit(void); */
int linenoiseHistoryAdd(const char *line);

/* FIXME: unused -> memory leak
static void freeHistory(void) {
    if (history) {
        int j;

        for (j = 0; j < history_len; j++) {
            free(history[j]);
        }
        free(history);
    }
}
*/

/* At exit we'll try to fix the terminal to the initial conditions. */
/*
static void linenoiseAtExit(void) {
    freeHistory();
}
*/


static void refreshLine(struct console_private_s *term, const char *prompt, char *buf, size_t len, size_t pos, size_t cols) {
    char seq[64];
    size_t plen = strlen(prompt);

    while((plen+pos) >= cols) {
        buf++;
        len--;
        pos--;
    }
    while (plen+len > cols) {
        len--;
    }
    /* Cursor to left edge */
    snprintf(seq, 64, "\x1b[0G");
    uimon_write_to_terminal(term, seq, strlen(seq));
    /* Write the prompt and the current buffer content */
    uimon_write_to_terminal(term, prompt, strlen(prompt));
    uimon_write_to_terminal(term, buf, len);
    /* Erase to right */
    snprintf(seq, 64, "\x1b[0K");
    uimon_write_to_terminal(term, seq, strlen(seq));
    /* Move cursor to original position. */
    snprintf(seq, 64, "\x1b[0G\x1b[%dC", (int)(pos+plen));
    uimon_write_to_terminal(term, seq, strlen(seq));
}

static void beep(struct console_private_s *term) {
    const char beepsequence[] = "\x7";
    uimon_write_to_terminal(term, beepsequence, strlen(beepsequence));
}

static void freeCompletions(linenoiseCompletions *lc) {
    size_t i;
    for (i = 0; i < lc->len; i++) {
        free(lc->cvec[i]);
    }
    if (lc->cvec != NULL) {
        free(lc->cvec);
    }
}

static int completeLine(struct console_private_s *term, const char *prompt, char *buf, size_t buflen, size_t *len, size_t *pos, size_t cols) {
    linenoiseCompletions lc = { 0, NULL };
    int nread, nwritten;
    char c = 0;

    completionCallback(buf,&lc);
    if (lc.len == 0) {
        beep(term);
    } else {
        size_t stop = 0, i = 0;
        size_t clen;

        while(!stop) {
            /* Show completion or original buffer */
            if (i < lc.len) {
                clen = strlen(lc.cvec[i]);
                refreshLine(term,prompt,lc.cvec[i],clen,clen,cols);
            } else {
                refreshLine(term,prompt,buf,*len,*pos,cols);
            }

            nread = uimon_get_string(term,&c,1);
            if (nread <= 0) {
                freeCompletions(&lc);
                return -1;
            }

            switch(c) {
                case 9: /* tab */
                    i = (i+1) % (lc.len+1);
                    if (i == lc.len) {
                        beep(term);
                    }
                    break;
                case 27: /* escape */
                    /* Re-show original buffer */
                    if (i < lc.len) {
                        refreshLine(term,prompt,buf,*len,*pos,cols);
                    }
                    stop = 1;
                    break;
                default:
                    /* Update buffer and return */
                    if (i < lc.len) {
                        nwritten = snprintf(buf,buflen,"%s",lc.cvec[i]);
                        *len = *pos = nwritten;
                    }
                    stop = 1;
                    break;
            }
        }
    }

    freeCompletions(&lc);
    return c; /* Return last read character */
}

void linenoiseClearScreen(struct console_private_s *term) {
    const char clearseq[] = "\x1b[H\x1b[2J";
    uimon_write_to_terminal(term, clearseq, strlen(clearseq));
}

static int linenoisePrompt(struct console_private_s *term, char *buf, size_t buflen, const char *prompt) {
    size_t plen = strlen(prompt);
    size_t pos = 0;
    size_t len = 0;
    size_t cols = uimon_get_columns(term);
    int history_index = 0;
    int i;

    buf[0] = '\0';
    buflen--; /* Make sure there is always space for the nulterm */

    /* The latest history entry is always our current buffer, that
     * initially is just an empty string. */
    linenoiseHistoryAdd("");

    /* HACK HACK HACK

       what we really want to do here is writing the prompt, and then tell VTE
       to flush its buffers and redraw its terminal. (necessary to make the
       initial prompt show up reliably, else it may be delayed until a key is
       pressed, which is confusing and annoying) unfortunately there seems to be 
       no distinct way to do this, however.

       the following loop seems to do the trick (using about 10 iterations, so
       i am using 20 to be on the safe side). yes its ugly :( 
    */
    for(i = 0; i < 20; i++) {
        uimon_write_to_terminal(term, "\r", 1);
        uimon_write_to_terminal(term, prompt, plen);
        gtk_main_iteration();
    }

    while(1) {
        int c;
        int nread;
        char seq[2], seq2[2];
        char tmp[1];

        nread = uimon_get_string(term, tmp, 1);
        if (nread <= 0) {
            return -1;
        }
        c = tmp[0];

        /* Only autocomplete when the callback is set. It returns < 0 when
         * there was an error reading from fd. Otherwise it will return the
         * character that should be handled next. */
        if (c == 9 && completionCallback != NULL) {
            c = completeLine(term, prompt, buf, buflen, &len, &pos, cols);
            /* Return on errors */
            if (c < 0) {
                return len;
            }
            /* Read next character when 0 */
            if (c == 0) {
                continue;
            }
        }

        switch(c) {
        case 13:    /* enter */
            history_len--;
            free(history[history_len]);
            return (int)len;
        case 3:     /* ctrl-c */
            errno = EAGAIN;
            return -1;
        case 127:   /* backspace */
        case 8:     /* ctrl-h */
            if (pos > 0 && len > 0) {
                memmove(buf+pos-1, buf+pos, len-pos);
                pos--;
                len--;
                buf[len] = '\0';
                refreshLine(term, prompt, buf, len, pos, cols);
            }
            break;
        case 4:     /* ctrl-d, remove char at right of cursor */
            if (len > 1 && pos < (len-1)) {
                memmove(buf+pos, buf+pos+1, len-pos);
                len--;
                buf[len] = '\0';
                refreshLine(term, prompt, buf, len, pos, cols);
            } else if (len == 0) {
                history_len--;
                free(history[history_len]);
                return -1;
            }
            break;
        case 20:    /* ctrl-t */
            if (pos > 0 && pos < len) {
                int aux = buf[pos-1];
                buf[pos-1] = buf[pos];
                buf[pos] = aux;
                if (pos != len-1) pos++;
                refreshLine(term, prompt, buf, len, pos, cols);
            }
            break;
        case 2:     /* ctrl-b */
            goto left_arrow;
        case 6:     /* ctrl-f */
            goto right_arrow;
        case 16:    /* ctrl-p */
            seq[1] = 65;
            goto up_down_arrow;
        case 14:    /* ctrl-n */
            seq[1] = 66;
            goto up_down_arrow;
            break;
        case 27:    /* escape sequence */
            if (uimon_get_string(term,seq,2) == -1) {
                break;
            }
            if (seq[0] == 91 && seq[1] == 68) {
left_arrow:
                /* left arrow */
                if (pos > 0) {
                    pos--;
                    refreshLine(term, prompt, buf, len, pos, cols);
                }
            } else if (seq[0] == 91 && seq[1] == 67) {
right_arrow:
                /* right arrow */
                if (pos != len) {
                    pos++;
                    refreshLine(term, prompt, buf, len, pos, cols);
                }
            } else if (seq[0] == 91 && (seq[1] == 65 || seq[1] == 66)) {
up_down_arrow:
                /* up and down arrow: history */
                if (history_len > 1) {
                    /* Update the current history entry before to
                     * overwrite it with tne next one. */
                    free(history[history_len-1-history_index]);
                    history[history_len-1-history_index] = strdup(buf);
                    /* Show the new entry */
                    history_index += (seq[1] == 65) ? 1 : -1;
                    if (history_index < 0) {
                        history_index = 0;
                        break;
                    } else if (history_index >= history_len) {
                        history_index = history_len-1;
                        break;
                    }
                    strncpy(buf, history[history_len-1-history_index], buflen);
                    buf[buflen] = '\0';
                    len = pos = strlen(buf);
                    refreshLine(term, prompt, buf, len, pos, cols);
                }
            } else if (seq[0] == 91 && seq[1] > 48 && seq[1] < 55) {
                /* extended escape */
                if (uimon_get_string(term,seq2,2) == -1) {
                    break;
                }
                if (seq[1] == 51 && seq2[0] == 126) {
                    /* delete */
                    if (len > 0 && pos < len) {
                        memmove(buf+pos, buf+pos+1, len-pos-1);
                        len--;
                        buf[len] = '\0';
                        refreshLine(term, prompt, buf, len, pos,cols);
                    }
                }
            }
            break;
        default:
            if (len < buflen) {
                if (len == pos) {
                    buf[pos] = c;
                    pos++;
                    len++;
                    buf[len] = '\0';
                    refreshLine(term, prompt, buf, len, pos, cols);
                } else {
                    memmove(buf+pos+1, buf+pos, len-pos);
                    buf[pos] = c;
                    len++;
                    pos++;
                    buf[len] = '\0';
                    refreshLine(term, prompt, buf, len, pos, cols);
                }
            }
            break;
        case 21: /* Ctrl+u, delete the whole line. */
            buf[0] = '\0';
            pos = len = 0;
            refreshLine(term, prompt, buf, len, pos, cols);
            break;
        case 11: /* Ctrl+k, delete from current to end of line. */
            buf[pos] = '\0';
            len = pos;
            refreshLine(term, prompt, buf, len, pos, cols);
            break;
        case 1: /* Ctrl+a, go to the start of the line */
            pos = 0;
            refreshLine(term, prompt, buf, len, pos, cols);
            break;
        case 5: /* ctrl+e, go to the end of the line */
            pos = len;
            refreshLine(term, prompt, buf, len, pos, cols);
            break;
        case 12: /* ctrl+l, clear screen */
            linenoiseClearScreen(term);
            refreshLine(term, prompt, buf, len, pos, cols);
        }
    }
}

char *linenoise(const char *prompt, struct console_private_s *term) {
    char buf[LINENOISE_MAX_LINE];
    int count;

    count = linenoisePrompt(term, buf, LINENOISE_MAX_LINE, prompt);
    uimon_write_to_terminal(term, "\r\n", 2);
    if (count == -1) {
        return NULL;
    }
    return strdup(buf);
}

/* Register a callback function to be called for tab-completion. */
void linenoiseSetCompletionCallback(linenoiseCompletionCallback *fn) {
    completionCallback = fn;
}

void linenoiseAddCompletion(linenoiseCompletions *lc, char *str) {
    size_t len = strlen(str);
    char *copy = malloc(len+1);
    memcpy(copy,str,len+1);
    lc->cvec = realloc(lc->cvec, sizeof(char*)*(lc->len+1));
    lc->cvec[lc->len++] = copy;
}

/* Using a circular buffer is smarter, but a bit more complex to handle. */
int linenoiseHistoryAdd(const char *line) {
    char *linecopy;

    if (history_max_len == 0) {
        return 0;
    }
    if (history == NULL) {
        history = malloc(sizeof(char*)*history_max_len);
        if (history == NULL) {
            return 0;
        }
        memset(history, 0, (sizeof(char*)*history_max_len));
    }
    linecopy = strdup(line);
    if (!linecopy) {
        return 0;
    }
    if (history_len == history_max_len) {
        free(history[0]);
        memmove(history, history+1, sizeof(char*)*(history_max_len-1));
        history_len--;
    }
    history[history_len] = linecopy;
    history_len++;
    return 1;
}

int linenoiseHistorySetMaxLen(int len) {
    char **new;

    if (len < 1) {
        return 0;
    }
    if (history) {
        int tocopy = history_len;

        new = malloc(sizeof(char*)*len);
        if (new == NULL) {
            return 0;
        }
        if (len < tocopy) tocopy = len;
        memcpy(new, history+(history_max_len-tocopy), sizeof(char*)*tocopy);
        free(history);
        history = new;
    }
    history_max_len = len;
    if (history_len > history_max_len) {
        history_len = history_max_len;
    }
    return 1;
}
