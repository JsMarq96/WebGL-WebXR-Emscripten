#ifndef RAW_SHADERS_H_
#define RAW_SHADERS_H_

namespace RawShaders {

    /// Basic sahders
const char basic_vertex[] = R"(#version 300 es
in  vec3 a_pos;
in  vec2 a_uv;
in vec3 a_normal;

void main() {
    gl_Position = vec4(a_pos, 1.0);
}
)";


const char basic_fragment[] = R"(#version 300 es
precision highp float;

out vec4 o_frag_color;

void main() {
    o_frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}
)";


};

#endif // RAW_SHADERS_H_
