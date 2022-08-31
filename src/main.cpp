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

#include "glm/ext/matrix_float4x4.hpp"
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
    //Render::sInstance *render = (Render::sInstance*) user_data;
    glm::mat4x4 view;
    glm::mat4x4 view_proj;
    renderer.base_framebuffer = framebuffer_id;

    for(uint16_t i = 0; i < 2; i++) {
        view = glm::make_mat4(views[i].viewPose.matrix);
        view_proj = glm::make_mat4(views[i].projectionMatrix) * view;

        glViewport(views[i].viewport[0],
                   views[i].viewport[1],
                   views[i].viewport[2],
                   views[i].viewport[3]);

        renderer.render_frame(view_proj,
                              glm::vec3{2.0f, 0.50f, 2.0f},
                              views[i].viewport[2],
                              views[i].viewport[3]);

        std::cout << "Frame " << std::endl;
    }
}

void init_XR_session(void* userData,
                     int mode) {
    std::cout << "Init" << std::endl;
    webxr_set_projection_params(0.01f, 100.f);
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
    uint8_t first_render_pass = renderer.add_render_pass(Render::FBO_TARGET,
                                                         first_pass_fbo_id);

    uint8_t second_render_pass = renderer.add_render_pass(Render::SCREEN_TARGET,
                                                          0);

    renderer.render_passes[second_render_pass].use_prev_color_attachment = true;
    renderer.render_passes[second_render_pass].color_attachment_pass_id = first_render_pass;

    // Set the clear value for the first pass
    renderer.render_passes[first_render_pass].rgba_clear_values[0] = 2.0f;
    renderer.render_passes[first_render_pass].rgba_clear_values[1] = 2.0f;
    renderer.render_passes[first_render_pass].rgba_clear_values[2] = 2.0f;
    renderer.render_passes[first_render_pass].rgba_clear_values[3] = 2.0f;

    renderer.render_passes[second_render_pass].rgba_clear_values[0] = 1.0f;
    renderer.render_passes[second_render_pass].rgba_clear_values[1] = 0.0f;
    renderer.render_passes[first_render_pass].rgba_clear_values[2] = 2.0f;
    renderer.render_passes[first_render_pass].rgba_clear_values[3] = 2.0f;


    renderer.add_drawcall_to_pass(first_render_pass, {
        .mesh_id = cube_mesh_id,
        .material_id = first_pass_material_id,
        .call_state = {
            .culling_mode = GL_FRONT
        }
    });

    renderer.add_drawcall_to_pass(second_render_pass, {
        .mesh_id = cube_mesh_id,
        .material_id = volume_material
    });

    //emscripten_set_main_loop(render_frame, 0, 0);

    int i;
    webxr_init( render_stereoscopic_frame,
                     init_XR_session,
                     close_XR_session,
                     [](void* userData, int error) {
                         std::cout << "ERROR " << error << std::endl;
                     },
                     &renderer);

}
