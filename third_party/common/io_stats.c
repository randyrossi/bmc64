/*
 * io_stats.c
 *
 * See io_stats.h. Pure C, no Circle dependency.
 *
 * Entirely compiled out unless BMC64_IO_STATS is defined - when it is not,
 * this is an empty translation unit and io_stats.h supplies inline no-ops.
 */

#include "io_stats.h"

#ifdef BMC64_IO_STATS

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

volatile int io_stats_enabled = 1;

#define NHIST 9
/* Upper bound (inclusive) of each 'count' histogram bucket; last is catch-all. */
static const unsigned hist_hi[NHIST] = { 1, 2, 4, 8, 16, 32, 64, 128, 0xffffffffu };
static const char *const hist_label[NHIST] = {
  "1", "2", "3-4", "5-8", "9-16", "17-32", "33-64", "65-128", ">128"
};

struct dev_stats {
  unsigned long long calls;
  unsigned long long sectors;
  unsigned long long elapsed_us;
  unsigned max_us;
  unsigned long long unaligned;
  unsigned long long hist[NHIST];
};

struct io_stats {
  struct dev_stats rd;
  struct dev_stats wr;

  /* new_io.cpp open() by mode + bootstat fast-fail */
  unsigned long long open_rdonly;
  unsigned long long open_wronly;
  unsigned long long open_rdwr;
  unsigned long long open_fastfail;

  /* slurp (whole-file read into RAM) */
  unsigned long long slurp_calls;
  unsigned long long slurp_bytes;
  unsigned long long slurp_us;
  unsigned slurp_max_us;
  unsigned long long slurp_by_lseek;
  unsigned long long slurp_by_open;

  /* read()/write() */
  unsigned long long read_disk_calls;
  unsigned long long read_disk_bytes;
  unsigned long long read_ram_calls;
  unsigned long long read_ram_bytes;
  unsigned long long write_calls;
  unsigned long long write_bytes;

  unsigned long long lseek_calls;
  unsigned long long stat_calls;
  unsigned long long stat_fastpath;
  unsigned long long opendir_calls;
  unsigned long long readdir_entries;
};

static struct io_stats g;

static int hist_index(unsigned count) {
  int i;
  for (i = 0; i < NHIST; i++) {
    if (count <= hist_hi[i]) {
      return i;
    }
  }
  return NHIST - 1;
}

static void dev_record(struct dev_stats *d, unsigned sectors, unsigned count,
                       unsigned elapsed_us, int unaligned) {
  d->calls++;
  d->sectors += sectors;
  d->elapsed_us += elapsed_us;
  if (elapsed_us > d->max_us) {
    d->max_us = elapsed_us;
  }
  if (unaligned) {
    d->unaligned++;
  }
  d->hist[hist_index(count)]++;
}

void io_stats_disk_read(unsigned sectors, unsigned count,
                        unsigned elapsed_us, int unaligned) {
  if (!io_stats_enabled) {
    return;
  }
  dev_record(&g.rd, sectors, count, elapsed_us, unaligned);
}

void io_stats_disk_write(unsigned sectors, unsigned count,
                         unsigned elapsed_us, int unaligned) {
  if (!io_stats_enabled) {
    return;
  }
  dev_record(&g.wr, sectors, count, elapsed_us, unaligned);
}

void io_stats_open(int mode, int fastfail_hit) {
  if (!io_stats_enabled) {
    return;
  }
  if (fastfail_hit) {
    g.open_fastfail++;
    return;
  }
  switch (mode) {
  case IO_STATS_MODE_WRONLY: g.open_wronly++; break;
  case IO_STATS_MODE_RDWR:   g.open_rdwr++;   break;
  default:                   g.open_rdonly++; break;
  }
}

void io_stats_slurp(unsigned bytes, unsigned elapsed_us, int by_lseek) {
  if (!io_stats_enabled) {
    return;
  }
  g.slurp_calls++;
  g.slurp_bytes += bytes;
  g.slurp_us += elapsed_us;
  if (elapsed_us > g.slurp_max_us) {
    g.slurp_max_us = elapsed_us;
  }
  if (by_lseek) {
    g.slurp_by_lseek++;
  } else {
    g.slurp_by_open++;
  }
}

