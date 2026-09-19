/*
 * perf_stats.c
 *
 * See perf_stats.h and docs/architecture/PERFORMANCE_TEST_PLAN.md.
 *
 * Entirely compiled out unless BMC64_PERF_STATS is defined - when it is not,
 * this is an empty translation unit and perf_stats.h supplies inline no-ops.
 */

#include "perf_stats.h"

#ifdef BMC64_PERF_STATS

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "circle.h"

extern const char *bmc64_version_string(void); /* menu.c */

/* Length of a reporting window. Sums are 32-bit microseconds, so keep this
 * comfortably under ~60 s. */
#ifndef BMC64_PERF_WINDOW_SECS
#define BMC64_PERF_WINDOW_SECS 10
#endif
#define WINDOW_US (BMC64_PERF_WINDOW_SECS * 1000000u)

/* A gap this long between frames means emulation was paused (menu open,
 * snapshot, ...), not that a frame was slow. Such frames are not recorded. */
#define GAP_US 500000u

#define PERF_JSON_VERSION 1

/* The firmware mailbox queries behind the "env" fields (temperature, throttle
 * bits, live clock) are answered by the GPU firmware, which also services the
 * emulator's frame present. Query only every Nth report and repeat the cached
 * values in between, and flag the reports that queried ("fresh"), so any effect
 * on the frame that follows can be seen and excluded. */
#define PERF_ENV_EVERY 6

/* Frame busy time is bucketed as a percentage of the nominal frame period so
 * the histogram means the same thing for PAL, NTSC and any custom timing.
 * Bucket i holds pct < hist_edge[i]; the last bucket is pct >= 100, i.e. a
 * frame that could not finish inside its period. */
#define NHIST 8
static const unsigned hist_edge[NHIST - 1] = {25, 50, 60, 70, 80, 90, 100};

struct pstat {
  unsigned n;
  unsigned sum;
  unsigned min;
  unsigned max;
};

struct perf_window {
  unsigned seq;
  unsigned dur_us;
  unsigned nominal_us;

  unsigned frames; /* recorded (vblank-synced, non-gap) frames */
  unsigned gaps;   /* frames skipped because emulation had been paused */
  unsigned missed; /* frame period > 1.5 x nominal: a vblank was missed */

  /* Per-frame stages. busy = emu + post + ready + tail, i.e. everything
   * except the vblank wait (swap), which is the slack. */
  struct pstat busy, emu, post, ready, swap, tail, period;
  unsigned busy_hist[NHIST];

  /* VC4 audio buffer level (samples) sampled once per frame, right after
   * vblank, which is when it is at its lowest. */
  struct pstat fill;
  unsigned fill_zero;
  unsigned fill_cap;
  struct pstat aud_write;
  unsigned aud_full_waits;
  unsigned aud_silent;

  struct pstat sid_wait;

  /* The slowest frame of the window, and how far into the window it began.
   * Lets a one-off stall be tied to what else was happening at that moment
   * (e.g. the report from the previous window being printed). */
  struct {
    unsigned at_ms;
    unsigned busy, emu, post, ready, tail;
  } worst;
};

/* Live window: written by the emulator core only. */
static struct perf_window live;
static unsigned win_start;
static int win_open;
static unsigned window_seq;

/* Hand-off slot to the reporting context, guarded by circle_lock. */
static struct perf_window pending;
static volatile int pending_ready;
static unsigned dropped_total;

static const char *core_name = "unknown";

/* Per-frame scratch, emulator core only. */
static struct {
  unsigned enter, post_done, swap_end, prev_enter, prev_exit;
  unsigned nominal, ready, swap, fill, cap;
  unsigned run; /* consecutive recorded frames leading up to this one */
  int have_exit, presented, sync, gap;
} f;

#ifdef RASPI_LITE
/* Pi 0 is single core: the reporter runs as a scheduler task inside
 * circle_yield() on the emulator's own core, so each UART chunk it writes
 * lengthens the frame in progress. The service sets this so the frame hooks
 * ignore that frame. */
static volatile unsigned skip_frames;
#endif

unsigned perf_now(void) { return (unsigned)circle_get_ticks(); }

static void pstat_add(struct pstat *s, unsigned v) {
  if (s->n == 0 || v < s->min) {
    s->min = v;
  }
  if (v > s->max) {
    s->max = v;
  }
  s->sum += v;
  s->n++;
}

static void close_window(unsigned now) {
  if (live.frames == 0) {
    /* Nothing recorded (e.g. all warp). Just start a fresh window. */
    memset(&live, 0, sizeof(live));
    win_start = now;
    return;
  }

  live.seq = ++window_seq;
  live.dur_us = now - win_start;

  circle_lock_acquire();
  if (!pending_ready) {
    pending = live;
    pending_ready = 1;
  } else {
    dropped_total++; /* reporter has not consumed the previous window */
  }
  circle_lock_release();

  memset(&live, 0, sizeof(live));
  win_start = perf_now();
}

