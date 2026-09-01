/*
 * io_stats.h
 *
 * Lightweight storage I/O instrumentation for BMC64.
 *
 * OPT-IN: the whole thing is compiled out unless BMC64_IO_STATS is defined
 * (build with: ./make_all.sh <board> --io-stats; see docs/BUILDING.md). With
 * it undefined every hook below is a static-inline no-op, io_stats.c is an
 * empty object, the menu screen / benchmark / auto-dumps are #ifdef'd out,
 * and circle_diskio_stats_patch.diff is not applied - zero code, zero BSS.
 *
 * When enabled: counters are accumulated in RAM by thin record hooks called
 * from the FatFs disk glue (diskio.cpp) and BMC64's newlib syscall layer
 * (new_io.cpp). Hooks also no-op at runtime while io_stats_enabled is zero.
 * Timing is measured by the C++ call sites with CTimer::GetClockTicks()
 * (1 MHz) and passed in as microseconds, so io_stats.c stays pure C.
 */

#ifndef BMC64_IO_STATS_H
#define BMC64_IO_STATS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Open modes, matching the low 2 bits of the POSIX open() flags used by
 * new_io.cpp (O_RDONLY 0, O_WRONLY 1, O_RDWR 2). */
#define IO_STATS_MODE_RDONLY 0
#define IO_STATS_MODE_WRONLY 1
#define IO_STATS_MODE_RDWR   2

#ifdef BMC64_IO_STATS

/* Non-zero: record hooks accumulate. Zero: hooks return immediately.
 * Defaults to 1. */
extern volatile int io_stats_enabled;

/* Physical device layer (FatFs diskio glue).
 *   sectors    - number of 512-byte sectors in the transfer
 *   count      - the FatFs multi-sector 'count' argument
 *   elapsed_us - wall time of the device Read()/Write() call
 *   unaligned  - non-zero if the transfer went through the bounce buffer */
void io_stats_disk_read(unsigned sectors, unsigned count,
                        unsigned elapsed_us, int unaligned);
void io_stats_disk_write(unsigned sectors, unsigned count,
                         unsigned elapsed_us, int unaligned);

/* Application layer (new_io.cpp). */
void io_stats_open(int mode, int fastfail_hit);
void io_stats_slurp(unsigned bytes, unsigned elapsed_us, int by_lseek);
void io_stats_read(int from_ram, unsigned bytes);
void io_stats_write(int mode, unsigned bytes);
void io_stats_lseek(void);
void io_stats_stat(int fast_path_hit);
void io_stats_opendir(void);
void io_stats_readdir_entry(void);

/* Control / reporting. */
void io_stats_reset(void);

/* Write a human-readable multi-line report into buf. Returns the number of
 * bytes written (excluding the terminating NUL), or the length that would
 * have been written if buf were large enough. */
int io_stats_format(char *buf, int len);

/* Like io_stats_format but with short lines (<= 34 chars) for the menu. */
int io_stats_format_compact(char *buf, int len);

/* printf the report to stdout (routed to /bmc64.log and serial), prefixed
 * with a header line naming 'label'. */
void io_stats_dump(const char *label);

#else /* !BMC64_IO_STATS - compile every call site down to nothing */

static inline void io_stats_disk_read(unsigned s, unsigned c, unsigned e, int u)
  { (void)s; (void)c; (void)e; (void)u; }
static inline void io_stats_disk_write(unsigned s, unsigned c, unsigned e, int u)
  { (void)s; (void)c; (void)e; (void)u; }
static inline void io_stats_open(int m, int f) { (void)m; (void)f; }
static inline void io_stats_slurp(unsigned b, unsigned e, int l)
  { (void)b; (void)e; (void)l; }
static inline void io_stats_read(int r, unsigned b) { (void)r; (void)b; }
static inline void io_stats_write(int m, unsigned b) { (void)m; (void)b; }
static inline void io_stats_lseek(void) {}
static inline void io_stats_stat(int f) { (void)f; }
static inline void io_stats_opendir(void) {}
static inline void io_stats_readdir_entry(void) {}
static inline void io_stats_reset(void) {}
static inline void io_stats_dump(const char *label) { (void)label; }

#endif /* BMC64_IO_STATS */

#ifdef __cplusplus
}
#endif

#endif /* BMC64_IO_STATS_H */
