/*
 * perf_stats.h
 *
 * Frame-budget, audio and SID performance instrumentation for BMC64. The
 * report format and metrics are described in tools/perftest/README.md.
 *
 * DESIGN RULE: the metrics here must not depend on VICE internals. The hooks
 * live in BMC64-owned code (the arch/raspi layer, kernel, audio device) so the
 * same numbers can be compared across emulator cores (VICE 3.3 today, a
 * future VICE upgrade, Plus4Emu). Keep VICE-side edits to the minimum needed.
 *
 * OPT-IN: the whole thing is compiled out unless BMC64_PERF_STATS is defined
 * (build with: ./make_all.sh <board> --perf-stats; see docs/BUILDING.md).
 * With it undefined every hook below is a static-inline no-op, perf_stats.c
 * is an empty object and perf_now() returns 0 without reading the timer, so a
 * normal build carries no extra code, BSS or timer reads.
 *
 * THREADING: all record hooks are called from the emulator core (core 1 on
 * Pi 2/3, the only core on Pi 0) and only touch that core's private window.
 * They are O(1) counter updates - never printf or take a lock from a hook.
 * When a window (BMC64_PERF_WINDOW_SECS, default 10) closes, the emulator
 * core hands a snapshot to perf_stats_service(), which runs on core 0 (or, on
 * Pi 0, from the USB/logging scheduler task) and prints one line:
 *
 *     [perf] {"v":1,"win":3,...}
 *
 * Times are microseconds from the 1 MHz system timer (circle_get_ticks()).
 */

#ifndef BMC64_PERF_STATS_H
#define BMC64_PERF_STATS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BMC64_PERF_STATS

/* Current 1 MHz timer value. Subtract with unsigned arithmetic (wraps). */
unsigned perf_now(void);

/* Frame hooks, called once per emulated frame from vsyncarch_postsync()
 * (arch/raspi/videoarch.c) in this order:
 *   perf_frame_begin        - first thing in the function
 *   perf_frame_post_done    - just before the VIC layer is presented
 *   perf_frame_present      - from CKernel::circle_frames_ready_fbl() for the
 *                             VIC layer: ready_us = copying/rendering the
 *                             frame, swap_us = the swap, which normally
 *                             blocks until vertical blank (the slack).
 *                             sync is the caller's vblank-sync flag; frames
 *                             without it (warp) are not recorded.
 *                             audio_fill/audio_capacity are the VC4 audio
 *                             buffer level in samples (capacity 0 = none).
 *   perf_frame_end          - last thing in the function
 * nominal_us is the machine's frame period (e.g. 19950 for PAL). */
void perf_frame_begin(unsigned nominal_us);
void perf_frame_post_done(void);
void perf_frame_present(unsigned ready_us, unsigned swap_us, int sync,
                        unsigned audio_fill, unsigned audio_capacity);
void perf_frame_end(void);

/* Audio path (vicesound.cpp / CKernel::circle_sound_write). */
void perf_audio_write(unsigned elapsed_us);  /* one AddChunk() call */
void perf_audio_full_wait(void);             /* GetChunk() found buffer full */
void perf_audio_silent_packet(void);         /* GetChunk() had no data */

/* Dual SID (sid.c): core 1 blocked waiting for the SID2 helper core. This is
 * BMC64's own code path, so it moves with the port when the VICE core is
 * upgraded. SID computation cost is deliberately NOT hooked inside VICE's
 * generic sound code; measure it differentially (SID workload vs idle
 * workload) so the suite stays independent of the VICE version. */
void perf_sid_wait(unsigned elapsed_us);

/* Name the emulator core producing the numbers, e.g. "vice-3.3". Reported in
 * every line so results from different cores are never mixed up. Call once
 * from the core's machine init (emu_machine_init). */
void perf_stats_set_core(const char *core);

/* Discard the live window and any unreported snapshot. Call from the
 * emulator core (CKernel::circle_boot_complete()). */
void perf_stats_reset(void);

/* Print any closed window. Call periodically from core 0 (or the Pi 0 usbpnp
 * task); cheap when nothing is pending. */
void perf_stats_service(void);

#else /* !BMC64_PERF_STATS - compile every call site down to nothing */

static inline unsigned perf_now(void) { return 0; }
static inline void perf_frame_begin(unsigned n) { (void)n; }
static inline void perf_frame_post_done(void) {}
static inline void perf_frame_present(unsigned r, unsigned s, int y,
                                      unsigned f, unsigned c)
  { (void)r; (void)s; (void)y; (void)f; (void)c; }
static inline void perf_frame_end(void) {}
static inline void perf_audio_write(unsigned e) { (void)e; }
static inline void perf_audio_full_wait(void) {}
static inline void perf_audio_silent_packet(void) {}
static inline void perf_sid_wait(unsigned e) { (void)e; }
static inline void perf_stats_set_core(const char *c) { (void)c; }
static inline void perf_stats_reset(void) {}
static inline void perf_stats_service(void) {}

#endif /* BMC64_PERF_STATS */

#ifdef __cplusplus
}
#endif

#endif /* BMC64_PERF_STATS_H */
