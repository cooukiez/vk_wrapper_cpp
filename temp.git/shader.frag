#version 450

// #define USE_UNIFORM
// #define USE_PUSH_CONSTANTS
// #define USE_SAMPLE_TEXTURE

#ifdef USE_UNIFORM
layout (binding = 0) uniform UBO {
    mat4 data;
} ubo;
#endif

#ifdef USE_PUSH_CONSTANTS
layout (push_constant) uniform PushConstants {
    mat4 view_proj;
    vec2 res;
    uint time;
} pc;
#endif

#ifdef USE_SAMPLE_TEXTURE
layout(binding = 0) uniform sampler2D tex_sampler;
#endif

layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 out_col;

void main() {
    #ifdef USE_SAMPLE_TEXTURE
    out_col = texture(tex_sampler, uv);
    #else
    out_col = vec4(uv, 1, 1);
    #endif
}
