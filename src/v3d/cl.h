//
// cl.h
//
// Writes control list packets (an opcode byte, then the packet's fields at
// their bit positions, little endian) and the records they point to, for
// V3D 4.2 (the Pi 4) or V3D 7.1 (the Pi 5). Only the packets and records
// BMC64 uses.
//
// Packet codes, field positions and values from Mesa,
// src/broadcom/cle/v3d_packet.xml (Copyright (C) 2016 Broadcom, MIT
// licence); packing rules from src/broadcom/cle/gen_pack_header.py; the
// V3D 7.1 render target and texture settings from
// src/gallium/drivers/v3d/v3dx_rcl.c, v3dx_emit.c and v3dx_state.c, and
// src/broadcom/common/v3d_util.c (tile size, render target stride).
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_V3D_CL_H_
#define BMC64_V3D_CL_H_

#include "shader.h"

#include <stdint.h>
#include <string.h>

namespace v3d {

// Values of enumerated fields.
const unsigned InternalBpp32 = 0;
const unsigned InternalType8 = 2;        // 8-bit unsigned normalised
const unsigned InternalDepth24 = 1;
const unsigned ClampNone = 0;
const unsigned OutputRgba8 = 27;
const unsigned MemoryRaster = 0;
const unsigned BufferRt0 = 0;
const unsigned BufferNone = 8;
const unsigned PrimTriangles = 4;
const unsigned PrimTriangleStrip = 5;
const unsigned PrimListTriangles = 2;    // Prim List Format
const unsigned CompareAlways = 7;
const unsigned AttributeFloat = 2;
const unsigned TextureRgba8 = 4;
const unsigned SwizzleRed = 2, SwizzleGreen = 3, SwizzleBlue = 4, SwizzleAlpha = 5;
const unsigned WrapClamp = 1;
const unsigned FlagsZeroed = 1;          // Varying Flags Action
#if RASPPI >= 5
const unsigned TypeClamp8 = 8;           // Render Target Type Clamp: 8-bit
const unsigned ZClipMinusOneToOne = 1;   // Z Clip Mode
const unsigned Log2Tile64 = 3;           // 64 pixel tiles
const unsigned TileWidth = 64;

// The clipper's XY scale is in 1/64 pixels.
const float ClipperXyGranularity = 64.0f;
#else
// The clipper's XY scale is in 1/256 pixels.
const float ClipperXyGranularity = 256.0f;
#endif

// Fields packed at their bit positions, for packets and for the records
// that packets point to.
struct Bits {
  uint8_t bytes[40];
  unsigned length;   // bytes used
  Bits(void) : length(0) { memset(bytes, 0, sizeof bytes); }
  void Set(unsigned start, unsigned bits, uint64_t value) {
    for (unsigned i = 0; i < bits; i++) {
      if ((value >> i) & 1) {
        bytes[(start + i) / 8] |= (uint8_t)(1u << ((start + i) % 8));
      }
    }
    unsigned end = (start + bits + 7) / 8;
    if (end > length) {
      length = end;
    }
  }
  void SetFloat(unsigned start, float value) {
    uint32_t u;
    memcpy(&u, &value, 4);
    Set(start, 32, u);
  }
  // Writes the record to memory, padded to SIZE bytes.
  void Write(void *to, unsigned size) const { memcpy(to, bytes, size); }
};

// A control list being written into memory the GPU reads.
class Cl {
public:
  Cl(void *buffer, unsigned size)
      : m_pStart((uint8_t *)buffer), m_pPos((uint8_t *)buffer),
        m_pEnd((uint8_t *)buffer + size), m_bOverflow(false) {}

  uint32_t Start(void) const { return (uint32_t)(uintptr_t)m_pStart; }
  uint32_t Here(void) const { return (uint32_t)(uintptr_t)m_pPos; }
  unsigned Used(void) const { return (unsigned)(m_pPos - m_pStart); }
  bool Overflow(void) const { return m_bOverflow; }

  // Packets with no fields.
  void Halt(void) { Op(0); }
  void EndOfRendering(void) { Op(13); }
  void ReturnFromSubList(void) { Op(18); }
  void FlushVcdCache(void) { Op(19); }
  void EndOfLoads(void) { Op(26); }
  void EndOfTileMarker(void) { Op(27); }
  void TileCoordinatesImplicit(void) { Op(125); }

  void Flush(void) { Op(4); }
  void StartTileBinning(void) { Op(6); }
  void ZeroAllCentroidFlags(void) { Op(88); }
  void ZeroAllFlatShadeFlags(void) { Op(97); }
  void ZeroAllNonPerspectiveFlags(void) { Op(99); }

