#ifndef RAW_SHADERS_H_
#define RAW_SHADERS_H_

namespace RawShaders {

    /// Basic sahders
const char basic_vertex[] = R"(#version 300 es
in  vec3 a_pos;
in  vec2 a_uv;
in  vec3 a_normal;

out vec2 v_uv;
out vec3 v_world_position;
out vec3 v_local_position;

uniform mat4 u_vp_mat;
uniform mat4 u_model_mat;

void main() {
    vec4 world_pos = u_model_mat * vec4(a_pos, 1.0);
    v_world_position = world_pos.xyz;
    v_local_position = a_pos;
    v_uv = a_uv;
    gl_Position = u_vp_mat * world_pos;
}
)";


const char basic_fragment[] = R"(#version 300 es
precision highp float;

in vec2 v_uv;
in vec3 v_world_position;
in vec3 v_local_position;

out vec4 o_frag_color;

uniform vec3 u_camera_eye_local;
uniform highp sampler3D u_volume_map;

const int MAX_ITERATIONS =100;
const float STEP_SIZE = 0.001;

vec4 render_volume() {
   vec3 ray_dir = normalize(v_local_position - u_camera_eye_local);
   vec3 it_pos = vec3(0.0);
   vec4 final_color = vec4(0.0);
   float ray_step = 1.0 / float(MAX_ITERATIONS);

   for(int i = 0; i < MAX_ITERATIONS; i++) {
      if (final_color.a >= 0.95) {
         break;
      }
      vec3 sample_pos = ((v_local_position - it_pos) / 2.0) + 0.5;

      float depth = texture(u_volume_map, sample_pos).r;
      vec4 sample_color = vec4(depth);
      final_color = final_color + (STEP_SIZE * (1.0 - final_color.a) * sample_color);

      it_pos = it_pos + (STEP_SIZE * ray_dir);
   }
   //return vec4(v_local_position, 1.0);
   //return vec4(final_color.xyz, 1.0);
   return final_color;
}

void main() {
   //o_frag_color = vec4(v_uv, 1.0, 1.0);
   o_frag_color = render_volume(); //* vec4(1.0, 0.0, 0.0, 1.0);
   //o_frag_color = vec4(textur)
}
)";


};

#endif // RAW_SHADERS_H_
