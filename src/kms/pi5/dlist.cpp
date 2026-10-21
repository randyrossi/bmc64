//
// dlist.cpp
//
// See dlist.h.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
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
// point with its lowest bit clear, and an initial phase of -1/2 pixel plus
// half a screen pixel's worth of source, plus half the step's rounding
// error.
uint32_t PpfWord(unsigned src, unsigned dst, bool nearest) {
  uint32_t step = (uint32_t)(((uint64_t)src << 16) / dst) & ~1u;
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

unsigned LbmBlocks(unsigned src_width, unsigned width, bool alpha) {
  // The narrower of source and screen, to 4 pixels; 4 lines for the
  // vertical PPF; 3 components for RGB, 4 with alpha; in 128s.
  unsigned w = src_width < width ? src_width : width;
  w = (w + 3) & ~3u;
  return (w * 4 * (alpha ? 4 : 3) + 127) / 128;
}

} // namespace

unsigned UpmUnitsFor(unsigned pitch) {
  unsigned line = (pitch + 62) / 32 * 32;
  return (line * hvs::UpmLines + hvs::UpmUnitBytes - 1) / hvs::UpmUnitBytes;
}

unsigned LbmBlocksFor(const Plane &plane) {
  if (ScalingFor(plane.src_height, plane.height) != Scaling::Ppf) {
    return 0;
  }
  return LbmBlocks(plane.src_width, plane.width, plane.alpha);
}

unsigned BuildList(const Plane &plane, const PlaneMemory &memory, bool stepping_d,
                   uint32_t *out) {
  if (plane.src_width == 0 || plane.src_height == 0 || plane.width == 0 ||
      plane.height == 0 || plane.src_width > 8192 || plane.src_height > 8192 ||
      plane.width > 8192 || plane.height > 8192 || plane.x > 8191 ||
      plane.y > 8191 || plane.pitch > 0x1FFFF ||
      memory.upm_handle >= hvs::UpmHandles || memory.upm_base > 0x1FFF ||
      (plane.address >> 40) != 0) {
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
             (stepping_d && !plane.alpha ? hvs::AlphaMaskFixed << hvs::CtlAlphaMaskLow : 0);
  out[n++] = Field(plane.x, hvs::Pos0XHigh, hvs::Pos0XLow) |
             Field(plane.y, hvs::Pos0YHigh, hvs::Pos0YLow);
  out[n++] = plane.alpha ? hvs::Ctl2PixelAlpha
                         : stepping_d ? hvs::Ctl2FixedD : hvs::Ctl2FixedC;
  if (!unity) {
    out[n++] = Field(plane.width - 1, hvs::SizeWHigh, hvs::SizeWLow) |
               Field(plane.height - 1, hvs::SizeHHigh, hvs::SizeHLow);
  }
  out[n++] = Field(plane.src_width - 1, hvs::SizeWHigh, hvs::SizeWLow) |
             Field(plane.src_height - 1, hvs::SizeHHigh, hvs::SizeHLow);
  out[n++] = hvs::Context;
  out[n++] = Field(memory.upm_base, hvs::Ptr0UpmBaseHigh, hvs::Ptr0UpmBaseLow) |
             Field(memory.upm_handle, hvs::Ptr0UpmHandleHigh, hvs::Ptr0UpmHandleLow) |
             Field(0, hvs::Ptr0UpmLinesHigh, hvs::Ptr0UpmLinesLow) |
             Field((uint32_t)(plane.address >> 32), hvs::Ptr0AddressHigh,
                   hvs::Ptr0AddressLow);
  out[n++] = (uint32_t)plane.address;
  out[n++] = Field(plane.pitch, hvs::PitchHigh, hvs::PitchLow);
  if (!unity) {
    if (sy == Scaling::Ppf) {
      out[n++] = memory.lbm;
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
    uint32_t k = Field(memory.kernel, hvs::KernelOffsetHigh, hvs::KernelOffsetLow);
    for (int i = 0; i < 4; i++) {
      out[n++] = k;
    }
  }
  // Each entry ends with an end word, counted in its size; another ends
  // the list.
  out[n++] = hvs::CtlEnd;
  out[0] |= Field(n, hvs::CtlSizeHigh, hvs::CtlSizeLow);
  out[n++] = hvs::CtlEnd;
  return n;
}

// The firmware's entries are read as RGB planes are laid out: pointer 0
// after the context word (one word later when scaled), pitch two words
// on, then, with vertical scaling, the line buffer start.
bool ReadEntry(const uint32_t *words, unsigned available, EntryUse *use) {
  if (available == 0 || (words[0] & hvs::CtlEnd)) {
    return false;
  }
  unsigned size = GetField(words[0], hvs::CtlSizeHigh, hvs::CtlSizeLow);
  if (size < 9 || size > available) {
    return false;
  }
  bool unity = (words[0] & hvs::CtlUnity) != 0;
  unsigned ptr0 = unity ? 5 : 6;
  use->words = size;
  use->has_upm = true;
  use->upm_handle = GetField(words[ptr0], hvs::Ptr0UpmHandleHigh, hvs::Ptr0UpmHandleLow);
  use->upm_base = GetField(words[ptr0], hvs::Ptr0UpmBaseHigh, hvs::Ptr0UpmBaseLow);
  use->upm_units = UpmUnitsFor(GetField(words[ptr0 + 2], hvs::PitchHigh, hvs::PitchLow));
  use->has_lbm = false;
  use->lbm = use->lbm_blocks = 0;
  unsigned scl = GetField(words[0], hvs::CtlScl0Low + 2, hvs::CtlScl0Low);
  if (!unity && (scl == hvs::SclPpfBoth || scl == hvs::SclPpfVertical) && ptr0 + 3 < size) {
    unsigned src_w = GetField(words[4], hvs::SizeWHigh, hvs::SizeWLow) + 1;
    unsigned dst_w = GetField(words[3], hvs::SizeWHigh, hvs::SizeWLow) + 1;
    use->has_lbm = true;
    use->lbm = words[ptr0 + 3];
    // 4 components, the most any RGB plane takes.
    use->lbm_blocks = LbmBlocks(src_w, dst_w, true);
  }
  return true;
}

} // namespace kms
