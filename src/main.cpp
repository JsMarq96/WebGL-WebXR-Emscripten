#include <GLES3/gl3.h>
#include <cstdlib>
#include <emscripten/emscripten.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/html5.h>
#include <webgl/webgl2.h>
#include <webxr.h>

#include <cassert>
#include <stdio.h>
#include <stdlib.h>

#include "material.h"
#include "raw_meshes.h"
#include "raw_shaders.h"
#include "render.h"


#include <iostream>


Render::sInstance renderer;

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

    renderer.render_frame(view_proj_mat, glm::vec3{2.0f, 0.50f, 2.0f});
}


int main() {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 3;
    attrs.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK;
    attrs.renderViaOffscreenBackBuffer = EM_TRUE;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(context);

    // Test emscripten_webgl_get_supported_extensions() API
    char *extensions = emscripten_webgl_get_supported_extensions();
    assert(extensions);
    assert(strlen(extensions) > 0);
    assert(strstr(extensions, "WEBGL") != 0);
    free(extensions);

    renderer.init();

    const uint8_t cube_mesh_id = 0;
    renderer.meshes[0].init_with_triangles(RawMesh::cube_geometry,
                                           sizeof(RawMesh::cube_geometry),
                                           RawMesh::cube_indices,
                                           sizeof(RawMesh::cube_indices));
    renderer.meshes_count++;

    const uint8_t basic_material_id = 0, volume_material = 1;
    renderer.materials[volume_material].add_raw_shader(RawShaders::basic_vertex,
                                                       RawShaders::basic_fragment);
    renderer.material_count += 2;

    /*renderer.materials[volume_material].add_volume_texture("/volumes/teapot_16_16.png",
                                                           16,
                                                           16,
                                                           16);*/
    renderer.materials[volume_material].load_async_texture3D("http://localhost:6931/resources/volumes/bonsai_256x256x256_uint8.raw",
                                                             256,
                                                             256,
                                                             256);

    renderer.draw_stack[0] = {
        .mesh_id = cube_mesh_id,
        .material_id = volume_material
    };
    renderer.draw_stack[0].call_state.set_default();

    //glDisable(GL_CULL_FACE);

    renderer.draw_stack_size++;

    emscripten_set_main_loop(render_frame, 0, 0);
}
