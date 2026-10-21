//
// draw.cpp
//
// See draw.h.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#include "draw.h"

#include "cl.h"
#include "shaders.h"

#include <circle/logger.h>
#include <circle/synchronize.h>
#include <string.h>

namespace v3d {

static const char FromGpu[] = "v3d";

namespace {

const unsigned TileSize = 64;           // 1 render target, 32 bpp
const unsigned MaxSupertiles = 256;
const unsigned BclBytes = 4096;
const unsigned RclBytes = 4096;
const unsigned TileListBytes = 256;
const unsigned IndirectBytes = 16384;
const unsigned TimeoutMs = 100;

unsigned AlignUp(unsigned value, unsigned align) {
  return (value + align - 1) & ~(align - 1);
}

uint32_t FloatBits(float f) {
  uint32_t u;
  memcpy(&u, &f, 4);
  return u;
}

} // namespace

Texture::Texture(void)
    : m_nWidth(0), m_nHeight(0), m_nCapacity(0), m_nColumnBytes(0), m_nBytes(0),
      m_pPixels(0), m_pState(0), m_pSampler(0) {}

bool Texture::Create(Gpu *gpu, unsigned max_width, unsigned max_height) {
  m_nCapacity = AlignUp(max_width, 32) * AlignUp(max_height, 8) * 4;
  m_pPixels = (uint32_t *)gpu->Allocate(m_nCapacity, 4096);
  m_pState = (uint8_t *)gpu->Allocate(32, 32);
  m_pSampler = (uint8_t *)gpu->Allocate(32, 32);
  return m_pPixels != 0 && m_pState != 0 && m_pSampler != 0;
}

// Level 0 only, strictly UIF without XOR, no extra padding: the height is
// padded to the 8-pixel blocks and the width to the 32-pixel columns.
bool Texture::Set(unsigned width, unsigned height, bool smooth) {
  unsigned padded_height = AlignUp(height, 8);
  unsigned bytes = AlignUp(width, 32) * padded_height * 4;
  if (m_pPixels == 0 || bytes > m_nCapacity) {
    return false;
  }
  m_nWidth = width;
  m_nHeight = height;
  m_nColumnBytes = padded_height * 32 * 4;
  m_nBytes = bytes;
  PackTextureState(m_pState, Gpu::Address(m_pPixels), width, height);
  PackSamplerState(m_pSampler, smooth);
  CleanAndInvalidateDataCacheRange((uintptr_t)m_pState, 32);
  CleanAndInvalidateDataCacheRange((uintptr_t)m_pSampler, 32);
  return true;
}

void Texture::Flush(void) {
  CleanAndInvalidateDataCacheRange((uintptr_t)m_pPixels, m_nBytes);
}

Renderer::Renderer(void)
    : m_pGpu(0), m_nMaxWidth(0), m_nMaxHeight(0), m_pBcl(0), m_pRcl(0), m_pTileList(0),
      m_pIndirect(0), m_nIndirectUsed(0), m_pTileAlloc(0), m_pTileState(0),
      m_nTileAllocBytes(0), m_pDefaults(0), m_nLoaded(0) {}

bool Renderer::Initialize(Gpu *gpu, unsigned max_width, unsigned max_height) {
  m_pGpu = gpu;
  m_nMaxWidth = max_width;
  m_nMaxHeight = max_height;
  unsigned tiles = ((max_width + TileSize - 1) / TileSize) *
                   ((max_height + TileSize - 1) / TileSize);

  // As Mesa: 64 bytes a tile to start, the binner's first two 4 KB
  // blocks, and plenty more so that it never runs out.
  m_nTileAllocBytes = AlignUp(tiles * 64, 4096) + 8192 + 512 * 1024;

  m_pBcl = (uint8_t *)gpu->Allocate(BclBytes);
  m_pRcl = (uint8_t *)gpu->Allocate(RclBytes);
  m_pTileList = (uint8_t *)gpu->Allocate(TileListBytes);
  m_pIndirect = (uint8_t *)gpu->Allocate(IndirectBytes, 4096);
  m_pTileAlloc = (uint8_t *)gpu->Allocate(m_nTileAllocBytes, 4096);
  m_pTileState = (uint8_t *)gpu->Allocate(tiles * 256, 4096);
  m_pDefaults = (float *)gpu->Allocate(64);
  if (m_pBcl == 0 || m_pRcl == 0 || m_pTileList == 0 || m_pIndirect == 0 ||
      m_pTileAlloc == 0 || m_pTileState == 0 || m_pDefaults == 0) {
    CLogger::Get()->Write(FromGpu, LogError, "Not enough memory to draw");
    return false;
  }

  // Default attribute values: (0, 0, 0, 1).
  m_pDefaults[3] = 1.0f;
  CleanAndInvalidateDataCacheRange((uintptr_t)m_pDefaults, 64);
  return true;
}

bool Renderer::Load(const Program &program) {
  if (Find(program) != 0) {
    return true;
  }
  if (m_pGpu == 0 || m_nLoaded == MaxPrograms) {
    return false;
  }
  Loaded &loaded = m_Loaded[m_nLoaded];
  const Shader *shaders[] = {&program.fs, &program.vs, &program.cs};
  uint32_t *addresses[] = {&loaded.fs, &loaded.vs, &loaded.cs};
  for (unsigned i = 0; i < 3; i++) {
    // Room past the end for the QPU's instruction prefetch.
    unsigned bytes = shaders[i]->instructions * 8;
    void *code = m_pGpu->Allocate(bytes + 64, 64);
    if (code == 0) {
      return false;
    }
    memcpy(code, shaders[i]->code, bytes);
    CleanAndInvalidateDataCacheRange((uintptr_t)code, bytes + 64);
    *addresses[i] = Gpu::Address(code);
  }
  loaded.program = &program;
  m_nLoaded++;
  return true;
}

const Renderer::Loaded *Renderer::Find(const Program &program) const {
  for (unsigned i = 0; i < m_nLoaded; i++) {
    if (m_Loaded[i].program == &program) {
      return &m_Loaded[i];
    }
  }
  return 0;
}

void *Renderer::Indirect(unsigned bytes, unsigned align) {
  unsigned start = AlignUp(m_nIndirectUsed, align);
  if (start + bytes > IndirectBytes) {
    return 0;
  }
  m_nIndirectUsed = start + bytes;
  return m_pIndirect + start;
}

// As Mesa's v3d_write_uniforms; one word more, which the hardware reads
// ahead.
uint32_t Renderer::WriteUniforms(const Shader &shader, const Texture &texture,
                                 const Target &target, const void *parameters,
                                 unsigned parameter_bytes) {
  uint32_t *u = (uint32_t *)Indirect((shader.uniform_count + 1) * 4, 4);
  if (u == 0) {
    return 0;
  }
  const uint8_t *params = (const uint8_t *)parameters;
  for (unsigned i = 0; i < shader.uniform_count; i++) {
    uint32_t data = shader.uniforms[i].data;
    uint32_t value = 0;
    switch (shader.uniforms[i].kind) {
    case UniformConstant:
      value = data;
      break;
    case UniformParameter:
      if (data * 4 + 4 <= parameter_bytes) {
        memcpy(&value, params + data * 4, 4);
      }
      break;
    case UniformViewportXScale:
      value = FloatBits(target.width * 0.5f * ClipperXyGranularity);
      break;
    case UniformViewportYScale:
      value = FloatBits(target.height * 0.5f * ClipperXyGranularity);
      break;
    case UniformViewportZOffset:
    case UniformViewportZScale:
      value = FloatBits(0.5f);
      break;
    case UniformTmuConfigP0:
      value = texture.State() | (data & 0xFFFFFF);
      break;
    case UniformTmuConfigP1:
      value = texture.Sampler() | (data & 0xFFFFFF);
      break;
    case UniformTextureWidth:
      value = texture.Width();
      break;
    case UniformTextureHeight:
      value = texture.Height();
      break;
    }
    u[i] = value;
  }
  u[shader.uniform_count] = 0;
  return Gpu::Address(u);
}

bool Renderer::Draw(const Program &program, const Texture &texture, const Target &target,
                    int x, int y, unsigned w, unsigned h, uint32_t clear,
                    const void *parameters, unsigned parameter_bytes) {
  const Loaded *loaded = Find(program);
  if (loaded == 0 || target.width > m_nMaxWidth || target.height > m_nMaxHeight) {
    return false;
  }
  const unsigned width = target.width, height = target.height;
  const unsigned tiles_x = (width + TileSize - 1) / TileSize;
  const unsigned tiles_y = (height + TileSize - 1) / TileSize;
  m_nIndirectUsed = 0;

  // The rectangle as a triangle strip: x, y in clip space (y down), and
  // the texture coordinates.
  float *vertices = (float *)Indirect(4 * 16, 16);
  if (vertices == 0) {
    return false;
  }
  float x0 = 2.0f * x / width - 1.0f, x1 = 2.0f * (x + (int)w) / width - 1.0f;
  float y0 = 2.0f * y / height - 1.0f, y1 = 2.0f * (y + (int)h) / height - 1.0f;
  const float strip[16] = {x0, y0, 0.0f, 0.0f, x1, y0, 1.0f, 0.0f,
                           x0, y1, 0.0f, 1.0f, x1, y1, 1.0f, 1.0f};
  memcpy(vertices, strip, sizeof strip);

  uint32_t fs_uniforms = WriteUniforms(program.fs, texture, target, parameters, parameter_bytes);
  uint32_t vs_uniforms = WriteUniforms(program.vs, texture, target, parameters, parameter_bytes);
  uint32_t cs_uniforms = WriteUniforms(program.cs, texture, target, parameters, parameter_bytes);

  // GL Shader State Record, then one attribute record.
  uint8_t *record = (uint8_t *)Indirect(ShaderRecordBytes + AttributeRecordBytes, 32);
  if (record == 0 || fs_uniforms == 0 || vs_uniforms == 0 || cs_uniforms == 0) {
    return false;
  }
  const uint32_t code[] = {loaded->fs, loaded->vs, loaded->cs};
  const uint32_t uniforms[] = {fs_uniforms, vs_uniforms, cs_uniforms};
#if RASPPI >= 5
  PackShaderRecord(record, program, code, uniforms, m_pGpu->DrawIndexRecord());
#else
  PackShaderRecord(record, program, code, uniforms, Gpu::Address(m_pDefaults));
#endif
  PackAttributeRecord(record + ShaderRecordBytes, Gpu::Address(vertices), program, 16);

  // Binning.
  Cl bcl(m_pBcl, BclBytes);
  bcl.TileBinningModeCfg(width, height, 1, InternalBpp32);
  bcl.FlushVcdCache();
  bcl.OcclusionQueryCounter(0);
  bcl.StartTileBinning();
  bcl.ClipWindow(0, 0, width, height);
  bcl.CfgBits(CompareAlways);
  bcl.PointSize(1.0f);
  bcl.LineWidth(1.0f);
  bcl.ClipperXyScaling(width * 0.5f, height * 0.5f);
  bcl.ClipperZScaleOffset(0.5f, 0.5f);
  bcl.ClipperZMinMax(0.0f, 1.0f);
  bcl.ViewportOffset(width * 0.5f, height * 0.5f);
  bcl.ColorWriteMasks(0);
  bcl.BlendEnables(0);
  bcl.SampleState(0xF);
  if (program.flat_shade_flags) {
    bcl.FlatShadeFlags(program.flat_shade_flags);
  } else {
    bcl.ZeroAllFlatShadeFlags();
  }
  bcl.ZeroAllNonPerspectiveFlags();
  bcl.ZeroAllCentroidFlags();
  bcl.VcmCacheSize(program.bin.vc, program.render.vc);
  bcl.GlShaderState(Gpu::Address(record), 1);
  bcl.VertexArrayPrims(PrimTriangleStrip, 4, 0);
  bcl.Flush();

  // The commands for every tile: draw what was binned into it, store it
  // and clear it for the next.
  Cl list(m_pTileList, TileListBytes);
  list.TileCoordinatesImplicit();
  list.EndOfLoads();
  list.PrimListFormat(PrimListTriangles, false);
  list.SetInstanceId(0);
  list.BranchToImplicitTileList(0);
  list.StoreTileBuffer(BufferRt0, Gpu::Address(target.pixels), OutputRgba8, target.stride);
  list.ClearTileBuffers(true, true);
  list.EndOfTileMarker();
  list.ReturnFromSubList();

  // Supertiles: grown until there are at most 256, and fewer than 256
  // across and down.
  unsigned st_w = 1, st_h = 1, st_x, st_y;
  for (;;) {
    st_x = (tiles_x + st_w - 1) / st_w;
    st_y = (tiles_y + st_h - 1) / st_h;
    if (st_x < MaxSupertiles && st_y < MaxSupertiles && st_x * st_y <= MaxSupertiles) {
      break;
    }
    if (st_w < st_h) {
      st_w++;
    } else {
      st_h++;
    }
  }

  // Rendering.
  Cl rcl(m_pRcl, RclBytes);
  rcl.RenderingModeCommon(width, height, 1, InternalBpp32);
  rcl.RenderingModeRenderTarget0(clear);
  rcl.RenderingModeZsClear(1.0f, 0);
  rcl.TileListInitialBlockSize(0, true);
  rcl.MulticoreTileListSetBase(Gpu::Address(m_pTileAlloc));
  rcl.MulticoreSupertileCfg(tiles_x, tiles_y, st_w, st_h);
  // Two dummy tiles that clear the tile buffer (and work around a
  // hardware problem with the first tile).
  for (int i = 0; i < 2; i++) {
    rcl.TileCoordinates(0, 0);
    rcl.EndOfLoads();
    rcl.StoreTileBuffer(BufferNone, 0, 0, 0);
    rcl.ClearTileBuffers(true, true);
    rcl.EndOfTileMarker();
  }
  rcl.FlushVcdCache();
  rcl.StartAddressOfGenericTileList(list.Start(), list.Here());
  for (unsigned row = 0; row < st_y; row++) {
    for (unsigned column = 0; column < st_x; column++) {
      rcl.SupertileCoordinates(column, row);
    }
  }
  rcl.EndOfRendering();

  if (bcl.Overflow() || list.Overflow() || rcl.Overflow()) {
    CLogger::Get()->Write(FromGpu, LogError, "Control lists too long");
    return false;
  }
  CleanAndInvalidateDataCacheRange((uintptr_t)m_pIndirect, m_nIndirectUsed);
  CleanAndInvalidateDataCacheRange((uintptr_t)m_pBcl, bcl.Used());
  CleanAndInvalidateDataCacheRange((uintptr_t)m_pTileList, list.Used());
  CleanAndInvalidateDataCacheRange((uintptr_t)m_pRcl, rcl.Used());

  if (!m_pGpu->Bin(bcl.Start(), bcl.Here(), Gpu::Address(m_pTileAlloc), m_nTileAllocBytes,
                   Gpu::Address(m_pTileState), TimeoutMs)) {
    return false;
  }
  return m_pGpu->Render(rcl.Start(), rcl.Here(), TimeoutMs);
}

// A 256x256 pattern copied one to one with nearest sampling into the
// middle of a 320x288 picture: every pixel must match, and the border
// must be the clear colour.
bool Renderer::CopyTest(void) {
  const unsigned Size = 256, Width = 320, Height = 288, Left = 32, Top = 16;
  const uint32_t Clear = 0xFF102030;
  Texture texture;
  uint32_t *pixels = (uint32_t *)m_pGpu->Allocate(Width * Height * 4, 4096);
  if (pixels == 0 || !texture.Create(m_pGpu, Size, Size) ||
      !texture.Set(Size, Size, false) || !Load(CopyProgram)) {
    CLogger::Get()->Write(FromGpu, LogWarning, "Copy test: not enough memory");
    return false;
  }
  for (unsigned y = 0; y < Size; y++) {
    for (unsigned x = 0; x < Size; x++) {
      texture.Pixel(x, y) = 0xFF000000 | (y << 8) | x | ((x ^ y) << 16);
    }
  }
  texture.Flush();
  CleanAndInvalidateDataCacheRange((uintptr_t)pixels, Width * Height * 4);

  Target target = {pixels, Width, Height, Width * 4};
  bool ok = Draw(CopyProgram, texture, target, Left, Top, Size, Size, Clear);
  InvalidateDataCacheRange((uintptr_t)pixels, Width * Height * 4);

  unsigned wrong = 0, first_x = 0, first_y = 0;
  uint32_t first_got = 0, first_expected = 0;
  for (unsigned y = 0; y < Height; y++) {
    for (unsigned x = 0; x < Width; x++) {
      uint32_t expected = Clear;
      if (x >= Left && x < Left + Size && y >= Top && y < Top + Size) {
        unsigned u = x - Left, v = y - Top;
        expected = 0xFF000000 | (v << 8) | u | ((u ^ v) << 16);
      }
      uint32_t got = pixels[y * Width + x];
      if (got != expected) {
        if (wrong == 0) {
          first_x = x;
          first_y = y;
          first_got = got;
          first_expected = expected;
        }
        wrong++;
      }
    }
  }
  bool passed = ok && wrong == 0;
  CLogger::Get()->Write(FromGpu, passed ? LogNotice : LogWarning,
                        "Copy test %s: %u of %u pixels wrong%s", passed ? "passed" : "failed",
                        wrong, Width * Height, ok ? "" : " (the GPU didn't finish)");
  if (wrong != 0) {
    CLogger::Get()->Write(FromGpu, LogWarning,
                          "First wrong pixel at %u,%u: %08X, expected %08X; "
                          "pixel 100,100 is %08X", first_x, first_y, first_got,
                          first_expected, pixels[100 * Width + 100]);
  }
  return passed;
}

} // namespace v3d