void io_stats_read(int from_ram, unsigned bytes) {
  if (!io_stats_enabled) {
    return;
  }
  if (from_ram) {
    g.read_ram_calls++;
    g.read_ram_bytes += bytes;
  } else {
    g.read_disk_calls++;
    g.read_disk_bytes += bytes;
  }
}

void io_stats_write(int mode, unsigned bytes) {
  if (!io_stats_enabled) {
    return;
  }
  (void)mode;
  g.write_calls++;
  g.write_bytes += bytes;
}

void io_stats_lseek(void) {
  if (io_stats_enabled) {
    g.lseek_calls++;
  }
}

void io_stats_stat(int fast_path_hit) {
  if (!io_stats_enabled) {
    return;
  }
  g.stat_calls++;
  if (fast_path_hit) {
    g.stat_fastpath++;
  }
}

void io_stats_opendir(void) {
  if (io_stats_enabled) {
    g.opendir_calls++;
  }
}

void io_stats_readdir_entry(void) {
  if (io_stats_enabled) {
    g.readdir_entries++;
  }
}

void io_stats_reset(void) {
  memset(&g, 0, sizeof(g));
}

/* Append helper: writes into buf[*off..len) and advances *off by the number of
 * characters the formatted string would occupy (snprintf semantics), so the
 * caller can detect truncation. */
static void ap(char *buf, int len, int *off, const char *fmt, ...) {
  va_list ap_;
  int n;
  int space = (*off < len) ? (len - *off) : 0;

  va_start(ap_, fmt);
  n = vsnprintf(space ? (buf + *off) : NULL, space, fmt, ap_);
  va_end(ap_);

  if (n > 0) {
    *off += n;
  }
}

/* This newlib is built without --enable-newlib-io-long-long, so printf/vsnprintf
 * do NOT understand %llu/%lld - using them corrupts the whole varargs list.
 * Everything below prints 32-bit only: counts as-is, byte totals as KiB, and
 * elapsed-time totals as ms. Per-call figures stay in us. */
#define IOU(x) ((unsigned)(x))
#define IOKIB(x) ((unsigned)((x) / 1024ull))
#define IOMS(x) ((unsigned)((x) / 1000ull))

static void fmt_dev(char *buf, int len, int *off, const char *name,
                    const struct dev_stats *d) {
  int i;
  unsigned avg = d->calls ? IOU(d->elapsed_us / d->calls) : 0;

  ap(buf, len, off,
     "%s: calls=%u sectors=%u (%u KiB) total=%ums avg=%uus max=%uus unaligned=%u\n",
     name, IOU(d->calls), IOU(d->sectors), IOU((d->sectors * 512ull) / 1024ull),
     IOMS(d->elapsed_us), avg, d->max_us, IOU(d->unaligned));
  ap(buf, len, off, "  count histogram:");
  for (i = 0; i < NHIST; i++) {
    if (d->hist[i]) {
      ap(buf, len, off, " %s=%u", hist_label[i], IOU(d->hist[i]));
    }
  }
  ap(buf, len, off, "\n");
}

int io_stats_format(char *buf, int len) {
  int off = 0;
  unsigned slurp_avg = g.slurp_calls ? IOU(g.slurp_us / g.slurp_calls) : 0;

  ap(buf, len, &off, "enabled=%d\n", io_stats_enabled);

  fmt_dev(buf, len, &off, "disk_read ", &g.rd);
  fmt_dev(buf, len, &off, "disk_write", &g.wr);

  ap(buf, len, &off,
     "open: rdonly=%u wronly=%u rdwr=%u fastfail=%u\n",
     IOU(g.open_rdonly), IOU(g.open_wronly), IOU(g.open_rdwr), IOU(g.open_fastfail));

  ap(buf, len, &off,
     "slurp: calls=%u bytes=%u KiB total=%ums avg=%uus max=%uus "
     "by_lseek=%u by_open=%u\n",
     IOU(g.slurp_calls), IOKIB(g.slurp_bytes), IOMS(g.slurp_us),
     slurp_avg, g.slurp_max_us, IOU(g.slurp_by_lseek), IOU(g.slurp_by_open));

  ap(buf, len, &off,
     "read: from_disk calls=%u KiB=%u | from_ram calls=%u KiB=%u\n",
     IOU(g.read_disk_calls), IOKIB(g.read_disk_bytes),
     IOU(g.read_ram_calls), IOKIB(g.read_ram_bytes));

  ap(buf, len, &off, "write: calls=%u KiB=%u\n",
     IOU(g.write_calls), IOKIB(g.write_bytes));

  ap(buf, len, &off,
     "lseek=%u stat=%u (fastpath=%u) opendir=%u readdir_entries=%u\n",
     IOU(g.lseek_calls), IOU(g.stat_calls), IOU(g.stat_fastpath),
     IOU(g.opendir_calls), IOU(g.readdir_entries));

  return off;
}