  void BranchToImplicitTileList(unsigned set) {
    Bits f;
    f.Set(0, 8, set);
    Emit(21, f);
  }

  void VertexArrayPrims(unsigned mode, unsigned count, unsigned first) {
    Bits f;
    f.Set(0, 8, mode);
    f.Set(8, 32, count);
    f.Set(40, 32, first);
    Emit(36, f);
  }

  void SetInstanceId(unsigned id) {
    Bits f;
    f.Set(0, 32, id);
    Emit(54, f);
  }

  void PrimListFormat(unsigned type, bool strip_or_fan) {
    Bits f;
    f.Set(0, 6, type);
    f.Set(7, 1, strip_or_fan);
    Emit(56, f);
  }

  // RECORD is 32-byte aligned; ATTRIBUTES records follow it.
  void GlShaderState(uint32_t record, unsigned attributes) {
    Bits f;
    f.Set(0, 32, (record & ~31u) | attributes);
    Emit(64, f);
  }

  void VcmCacheSize(unsigned binning, unsigned rendering) {
    Bits f;
    f.Set(0, 4, binning);
    f.Set(4, 4, rendering);
    Emit(71, f);
  }

  void BlendEnables(unsigned mask) {
    Bits f;
    f.Set(0, 8, mask);
    Emit(83, f);
  }

  // Set bits stop writes to that channel (4 per render target).
  void ColorWriteMasks(uint32_t disabled) {
    Bits f;
    f.Set(0, 32, disabled);
    Emit(87, f);
  }

  void SampleState(unsigned mask) {
    Bits f;
    f.Set(0, 4, mask);
    f.Set(16, 16, 0x3F80);              // coverage 1.0 (top half of the float)
    Emit(91, f);
  }

  void OcclusionQueryCounter(uint32_t address) {
    Bits f;
    f.Set(0, 32, address);
    Emit(92, f);
  }

  // Cfg Bits: both faces drawn, depth test COMPARE, no depth writes; on
  // V3D 7.1 Z clipped to -1..1.
  void CfgBits(unsigned compare) {
    Bits f;
    f.Set(0, 1, 1);
    f.Set(1, 1, 1);
    f.Set(12, 3, compare);
#if RASPPI >= 5
    f.Set(22, 2, ZClipMinusOneToOne);
#endif
    Emit(96, f, 3);
  }

  void FlatShadeFlags(uint32_t flags) {
    Bits f;
    f.Set(4, 2, 0);                     // lower varyings: none
    f.Set(6, 2, FlagsZeroed);           // higher varyings: zeroed
    f.Set(8, 24, flags);
    Emit(98, f);
  }

  void PointSize(float size) {
    Bits f;
    f.SetFloat(0, size);
    Emit(104, f);
  }

  void LineWidth(float width) {
    Bits f;
    f.SetFloat(0, width);
    Emit(105, f);
  }

  void ClipWindow(unsigned left, unsigned bottom, unsigned width, unsigned height) {
    Bits f;
    f.Set(0, 16, left);
    f.Set(16, 16, bottom);
    f.Set(32, 16, width);
    f.Set(48, 16, height);
    Emit(107, f);
  }

  // The viewport centre, in pixels (not negative).
  void ViewportOffset(float x, float y) {
    Bits f;
    f.Set(0, 22, (uint32_t)(x * 256.0f));
    f.Set(32, 22, (uint32_t)(y * 256.0f));
    Emit(108, f, 8);                    // coarse x and y (bits 22, 54): 0
  }

  void ClipperZMinMax(float min, float max) {
    Bits f;
    f.SetFloat(0, min);
    f.SetFloat(32, max);
    Emit(109, f);
  }

  // Half the viewport width and height, in pixels.
  void ClipperXyScaling(float half_width, float half_height) {
    Bits f;
    f.SetFloat(0, half_width * ClipperXyGranularity);
    f.SetFloat(32, half_height * ClipperXyGranularity);
    Emit(110, f);
  }

  void ClipperZScaleOffset(float scale, float offset) {
    Bits f;
    f.SetFloat(0, scale);
    f.SetFloat(32, offset);
    Emit(111, f);
  }

  // Tile Binning Mode Cfg: 64-byte tile allocation blocks. V3D 7.1 takes
  // the tile size (64x64) instead of the render targets.
  void TileBinningModeCfg(unsigned width, unsigned height, unsigned targets,
                          unsigned max_bpp) {
    Bits f;
    f.Set(2, 2, 0);
    f.Set(4, 2, 0);
#if RASPPI >= 5
    (void)targets;
    (void)max_bpp;
    f.Set(8, 3, Log2Tile64);
    f.Set(11, 3, Log2Tile64);
#else
    f.Set(8, 4, targets - 1);
    f.Set(12, 2, max_bpp);
#endif
    f.Set(32, 16, width - 1);
    f.Set(48, 16, height - 1);
    Emit(120, f);
  }

