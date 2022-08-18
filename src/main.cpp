#include <cstdlib>
#include <emscripten/emscripten.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/html5.h>
#include <webgl/webgl2.h>
#include <webxr.h>

#include <cassert>
#include <stdio.h>
#include <stdlib.h>


static const char* vertex_shader_text =
    "#version 300 es\n"
    "in lowp vec3 v_pos;\n"
    "in lowp vec3 v_color;\n"
    "out lowp vec3 i_color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(v_pos, 1.0);\n"
    "    i_color = v_color;\n"
    "}\n";
static const char* fragment_shader_text =
    "#version 300 es\n"
    "in lowp vec3 i_color;\n"
    "out lowp vec4 o_color;\n"
    "void main()\n"
    "{\n"
    "    o_color = vec4(i_color, 1.0);\n"
    "}\n";

uint32_t  shader_program;
uint32_t vertex_buffer;

#include <iostream>
void render_frame() {
	int width, height, lef, right;
	emscripten_get_canvas_element_size("#canvas", &width, &height);

	glViewport(0, 0, width, height);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shader_program);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

const float raw_vertices[] = {
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f, 0.0f,
         -1.0f, -1.0f, .0f,    0.0, 1.0f, 0.0f,
         0.0f, 1.0f, 0.0f,      0.0f, 0.0f, 1.0f
};

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

	glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(raw_vertices), raw_vertices, GL_STATIC_DRAW);

    uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*) (sizeof(float) * 3));

    emscripten_set_main_loop(render_frame, 0, 0);
}
