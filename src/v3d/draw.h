//
// draw.h
//
// Drawing with the 3D GPU (Pi 4 and Pi 5): a textured rectangle drawn through a
// compiled shader program (shader.h) into an RGBA picture in memory. A
// binning job sorts the rectangle into 64x64 tiles; a rendering job runs
// the fragment shader on each tile and stores it.
//
// Sources, all Mesa (Copyright (C) 2014-2021 Broadcom and Raspberry Pi Ltd,
// MIT licence):
//  - src/gallium/drivers/v3d/v3dx_draw.c, v3dx_emit.c, v3dx_rcl.c,
//    v3d_uniforms.c, v3d_job.c: the binning and rendering control lists,
//    the shader state record, uniforms, tile memory sizes.
//  - src/gallium/drivers/v3d/v3dx_state.c: texture and sampler state.
//  - src/gallium/drivers/v3d/v3d_resource.c, src/broadcom/common/
//    v3d_tiling.c: the UIF texture layout.
//  - src/broadcom/cle/v3d_packet.xml: the record layouts.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_V3D_DRAW_H_
#define BMC64_V3D_DRAW_H_

#include "gpu.h"
#include "shader.h"

#include <stdint.h>

namespace v3d {

// A picture the GPU samples: RGBA8 (red in the low byte) in the GPU's UIF
// layout of 8x8 pixel blocks in columns 32 pixels wide.
class Texture {
public:
  Texture(void);

  // Room for pictures up to MAX_WIDTH x MAX_HEIGHT.
  bool Create(Gpu *gpu, unsigned max_width, unsigned max_height);

  // The picture's size, and bilinear (SMOOTH) or nearest sampling.
  bool Set(unsigned width, unsigned height, bool smooth);

  unsigned Width(void) const { return m_nWidth; }
  unsigned Height(void) const { return m_nHeight; }

  // The pixel at (X, Y).
  uint32_t &Pixel(unsigned x, unsigned y) {
    return m_pPixels[Offset(x, y) / 4];
  }

  // Makes the pixels written visible to the GPU.
  void Flush(void);

  uint32_t State(void) const { return Gpu::Address(m_pState); }
  uint32_t Sampler(void) const { return Gpu::Address(m_pSampler); }

private:
  // Byte offset of (X, Y): 8x8 blocks of 256 bytes, each four 4x4
  // micro-tiles, the blocks running down columns four blocks wide.
  unsigned Offset(unsigned x, unsigned y) const {
    return (x >> 5) * m_nColumnBytes + (y >> 3) * 1024 + ((x >> 3) & 3) * 256 +
           ((y & 4) << 5) + ((x & 4) << 4) + (y & 3) * 16 + (x & 3) * 4;
  }

  unsigned m_nWidth, m_nHeight;
  unsigned m_nCapacity;     // bytes
  unsigned m_nColumnBytes;
  unsigned m_nBytes;
  uint32_t *m_pPixels;
  uint8_t *m_pState;
  uint8_t *m_pSampler;
};

// An RGBA8 picture in memory, row by row, that the GPU draws into.
struct Target {
  void *pixels;
  unsigned width, height;
  unsigned stride;   // bytes a row
};

class Renderer {
public:
  Renderer(void);

  // Sets up for targets up to MAX_WIDTH x MAX_HEIGHT.
  bool Initialize(Gpu *gpu, unsigned max_width, unsigned max_height);

  // Copies PROGRAM's code to GPU memory; once per program.
  bool Load(const Program &program);

  // Draws TEXTURE through PROGRAM into the rectangle (X, Y, W, H) of
  // TARGET, which is first cleared to CLEAR (RGBA, red low). PARAMETERS
  // are the shader's push constants. Waits for the GPU.
  bool Draw(const Program &program, const Texture &texture, const Target &target,
            int x, int y, unsigned w, unsigned h, uint32_t clear,
            const void *parameters = 0, unsigned parameter_bytes = 0);

  // Copies a test pattern through the GPU and checks it. Logs the result.
  bool CopyTest(void);

private:
  struct Loaded {
    const Program *program;
    uint32_t fs, vs, cs;   // code addresses
  };

  const Loaded *Find(const Program &program) const;
  uint32_t WriteUniforms(const Shader &shader, const Texture &texture, const Target &target,
                         const void *parameters, unsigned parameter_bytes);
  void *Indirect(unsigned bytes, unsigned align);

  static const unsigned MaxPrograms = 8;

  Gpu *m_pGpu;
  unsigned m_nMaxWidth, m_nMaxHeight;
  uint8_t *m_pBcl, *m_pRcl, *m_pTileList;
  uint8_t *m_pIndirect;
  unsigned m_nIndirectUsed;
  uint8_t *m_pTileAlloc, *m_pTileState;
  unsigned m_nTileAllocBytes;
  float *m_pDefaults;
  Loaded m_Loaded[MaxPrograms];
  unsigned m_nLoaded;
};

} // namespace v3d

#endif
