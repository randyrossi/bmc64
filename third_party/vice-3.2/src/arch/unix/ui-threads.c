/*
 * ui-threads.c
 * Foundation for multithreaded GUIs to decouple machine (=emulation) from
 * display drawing routines and from GUI event-processing based on pthreads.
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

#if !defined(USE_UI_THREADS) || !defined(HAVE_HWSCALE)
#error "USE_UI_THREADS not evailable in config.h - check config.log"
#endif

/* #define DEBUG_MBUFFER */

#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

#include "cmdline.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "machine.h"
#include "translate.h"
#include "videoarch.h"
#include "vsync.h"
#include "ui-threads.h"

#ifdef DEBUG_MBUFFER
#define DBG(_x_) log_debug _x_
#define DBG2(_x_)
#else
#define DBG(_x_)
#define DBG2(_x_)
#endif

#define TS_TOUSEC(x) (x.tv_sec * 1000000L + (x.tv_nsec / 1000))
#define TS_TOMSEC(x) (x.tv_sec * 1000L + (x.tv_nsec / 1000000L))

/* the freq. should be configurable */
#define REFRESH_FREQ (8 * 1000 * 1000)

static struct timespec reltime = { 0, REFRESH_FREQ };
static int dthread_rfp = 8;
static int dthread_ghosting = 2;
static pthread_t dthread;
static pthread_t ethread;
static pthread_cond_t cond  = PTHREAD_COND_INITIALIZER;
static pthread_cond_t coroutine  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t dlock = PTHREAD_MUTEX_INITIALIZER;
static sem_t mthread_sem, ethread_sem;
static void *widget, *event, *client_data;
static int is_coroutine = 1;    /* start with single threaded execution */
static double machine_freq;
static long laststamp, mrp_usec;

typedef struct {
    int cpos;
    int lpos;
    int csize;
    void *canvas;
    void *widget;
} buffer_ptrs_s;

static struct s_mbufs buffers[MAX_APP_SHELLS][MAX_BUFFERS];
static buffer_ptrs_s bptrs[MAX_APP_SHELLS];
static int update = 0;
static int emu_running = 1;
static int app_shell_count = 0;

#define NEXT(x,y) (((x)+(y)) % MAX_BUFFERS)
#define PREV(x,y) ((((x)-(y)) + MAX_BUFFERS) % MAX_BUFFERS)

static int width, height, no_autorepeat;
const char *title;
static int *argc;
static char **argv;
static int int_ret;
static video_canvas_t *canvas;

/* coroutine func IDs */
typedef enum {
    CR_NOTHING,
    CR_REDRAW,
    CR_CANVAS_WIDGET,
    CR_OPEN_CANVAS,
    CR_DISPATCH_EVENTS,
    CR_INIT,
    CR_INIT_FINISH,
    CR_CONFIGURE_CALLBACK,
    CR_RESIZE,
    CR_WINDOW_RESIZE,
    CR_SHUTDOWN
} coroutine_t;

static coroutine_t do_action = CR_NOTHING;

/* prototypes for internals */
static void dthread_coroutine(coroutine_t a);
static void *dthread_func(void *attr);
static void *ethread_func(void *attr);

/* resources */
static int do_blending = 1;

static int set_alpha_blending(int val, void *p)
{
    log_message(LOG_DEFAULT, _("Alpha blending %s"), val ? _("enabled") : _("disabled"));

    do_blending = val ? 1 : 0;

    return 0;
}

static int set_dthread_rfp(int val, void *p)
{
    DBG(("Setting dthread rfp %dms", dthread_rfp));
    dthread_rfp = val;
    reltime.tv_nsec = val * 1000L * 1000L;
    return 0;
}

static int set_dthread_ghosting(int val, void *p)
{
    val = (val < 2) ? 2 : (val > 6) ? 6 : val;
    DBG(("Setting dthread ghosting %d frames", dthread_ghosting));
    dthread_ghosting = val;
    return 0;
}