void perf_frame_begin(unsigned nominal_us) {
  unsigned now = perf_now();
  f.enter = now;
  f.post_done = now;
  f.nominal = nominal_us;
  f.presented = 0;
  f.gap = f.have_exit && (unsigned)(now - f.prev_exit) >= GAP_US;
}

void perf_frame_post_done(void) { f.post_done = perf_now(); }

void perf_frame_present(unsigned ready_us, unsigned swap_us, int sync,
                        unsigned audio_fill, unsigned audio_capacity) {
  f.ready = ready_us;
  f.swap = swap_us;
  f.sync = sync;
  f.fill = audio_fill;
  f.cap = audio_capacity;
  f.swap_end = perf_now();
  f.presented = 1;
}

void perf_frame_end(void) {
  unsigned now = perf_now();
  int skipping = 0;
  int record;

#ifdef RASPI_LITE
  if (skip_frames) {
    skip_frames--;
    skipping = 1;
  }
#endif

  record = f.presented && f.sync && f.have_exit && !f.gap && !skipping &&
           f.nominal != 0;

  if (record) {
    unsigned emu = f.enter - f.prev_exit;
    unsigned post = f.post_done - f.enter;
    unsigned tail = now - f.swap_end;
    unsigned busy = emu + post + f.ready + tail;
    unsigned pct = (busy * 100u) / f.nominal;
    unsigned b = 0;

    while (b < NHIST - 1 && pct >= hist_edge[b]) {
      b++;
    }

    if (busy > live.worst.busy) {
      live.worst.at_ms = (f.enter - win_start) / 1000u;
      live.worst.busy = busy;
      live.worst.emu = emu;
      live.worst.post = post;
      live.worst.ready = f.ready;
      live.worst.tail = tail;
    }

    live.nominal_us = f.nominal;
    live.frames++;
    live.busy_hist[b]++;
    pstat_add(&live.busy, busy);
    pstat_add(&live.emu, emu);
    pstat_add(&live.post, post);
    pstat_add(&live.ready, f.ready);
    pstat_add(&live.swap, f.swap);
    pstat_add(&live.tail, tail);

    /* A frame starts locked to vblank only if the frame before it was
     * presented with a vblank sync. A period spans two frame starts, so both
     * bounding frames must be locked, i.e. the two previous frames were
     * recorded. Otherwise the first period after warp or a menu pause looks
     * like a missed vblank when it is not. */
    if (f.run >= 2) {
      unsigned period = f.enter - f.prev_enter;
      pstat_add(&live.period, period);
      if (period * 2u > f.nominal * 3u) {
        live.missed++;
      }
    }

    if (f.cap != 0) {
      pstat_add(&live.fill, f.fill);
      live.fill_cap = f.cap;
      if (f.fill == 0) {
        live.fill_zero++;
      }
    }
  } else if (f.presented && f.sync && f.gap && !skipping) {
    live.gaps++;
  }

  f.run = record ? f.run + 1 : 0;
  f.prev_enter = f.enter;

  if (!win_open) {
    win_start = now;
    win_open = 1;
  } else if ((unsigned)(now - win_start) >= WINDOW_US) {
    close_window(now);
  }

  /* Taken last so the window hand-off is not billed to the next frame. */
  f.prev_exit = perf_now();
  f.have_exit = 1;
}

void perf_audio_write(unsigned elapsed_us) {
  pstat_add(&live.aud_write, elapsed_us);
}

void perf_audio_full_wait(void) { live.aud_full_waits++; }

void perf_audio_silent_packet(void) { live.aud_silent++; }

void perf_sid_wait(unsigned elapsed_us) { pstat_add(&live.sid_wait, elapsed_us); }

void perf_stats_set_core(const char *core) {
  if (core != 0) {
    core_name = core;
  }
}

void perf_stats_reset(void) {
  memset(&live, 0, sizeof(live));
  win_open = 0;
  window_seq = 0;
  f.have_exit = 0;
  f.run = 0;

  circle_lock_acquire();
  pending_ready = 0;
  dropped_total = 0;
  circle_lock_release();
}

/* ---- Reporting (core 0 / Pi 0 scheduler task) ------------------------- */

struct out {
  char *b;
  int cap;
  int len;
};

static void put(struct out *o, const char *fmt, ...) {
  va_list ap;
  int n;

  if (o->len >= o->cap - 1) {
    return;
  }
  va_start(ap, fmt);
  n = vsnprintf(o->b + o->len, (size_t)(o->cap - o->len), fmt, ap);
  va_end(ap);
  if (n < 0) {
    return;
  }
  o->len += n;
  if (o->len > o->cap - 1) {
    o->len = o->cap - 1;
  }
}

static void put_stat(struct out *o, const char *name, const struct pstat *s,
                     const char *sep) {
  put(o, "\"%s\":{\"n\":%u,\"min\":%u,\"avg\":%u,\"max\":%u}%s", name, s->n,
      s->n ? s->min : 0u, s->n ? s->sum / s->n : 0u, s->max, sep);
}

