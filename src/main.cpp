#include <GLES3/gl3.h>
#include <cstdlib>
#include <emscripten/em_asm.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/html5.h>
#include <webgl/webgl2.h>
#include <webxr.h>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <stdio.h>
#include <stdlib.h>

#include <glm/gtx/string_cast.hpp>
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/gtx/dual_quaternion.hpp"
#include "material.h"
#include "raw_meshes.h"
#include "raw_shaders.h"
#include "render.h"
#include "application.h"
#include "transform.h"

#include <iostream>

#define XR_ENABLE true


Render::sInstance renderer;
Application::sState app_state;

uint8_t first_render_pass = 0;

void render_stereoscopic_frame(void *user_data,
                               int framebuffer_id,
                               int time,
                               WebXRRigidTransform *head_pose,
                               WebXRView views[2],
                               int view_count) {
    // Update
    app_state.get_current_state();


    if (app_state.enabled_controllers[Application::LEFT_HAND]) {
        renderer.use_drawcall(app_state.final_render_pass_id,
                              app_state.left_controller_drawcall_id,
                              true);
        sTransform *controller_transf = renderer.get_transform_of_drawcall(app_state.final_render_pass_id,
                                                                           app_state.left_controller_drawcall_id);
        controller_transf->position = app_state.controller_position[Application::LEFT_HAND];

        //std::cout << app_state.controller_position[Application::LEFT_HAND][0] << " " << app_state.controller_position[Application::LEFT_HAND][1] << std::endl;
    } else {
        renderer.use_drawcall(app_state.final_render_pass_id,
                              app_state.left_controller_drawcall_id,
                              false);
    }

    /*if (app_state.enabled_controllers[Application::RIGHT_HAND]) {
        renderer.use_drawcall(app_state.final_render_pass_id,
                              app_state.right_controller_drawcall_id,
                              true);
        sTransform &controller_transf = renderer.get_transform_of_drawcall(app_state.final_render_pass_id,
                                                                           app_state.right_controller_drawcall_id);
        controller_transf.position = app_state.controller_position[Application::RIGHT_HAND];
    } else {
        renderer.use_drawcall(app_state.final_render_pass_id,
                              app_state.right_controller_drawcall_id,
                              false);
    }*/


    glm::mat4x4 view;
    glm::vec3 eye_pos;
    glm::mat4x4 view_proj;
    renderer.base_framebuffer = framebuffer_id;

    //glm::mat4 headest_model = (glm::make_mat4(head_pose[0].matrix));

    for(uint16_t i = 0; i < 2; i++) {
        view = glm::inverse(glm::make_mat4(views[i].viewPose.matrix));

        view_proj = (glm::make_mat4(views[i].projectionMatrix)) * view;

        glViewport((int)views[i].viewport[0],
                   (int)views[i].viewport[1],
                   (int)views[i].viewport[2],
                   (int)views[i].viewport[3]);

        eye_pos = glm::vec3(glm::inverse(view)[3]);

        renderer.render_frame(view_proj,
                              eye_pos,
                              views[i].viewport[2],
                              views[i].viewport[3],
                              i == 0);
    }
}

void init_XR_session(void* userData,
                     int mode) {
    std::cout << "Init" << std::endl;
    webxr_set_projection_params(0.01f, 10000.f);
}

void close_XR_session(void* userData,
                      int mode) {

}


void render_frame() {
    int width, height, lef, right;
    emscripten_get_canvas_element_size("#canvas", &width, &height);

    glm::mat4x4 persp;
    glm::mat4x4 view_mat = glm::lookAt(glm::vec3{2.0f, 0.50f, 2.0f},
                                       glm::vec3{0.0f, 0.0f ,0.0f},
                                       glm::vec3{0.0f, 1.0f, 0.0f});


    glm::mat4x4 view_proj_mat = glm::perspective(glm::radians(90.0f),
                                                 (float) ((float) width / height),
                                                 0.1f,
                                                 10000.0f) * view_mat;

    renderer.base_framebuffer = 0;

    glViewport(0, 0, width, height);

    renderer.render_frame(view_proj_mat,
                          glm::vec3{2.0f, 0.50f, 2.0f},
                          width,
                          height, true);
}

void xr_session_start(void *user_data,
                      int mode) {
    std::cout << mode << "start" << std::endl;
}

