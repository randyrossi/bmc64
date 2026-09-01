//
// io_stats_bench.cpp
//
// Opt-in storage benchmark for the SD/FatFs performance investigation.
// Compiled to an empty object unless BMC64_IO_STATS is defined
// (./make_all.sh <board> --io-stats). See docs/BUILDING.md.
//
// This is the implementation of the circle_io_benchmark() /
// circle_io_stats_dump() entry points declared in
// third_party/common/circle.h. It is self-contained: the SD device is
// resolved through Circle's device name service ("emmc1"), the same way
// diskio.cpp does, so it needs nothing from CKernel.
//
// Licensed under the Apache License, Version 2.0.
//

extern "C" {
#include "../third_party/common/circle.h"
}
#include "../third_party/common/io_stats.h"

#ifdef BMC64_IO_STATS

#include <stdint.h>
#include <stdio.h>

#include <circle/device.h>
#include <circle/devicenameservice.h>
#include <circle/timer.h>
#include <ff.h>

extern "C" {

void circle_io_stats_dump(void) { io_stats_dump("on demand"); }

// Raw block-device read benchmark plus a FatFs read benchmark. Bypasses the
// new_io.cpp slurp layer so we can separate physical-device time from FatFs
// and glue overhead. Prints a table to the log/serial. Runs on core 0 while
// the emulator continues on core 1, so results include normal background
// contention (audio/video), which is what we actually care about for BMC64.
//
// NOTE: reads the raw device and the FatFs volume directly. Do not run it
// while a disk/tape image load is in progress - concurrent access from the
// emulator core is not serialized against this path.
void circle_io_benchmark(void) {
  static const unsigned kSizes[] = {512, 4096, 16384, 65536, 131072};
  static const unsigned kIters = 64;
  // 4 KiB aligned so diskio.cpp / the SD driver do not bounce-buffer.
  static uint8_t buf[131072] __attribute__((aligned(4096)));

  CDevice *dev = CDeviceNameService::Get()->GetDevice("emmc1", TRUE);
  if (dev == 0) {
    printf("io_benchmark: emmc1 not found\n");
    return;
  }

  // NOTE: this newlib's printf has no %llu, so only 32-bit values are printed.
  unsigned long long devBytes = dev->GetSize();
  unsigned long long devSectors =
      (devBytes == (unsigned long long)-1) ? 0 : (devBytes / 512);
  if (devSectors <= (sizeof(buf) / 512)) {
    printf("io_benchmark: EMMC size unavailable\n");
    return;
  }

  printf("=== I/O benchmark (background emulation running) ===\n");
  printf("device: emmc1 size=%u MiB, %u sectors\n",
         (unsigned)(devBytes >> 20), (unsigned)devSectors);
  printf("%-9s %8s %8s %8s %8s %9s\n",
         "test", "size", "avg-us", "min-us", "max-us", "KB/s");

  for (int pass = 0; pass < 2; pass++) {
    int scattered = (pass == 1);
    unsigned lcg = 0x12345678u; // fixed seed -> repeatable scattered LBAs
    for (unsigned si = 0; si < sizeof(kSizes) / sizeof(kSizes[0]); si++) {
      unsigned size = kSizes[si];
      unsigned nsect = size / 512;
      unsigned totUs = 0, minUs = ~0u, maxUs = 0, iters = 0;
      int failed = 0;

      for (unsigned it = 0; it < kIters; it++) {
        unsigned long long lba;
        if (scattered) {
          lcg = lcg * 1103515245u + 12345u;
          lba = (unsigned long long)lcg % (devSectors - nsect - 1);
        } else {
          lba = (unsigned long long)it * nsect + 2048;
        }
        dev->Seek(lba * 512);
        unsigned t0 = CTimer::GetClockTicks();
        int r = dev->Read(buf, size);
        unsigned dt = CTimer::GetClockTicks() - t0;
        if (r != (int)size) {
          printf("  read failed (size=%u r=%d)\n", size, r);
          failed = 1;
          break;
        }
        totUs += dt;
        iters++;
        if (dt < minUs) minUs = dt;
        if (dt > maxUs) maxUs = dt;
      }

      if (failed || iters == 0) {
        continue;
      }
      unsigned avgUs = totUs / iters;
      // KB/s = size / avgUs * 1000  (size <= 128 KiB keeps this in 32 bits)
      unsigned kbps = avgUs ? (unsigned)((size * 1000ull) / avgUs) : 0;
      printf("%-9s %8u %8u %8u %8u %9u\n",
             scattered ? "raw-scat" : "raw-seq", size,
             avgUs, minUs, maxUs, kbps);
    }
  }

  // FatFs read of the largest file we can find, in a few chunk sizes.
  const char *machine_dir =
#if defined(RASPI_C64)
      "/C64";
#elif defined(RASPI_C128)
      "/C128";
#elif defined(RASPI_VIC20)
      "/VIC20";
#elif defined(RASPI_PLUS4) || defined(RASPI_PLUS4EMU)
      "/PLUS4";
#elif defined(RASPI_PET)
      "/PET";
#else
      "";
#endif
  char rom_path[64];
  char stat_path[64];
  snprintf(rom_path, sizeof(rom_path), "%s/kernal", machine_dir);
  snprintf(stat_path, sizeof(stat_path), "%s/bootstat.txt", machine_dir);
  // Prefer a multi-MB file (the kernel image at the volume root) so the
  // throughput number is meaningful; fall back to a ROM, then bootstat.
  const char *candidates[] = {"/kernel8-32.img", rom_path, stat_path};

  static const unsigned kFatChunks[] = {1024, 32768, 131072};
  for (unsigned ci = 0; ci < sizeof(kFatChunks) / sizeof(kFatChunks[0]); ci++) {
    unsigned chunk = kFatChunks[ci];

    FIL fil;
    const char *use = 0;
    for (unsigned i = 0; i < sizeof(candidates) / sizeof(candidates[0]) && !use;
         i++) {
      if (f_open(&fil, candidates[i], FA_READ) == FR_OK) {
        use = candidates[i];
      }
    }
    if (!use) {
      printf("fatfs: no test file under '%s'\n", machine_dir);
      break;
    }

    unsigned totKiB = 0;
    unsigned t0 = CTimer::GetClockTicks();
    for (;;) {
      UINT nread = 0;
      if (f_read(&fil, buf, chunk, &nread) != FR_OK || nread == 0) {
        break;
      }
      totKiB += nread / 1024;
    }
    unsigned totUs = CTimer::GetClockTicks() - t0;
    f_close(&fil);

    unsigned kbps = totUs ? (unsigned)((totKiB * 1000000ull) / totUs) : 0;
    printf("%-9s %8u %8s %8s %8u %9u  (%s %uKiB)\n",
           "fatfs", chunk, "-", "-", totUs, kbps, use, totKiB);
  }

  printf("==================================================\n");
}

} // extern "C"

#endif /* BMC64_IO_STATS */
