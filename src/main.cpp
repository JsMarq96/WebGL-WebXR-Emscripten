#include <cstdlib>
#include <emscripten/emscripten.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/html5.h>
#include <webgl/webgl2.h>
#include <webxr.h>

#include <cassert>
#include <stdio.h>
#include <stdlib.h>

#include "raw_meshes.h"
#include "raw_shaders.h"
#include "render.h"


#include <iostream>


Render::sInstance renderer;

void render_frame() {
    glm::mat4x4 persp;
    glm::mat4x4 view_mat = glm::lookAt(glm::vec3{2.0f, 3.0f, 1.0f},
                                       glm::vec3{0.0f, 0.0f ,0.0f},
                                       glm::vec3{0.0f, 1.0f, 0.0f});

    renderer.render_frame(view_mat);
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

    renderer.meshes[0].init_with_triangles(RawMesh::cube_geometry,
                                           sizeof(RawMesh::cube_geometry),
                                           RawMesh::cube_indices,
                                           sizeof(RawMesh::cube_indices));
    renderer.meshes_count++;


    renderer.shaders[0].load_shaders(RawShaders::basic_vertex,
                                     RawShaders::basic_fragment);
    renderer.shader_count++;

    renderer.draw_stack[0] = {
        .mesh_id = 0,
        .shader_id = 0
    };

    renderer.draw_stack_size++;

    emscripten_set_main_loop(render_frame, 0, 0);
}