  void StartAddressOfGenericTileList(uint32_t start, uint32_t end) {
    Bits f;
    f.Set(0, 32, start);
    f.Set(32, 32, end);
    Emit(20, f);
  }

  void SupertileCoordinates(unsigned column, unsigned row) {
    Bits f;
    f.Set(0, 8, column);
    f.Set(8, 8, row);
    Emit(23, f);
  }

  // On V3D 7.1 Clear Render Targets, which takes no fields.
  void ClearTileBuffers(bool render_targets, bool z_stencil) {
#if RASPPI >= 5
    (void)render_targets;
    (void)z_stencil;
    Op(25);
#else
    Bits f;
    f.Set(0, 1, render_targets);
    f.Set(1, 1, z_stencil);
    Emit(25, f);
#endif
  }

  // Store Tile Buffer General: BUFFER to raster memory at ADDRESS, STRIDE
  // bytes a line.
  void StoreTileBuffer(unsigned buffer, uint32_t address, unsigned format,
                       unsigned stride) {
    Bits f;
    f.Set(0, 4, buffer);
    f.Set(4, 3, MemoryRaster);
    f.Set(12, 6, format);
    f.Set(28, 20, stride);
    f.Set(64, 32, address);
    Emit(29, f);
  }

  // Tile Rendering Mode Cfg (Common), sub-id 0. On V3D 7.1 the tile size
  // (64x64) instead of the largest bpp.
  void RenderingModeCommon(unsigned width, unsigned height, unsigned targets,
                           unsigned max_bpp) {
    Bits f;
    f.Set(0, 4, 0);
    f.Set(4, 4, targets - 1);
    f.Set(8, 16, width);
    f.Set(24, 16, height);
#if RASPPI >= 5
    (void)max_bpp;
    f.Set(52, 3, Log2Tile64);
    f.Set(55, 3, Log2Tile64);
#else
    f.Set(40, 2, max_bpp);
#endif
    f.Set(46, 1, 1);                    // early Z disable
    f.Set(47, 4, InternalDepth24);
    Emit(121, f, 8);
  }

  // Render target 0, 32 bpp, 8-bit unsigned normalised, unclamped, cleared
  // to CLEAR. V3D 4.2: Tile Rendering Mode Cfg (Clear Colors Part1), sub-id
  // 3, and (Color), sub-id 1. V3D 7.1: (Render Target Part1), sub-id 2,
  // at the start of the tile buffer, 64 pixels a row.
  void RenderingModeRenderTarget0(uint32_t clear) {
#if RASPPI >= 5
    Bits f;
    f.Set(0, 3, 2);
    f.Set(3, 3, 0);
    f.Set(7, 11, 0);                    // base address
    f.Set(18, 7, TileWidth * 1 / 2 - 1); // stride: 128 bits, two rows
    f.Set(25, 2, InternalBpp32);
    f.Set(27, 5, TypeClamp8);
    f.Set(32, 32, clear);
    Emit(121, f, 8);
#else
    Bits c;
    c.Set(0, 4, 3);
    c.Set(4, 4, 0);
    c.Set(8, 32, clear);
    c.Set(40, 24, 0);
    Emit(121, c, 8);
    Bits f;
    f.Set(0, 4, 1);
    f.Set(4, 2, InternalBpp32);
    f.Set(6, 4, InternalType8);
    f.Set(10, 2, ClampNone);
    Emit(121, f, 8);
#endif
  }

  // Tile Rendering Mode Cfg (ZS Clear Values): sub-id 2 on V3D 4.2, 1 on
  // V3D 7.1.
  void RenderingModeZsClear(float z, unsigned stencil) {
    uint32_t zbits;
    memcpy(&zbits, &z, 4);
    Bits f;
#if RASPPI >= 5
    f.Set(0, 4, 1);
#else
    f.Set(0, 4, 2);
#endif
    f.Set(8, 8, stencil);
    f.Set(16, 32, zbits);
    Emit(121, f, 8);
  }

