//
// shaders.h
//
// The compiled GPU shader programs (tools/v3dshader): the generated
// headers, shader_*.h for V3D 4.2 or shader71_*.h for V3D 7.1, each
// included once, in shaders.cpp.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
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
