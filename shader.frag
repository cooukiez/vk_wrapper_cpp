#version 450

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 out_col;

void main() {
    // out_col = texture(tex_sampler, uv);
    out_col = vec4(uv, 1, 1);
}
