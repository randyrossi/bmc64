
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2017 Istvan Varga <istvanv@users.sourceforge.net>
// https://github.com/istvan-v/plus4emu/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef PLUS4EMU_SHADERS_HPP
#define PLUS4EMU_SHADERS_HPP

// default shader sources for PAL/NTSC emulation

static const char *shaderSourcePAL = {
  "uniform sampler2D textureHandle;\n"
  "uniform float lineShade;\n"
  "const mat4 yuv2rgbMatrix = mat4( 0.50000,  0.00000,  0.18236, -0.70100,\n"
  "                                 0.50000, -0.04476, -0.09289,  0.52914,\n"
  "                                 0.50000,  0.23049,  0.00000, -0.88600,\n"
  "                                 0.00000,  0.00000,  0.00000,  0.00000);\n"
  "void main()\n"
  "{\n"
  "  float txc = gl_TexCoord[0][0];\n"
  "  float tyc = gl_TexCoord[0][1];\n"
  "  float txcm1 = txc - 0.00296;\n"
  "  float txcm0 = txc - 0.00085;\n"
  "  float txcp0 = txc + 0.00085;\n"
  "  float txcp1 = txc + 0.00296;\n"
  "  float tyc0 = tyc + 0.00048828125;\n"
  "  float tyc1 = tyc - 0.00146484375;\n"
  "  vec4 p00 =   texture(textureHandle, vec2(txcm0, tyc0))\n"
  "             + texture(textureHandle, vec2(txcp0, tyc0));\n"
  "  vec4 p10 =   texture(textureHandle, vec2(txcm0, tyc1))\n"
  "             + texture(textureHandle, vec2(txcp0, tyc1));\n"
  "  vec4 p01 =   texture(textureHandle, vec2(txcm1, tyc0))\n"
  "             + texture(textureHandle, vec2(txcp1, tyc0))\n"
  "             + texture(textureHandle, vec2(txcm1, tyc1))\n"
  "             + texture(textureHandle, vec2(txcp1, tyc1));\n"
  "  float f = mix(sin(tyc * 3216.991) * 0.5 + 0.5, 1.0, lineShade);\n"
  "  vec4 tmp = (p00 + p10) + (p01 * 0.922);\n"
  "  gl_FragColor = (vec4(p00[0], tmp[1], tmp[2], 1.0) * yuv2rgbMatrix) * f;\n"
  "}\n"
};

static const char *shaderSourceNTSC = {
  "uniform sampler2D textureHandle;\n"
  "uniform float lineShade;\n"
  "const mat4 yuv2rgbMatrix = mat4( 0.50000,  0.00000,  0.37387, -0.70100,\n"
  "                                 0.50000, -0.09177, -0.19044,  0.52914,\n"
  "                                 0.50000,  0.47253,  0.00000, -0.88600,\n"
  "                                 0.00000,  0.00000,  0.00000,  0.00000);\n"
  "void main()\n"
  "{\n"
  "  float txc = gl_TexCoord[0][0];\n"
  "  float tyc = gl_TexCoord[0][1];\n"
  "  float txcm1 = txc - 0.00317;\n"
  "  float txcm0 = txc - 0.00095;\n"
  "  float txcp0 = txc + 0.00095;\n"
  "  float txcp1 = txc + 0.00317;\n"
  "  float tyc0 = tyc + 0.00048828125;\n"
  "  vec4 p00 =   texture(textureHandle, vec2(txcm0, tyc0))\n"
  "             + texture(textureHandle, vec2(txcp0, tyc0));\n"
  "  vec4 p01 =   texture(textureHandle, vec2(txcm1, tyc0))\n"
  "             + texture(textureHandle, vec2(txcp1, tyc0));\n"
  "  float f = mix(sin(tyc * 3216.991) * 0.5 + 0.5, 1.0, lineShade);\n"
  "  vec4 tmp = p00 + (p01 * 0.875);\n"
  "  gl_FragColor = (vec4(p00[0], tmp[1], tmp[2], 1.0) * yuv2rgbMatrix) * f;\n"
  "}\n"
};

#endif  // PLUS4EMU_SHADERS_HPP

