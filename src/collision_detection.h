#ifndef COLLISION_DETECTION_H_
#define COLLISION_DETECTION_H_

#include <cstdint>
#include <cstdio>
#include <glm/glm.hpp>

#include "transform.h"

namespace COL_DET {

    inline void project_vertices_to_axis(const glm::vec3 &axis,
                                         const glm::vec3 *vertices,
                                         const uint16_t vertices_count,
                                         float *min,
                                         float *max) {
        float min_it = FLT_MAX;
        float max_it = -FLT_MAX;

        for(uint16_t i = 0; i < vertices_count; i++) {
            float facing = glm::dot(axis,
                                    vertices[0]);

            min_it = glm::min(min_it,
                              facing);
            max_it = glm::max(max_it,
                              facing);
        }

        *min = min_it;
        *max = max_it;
    }


    inline bool sphere_OBB_collision(const sTransform &OBB_transform,
                                     const glm::vec3 &sphere_center,
                                     const float sphere_radius) {
        glm::vec3 axis[3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

        glm::vec3 box_vertices[8] = {
             {1.0f, 1.0f, 1.0f},
             {1.0f, 1.0f, -1.0f},
             {1.0f, -1.0f, 1.0f},
             {1.0f, -1.0f, -1.0f},
             {-1.0f, 1.0f, 1.0f},
             {-1.0f, 1.0f, -1.0f},
             {-1.0f, -1.0f, 1.0f},
             {-1.0f, -1.0f, -1.0f},
        };

        glm::mat4 model = OBB_transform.get_model();
        // Get box verticies
        for (uint16_t i = 0; i < 8; i++) {
            box_vertices[i] = glm::vec3(model * glm::vec4(box_vertices[i], 1.0));
        }

        // SAT test
        for(uint16_t i = 0; i < 3; i++) {
            // Test each axis

            // Get box projection
            float box_max_on_axis = 0.0f, box_min_on_axis = 0.0f;

            project_vertices_to_axis(axis[i],
                                     NULL,
                                     0,
                                     &box_min_on_axis,
                                     &box_max_on_axis);

            // Get sphere projection
            float sphere_center_on_axis = glm::dot(sphere_center,
                                                   axis[i]);

            float sphere_max_on_axis = sphere_center_on_axis + sphere_radius;
            float sphere_min_on_axis = sphere_center_on_axis - sphere_radius;

            // Check overlap
            float max_range = glm::max(box_max_on_axis, sphere_max_on_axis);
            float min_range = glm::min(box_min_on_axis, sphere_min_on_axis);

            float total_len = (box_max_on_axis - box_min_on_axis) + (sphere_max_on_axis - sphere_min_on_axis);

            // Early out
            if ((max_range - min_range) > total_len) {
                return false;
            }
        }

        return true;
    }

};

#endif // COLLISION_DETECTION_H_
