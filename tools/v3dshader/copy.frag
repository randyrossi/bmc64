#version 450
// Copies the picture, sampled at the texture coordinate.

layout(binding = 0) uniform sampler2D picture;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 colour;

void main() {
  colour = texture(picture, uv);
}
