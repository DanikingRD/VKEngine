#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 0) out vec3 frag_color;

layout(set = 0, binding = 0) uniform Globals{
    mat4 proj;
    mat4 view;
} globals;

void main() {
    gl_Position = globals.proj * globals.view * vec4(in_pos, 1.0);
    frag_color = vec3(in_pos.x, in_pos.y, 0.9);
}