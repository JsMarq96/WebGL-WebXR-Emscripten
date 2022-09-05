#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <glm/glm.hpp>
#include "transform.h"

namespace Application {
    enum eControllers : uint8_t {
        LEFT_HAND = 0,
        RIGHT_HAND,
        TOTAL_CONTROLLER_COUNT
    };

    struct sState {
        // Controller state
        bool enabled_controllers[TOTAL_CONTROLLER_COUNT];
        glm::mat4x4 controller_model[TOTAL_CONTROLLER_COUNT];
        // TODO add controller key state


        void get_current_state();
    };
}

#endif // APPLICATION_H_
