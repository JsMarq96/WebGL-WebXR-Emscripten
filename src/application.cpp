#include "application.h"

#include <webxr.h>
#include "glm/ext/matrix_float4x4.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

void Application::sState::get_current_state() {
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

        controller_model[controllers[i].handedness] = glm::make_mat4(transf.matrix);
    }

    // TODO: controller inputs
}