//
// shaders.h
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
// From BMA800XL's src/v3d/shaders.h, unchanged but for the include guard's
// name.
//
// The compiled GPU shader programs (tools/v3dshader): the generated
// headers, shader_*.h for V3D 4.2 or shader71_*.h for V3D 7.1, each
// included once, in shaders.cpp.
//

#ifndef BMC64_V3D_SHADERS_H_
#define BMC64_V3D_SHADERS_H_

#include "shader.h"

namespace v3d {

extern const Program &CopyProgram;            // copy.frag
extern const Program &CrtProgram;             // crt.frag
extern const Program &CrtSmoothProgram;       // with SMOOTH
extern const Program &CrtCurveProgram;        // with CURVATURE
extern const Program &CrtCurveSmoothProgram;  // with both

} // namespace v3d

#endif
