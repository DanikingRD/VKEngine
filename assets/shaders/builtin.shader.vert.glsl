#version 450

layout(location = 0) in vec2 in_pos;
layout(location = 0) out vec3 frag_color; 

void main() {
    gl_Position = vec4(in_pos, 0.0, 1.0);
	frag_color = vec3(in_pos, 1.0);
}
