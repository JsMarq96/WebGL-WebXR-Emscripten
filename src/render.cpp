#include "render.h"

#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/html5.h>
#include <webgl/webgl2.h>


void Render::sInstance::render_frame(const glm::mat4x4 &view_proj_frame) {
    int width, height, lef, right;
    emscripten_get_canvas_element_size("#canvas", &width, &height);

    glViewport(0, 0, width, height);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4x4 model;
    for(uint16_t i = 0; i < draw_stack_size; i++) {
        sDrawCall &draw_call = draw_stack[i];
        sShader &shader = shaders[draw_call.shader_id];
        sMeshBuffers &mesh = meshes[draw_call.mesh_id];

        model = draw_call.transform.get_model();

        shader.activate();

        glBindVertexArray(mesh.VAO);

        shader.set_uniform_matrix4("u_model_mat", model);
        shader.set_uniform_matrix4("u_vp_mat", view_proj_frame);

        if (mesh.is_indexed) {

        } else {
            glDrawArrays(mesh.primitive, 0, mesh.primitive_count);
        }
    }
    //glUseProgram(shader_program);
    //glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

}
