#version 450

layout (binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;

layout (location = 1) out vec2 uv;

#define NEW

mat4 x = mat4(
    vec4(1.0, 0.0, 0.0, 0.0),
    vec4(0.0, -1.0, 0.0, 0.0),
    vec4(0.0, 0.0, -1.0, 0.0),
    vec4(0.0, 0.0, 0.0, 1.0)
);

void main() {
    #ifdef NEW
    gl_Position = ubo.view * x * vec4(in_pos, 1.0);
    #else
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_pos, 1.0);
    #endif
    uv = in_uv;
}