static resource_int_t resources_uithreads[] = {
    { "AlphaBlending", 1, RES_EVENT_NO, NULL,
      &do_blending, set_alpha_blending, NULL },
    { "DThreadRate", 8, RES_EVENT_NO, NULL,
      &dthread_rfp, set_dthread_rfp, NULL },
    { "DThreadGhosting", 2, RES_EVENT_NO, NULL,
      &dthread_ghosting, set_dthread_ghosting, NULL },
    RESOURCE_INT_LIST_END
};

static const cmdline_option_t cmdline_options[] =
{
    { "-alphablending", SET_RESOURCE, 0,
      NULL, NULL, "AlphaBlending", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Enable alpha blending") },
    { "+alphablending", SET_RESOURCE, 0,
      NULL, NULL, "AlphaBlending", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Disable alpha blending") },
    { "-displaythreadrate", SET_RESOURCE, 1,
      NULL, NULL, "DThreadRate", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      N_("<milliseconds>"), N_("Set the display thread rate") },
    { "-displaythreadghosting", SET_RESOURCE, 1,
      NULL, NULL, "DThreadGhosting", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      N_("<frames>"), N_("Set the display thread frame backbuffer amount") },
    CMDLINE_LIST_END
};

void mbuffer_init(void *canvas, int w, int h, int depth, int shell)
{
    int i;

    DBG(("widget %p, w=%d, h=%d, shell=%d", widget, w, h, shell));
    bptrs[shell].csize = w * h * depth;
    for (i = 0; i < MAX_BUFFERS; i++) {
        lib_free(buffers[shell][i].buffer);
        buffers[shell][i].buffer = lib_malloc(bptrs[shell].csize);
        memset(buffers[shell][i].buffer, -1, bptrs[shell].csize);
        buffers[shell][i].w = w;
        buffers[shell][i].h = h;
        if (i > 0) {
            buffers[shell][i-1].next = &buffers[shell][i];
        }
    }
    buffers[shell][i-1].next = &buffers[shell][0];
    bptrs[shell].cpos = 0;
    bptrs[shell].canvas = canvas;
    if (shell > app_shell_count) {
        app_shell_count = shell;
    }
}

unsigned char *mbuffer_get_buffer(struct timespec *t, int shell)
{
    unsigned char *curr;
    int tmppos = bptrs[shell].cpos;
    long tmpstamp, j;
    struct timespec ts = *t;

    if (machine_freq != vsync_get_refresh_frequency()) {
        machine_freq = vsync_get_refresh_frequency();
        mrp_usec = 1000.0 / machine_freq * 1000;
        DBG(("machine freq = %f ms period %ld us", (float) machine_freq, mrp_usec));
    }

    /* stamp in usecs */
    curr = buffers[shell][bptrs[shell].cpos].buffer;
    tmppos = NEXT(bptrs[shell].cpos, 1);

    /* copy fullframe */
    memcpy(buffers[shell][tmppos].buffer, curr, bptrs[shell].csize);
    bptrs[shell].cpos = tmppos;
    if (bptrs[shell].cpos == bptrs[shell].lpos) {
        DBG(("out of buffers: %s", __FUNCTION__));
    }

    laststamp += mrp_usec; /* advance by machine cycle */
    tmpstamp = TS_TOUSEC(ts);
    j = tmpstamp - laststamp;
    /* DBG(("emu jitter of: %5ld us", j)); */
    if ((j > 15000) || (j < -15000)) {
        DBG(("resetting jitter: %5ld us", j));
        laststamp = tmpstamp;
    }

    buffers[shell][bptrs[shell].cpos].stamp = laststamp;
    return buffers[shell][bptrs[shell].cpos].buffer;
}

static void tsAdd (const struct timespec *time1, const struct timespec *time2, struct timespec *result)
{
    /* Add the two times together. */
    result->tv_sec = time1->tv_sec + time2->tv_sec ;
    result->tv_nsec = time1->tv_nsec + time2->tv_nsec ;
    if (result->tv_nsec >= 1000000000L) {   /* Carry? */
        result->tv_sec++ ;
        result->tv_nsec = result->tv_nsec - 1000000000L ;
    }
}

/* display thread routines - should go elsewehere later on */
void dthread_build_screen_canvas(video_canvas_t *c)
{
    if (is_coroutine) {
        build_screen_canvas_widget2(c);
        return;
    }
    canvas = bptrs[c->app_shell].canvas = c;
    widget = bptrs[c->app_shell].widget = c->emuwindow;
    dthread_coroutine(CR_CANVAS_WIDGET);
}

int dthread_ui_open_canvas_window(video_canvas_t *c, const char *t, int wi, int he, int na)
{
    if (is_coroutine) {
        return ui_open_canvas_window2(c, t, wi, he, na);
    }

    canvas = bptrs[c->app_shell].canvas = c;
    widget = bptrs[c->app_shell].widget = c->emuwindow;
    title = t;
    width = wi;
    height = he;
    no_autorepeat = na;
    dthread_coroutine(CR_OPEN_CANVAS);
    return int_ret;
}

int dthread_ui_init(int *ac, char **av)
{
    if (is_coroutine) {
        return ui_init2(ac, av);
    }

    argc=ac;
    argv=av;
    dthread_coroutine(CR_INIT);
    return 0;
}

void dthread_ui_dispatch_events(void)
{
    if (update || is_coroutine) {
        DBG2(("recursive call to %s - update: %d, is_coroutine %d", __FUNCTION__, update, is_coroutine));
        ui_dispatch_events2();
        return;
    } else {
        DBG2(("call to %s - update: %d, is_coroutine %d", __FUNCTION__, update, is_coroutine));
        update = 1;
        if (sem_post(&ethread_sem) != 0) {
            log_debug("sem_post() failed, %s", __FUNCTION__);
            exit (-1);
        }
        if (sem_wait(&mthread_sem) != 0) {
            log_debug("sem_post() failed, %s", __FUNCTION__);
            exit (-1);
        }
        update = 0;
    }
}

int dthread_ui_init_finish()
{
    if (is_coroutine) {
        return ui_init_finish2();
    }
    dthread_coroutine(CR_INIT_FINISH);
    return int_ret;
}

int dthread_configure_callback_canvas(void *w, void *e, void *cd)
{
    if (is_coroutine) {
        DBG(("recursive call to %s, update: %d, is_coroutine: %d", __FUNCTION__, update, is_coroutine));
        return configure_callback_canvas2(w, e, cd);
    }

    widget = bptrs[((video_canvas_t *) cd)->app_shell].widget = w;
    event = e;
    client_data = cd;
    dthread_coroutine(CR_CONFIGURE_CALLBACK);
    return int_ret;
}

void dthread_ui_trigger_resize(void)
{
    if (is_coroutine) {
        return ui_trigger_resize2();
    }

    dthread_coroutine(CR_RESIZE);
}

void dthread_ui_trigger_window_resize(video_canvas_t *c)
{
    if (is_coroutine) {
        ui_trigger_window_resize2(c);
        return;
    }
    canvas = bptrs[canvas->app_shell].canvas = c;
    widget = bptrs[c->app_shell].widget = c->emuwindow;
    dthread_coroutine(CR_WINDOW_RESIZE);
}

void dthread_init(void)
{
    struct sched_param param;
    pthread_attr_t attr;

    if (resources_register_int(resources_uithreads) < 0) {
        exit (-1);
    }

    if (cmdline_register_options(cmdline_options) < 0) {
        exit (-1);
    }

    if (console_mode) {
        is_coroutine = 1;   /* enforce single threaded execution */
        return;
    }

    if (pthread_attr_init(&attr)) {
        log_debug("pthread_attr_init() failed, %s", __FUNCTION__);
        exit (-1);
    }

    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
        log_debug("pthread_setinheritsched() failed, %s", __FUNCTION__);
        exit (-1);
    }

    if (sem_init(&mthread_sem, 0, 0) < 0) {
        log_debug("sem_init() failed, %s", __FUNCTION__);
        exit (-1);
    }

    if (sem_init(&ethread_sem, 0, 0) < 0) {
        log_debug("sem_init() failed, %s", __FUNCTION__);
        exit (-1);
    }

    is_coroutine = 0;   /* use multithreaded executionfrom now on */
    if (pthread_create(&dthread, &attr, dthread_func, NULL) < 0) {
        log_debug("pthread_create() failed, %s", __FUNCTION__);
        exit (-1);
    }
    if (pthread_create(&ethread, &attr, ethread_func, NULL) < 0) {
        log_debug("pthread_create() failed, %s", __FUNCTION__);
        exit (-1);
    }

    param.sched_priority = 20;
    if (pthread_setschedparam(dthread, SCHED_RR, &param)) {
        log_message(LOG_DEFAULT, "ui-threads: failed to set realtime priority for VICE - this is no problem!\n\trefer to Readme-Unix.txt (http://vice-emu.svn.sourceforge.net/viewvc/vice-emu/trunk/vice/doc/readmes/Readme-Unix.txt)");
    }
    pthread_detach(dthread);
    pthread_detach(ethread);
}

void dthread_shutdown(void)
{
    dthread_coroutine(CR_SHUTDOWN);
}

void dthread_lock(void)
{
    if (pthread_mutex_lock(&dlock) < 0) {
        log_debug("pthread_mutex_lock() failed, %s", __FUNCTION__);
        exit (-1);
    }
}

void dthread_unlock(void)
{
    if (pthread_mutex_unlock(&dlock) < 0) {
        log_debug("pthread_mutex_unlock() failed, %s", __FUNCTION__);
        exit (-1);
    }
}

/* internal routines */

/* weights for frames used for ghosting */
static float weights[MAX_BUFFERS][MAX_BUFFERS] = {
    { -1, -1, -1, -1, -1, -1, -1, -1 }, /* not used*/
    { 1, -1, -1, -1, -1, -1, -1, -1 },  /* not used */
    { 1, 1, -1, -1, -1, -1, -1, -1 },
    { 1.0, 0.8, 0.2, -1, -1, -1, -1, -1 },
    { 0.2, 0.8, 0.8, 0.2, -1, -1, -1, -1 },
    { 0.2, 0.8, 0.65, 0.25, 0.1, -1, -1, -1 },
    { 0.1, 0.25, 0.65, 0.65, 0.25, 0.1, -1, -1 },
    { 0.1, 0.25, 0.65, 0.55, 0.25, 0.15, 0.05, -1 } /* not used */
};

static int dthread_calc_frames(unsigned long now, int *from, int *to, int shell)
{
    int last;               /* index of last frame to be drawn */
    int first;              /* index of first frame to be drawn */
    int count;              /* number of frames not drawn yet */
    unsigned long dt1, dt2;
    int i, dg2;             /* helpers */
    float a1, a2;
    struct s_mbufs *t;

    if (!do_blending || ui_emulation_is_paused()) {
        *from = *to = bptrs[shell].cpos;
        buffers[shell][*from].alpha = 1.0;
        return 1;       /* render frame */
    }

    last = bptrs[shell].lpos;
    count = PREV(bptrs[shell].cpos, last);
    /* subtract machine cycles */
    dg2 = dthread_ghosting / 2;
    now -= (dg2 * mrp_usec);    /* adjust `now' to fit ghosting */

    /* find display frame interval where we fit in */
    for (i = 0; i < count; i++) {
        if (buffers[shell][last].stamp > now) {
            break;
        }
        last = NEXT(last, 1);
    }
    first = PREV(last, 1);
    dt1 = now - buffers[shell][first].stamp;
    dt2 = buffers[shell][last].stamp - buffers[shell][first].stamp;
    if (dt1 > dt2) {
        /* DBG(("dthread dropping frames")); */
        return 0;       /* do not render frames */
    }
    /* calculate alpha according to position of `now' between 2 frames */
    a1 = ((float) dt1 / dt2);
    a2 = (1.0 - a1);

    /* full range of to be drawn frames */
    first = NEXT(PREV(last, dthread_ghosting), 1);
    t = &buffers[shell][first];

    /* calculate alpha for frames prior to adjusted `now' */
    for (i = 0; i < dg2; i++) {
        t->alpha = a2 * weights[dthread_ghosting][i];
        DBG2(("will draw from: %d, stamp: %ld, alpha: %f", first, t->stamp, t->alpha));
        t = t->next;
    }
    /* calculate alpha for frames after adjusted `now' */
    for (i = dg2; i < dthread_ghosting; i++) {
        t->alpha = a1 * weights[dthread_ghosting][i];
        DBG2(("will draw to:   %d, stamp: %ld, alpha: %f", last, t->stamp, t->alpha));
        t = t->next;
    }

    *from = first;
    *to = last;

#if 0
    for (i = 0; i < MAX_BUFFERS; i++) {
        long ddd;
        ddd = buffers[shell][i].next->stamp - buffers[shell][i].stamp;
        DBG(("shell %d: from %d, i %d  stamp %ld  diff %ld", shell, first, i, buffers[shell][i].stamp, ddd));
    }
#endif

    return 1;           /* render frames */
}

static void *dthread_func(void *arg)
{
    static struct timespec now, to;
    int do_draw = 0;
    /* static struct timespec t1; */

    int ret;
    DBG(("Display thread started..."));

    while (emu_running) {
        if (pthread_mutex_lock(&mutex) < 0) {
            log_debug("pthread_mutex_lock() failed, %s", __FUNCTION__);
            exit (-1);
        }
        while (do_action == CR_NOTHING) {
            clock_gettime(CLOCK_REALTIME, &now);
            tsAdd(&now, &reltime, &to);
            ret = pthread_cond_timedwait(&cond, &mutex, &to);
            if (ret == ETIMEDOUT) {
                if (do_action != CR_NOTHING) {
                    DBG(("%s: race condition with co-routine, action %d triggered", __FUNCTION__, do_action));
                } /* this can go away again - just want to know if it's triggered */
                do_draw = 1;
                break;
            }
            if (ret < 0) {
                log_debug("pthread_cond_wait() failed, %s", __FUNCTION__);
                exit (-1);
            }
        }
        DBG2(("action is: %d, %ld", do_action, TS_TOUSEC(now)/1000));

        if (do_draw) {
            int from, to, shell;

            do_draw = 0;
            pthread_mutex_unlock(&mutex);
            /* clock_gettime(CLOCK_REALTIME, &t1); */

            /* find frame to time 'now' as this is best in sync with the display
               refresh cycle, in case vblank synchronization is active */
            for (shell = 0; shell <= app_shell_count; shell++) {
                if (dthread_calc_frames(TS_TOUSEC(now), &from, &to, shell)) {
                    gl_render_canvas(bptrs[shell].widget, bptrs[shell].canvas, buffers[shell], from, to, shell == get_active_shell());

                    /* set to `from' as a frames may be drawn twice */
                    bptrs[shell].lpos = from;
#if 0
                    /* timing probe */
                    {
                        static struct timespec t2, t3;

                        clock_gettime(CLOCK_REALTIME, &t2);
                        long diff = TS_TOUSEC(t2) - TS_TOUSEC(t1);
                        float fps = 1000 * 1000.0 / (TS_TOUSEC(t1) - TS_TOUSEC(t3));
                        DBG(("glrender time: %5ldus  fps %3.2f", diff, fps));
                        memcpy(&t3, &t1, sizeof(struct timespec));
                    }
#endif
                }
            }
            continue;
        } else if (do_action == CR_CANVAS_WIDGET) {
            is_coroutine = 1;
            build_screen_canvas_widget2(canvas);
        } else if (do_action == CR_OPEN_CANVAS) {
            is_coroutine = 1;
            int_ret = ui_open_canvas_window2(canvas, title, width, height, no_autorepeat);
        } else if (do_action == CR_INIT) {
            is_coroutine = 1;
            (void)ui_init2(argc, argv);
        } else if (do_action == CR_INIT_FINISH) {
            is_coroutine = 1;
            int_ret = ui_init_finish2();
        } else if (do_action == CR_CONFIGURE_CALLBACK) {
            is_coroutine = 1;
            int_ret = configure_callback_canvas2(widget, event, client_data);
        } else if (do_action == CR_RESIZE) {
            is_coroutine = 1;
            ui_trigger_resize2();
        } else if (do_action == CR_WINDOW_RESIZE) {
            is_coroutine = 1;
            ui_trigger_window_resize2(canvas);
        } else if (do_action == CR_SHUTDOWN) {
            emu_running = 0;
        }
        if (is_coroutine) {
            DBG2(("syncronised call for action: %d- intermediate", do_action));
            if (pthread_cond_signal(&coroutine) < 0) {
                log_debug("pthread_cond_signal() failed, %s", __FUNCTION__);
                exit (-1);
            }
        }
        do_action = CR_NOTHING;
        if (pthread_mutex_unlock(&mutex) < 0) {
            log_debug("pthread_mutex_unlock() failed, %s", __FUNCTION__);
            exit (-1);
        }
    }
    exit(0);
}

static void dthread_coroutine(coroutine_t action)
{
    struct timespec ts;
    int ret;

    if (pthread_mutex_lock(&mutex) < 0) {
        log_debug("pthread_mutex_lock() failed, %s", __FUNCTION__);
        exit (-1);
    }
    /* retry:    */
    do_action = action;
    DBG2(("syncronised call for action: %d - start", action));
    if (pthread_cond_signal(&cond) < 0) {
        log_debug("pthread_cond_signal() failed, %s", __FUNCTION__);
        exit (-1);
    }
    while (!is_coroutine) {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 10;
        DBG2(("syncronised call for action: %d - before condwait", action));
        ret = pthread_cond_timedwait(&coroutine, &mutex, &ts);
        if (ret == ETIMEDOUT) {
            log_message(LOG_DEFAULT, "%s: timeout synchronized call for action %d", __FUNCTION__, action);
            /* goto retry; */
            exit (-1);      /* continuation is probably not meaningful */
        }
        if (ret < 0) {
            log_debug("pthread_cond_timedwait() failed, %s", __FUNCTION__);
            exit (-1);
        }
    }

    is_coroutine = 0;
    if (pthread_mutex_unlock(&mutex) < 0) {
        log_debug("pthread_mutex_unlock() failed, %s", __FUNCTION__);
        exit (-1);
    }
    DBG2(("syncronised call for action: %d - done", action));
}

static void *ethread_func(void *arg)
{
    int ret;

    /* this thread takes only care on gtk+ events */
    DBG(("GUI Event handler thread started..."));
    while (emu_running) {
        ret = sem_wait(&ethread_sem);
        if (ret < 0) {
            if (errno == EINTR) {
                continue; /* don't ask */
            }
            log_debug("sem_wait() failed, %s", __FUNCTION__);
            exit (-1);
        }
        ui_dispatch_events2();
        if (sem_post(&mthread_sem) != 0) {
            log_debug("sem_wait() failed, %s", __FUNCTION__);
            exit (-1);
        }
    }
    exit(0);
}
