//
// crt.cpp
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
// Modified for BMC64 from BMA800XL's src/v3d/crt.cpp: without BMA800XL's self-
// test against its ARM scaler.
//
// See crt.h.
//

#include "crt.h"

#include "shaders.h"


#include <circle/logger.h>
#include <circle/synchronize.h>
#include <circle/timer.h>

namespace v3d {

static const char FromGpu[] = "v3d";

namespace {

// crt.frag's push constants.
struct CrtParameters {
  float texture_width, texture_height;
  float filter_width;
};

} // namespace

Crt::Crt(void)
    : m_pGpu(0), m_pRenderer(0), m_bReady(false), m_bCrt(false), m_bSmooth(false),
      m_bCurvature(false), m_nPitch(0), m_nBytesPerPixel(1), m_nViewX(0), m_nViewY(0),
      m_nViewW(0), m_nViewH(0), m_nDestW(0), m_nDestH(0), m_nFrames(0), m_nFillUs(0),
      m_nBinUs(0), m_nRenderUs(0) {
  for (unsigned i = 0; i < 256; i++) {
    m_Palette[i] = 0xFF000000;
  }
}

bool Crt::Initialize(Gpu *gpu, Renderer *renderer, unsigned max_view_width,
                     unsigned max_view_height) {
  m_pGpu = gpu;
  m_pRenderer = renderer;
  const Program *programs[] = {&CopyProgram, &CrtProgram, &CrtSmoothProgram, &CrtCurveProgram,
                                 &CrtCurveSmoothProgram};
  for (const Program *program : programs) {
    if (!renderer->Load(*program)) {
      return false;
    }
  }
  return m_Texture.Create(gpu, max_view_width, max_view_height);
}

void Crt::SetMode(bool crt, bool smooth, bool curvature) {
  m_bCrt = crt;
  m_bSmooth = smooth;
  m_bCurvature = crt && curvature;
  m_bReady = m_nViewW > 0 && m_Texture.Set(m_nViewW, m_nViewH, m_bSmooth);
}

bool Crt::Configure(unsigned src_pitch, int vx, int vy, int vw, int vh, unsigned dw,
                    unsigned dh, unsigned bytes_per_pixel) {
  m_nPitch = src_pitch;
  m_nBytesPerPixel = bytes_per_pixel == 2 ? 2 : 1;
  m_nViewX = vx;
  m_nViewY = vy;
  m_nViewW = vw;
  m_nViewH = vh;
  m_nDestW = dw;
  m_nDestH = dh;
  m_bReady = vw > 0 && vh > 0 && m_Texture.Set(vw, vh, m_bSmooth);
  m_nFrames = 0;
  m_nFillUs = m_nBinUs = m_nRenderUs = 0;
  return m_bReady;
}

void Crt::SetPalette(int index, uint32_t rgb) {
  m_Palette[index & 255] = 0xFF000000 | (rgb & 0xFFFFFF);
}

// An RGB565 pixel as 0xFFRRGGBB, each component's top bits repeated in
// its low ones.
static uint32_t FromRgb565(uint16_t c) {
  unsigned r = (c >> 11) & 0x1F, g = (c >> 5) & 0x3F, b = c & 0x1F;
  return 0xFF000000 | ((r << 3) | (r >> 2)) << 16 | ((g << 2) | (g >> 4)) << 8 |
         ((b << 3) | (b >> 2));
}

// The view, in colour.
void Crt::Fill(const uint8_t *src) {
  for (int y = 0; y < m_nViewH; y++) {
    const uint8_t *line = src + (m_nViewY + y) * m_nPitch;
    if (m_nBytesPerPixel == 2) {
      const uint16_t *pixels = (const uint16_t *)line + m_nViewX;
      for (int x = 0; x < m_nViewW; x++) {
        m_Texture.Pixel(x, y) = FromRgb565(pixels[x]);
      }
    } else {
      for (int x = 0; x < m_nViewW; x++) {
        m_Texture.Pixel(x, y) = m_Palette[line[m_nViewX + x]];
      }
    }
  }
  m_Texture.Flush();
}

bool Crt::Draw(const uint8_t *src, const Target &target, int x, int y) {
  if (!m_bReady) {
    return false;
  }
  u64 start = CTimer::GetClockTicks64();
  Fill(src);
  m_nFillUs += (CTimer::GetClockTicks64() - start) * 1000000 / CLOCKHZ;
  bool ok;
  if (!m_bCrt) {
    ok = m_pRenderer->Draw(CopyProgram, m_Texture, target, x, y, m_nDestW, m_nDestH,
                           0xFF000000);
  } else {
    const Program *program;
    if (m_bCurvature) {
      program = m_bSmooth ? &CrtCurveSmoothProgram : &CrtCurveProgram;
    } else {
      program = m_bSmooth ? &CrtSmoothProgram : &CrtProgram;
    }
    CrtParameters parameters = {(float)m_nViewW, (float)m_nViewH,
                                ((float)m_nViewH / (float)m_nDestH) / 3.0f};
    ok = m_pRenderer->Draw(*program, m_Texture, target, x, y, m_nDestW, m_nDestH, 0xFF000000,
                           &parameters, sizeof parameters);
  }
  m_nBinUs += m_pGpu->BinMicroseconds();
  m_nRenderUs += m_pGpu->RenderMicroseconds();
  if (++m_nFrames == 250) {
    CLogger::Get()->Write(FromGpu, LogNotice,
                          "Frame times (average of %u, %ux%u): texture %u us, binning %u us, "
                          "rendering %u us", m_nFrames, m_nDestW, m_nDestH,
                          (unsigned)(m_nFillUs / m_nFrames), (unsigned)(m_nBinUs / m_nFrames),
                          (unsigned)(m_nRenderUs / m_nFrames));
  }
  return ok;
}

} // namespace v3d
