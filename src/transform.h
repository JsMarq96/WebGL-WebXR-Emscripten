#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "glm/ext/matrix_double4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>


#include <iostream>
#include <glm/gtx/string_cast.hpp>
struct sTransform {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};

    void set_identity() {
        position = glm::vec3{0.0f, 0.0f, 0.0f};
        scale = glm::vec3{1.0f, 1.0f, 1.0f};
        rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
    }

    glm::mat4x4 get_model() {
        glm::mat4x4 rot_mat = glm::mat4_cast(rotation);

        std::cout << glm::to_string(glm::scale(glm::translate(glm::mat4(1.0f), position), scale)) << std::endl;
        return glm::scale(glm::translate(glm::mat4(1.0f), position), scale);
    }
};

#endif // TRANSFORM_H_