int io_stats_format_compact(char *buf, int len) {
  int off = 0;
  int i;
  unsigned rd_avg = g.rd.calls ? IOU(g.rd.elapsed_us / g.rd.calls) : 0;
  unsigned wr_avg = g.wr.calls ? IOU(g.wr.elapsed_us / g.wr.calls) : 0;
  unsigned sl_avg = g.slurp_calls ? IOU(g.slurp_us / g.slurp_calls) : 0;

  ap(buf, len, &off, "collecting: %s\n", io_stats_enabled ? "on" : "off");

  ap(buf, len, &off, "disk_read %u calls %u sec\n",
     IOU(g.rd.calls), IOU(g.rd.sectors));
  ap(buf, len, &off, "  avg %uus max %uus tot %ums\n",
     rd_avg, g.rd.max_us, IOMS(g.rd.elapsed_us));
  ap(buf, len, &off, "  hist");
  for (i = 0; i < NHIST; i++) {
    if (g.rd.hist[i]) {
      ap(buf, len, &off, " %s:%u", hist_label[i], IOU(g.rd.hist[i]));
    }
  }
  ap(buf, len, &off, "\n");

  ap(buf, len, &off, "disk_write %u calls %u sec\n",
     IOU(g.wr.calls), IOU(g.wr.sectors));
  if (g.wr.calls) {
    ap(buf, len, &off, "  avg %uus max %uus\n", wr_avg, g.wr.max_us);
  }

  ap(buf, len, &off, "open r/w/rw/ff %u/%u/%u/%u\n",
     IOU(g.open_rdonly), IOU(g.open_wronly), IOU(g.open_rdwr), IOU(g.open_fastfail));

  ap(buf, len, &off, "slurp %u x  %u KiB\n",
     IOU(g.slurp_calls), IOKIB(g.slurp_bytes));
  if (g.slurp_calls) {
    ap(buf, len, &off, "  avg %uus max %uus tot %ums\n",
       sl_avg, g.slurp_max_us, IOMS(g.slurp_us));
    ap(buf, len, &off, "  by lseek/open %u/%u\n",
       IOU(g.slurp_by_lseek), IOU(g.slurp_by_open));
  }

  ap(buf, len, &off, "read disk %u c  %u KiB\n",
     IOU(g.read_disk_calls), IOKIB(g.read_disk_bytes));
  ap(buf, len, &off, "read ram  %u c  %u KiB\n",
     IOU(g.read_ram_calls), IOKIB(g.read_ram_bytes));
  ap(buf, len, &off, "write %u c  %u KiB\n",
     IOU(g.write_calls), IOKIB(g.write_bytes));

  ap(buf, len, &off, "lseek %u\n", IOU(g.lseek_calls));
  ap(buf, len, &off, "stat %u (fast %u)\n", IOU(g.stat_calls), IOU(g.stat_fastpath));
  ap(buf, len, &off, "dir open %u  entries %u\n",
     IOU(g.opendir_calls), IOU(g.readdir_entries));

  return off;
}

void io_stats_dump(const char *label) {
  static char buf[2048];
  io_stats_format(buf, (int)sizeof(buf));
  printf("=== I/O stats: %s ===\n%s=====================\n",
         label ? label : "", buf);
}

#endif /* BMC64_IO_STATS */
