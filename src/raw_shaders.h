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
out vec2 v_screen_position;

uniform mat4 u_vp_mat;
uniform mat4 u_model_mat;

void main() {
    vec4 world_pos = u_model_mat * vec4(a_pos, 1.0);
    v_world_position = world_pos.xyz;
    v_local_position = a_pos;
    v_uv = a_uv;
    gl_Position =  u_vp_mat * world_pos;
    v_screen_position = ((gl_Position.xy / gl_Position.w) + 1.0) / 2.0;
}
)";


const char volumetric_fragment[] = R"(#version 300 es
precision highp float;

in vec2 v_uv;
in vec3 v_world_position;
in vec3 v_local_position;
in vec2 v_screen_position;

out vec4 o_frag_color;

uniform vec3 u_camera_eye_local;
uniform highp sampler3D u_volume_map;
uniform highp sampler2D u_frame_color_attachment;

const int MAX_ITERATIONS = 100;
const float STEP_SIZE = 0.02;

vec4 render_volume() {
   vec3 ray_dir = -normalize(u_camera_eye_local - v_local_position);
   vec3 it_pos = vec3(0.0);
   vec4 final_color = vec4(0.0);
   float ray_step = 1.0 / float(MAX_ITERATIONS);

   // TODO: optimize iterations size, and step size
   for(int i = 0; i < MAX_ITERATIONS; i++) {
      if (final_color.a >= 0.95) {
         break;
      }
      vec3 sample_pos = ((u_camera_eye_local - it_pos) / 2.0) + 0.5;

      // Aboid clipping outside
      if (sample_pos.x < 0.0 || sample_pos.y < 0.0 || sample_pos.z < 0.0) {
         break;
      }
      if (sample_pos.x > 1.0 || sample_pos.y > 1.0 || sample_pos.z > 1.0) {
         break;
      }

      float depth = texture(u_volume_map, sample_pos).r;
      // Increase luminosity, only on the colors
      vec4 sample_color = vec4(04.6 * depth);
      sample_color.a = depth;

      final_color = final_color + (STEP_SIZE * (1.0 - final_color.a) * sample_color);
      it_pos = it_pos - (STEP_SIZE * ray_dir);
   }

   return vec4(final_color.xyz, 1.0);
}

void main() {
   //o_frag_color = v_local_position;
   o_frag_color = render_volume(); //*
   //o_frag_color = vec4(v_local_position / 2.0 + 0.5, 1.0);
   //o_frag_color = texture(u_frame_color_attachment, v_screen_position);
}
)";


const char volumetric_fragment_outside[] = R"(#version 300 es
precision highp float;

in vec2 v_uv;
in vec3 v_world_position;
in vec3 v_local_position;
in vec2 v_screen_position;

out vec4 o_frag_color;

uniform vec3 u_camera_eye_local;
uniform highp sampler3D u_volume_map;
uniform highp sampler2D u_frame_color_attachment;

const int MAX_ITERATIONS = 100;
const float STEP_SIZE = 0.02;


void ray_AABB_intersection(in vec3 ray_origin,
                           in vec3 ray_dir,
                           in vec3 box_origin,
                           in vec3 box_size,
                           out vec3 near_intersection,
                           out vec3 far_intersection) {
    vec3 box_min = box_origin;
    vec3 box_max = box_origin + box_size;

    // Testing X axis slab
    float tx1 = (box_min.x - ray_origin.x) / ray_dir.x;
    float tx2 = (box_max.x - ray_origin.x) / ray_dir.x;

    float tmin = min(tx1, tx2);
    float tmax = max(tx1, tx2);

    // Testing Y axis slab
    float ty1 = (box_min.y - ray_origin.y) / ray_dir.y;
    float ty2 = (box_max.y - ray_origin.y) / ray_dir.y;

    tmin = max(min(ty1, ty2), tmin);
    tmax = min(max(ty1, ty2), tmax);

    // Testing Z axis slab
    float tz1 = (box_min.z - ray_origin.z) / ray_dir.z;
    float tz2 = (box_max.z - ray_origin.z) / ray_dir.z;

    tmin = max(min(tz1, tz2), tmin);
    tmax = min(max(tz1, tz2), tmax);

    near_intersection = ray_dir * tmin + ray_origin;
    far_intersection = ray_dir * tmax + ray_origin;
}

vec4 render_volume() {
   vec3 ray_dir = normalize(u_camera_eye_local - v_local_position);
   vec3 it_pos = vec3(0.0);
   vec4 final_color = vec4(0.0);
   float ray_step = 1.0 / float(MAX_ITERATIONS);

   // TODO: optimize iterations size, and step size
   int i = 0;
   for(; i < MAX_ITERATIONS; i++) {
      if (final_color.a >= 0.95) {
         break;
      }
      vec3 sample_pos = ((v_local_position - it_pos) / 2.0) + 0.5;

      // Aboid clipping outside
      if (sample_pos.x < 0.0 || sample_pos.y < 0.0 || sample_pos.z < 0.0) {
         break;
      }
      if (sample_pos.x > 1.0 || sample_pos.y > 1.0 || sample_pos.z > 1.0) {
         break;
      }

      float depth = texture(u_volume_map, sample_pos).r;
      // Increase luminosity, only on the colors
      vec4 sample_color = vec4(04.6 * depth);
      sample_color.a = depth;

      final_color = final_color + (STEP_SIZE * (1.0 - final_color.a) * sample_color);
      it_pos = it_pos + (STEP_SIZE * ray_dir);
   }

   //return vec4(vec3(i / MAX_ITERATIONS), 1.0);
   return vec4(final_color.xyz, 1.0);
}

void main() {
   o_frag_color = vec4(v_world_position.rgb, 1.0);
   //o_frag_color = render_volume(); //*
   //o_frag_color = vec4(v_local_position / 2.0 + 0.5, 1.0);
   //o_frag_color = texture(u_frame_color_attachment, v_screen_position);
    vec3 ray_dir = normalize(u_camera_eye_local - v_local_position);
    vec3 ray_origin = u_camera_eye_local;
   vec3 near, far;
   ray_AABB_intersection(ray_origin, ray_dir, vec3(0.0, 0.0, 0.0), vec3(1.0), near, far);
   o_frag_color = vec4(far, 1.0);
}
)";


const char local_fragment[] = R"(#version 300 es
precision highp float;

in vec2 v_uv;
in vec3 v_world_position;
in vec3 v_local_position;
in vec2 v_screen_position;

out vec4 o_frag_color;

void main() {
    o_frag_color = vec4(v_local_position, 1.0);
}
)";


const char basic_fragment[] = R"(#version 300 es
precision highp float;

in vec2 v_uv;
in vec3 v_world_position;
in vec3 v_local_position;
in vec2 v_screen_position;

out vec4 o_frag_color;

void main() {
    o_frag_color = vec4(0.50, 0.5, 0.50, 1.0);
}
)";

};

#endif // RAW_SHADERS_H_