  void MulticoreSupertileCfg(unsigned tiles_x, unsigned tiles_y,
                             unsigned supertile_w, unsigned supertile_h) {
    unsigned st_x = (tiles_x + supertile_w - 1) / supertile_w;
    unsigned st_y = (tiles_y + supertile_h - 1) / supertile_h;
    Bits f;
    f.Set(0, 8, supertile_w - 1);
    f.Set(8, 8, supertile_h - 1);
    f.Set(16, 8, st_x);
    f.Set(24, 8, st_y);
    f.Set(32, 12, tiles_x);
    f.Set(44, 12, tiles_y);
    f.Set(61, 3, 0);                    // one bin tile list
    Emit(122, f);
  }

  // The address is 64-byte aligned; set number in the low bits.
  void MulticoreTileListSetBase(uint32_t address) {
    Bits f;
    f.Set(0, 32, address & ~63u);
    Emit(123, f);
  }

  void TileCoordinates(unsigned column, unsigned row) {
    Bits f;
    f.Set(0, 12, column);
    f.Set(12, 12, row);
    Emit(124, f);
  }

  void TileListInitialBlockSize(unsigned size_enum, bool auto_chain) {
    Bits f;
    f.Set(0, 2, size_enum);
    f.Set(2, 1, auto_chain);
    Emit(126, f);
  }

private:
  void Op(uint8_t opcode) {
    if (m_pPos + 1 > m_pEnd) {
      m_bOverflow = true;
      return;
    }
    *m_pPos++ = opcode;
  }

  // LENGTH: the packet's field bytes, when its last fields are padding.
  void Emit(uint8_t opcode, const Bits &f, unsigned length = 0) {
    unsigned n = length > f.length ? length : f.length;
    if (m_pPos + 1 + n > m_pEnd) {
      m_bOverflow = true;
      return;
    }
    *m_pPos++ = opcode;
    memcpy(m_pPos, f.bytes, n);
    m_pPos += n;
  }

