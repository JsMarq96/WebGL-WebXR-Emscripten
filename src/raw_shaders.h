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

int get_octant_of_pos(in vec3 pos, out vec3 octant_center_delta) {
  int index = 0;
  octant_center_delta = vec3(0.0);
  if (pos.x > EPSILON) {
    index += 1;
    octant_center_delta.x = 1.0;
  } else {
    octant_center_delta.x = -1.0;
  }
  if (pos.y > EPSILON) {
   index += 2;
   octant_center_delta.y = 1.0;
  }  else {
    octant_center_delta.y = -1.0;
  }

  if (pos.z > EPSILON) {
    index += 4;
    octant_center_delta.z = 1.0;
  } else {
    octant_center_delta.z = -1.0;
  }


  return index;
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

uint get_next_node(in int octant, in uvec4 octree_node) {
	uint next_node_index = 0u;
	switch(octant) {
		case 0: next_node_index = octree_node.g; break;
		case 1: next_node_index = octree_node.b; break;
		case 2: next_node_index = octree_node.a; break;
		case 3: next_node_index = octree_node.r; break;
		case 4: next_node_index = octree_node.g; break;
		case 5: next_node_index = octree_node.b; break;
		case 6: next_node_index = octree_node.a; break;
		case 7: next_node_index = octree_node.r; break;
	}

	return next_node_index;
}

ivec3 get_texel_coords_of_index(in uint index) {
	return ivec3(uvec3(index % 386u, (index / 386u) % 386u, index / (386u * 386u)));
}

uint get_index_of_texel_coords(in ivec3 coords) {
	uvec3 coord = uvec3(coords);
	return coord.x + (coord.y * 386u) + coord.z * (386u * 386u);
}

ivec3 get_child_index_at_octant(in uvec4 curr_node, in ivec3 coords, in int octant) {
	if (octant <= 2) {
		return get_texel_coords_of_index(get_next_node(octant, curr_node));
	}
	uint child_index = 0u;
	// Selec the important texel
	if (octant > 2 && octant <= 6) {
       // Second texel
	   child_index += 1u;
	} else if (octant > 6) {
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

   for(float i = 0.0; i < 20.0; i++) {
		if (octree_node.r == 0u) { // Full node
			return vec3(i, 0.0, 0.0);
		} else if (octree_node.r == 1u) { // Empty node
			return vec3(1.0);
		}
		// Iterate based on the intersection octant
		ray_AABB_intersection(ray_origin, ray_dir, box_center, box_size, near, far);

		vec3 octant_relative_center = vec3(0.0);
		int octant = get_octant_of_pos(near - box_center, octant_relative_center);

		box_size = box_size/ 2.0;
		box_center = box_center + octant_relative_center * (box_size / 2.0);

		node_coords = get_child_index_at_octant(octree_node, node_coords, octant);

		octree_node = texelFetch(u_volume_octree, node_coords, 0);
	}

   return vec3(0.0, 0.0, 1.0);

}

// TODO: test this in object space  and test to center the cube
void main() {
   vec3 ray_origin = u_camera_eye_local; //(u_model_mat *  vec4(u_camera_eye_local, 1.0)).rgb;
   vec3 ray_dir = normalize(v_local_position - ray_origin);
   vec3 near, far, box_origin = vec3(0.0, 0.0, 0.0), box_size = vec3(1.0);
   ray_AABB_intersection(ray_origin, ray_dir, box_origin, box_size, near, far);

   vec3 box_center = box_size / 2.0, octant_center = vec3(0.0);
   int octant = get_octant_of_pos(normalize(near - box_center), octant_center);
   vec3 it_octant_center = box_center + octant_center * (box_size / 4.0);
   int octant_of_octant = get_octant_of_pos(near - it_octant_center, octant_center);
   o_frag_color = vec4(vec3(float(octant_of_octant) / 7.0), 1.0);
   o_frag_color = vec4(0.0, 0.0, 0.0, 1.0);
   if (texelFetch(u_volume_octree, ivec3(0,0,0), 0).r != 0u) {
      o_frag_color = vec4(1.0);
    }
    o_frag_color = vec4(texelFetch(u_volume_octree, ivec3(0,0,0), 0) / 255u);
   //o_frag_color = vec4(iterate_octree(ray_dir, ray_origin, box_origin, box_size).rgb, 1.0);
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
