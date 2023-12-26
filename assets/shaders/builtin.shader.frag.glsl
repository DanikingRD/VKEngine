#version 450

layout(location = 0) out vec4 out_color;
layout(location = 0) in vec3 frag_color;

void main() {
    out_color = vec4(frag_color.r + 0.5, frag_color.g + 0.2, frag_color.b + 0.1, 1.0);
}