void xr_session_end(void *user_data,
                      int mode) {
    //
}



void xr_error(void* user_data, int error) {
    std::cout << "Error " << error << std::endl;
}

void xr_supported_session_callback(int mode, int supported) {
    std::cout << mode << " " << supported << std::endl;

    EM_ASM(alert($0), supported);
}

int main() {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 3;
    attrs.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK;
    attrs.renderViaOffscreenBackBuffer = EM_TRUE;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(context);

    //webxr_is_session_supported(WEBXR_SESSION_MODE_IMMERSIVE_VR, );


    // Test emscripten_webgl_get_supported_extensions() API
    char *extensions = emscripten_webgl_get_supported_extensions();
    assert(extensions);
    assert(strlen(extensions) > 0);
    assert(strstr(extensions, "WEBGL") != 0);
    std::cout << extensions << "Loaded extensions" << std::endl;
    free(extensions);

    // Init the XR runtime
    webxr_init(render_stereoscopic_frame,
               xr_session_start,
               xr_session_end,
               xr_error,
               NULL);

    renderer.init();

    // Loading assets
    const uint8_t cube_mesh_id = renderer.get_new_mesh_id();
    renderer.meshes[cube_mesh_id].init_with_triangles(RawMesh::cube_geometry,
                                           sizeof(RawMesh::cube_geometry),
                                           RawMesh::cube_indices,
                                           sizeof(RawMesh::cube_indices));

    const uint8_t first_pass_material_id = renderer.get_new_material_id();
    renderer.materials[first_pass_material_id].add_raw_shader(RawShaders::basic_vertex,
                                                              RawShaders::local_fragment);

    const uint8_t volume_material = renderer.get_new_material_id();
    renderer.materials[volume_material].add_raw_shader(RawShaders::basic_vertex,
                                                       RawShaders::volumetric_fragment);

    const uint8_t basic_material = renderer.get_new_material_id();
    renderer.materials[basic_material].add_raw_shader(RawShaders::basic_vertex,
                                                      RawShaders::basic_fragment);

    renderer.materials[volume_material].load_async_texture3D("resources/volumes/bonsai_256x256x256_uint8.raw",
                                                             256,
                                                             256,
                                                             256);

    // Create render pipeline
    //uint8_t first_pass_fbo_id = renderer.get_new_fbo_id();

    app_state.final_render_pass_id = renderer.add_render_pass(Render::SCREEN_TARGET,
                                                              0);


    //renderer.transforms[cube_transform_id] = {.position = , .scale = {0.25f, 0.25f, 0.25f}};

    app_state.volumetric_drawcall_id = renderer.add_drawcall_to_pass(app_state.final_render_pass_id,
                                                                     {
                                                                     .transform_id = renderer.get_new_transform(),
                                                                     .mesh_id = cube_mesh_id,
                                                                     .material_id = volume_material,
                                                                     .call_state = {.culling_enabled = false}
                                                                 });
    sTransform *vol_transf = renderer.get_transform_of_drawcall(app_state.final_render_pass_id,
                                                               app_state.volumetric_drawcall_id);

    //vol_transf->position = glm::vec3(0.0f, 4.0f, -3.0f);
    //vol_transf->scale = {0.25f, 0.25f, 0.25f};

    // COntroller drawcall
    app_state.left_controller_drawcall_id = renderer.add_drawcall_to_pass(app_state.final_render_pass_id,
                                                                          {
                                                                              .transform_id = renderer.get_new_transform(),
                                                                              .mesh_id = cube_mesh_id,
                                                                              .material_id = basic_material,
                                                                              .enabled = false
                                                                          });

    app_state.right_controller_drawcall_id = renderer.add_drawcall_to_pass(app_state.final_render_pass_id,
                                                                          {
                                                                              .transform_id = renderer.get_new_transform(),
                                                                              .mesh_id = cube_mesh_id,
                                                                              .material_id = basic_material,
                                                                              .enabled = false
                                                                          });

    renderer.get_transform_of_drawcall(app_state.final_render_pass_id,
                                       app_state.left_controller_drawcall_id)->scale = {0.05f, 0.05f, 0.05f};

    renderer.get_transform_of_drawcall(app_state.final_render_pass_id,
                                       app_state.right_controller_drawcall_id)->scale = {0.05f, 0.05f, 0.05f};
}
