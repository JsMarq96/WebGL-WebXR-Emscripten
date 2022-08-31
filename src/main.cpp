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
#include "glm/gtx/dual_quaternion.hpp"
#include "material.h"
#include "raw_meshes.h"
#include "raw_shaders.h"
#include "render.h"


#include <iostream>

#define XR_ENABLE true

Render::sInstance renderer;


void render_stereoscopic_frame(void *user_data,
                               int framebuffer_id,
                               int time,
                               WebXRRigidTransform *head_pose,
                               WebXRView views[2],
                               int view_count) {
    glm::mat4x4 view;
    glm::mat4x4 view_proj;
    renderer.base_framebuffer = framebuffer_id;

    glm::mat4x4 view_mat = glm::lookAt(glm::vec3{2.0f, 0.50f, 2.0f},
                                       glm::vec3{0.0f, 0.0f ,0.0f},
                                       glm::vec3{0.0f, 1.0f, 0.0f});

   // std::cout << glm::to_string(view_mat) << " <- perfect" << std::endl;


    glm::mat4 headest_model = glm::make_mat4(head_pose[0].matrix);

    float col[2] = {0.0, 1.0f};

    for(uint16_t i = 0; i < 2; i++) {
        view = headest_model * glm::inverse(glm::make_mat4(views[i].viewPose.matrix));
        view = glm::translate(view, glm::vec3(0.0, -5.5, 0.0));
        std::cout << glm::to_string(view) << " <- other" << std::endl;

        view_proj = glm::make_mat4(views[i].projectionMatrix) * view;

        //renderer.current_state.set_default();

        glViewport((int)views[i].viewport[0],
                   (int)views[i].viewport[1],
                   (int)views[i].viewport[2],
                   (int)views[i].viewport[3]);

            /*glClearColor(col[i], 0.0, 0.0, 1.0);
                std::cout << i << std::endl;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

         renderer.render_frame(view_proj,
                          glm::vec3{2.0f, 0.50f, 2.0f},
                          views[i].viewport[2],
                          views[i].viewport[3]);

        //std::cout << "Frame "  << views[i].viewport[0] << " " << views[i].viewport[1] << " " << views[i].viewport[2] << " "  << views[i].viewport[3] << " "<< std::endl;
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

    float mat[] = {0.992153, 0.046731, -0.115965, 0.000000,-0.113348, 0.727629, -0.676541, 0.000000,0.052764, 0.684377, 0.727217, 0.000000,0.007016, -0.003467, 0.001067, 1.000000};
    //view_proj_mat = glm::make_mat4(mat) * view_mat;


    glm::mat4x4 view_proj_mat = glm::perspective(glm::radians(90.0f),
                                                 (float) ((float) width / height),
                                                 0.1f,
                                                 10000.0f) * (glm::make_mat4(mat));

        renderer.base_framebuffer = 0;

    glViewport(0, 0, width, height);

    renderer.render_frame(view_proj_mat,
                          glm::vec3{2.0f, 0.50f, 2.0f},
                          width,
                          height);
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

    renderer.materials[volume_material].load_async_texture3D("resources/volumes/bonsai_256x256x256_uint8.raw",
                                                             256,
                                                             256,
                                                             256);

    // Create render pipeline
    uint8_t first_pass_fbo_id = renderer.get_new_fbo_id();

    uint8_t first_render_pass = renderer.add_render_pass(Render::SCREEN_TARGET,
                                                          0);
    renderer.add_drawcall_to_pass(first_render_pass, {
        .mesh_id = cube_mesh_id,
        .material_id = volume_material,
        .call_state = {.culling_enabled = false}
    });

    //renderer.render_passes[first_render_pass].draw_stack[0].transform.position = glm::vec3(0.0, 5.0f, 0.0f);


    //emscripten_set_main_loop(render_frame, 0, 0);

    int i;
}
