#include "render.h"

#include <emscripten/emscripten.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/html5.h>
#include <webgl/webgl2.h>


void Render::render_frame() {
    int width, height, lef, right;
    emscripten_get_canvas_element_size("#canvas", &width, &height);

    glViewport(0, 0, width, height);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    //glUseProgram(shader_program);
    //glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    glDrawArrays(GL_TRIANGLES, 0, 3);

}
