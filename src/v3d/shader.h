//
// shader.h
//
// Compiled GPU shaders, as written by tools/v3dshader: QPU code, the list
// of uniforms the code reads, and the facts about each shader that the
// shader state record needs.
//
// The uniform kinds and fields follow Mesa, src/broadcom/compiler/
// v3d_compiler.h (Copyright (C) 2016 Broadcom, MIT licence).
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_V3D_SHADER_H_
#define BMC64_V3D_SHADER_H_

#include <stdint.h>

namespace v3d {

// What a uniform holds.
enum UniformKind {
  UniformConstant,        // DATA itself
  UniformParameter,       // 32-bit word DATA of the shader parameters
  UniformViewportXScale,  // half the viewport width, in the clipper's units
  UniformViewportYScale,
  UniformViewportZOffset,
  UniformViewportZScale,
  UniformTmuConfigP0,     // texture state address | DATA (texture DATA >> 24)
  UniformTmuConfigP1,     // sampler state address | DATA
  UniformTextureWidth,    // of texture DATA
  UniformTextureHeight,
};

struct Uniform {
  UniformKind kind;
  uint32_t data;
};

struct Shader {
  const uint64_t *code;
  unsigned instructions;
  const Uniform *uniforms;
  unsigned uniform_count;
  unsigned threads;            // 1, 2 or 4
  bool single_segment;         // starts in the final thread section
  // Vertex and coordinate shaders only.
  unsigned vpm_input_size;
  unsigned vpm_output_size;
  bool separate_segments;
  bool uses_vertex_id;
  bool uses_instance_id;
  unsigned attribute_size;     // values read from vertex attribute 0
};

// VPM set-up for one pass (binning or rendering).
struct Vpm {
  unsigned as, ve, vc;
};

// A fragment shader with the vertex shader that feeds it, and the
// vertex shader's binning (coordinate) version.
struct Program {
  Shader fs, vs, cs;
  unsigned varyings;
  uint32_t flat_shade_flags;
  bool writes_z;
  bool disable_early_z;
  bool uses_centre_w;
  bool scoreboard_wait_on_first_switch;
  bool implicit_point_line_varyings;
  Vpm bin, render;
};

} // namespace v3d

#endif
