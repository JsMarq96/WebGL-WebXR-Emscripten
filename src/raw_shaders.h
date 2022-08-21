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

uniform mat4 u_vp_mat;
uniform mat4 u_model_mat;

void main() {
    vec4 world_pos = u_model_mat * vec4(a_pos, 1.0);
    v_world_position = world_pos.xyz;
    v_uv = a_uv;
    gl_Position = u_vp_mat * world_pos;
}
)";


const char basic_fragment[] = R"(#version 300 es
precision highp float;

in vec2 v_uv;
in vec3 v_world_position;

out vec4 o_frag_color;

uniform vec3 u_camera_eye;
uniform mediump sampler3D u_volume_map;

const int MAX_ITERATIONS = 2000;
const float STEP_SIZE = 0.0005;

vec4 render_volume() {
   vec3 ray_dir = normalize(u_camera_eye - v_world_position);
   vec3 it_pos = vec3(0.0);
   vec4 final_color = vec4(0.0);

   for(int i = 0; i < MAX_ITERATIONS; i++) {
      vec3 sample_pos = ((v_world_position - it_pos) / 2.0) + 0.5;

      if (final_color.a >= 1.0) {
         break;
      }

      if (sample_pos.x < 0.0 && sample_pos.y < 0.0 && sample_pos.z < 0.0) {
         break;
      }
      if (sample_pos.x > 1.0 && sample_pos.y > 1.0 && sample_pos.z > 1.0) {
         break;
      }

      float depth = texture(u_volume_map, sample_pos).x;
      vec4 sample_color = vec4(depth, depth, depth, depth);
      final_color = final_color + sample_color;

      it_pos = it_pos + (ray_dir * STEP_SIZE);
   }
   return final_color;
}

void main() {
    o_frag_color = render_volume();
}
)";


};

#endif // RAW_SHADERS_H_
