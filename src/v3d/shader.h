//
// shader.h
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
// Parts adapted from Mesa (26.2.4 and 25.2.8), which remain under their
// license:
//
//     src/broadcom/compiler/v3d_compiler.h:
//         Copyright © 2016 Broadcom
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice (including the next
// paragraph) shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// From BMA800XL's src/v3d/shader.h, unchanged but for the include guard's
// name.
//
// Compiled GPU shaders, as written by tools/v3dshader: QPU code, the list
// of uniforms the code reads, and the facts about each shader that the
// shader state record needs.
//
// The uniform kinds and fields follow Mesa, src/broadcom/compiler/
// v3d_compiler.h (Copyright (C) 2016 Broadcom, MIT licence).
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
