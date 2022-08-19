#include "render.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_projection.hpp"

#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/html5.h>
#include <webgl/webgl2.h>

void Render::sMeshBuffers::init_with_triangles(const float *geometry,
                                               const uint32_t geometry_size,
                                               const uint16_t *indices,
                                               const uint32_t indices_size) {
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, geometry_size, geometry, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Load vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);

    // UV coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (sizeof(float) * 3));

    // Vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (sizeof(float) * 5));

    // Load vertex indexing
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    primitive = GL_TRIANGLES;
    primitive_count = (uint16_t) (indices_size / sizeof(uint16_t));
    is_indexed = true;
}


#include <iostream>
void Render::sInstance::render_frame(const glm::mat4x4 &view_mat) {
    int width, height, lef, right;
    emscripten_get_canvas_element_size("#canvas", &width, &height);

    std::cout << width << " " << height << std::endl;

    glViewport(0, 0, width, height);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4x4 view_proj_mat = glm::perspective(glm::radians(90.0f),
                                                 (float) ((float) width / height),
                                                 0.1f,
                                                 10000.0f) * view_mat;
    //view_proj_mat = view_mat * view_proj_mat;

    glm::mat4x4 model;
    for(uint16_t i = 0; i < draw_stack_size; i++) {
        sDrawCall &draw_call = draw_stack[i];
        sShader &shader = shaders[draw_call.shader_id];
        sMeshBuffers &mesh = meshes[draw_call.mesh_id];

        model = draw_call.transform.get_model();

        shader.activate();

        glBindVertexArray(mesh.VAO);

        shader.set_uniform_matrix4("u_model_mat", model);
        shader.set_uniform_matrix4("u_vp_mat", view_proj_mat);

        if (mesh.is_indexed) {
            glDrawElements(mesh.primitive, mesh.primitive_count, GL_UNSIGNED_SHORT, 0);
        } else {
            glDrawArrays(mesh.primitive, 0, mesh.primitive_count);
        }

        shader.deactivate();
    }
    //glUseProgram(shader_program);
    //glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

}