  uint8_t *m_pStart, *m_pPos, *m_pEnd;
  bool m_bOverflow;
};

// Records that control lists point to.
#if RASPPI >= 5
const unsigned ShaderRecordBytes = 32;
#else
const unsigned ShaderRecordBytes = 36;
#endif
const unsigned AttributeRecordBytes = 16;
const unsigned TextureStateBytes = 24;
const unsigned SamplerStateBytes = 24;

#if RASPPI >= 5
// GL Shader State Record for PROGRAM: CODE and UNIFORMS are the addresses
// for its fragment, vertex and coordinate shaders, in that order. DRAW_INDEX:
// the layout with the draw index fields (Gpu::DrawIndexRecord()). There are
// no default attribute values on V3D 7.1.
inline void PackShaderRecord(void *to, const Program &program, const uint32_t code[3],
                             const uint32_t uniforms[3], bool draw_index) {
  const Shader &vs = program.vs, &cs = program.cs;
  Bits f;
  f.Set(1, 1, 1);                       // clipping
  f.Set(2, 1, cs.uses_vertex_id);
  f.Set(3, 1, cs.uses_instance_id);
  unsigned at = draw_index ? 7 : 5;     // vertex shader flags
  f.Set(at, 1, vs.uses_vertex_id);
  f.Set(at + 1, 1, vs.uses_instance_id);
  at = draw_index ? 12 : 8;             // fragment shader flags
  f.Set(at, 1, program.writes_z);
  f.Set(at + 1, 1, program.disable_early_z);
  at = draw_index ? 15 : 12;
  f.Set(at, 1, program.uses_centre_w);
  f.Set(at + 5, 1, program.scoreboard_wait_on_first_switch);
  f.Set(at + 6, 1, !program.implicit_point_line_varyings);
  f.Set(24, 8, program.varyings);
  f.Set(32, 4, cs.vpm_output_size);
  f.Set(36, 4, program.bin.ve);
  f.Set(40, 4, cs.vpm_input_size);
  f.Set(44, 4, program.bin.as - 1);
  f.Set(48, 4, vs.vpm_output_size);
  f.Set(52, 4, program.render.ve);
  f.Set(56, 4, vs.vpm_input_size);
  f.Set(60, 4, program.render.as - 1);
  const Shader *shaders[] = {&program.fs, &program.vs, &program.cs};
  for (unsigned i = 0; i < 3; i++) {
    at = 64 + i * 64;
    f.Set(at, 32, code[i]);
    f.Set(at, 1, shaders[i]->threads == 4);
    f.Set(at + 1, 1, shaders[i]->single_segment);
    f.Set(at + 32, 32, uniforms[i]);
  }
  f.Write(to, ShaderRecordBytes);
}
#else
// GL Shader State Record for PROGRAM: CODE and UNIFORMS are the addresses
// for its fragment, vertex and coordinate shaders, in that order; DEFAULTS
// the default attribute values.
inline void PackShaderRecord(void *to, const Program &program, const uint32_t code[3],
                             const uint32_t uniforms[3], uint32_t defaults) {
  const Shader &vs = program.vs, &cs = program.cs;
  Bits f;
  f.Set(1, 1, 1);                       // clipping
  f.Set(2, 1, cs.uses_vertex_id);
  f.Set(3, 1, cs.uses_instance_id);
  f.Set(5, 1, vs.uses_vertex_id);
  f.Set(6, 1, vs.uses_instance_id);
  f.Set(8, 1, program.writes_z);
  f.Set(9, 1, program.disable_early_z);
  f.Set(10, 1, cs.separate_segments);
  f.Set(11, 1, vs.separate_segments);
  f.Set(12, 1, program.uses_centre_w);
  f.Set(17, 1, program.scoreboard_wait_on_first_switch);
  f.Set(18, 1, !program.implicit_point_line_varyings);
  f.Set(24, 8, program.varyings);
  f.Set(32, 4, cs.vpm_output_size);
  f.Set(36, 4, program.bin.ve);
  f.Set(40, 4, cs.separate_segments ? cs.vpm_input_size : 1);
  f.Set(44, 4, program.bin.as - 1);
  f.Set(48, 4, vs.vpm_output_size);
  f.Set(52, 4, program.render.ve);
  f.Set(56, 4, vs.separate_segments ? vs.vpm_input_size : 1);
  f.Set(60, 4, program.render.as - 1);
  f.Set(64, 32, defaults);
  const Shader *shaders[] = {&program.fs, &program.vs, &program.cs};
  for (unsigned i = 0; i < 3; i++) {
    unsigned at = 96 + i * 64;
    f.Set(at, 32, code[i]);
    f.Set(at, 1, shaders[i]->threads == 4);
    f.Set(at + 1, 1, shaders[i]->single_segment);
    f.Set(at + 2, 1, 1);                // propagate NaNs
    f.Set(at + 32, 32, uniforms[i]);
  }
  f.Write(to, ShaderRecordBytes);
}
#endif

// GL Shader State Attribute Record: 4 floats a vertex at ADDRESS, STRIDE
// bytes apart. The coordinate shader must read at least one (GFXH-930).
inline void PackAttributeRecord(void *to, uint32_t address, const Program &program,
                                unsigned stride) {
  Bits f;
  f.Set(0, 32, address);
  f.Set(32, 2, 0);                      // 4 values
  f.Set(34, 3, AttributeFloat);
  f.Set(40, 4, program.cs.attribute_size ? program.cs.attribute_size : 1);
  f.Set(44, 4, program.vs.attribute_size);
  f.Set(64, 32, stride);
  f.Set(96, 32, 0xFFFFFF);              // maximum index
  f.Write(to, AttributeRecordBytes);
}

// Texture Shader State: an RGBA8 picture at ADDRESS, level 0 only, strictly
// UIF without XOR and without extra padding. On V3D 7.1 the chroma
// settings as Mesa sets them for every texture.
inline void PackTextureState(void *to, uint32_t address, unsigned width, unsigned height) {
  Bits f;
  f.Set(0, 32, address);
#if RASPPI >= 5
  f.Set(136, 1, 1);                     // chroma offset x
  f.Set(137, 1, 1);                     // chroma offset y
  f.Set(138, 26, address >> 6);         // Cb base
  f.Set(164, 26, address >> 6);         // Cr base
#endif
  f.Set(58, 14, width);
  f.Set(72, 14, height);
  f.Set(86, 14, 1);                     // depth
  f.Set(100, 7, TextureRgba8);
  f.Set(107, 1, 1);                     // extended (needed for strictly UIF)
  f.Set(108, 3, SwizzleRed);
  f.Set(111, 3, SwizzleGreen);
  f.Set(114, 3, SwizzleBlue);
  f.Set(117, 3, SwizzleAlpha);
  f.Set(134, 1, 1);                     // level 0 is strictly UIF
  f.Write(to, TextureStateBytes);
}

// Sampler State: clamped to the edges, level 0 only; bilinear if SMOOTH,
// else nearest.
inline void PackSamplerState(void *to, bool smooth) {
  Bits f;
  f.Set(0, 1, !smooth);                 // magnification nearest
  f.Set(1, 1, !smooth);                 // minification nearest
  f.Set(2, 1, 1);                       // mipmap nearest
  f.Set(48, 3, WrapClamp);
  f.Set(51, 3, WrapClamp);
  f.Set(54, 3, WrapClamp);
  f.Write(to, SamplerStateBytes);
}

} // namespace v3d

#endif
