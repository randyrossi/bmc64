//
// shaders.cpp
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
// From BMA800XL's src/v3d/shaders.cpp, unchanged.
//
// See shaders.h.
//

#include "shaders.h"

#if RASPPI >= 5
#include "shader71_copy.h"
#include "shader71_crt.h"
#include "shader71_crt_curve.h"
#include "shader71_crt_curve_smooth.h"
#include "shader71_crt_smooth.h"
#else
#include "shader_copy.h"
#include "shader_crt.h"
#include "shader_crt_curve.h"
#include "shader_crt_curve_smooth.h"
#include "shader_crt_smooth.h"
#endif

namespace v3d {

const Program &CopyProgram = copy;
const Program &CrtProgram = crt;
const Program &CrtSmoothProgram = crt_smooth;
const Program &CrtCurveProgram = crt_curve;
const Program &CrtCurveSmoothProgram = crt_curve_smooth;

} // namespace v3d
