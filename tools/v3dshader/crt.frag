#version 450
// crt-pi, "A Raspberry Pi friendly CRT shader", Copyright (C) 2015-2016
// davej, GPL 2 or later; adapted from the copy BMC64 ships in
// src/crt_pi_idx.c (the fragment shader), with its default settings:
// scan lines with MULTISAMPLE, GAMMA with FAKE_GAMMA, no mask, not
// SHARPER, and CURVATURE when built with it.
//
// The picture is already in colour (no palette texture), the texture is
// just the visible part (screenScale 1), and with SMOOTH the hardware's
// bilinear filter is used with the scan lines on the texel centres.

layout(binding = 0) uniform sampler2D picture;

layout(push_constant) uniform Parameters {
  vec2 texture_size;     // the picture, in pixels
  float filter_width;    // (picture height / output height) / 3
} p;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 colour;

const float ScanlineWeight = 6.0;
const float ScanlineGapBrightness = 0.12;
const float BloomFactor = 1.5;
const vec2 Curvature = vec2(0.10, 0.15);

float LineWeight(float dist) {
  return max(1.0 - dist * dist * ScanlineWeight, ScanlineGapBrightness);
}

float ScanLine(float dy) {
  return (LineWeight(dy) + LineWeight(dy - p.filter_width) +
          LineWeight(dy + p.filter_width)) * 0.3333333;
}

void main() {
  vec2 texcoord = uv;
#ifdef CURVATURE
  // Distort(), with barrel scaling.
  vec2 c = texcoord - vec2(0.5);
  float rsq = c.x * c.x + c.y * c.y;
  c += c * (Curvature * rsq);
  c *= vec2(1.0) - 0.23 * Curvature;
  if (abs(c.x) >= 0.5 || abs(c.y) >= 0.5) {
    colour = vec4(0.0, 0.0, 0.0, 1.0);
    return;
  }
  texcoord = c + vec2(0.5);
#endif

  vec2 pixels = texcoord * p.texture_size;
  float centre = floor(pixels.y) + 0.5;
  float dy = pixels.y - centre;
  float weight = ScanLine(dy);
#ifdef SMOOTH
  vec3 rgb = texture(picture, texcoord).rgb;
#else
  // The row sampled moves towards the next one by 8 * dy^4.
  float d = dy * dy;
  d = d * d * 8.0;
  vec2 tc = vec2(texcoord.x, (centre + sign(dy) * d) / p.texture_size.y);
  vec3 rgb = texture(picture, tc).rgb;
#endif

  // Fake gamma: square, scan line weight and bloom, square root.
  rgb = sqrt(rgb * rgb * (weight * BloomFactor));
  colour = vec4(rgb, 1.0);
}
