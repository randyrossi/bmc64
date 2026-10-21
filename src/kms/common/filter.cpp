//
// filter.cpp
//
// See filter.h.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#include "filter.h"

#include <math.h>

namespace kms {

namespace {

// Mitchell and Netravali's cubic, k(x), for B = C = 1/3.
double MitchellNetravali(double x) {
  const double B = 1.0 / 3.0, C = 1.0 / 3.0;
  x = fabs(x);
  if (x < 1.0) {
    return ((12 - 9 * B - 6 * C) * x * x * x + (-18 + 12 * B + 6 * C) * x * x +
            (6 - 2 * B)) / 6.0;
  }
  if (x < 2.0) {
    return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x +
            (-12 * B - 48 * C) * x + (8 * B + 24 * C)) / 6.0;
  }
  return 0.0;
}

int RoundToInt(double v) { return (int)floor(v + 0.5); }

} // namespace

void KernelCoefficients(Filter filter, int coeff[32]) {
  // The first half; the second mirrors it, coeff[31 - i] = coeff[i].
  int half[16];
  for (int i = 0; i < 16; i++) {
    double x = (i - 15.5) / 8.0;
    if (filter == Filter::Smooth) {
      half[i] = RoundToInt(MitchellNetravali(x) * 256.0);
    } else {
      half[i] = fabs(x) < 0.5 ? 256 : 0;
    }
  }

  // Phase p (0..3) uses half[p], half[p + 8], half[15 - p] and
  // half[7 - p] (the last two through the mirror); phases 4..7 are their
  // mirror images. Make each set add up to 256 by adjusting its largest
  // tap, then keep every tap within 9 signed bits: a 256 becomes 255 and
  // the tap next to it outwards (in the same set) gets the 1.
  for (int p = 0; p < 4; p++) {
    int taps[4] = {p, p + 8, 15 - p, 7 - p};
    int sum = 0, largest = taps[0];
    for (int t : taps) {
      sum += half[t];
      if (half[t] > half[largest]) {
        largest = t;
      }
    }
    half[largest] += 256 - sum;
    if (half[largest] > 255) {
      int spill = half[largest] - 255;
      half[largest] = 255;
      // Outwards from the centre: lower index in the first half.
      int outer = largest >= 8 ? largest - 8 : -1;
      if (outer < 0) {
        // Can't happen for these kernels; keep the sum short by SPILL
        // (the AGC bit makes the hardware correct it).
        continue;
      }
      half[outer] += spill;
    }
  }

  for (int i = 0; i < 16; i++) {
    coeff[i] = half[i];
    coeff[31 - i] = half[i];
  }
}

void KernelWords(Filter filter, uint32_t words[11]) {
  int c[32];
  KernelCoefficients(filter, c);
  auto pack = [](int a, int b, int d) {
    return ((uint32_t)a & 0x1FF) | (((uint32_t)b & 0x1FF) << 9) |
           (((uint32_t)d & 0x1FF) << 18);
  };
  for (int w = 0; w < 5; w++) {
    words[w] = pack(c[3 * w], c[3 * w + 1], c[3 * w + 2]);
  }
  words[5] = pack(c[15], c[15], 0);
  for (int w = 6; w < 11; w++) {
    words[w] = words[10 - w];
  }
}

} // namespace kms
