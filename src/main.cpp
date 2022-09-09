#include <GLES3/gl3.h>
#include <cstdint>
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
#include "collision_detection.h"

#include <iostream>

#define XR_ENABLE true


Render::sInstance renderer;
Application::sInstance app_state;

uint8_t first_render_pass = 0;

void render_stereoscopic_frame(void *user_data,
                               int framebuffer_id,
                               int time,
                               WebXRRigidTransform *head_pose,
                               WebXRView views[2],
                               int view_count) {
    // Update
    app_state.get_current_state();


    // Controller UI
    for(uint8_t controller_index = 0; controller_index < 2; controller_index++) {
        if (app_state.enabled_controllers[controller_index]) {
            renderer.use_drawcall(app_state.final_render_pass_id,
                                  app_state.controller_drawcalls[controller_index],
                                  true);
            sTransform controller_transf;

            controller_transf.position = app_state.controller_position[controller_index];
            controller_transf.rotation = app_state.controller_rotation[controller_index];
            controller_transf.scale = {0.025f, 0.025f, 0.025f};

            renderer.set_transform_of_drawcall(app_state.final_render_pass_id,
                                               app_state.controller_drawcalls[controller_index],
                                               controller_transf);
        } else {
            renderer.use_drawcall(app_state.final_render_pass_id,
                                  app_state.controller_drawcalls[controller_index],
                                  false);
        }

    }

    glm::mat4x4 view_proj;
    renderer.base_framebuffer = framebuffer_id;

    // Get view and eye positions
    glm::mat4 view_mats[2];
    glm::vec3 eye_poses[2];
    for (uint16_t i = 0; i < 2; i++) {
        view_mats[i] = glm::inverse(glm::make_mat4(views[i].viewPose.matrix));
        eye_poses[i] = glm::vec3(glm::inverse(view_mats[i])[3]);
    }

    // Test head inside volume
    Render::sDrawCall *volume_draw_call = renderer.get_draw_call(app_state.final_render_pass_id,
                                                                 app_state.volumetric_drawcall_id);
    if (COL_DET::sphere_OBB_collision(volume_draw_call->transform,
                                      eye_poses[0],
                                      0.10f)) {
        // Render backface
        volume_draw_call->call_state.culling_mode = GL_FRONT;
        volume_draw_call->material_id = app_state.volume_material_inside;
    } else {
        volume_draw_call->call_state.culling_mode = GL_BACK;
        volume_draw_call->material_id = app_state.volume_material_outside;
    }


    for(uint16_t i = 0; i < 2; i++) {
        view_proj = (glm::make_mat4(views[i].projectionMatrix)) * view_mats[i];

        glViewport((int)views[i].viewport[0],
                   (int)views[i].viewport[1],
                   (int)views[i].viewport[2],
                   (int)views[i].viewport[3]);

        renderer.render_frame(view_proj,
                              eye_poses[i],
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

    renderer.base_framebuffer = 0;

    glViewport(0, 0, width, height);

    glm::vec3 eye_pos = glm::vec3{0.10f, 1.00f, 0.10f};

     Render::sDrawCall *volume_draw_call = renderer.get_draw_call(app_state.final_render_pass_id,
                                                                 app_state.volumetric_drawcall_id);

    if (COL_DET::sphere_OBB_collision(volume_draw_call->transform,
                                      eye_pos,
                                      0.10f)) {
        // Render backface
        volume_draw_call->call_state.culling_mode = GL_FRONT;
        volume_draw_call->material_id = app_state.volume_material_inside;
    } else {
        volume_draw_call->call_state.culling_mode = GL_BACK;
        volume_draw_call->material_id = app_state.volume_material_outside;
    }

    glm::mat4x4 persp;
    glm::mat4x4 view_mat = glm::lookAt(eye_pos,
                                       volume_draw_call->transform.position,
                                       glm::vec3{0.0f, 1.0f, 0.0f});


    glm::mat4x4 view_proj_mat = glm::perspective(glm::radians(90.0f),
                                                 (float) ((float) width / height),
                                                 0.1f,
                                                 10000.0f) * view_mat;


    renderer.render_frame(view_proj_mat,
                          eye_pos,
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

    // Create shaders
    const uint8_t inside_volume_shader = renderer.material_man.add_raw_shader(RawShaders::basic_vertex,
                                                                              RawShaders::volumetric_fragment);

    const uint8_t outside_volume_shader = renderer.material_man.add_raw_shader(RawShaders::basic_vertex,
                                                                            RawShaders::volumetric_fragment_outside);

    const uint8_t plaincolor_shaders = renderer.material_man.add_raw_shader(RawShaders::basic_vertex,
                                                                         RawShaders::basic_fragment);

    // Load textures
    const uint8_t volumetric_texture = renderer.material_man.load_async_texture3D("resources/volumes/bonsai_256x256x256_uint8.raw",
                                                                               256,
                                                                               256,
                                                                               256);

    // Define the materials with the prevousle loaded resources
    app_state.volume_material_inside = renderer.material_man.add_material(inside_volume_shader,
                                                                       {  .volume_tex = volumetric_texture,
                                                                          .enabled_volume = true});
    app_state.volume_material_outside = renderer.material_man.add_material(outside_volume_shader,
                                                                       {  .volume_tex = volumetric_texture,
                                                                          .enabled_volume = true});

    const uint8_t basic_material = renderer.material_man.add_material(plaincolor_shaders,
                                                                   {});

    // Create render pipeline
    uint8_t first_pass_fbo_id = renderer.get_new_fbo_id();

    app_state.final_render_pass_id = renderer.add_render_pass(Render::SCREEN_TARGET,
                                                              0);

    sTransform vol_transf;

    vol_transf.position = glm::vec3(0.0f, 1.0f, -0.60f);
    vol_transf.scale = {0.25f, 0.25f, 0.25f};

    // First pass, for the volumes
    app_state.volumetric_drawcall_id = renderer.add_drawcall_to_pass(app_state.final_render_pass_id,
                                                                     {
                                                                     .mesh_id = cube_mesh_id,
                                                                     .material_id = app_state.volume_material_outside,
                                                                     .transform = vol_transf,
                                                                     .call_state = {.culling_enabled = false}
                                                                 });



    // COntroller drawcall
    app_state.controller_drawcalls[Application::RIGHT_HAND] = renderer.add_drawcall_to_pass(app_state.final_render_pass_id,
                                                                          {
                                                                              .mesh_id = cube_mesh_id,
                                                                              .material_id = basic_material,
                                                                              .call_state = {.depth_function = GL_ALWAYS},
                                                                              .enabled = false
                                                                          });

    app_state.controller_drawcalls[Application::LEFT_HAND] = renderer.add_drawcall_to_pass(app_state.final_render_pass_id,
                                                                           {
                                                                              .mesh_id = cube_mesh_id,
                                                                              .material_id = basic_material,
                                                                              .call_state = {.depth_function = GL_ALWAYS},
                                                                              .enabled = false
                                                                          });

    emscripten_set_main_loop(render_frame,
                             0,
                             0);
}
