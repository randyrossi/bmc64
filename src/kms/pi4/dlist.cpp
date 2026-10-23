//
// dlist.cpp
//
// BMA800XL - A bare metal Atari 800XL emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Modified for BMC64 from BMA800XL's src/kms/pi4/dlist.cpp: planes can be
// blended by per-pixel alpha.
//
// See dlist.h.
//

#include "dlist.h"

#include "regs.h"

namespace kms {

namespace {

// How one direction is scaled.
enum class Scaling { None, Ppf, Unsupported };

Scaling ScalingFor(unsigned src, unsigned dst) {
  if (src == dst) {
    return Scaling::None;
  }
  // The PPF enlarges, or shrinks to no less than 2/3.
  if (3 * dst >= 2 * src) {
    return Scaling::Ppf;
  }
  return Scaling::Unsupported;
}

// The PPF word for SRC source pixels shown as DST: the step in 16.16 fixed
// point, and an initial phase of -1/2 pixel plus half a screen pixel's
// worth of source, plus half the step's rounding error.
uint32_t PpfWord(unsigned src, unsigned dst, bool nearest) {
  uint32_t step = (uint32_t)(((uint64_t)src << 16) / dst);
  int32_t error = (int32_t)(((int64_t)src << 16) - (int64_t)dst * step);
  int32_t phase = -(1 << (hvs::PhaseBits - 1)) +
                  (int32_t)(step >> (17 - hvs::PhaseBits)) +
                  ((error >> (16 - hvs::PhaseBits)) >> 1);
  if (phase >= (1 << hvs::PhaseBits)) {
    phase = (1 << hvs::PhaseBits) - 1;
  }
  return (nearest ? hvs::PpfNoInterpolation : 0) | hvs::PpfAgc |
         Field(step, hvs::PpfScaleHigh, hvs::PpfScaleLow) |
         Field((uint32_t)phase, hvs::PpfPhaseHigh, hvs::PpfPhaseLow);
}

} // namespace

unsigned LbmWords(const Plane &plane) {
  if (ScalingFor(plane.src_height, plane.height) != Scaling::Ppf) {
    return 0;
  }
  // RGB, vertical PPF: 16 per source pixel, rounded up to 128, in words
  // of 4, then aligned to 64 words.
  unsigned bytes = (plane.src_width * 16 + 127) / 128 * 128;
  unsigned words = bytes / 4;
  return (words + 63) / 64 * 64;
}

unsigned BuildList(const Plane &plane, unsigned lbm, unsigned kernel,
                   uint32_t *out) {
  if (plane.src_width == 0 || plane.src_height == 0 || plane.width == 0 ||
      plane.height == 0 || plane.src_width > 8191 || plane.src_height > 8191 ||
      plane.width > 8191 || plane.height > 8191 || plane.x > 8191 ||
      plane.y > 4095) {
    return 0;
  }
  Scaling sx = ScalingFor(plane.src_width, plane.width);
  Scaling sy = ScalingFor(plane.src_height, plane.height);
  if (sx == Scaling::Unsupported || sy == Scaling::Unsupported) {
    return 0;
  }
  bool unity = sx == Scaling::None && sy == Scaling::None;

  uint32_t scl = 0;
  if (sx == Scaling::Ppf && sy == Scaling::Ppf) {
    scl = hvs::SclPpfBoth;
  } else if (sx == Scaling::Ppf) {
    scl = hvs::SclPpfHorizontal;
  } else if (sy == Scaling::Ppf) {
    scl = hvs::SclPpfVertical;
  }

  unsigned n = 0;
  out[n++] = hvs::CtlValid | (hvs::OrderArgb << hvs::CtlOrderLow) |
             hvs::FormatRgba8888 | (unity ? hvs::CtlUnity : 0) |
             (scl << hvs::CtlScl0Low) | (scl << hvs::CtlScl1Low) |
             hvs::CtlAlphaExpand | hvs::CtlRgbExpand;  // size added below
  out[n++] = Field(plane.x, hvs::Pos0XHigh, hvs::Pos0XLow) |
             Field(plane.y, hvs::Pos0YHigh, hvs::Pos0YLow);
  out[n++] = ((plane.alpha ? hvs::AlphaModePipeline : hvs::AlphaModeFixed)
              << hvs::Ctl2AlphaModeLow) |
             Field(0xFFF, hvs::Ctl2AlphaHigh, hvs::Ctl2AlphaLow);
  if (!unity) {
    out[n++] = Field(plane.width, hvs::SizeWHigh, hvs::SizeWLow) |
               Field(plane.height, hvs::SizeHHigh, hvs::SizeHLow);
  }
  out[n++] = Field(plane.src_width, hvs::SizeWHigh, hvs::SizeWLow) |
             Field(plane.src_height, hvs::SizeHHigh, hvs::SizeHLow);
  out[n++] = hvs::Context;
  out[n++] = plane.address;
  out[n++] = hvs::Context;
  out[n++] = Field(plane.pitch, hvs::PitchHigh, hvs::PitchLow);
  if (!unity) {
    if (sy == Scaling::Ppf) {
      out[n++] = lbm;
    }
    if (sx == Scaling::Ppf) {
      out[n++] = PpfWord(plane.src_width, plane.width, plane.nearest);
    }
    if (sy == Scaling::Ppf) {
      out[n++] = PpfWord(plane.src_height, plane.height, plane.nearest);
      out[n++] = hvs::Context;
    }
    // Kernels for the horizontal and vertical PPF of both scaler
    // channels.
    uint32_t k = Field(kernel, hvs::KernelOffsetHigh, hvs::KernelOffsetLow);
    for (int i = 0; i < 4; i++) {
      out[n++] = k;
    }
  }
  out[0] |= Field(n, hvs::CtlSizeHigh, hvs::CtlSizeLow);
  out[n++] = hvs::CtlEnd;
  return n;
}

} // namespace kms
