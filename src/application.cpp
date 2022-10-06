#include "application.h"

#ifdef __EMSCRIPTEN__
#include <webxr.h>
#endif
#include "glm/ext/matrix_float4x4.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

void Application::sInstance::get_current_state() {
#ifdef __EMSCRIPTEN__
    WebXRInputSource controllers[2];
    int controller_count = 0;

    // Get controller state and count
    webxr_get_input_sources(controllers,
                            2,
                            &controller_count);

    // Load controller position
    enabled_controllers[LEFT_HAND] = false;
    enabled_controllers[RIGHT_HAND] = false;
    WebXRRigidTransform transf;
    for(uint8_t i = 0; i < controller_count; i++) {
        webxr_get_input_pose(&controllers[i], &transf);

        // Enable the controller for this frame
        enabled_controllers[controllers[i].handedness] = true;

        float formated_quat[4] = {transf.orientation[3],
                                                                         transf.orientation[0],
                                                                         transf.orientation[1],
                                                                         transf.orientation[2]};

        controller_position[controllers[i].handedness] = glm::make_vec3(transf.position);
        controller_rotation[controllers[i].handedness] = glm::make_quat(formated_quat);
    }

    // TODO: controller inputs
#endif
}
