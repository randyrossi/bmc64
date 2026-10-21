//
// shaders.cpp
//
// See shaders.h.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
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
