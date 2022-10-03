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
uniform highp usampler3D u_volume_map;
uniform highp usampler3D u_volume_octree;
uniform mat4 u_model_mat;

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

const float EPSILON = 0.001;

uint get_octant_of_pos(in vec3 pos, out vec3 octant_center_delta) {
  uint index = 0u;
  octant_center_delta = vec3(0.0);
  if (pos.x > EPSILON) {
    index += 1u;
    octant_center_delta.x = 1.0;
  } else {
    octant_center_delta.x = -1.0;
  }
  if (pos.y > EPSILON) {
   index += 2u;
   octant_center_delta.y = 1.0;
  }  else {
    octant_center_delta.y = -1.0;
  }

  if (pos.z > EPSILON) {
    index += 4u;
    octant_center_delta.z = 1.0;
  } else {
    octant_center_delta.z = -1.0;
  }

  return index;
}

uint get_next_node(in uint octant, in uvec4 octree_node) {
	uint next_node_index = 0u;
	switch(octant) {
		case 0u: next_node_index = octree_node.g; break;
		case 1u: next_node_index = octree_node.b; break;
		case 2u: next_node_index = octree_node.a; break;
		case 3u: next_node_index = octree_node.r; break;
		case 4u: next_node_index = octree_node.g; break;
		case 5u: next_node_index = octree_node.b; break;
		case 6u: next_node_index = octree_node.a; break;
		case 7u: next_node_index = octree_node.r; break;
	}

	return next_node_index;
}

const uint width = 7u; // 386u;

ivec3 get_texel_coords_of_index(in uint index) {
    uint index_n = index;
	return ivec3(uvec3(index_n % width, (index_n / width) % width, index_n / (width * width)));
}

uint get_index_of_texel_coords(in ivec3 coords) {
	uvec3 coord = uvec3(coords);
	return (coord.x + (coord.y * width) + coord.z * (width * width));
}

ivec3 get_child_index_at_octant(in uvec4 curr_node, in ivec3 coords, in uint octant) {
	if (octant <= 2u) {
		return get_texel_coords_of_index(get_next_node(octant, curr_node));
	}
	uint child_index = 0u;
	// Selec the important texel
	if (octant > 2u && octant <= 6u) {
       // Second texel
	   child_index += 1u;
	} else if (octant > 6u) {
	   // Third texel, child 7
	   child_index += 2u;
	}

    ivec3 texel_index = get_texel_coords_of_index(get_index_of_texel_coords(coords) + child_index);
    uvec4 octree_node = texelFetch(u_volume_octree, texel_index, 0);

    return get_texel_coords_of_index(get_next_node(octant, octree_node));
}


vec3 iterate_octree(in vec3 ray_dir, in vec3 ray_origin, in vec3 box_origin, in vec3 box_size) {
   vec3 near, far;
   ray_AABB_intersection(ray_origin, ray_dir, box_origin, box_size, near, far);

   vec3 box_center = (box_size/ 2.0);
   vec3 octant_relative_center = vec3(0.0);
   ivec3 node_coords = ivec3(0,0,0);

   uvec4 octree_node = texelFetch(u_volume_octree, node_coords, 0);
   uint octant = get_octant_of_pos(near - box_center, octant_relative_center);

   uvec4 index = texelFetch(u_volume_octree, get_child_index_at_octant(octree_node, node_coords, octant), 0);

   return vec3(index.r / 2u) / 20.0;

   return vec3(0.0, 0.0, 1.0);

}

// TODO: test this in object space  and test to center the cube
void main() {
   vec3 ray_origin = u_camera_eye_local; //(u_model_mat *  vec4(u_camera_eye_local, 1.0)).rgb;
   vec3 ray_dir = normalize(v_local_position - ray_origin);
   vec3 near, far, box_origin = vec3(0.0, 0.0, 0.0), box_size = vec3(1.0);

    //o_frag_color = vec4(texture(u_volume_octree, vec3(v_uv,0.0)));
   o_frag_color = vec4(iterate_octree(ray_dir, ray_origin, box_origin, box_size).rgb, 1.0);
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
