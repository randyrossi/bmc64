#version 450
// Draws a picture on a rectangle: each vertex is x, y (clip space) and
// u, v (texture).

layout(location = 0) in vec4 vertex;
layout(location = 0) out vec2 uv;

void main() {
  uv = vertex.zw;
  gl_Position = vec4(vertex.xy, 0.0, 1.0);
}