static void format_window(struct out *o, const struct perf_window *w,
                          unsigned dropped, int env_fresh, int temp_mc,
                          unsigned throttled, unsigned arm_hz) {
  unsigned i;

  put(o,
      "{\"v\":%d,\"core\":\"%s\",\"bmc64\":\"%s\",\"pi\":%d,\"win\":%u,"
      "\"dur_ms\":%u,\"nominal_us\":%u,\"frames\":%u,\"gaps\":%u,"
      "\"missed\":%u,\"dropped\":%u,",
      PERF_JSON_VERSION, core_name, bmc64_version_string(), circle_get_model(),
      w->seq, w->dur_us / 1000u, w->nominal_us, w->frames, w->gaps, w->missed,
      dropped);

  put_stat(o, "busy", &w->busy, ",");
  put_stat(o, "emu", &w->emu, ",");
  put_stat(o, "post", &w->post, ",");
  put_stat(o, "ready", &w->ready, ",");
  put_stat(o, "swap", &w->swap, ",");
  put_stat(o, "tail", &w->tail, ",");
  put_stat(o, "period", &w->period, ",");

  put(o, "\"busy_hist\":{\"edges_pct\":[");
  for (i = 0; i < NHIST - 1; i++) {
    put(o, "%s%u", i ? "," : "", hist_edge[i]);
  }
  put(o, "],\"n\":[");
  for (i = 0; i < NHIST; i++) {
    put(o, "%s%u", i ? "," : "", w->busy_hist[i]);
  }
  put(o, "]},");

  put(o, "\"audio\":{");
  put_stat(o, "fill", &w->fill, ",");
  put(o, "\"fill_zero\":%u,\"fill_cap\":%u,", w->fill_zero, w->fill_cap);
  put_stat(o, "write", &w->aud_write, ",");
  put(o, "\"full_waits\":%u,\"silent\":%u},", w->aud_full_waits,
      w->aud_silent);

  put(o, "\"sid\":{");
  put_stat(o, "wait", &w->sid_wait, "");
  put(o, "},");

  put(o,
      "\"worst\":{\"at_ms\":%u,\"busy\":%u,\"emu\":%u,\"post\":%u,"
      "\"ready\":%u,\"tail\":%u},",
      w->worst.at_ms, w->worst.busy, w->worst.emu, w->worst.post,
      w->worst.ready, w->worst.tail);

  put(o,
      "\"env\":{\"fresh\":%d,\"temp_mc\":%d,\"throttled\":%u,"
      "\"arm_hz\":%u}}",
      env_fresh, temp_mc, throttled, arm_hz);
}

/* The report line is about 900 bytes. Written to the UART in one call it takes
 * ~80 ms at 115200 baud, and the frame present on the emulator core was
 * observed to stall for the whole of that time. So it is sent in small chunks,
 * spaced out, each short enough (~3 ms) to disappear into the ~10 ms of
 * vblank slack. Anything else printing in between can split the line; the host
 * tool treats such a line as malformed and drops that one window. */
#define PERF_CHUNK_BYTES 32
#define PERF_CHUNK_GAP_US 20000u

void perf_stats_service(void) {
  static struct perf_window snap;
  static char buf[1792];
  static unsigned out_len;
  static unsigned out_pos;
  static unsigned last_chunk;
  static unsigned reports;
  static int temp_mc;
  static unsigned throttled = 0xFFFFFFFFu; /* "unknown" until first query */
  static unsigned arm_hz;
  struct out o;
  unsigned dropped;
  int env_fresh;
  unsigned n;

  if (out_pos < out_len) {
    /* Still sending the previous report. */
    if ((unsigned)(perf_now() - last_chunk) < PERF_CHUNK_GAP_US) {
      return;
    }
  } else {
    if (!pending_ready) {
      return;
    }

    circle_lock_acquire();
    if (!pending_ready) {
      circle_lock_release();
      return;
    }
    snap = pending;
    dropped = dropped_total;
    pending_ready = 0;
    circle_lock_release();

    env_fresh = (reports++ % PERF_ENV_EVERY) == 0;
    if (env_fresh) {
      circle_perf_get_env(&temp_mc, &throttled, &arm_hz);
    }

    o.b = buf;
    o.cap = (int)sizeof(buf);
    o.len = 0;
    buf[0] = '\0';
    put(&o, "[perf] ");
    format_window(&o, &snap, dropped, env_fresh, temp_mc, throttled, arm_hz);
    put(&o, "\n");
    out_len = (unsigned)o.len;
    out_pos = 0;
  }

  n = out_len - out_pos;
  if (n > PERF_CHUNK_BYTES) {
    n = PERF_CHUNK_BYTES;
  }
  fwrite(buf + out_pos, 1, n, stdout);
  fflush(stdout); /* stdio must not gather the chunks back into one write */
  out_pos += n;
  last_chunk = perf_now();

#ifdef RASPI_LITE
  /* Pi 0 is single core: this runs inside the emulator's frame, so the frame
   * that carried a chunk is not representative. */
  skip_frames = 1;
#endif
}

#endif /* BMC64_PERF_STATS */
