#ifndef RENDER_H_
#define RENDER_H_

#include "transform.h"
#include "shader.h"
#include "material.h"

#define SHADER_TOTAL_COUNT 10
#define MESH_TOTAL_COUNT 20
#define MATERIAL_TOTAL_COUNT 30
#define DRAW_CALL_STACK_SIZE 30
/**
 * A Wrapper for the rendering backend, for now with webgl
 * TODO:
 *  1) Basic VAO, VBO functionality
 *  2) Add perspective
 *  2) Basic I/O of meshes
 *  3) Basic I/O of textures
 *  4) Scene representation
 *  4) GLTF import
 *  5) Culling (VR culling??)
 *  5) Render graph..?
 *
 *  Try an "stateless" API?
 *  Keep on instance state of the OpenGL flags
 *  Set this flags on each drawcall, and change when necesary
 * */
namespace Render {

    struct sGLState {
        // Depth test config
        bool depth_test_enabled = true;
        bool write_to_depth_buffer = true;
        uint32_t depth_function = GL_LESS;

        // Culling info
        bool culling_enabled = true;
        uint32_t culling_mode = GL_BACK;
        uint32_t front_face = GL_CCW;

        // Blending
        bool blending_enabled = true;
        uint32_t blend_func_x = GL_ONE;
        uint32_t blend_func_y = GL_ONE_MINUS_SRC_ALPHA;

        void set_default() {
            depth_test_enabled = true;
            write_to_depth_buffer = true;
            depth_function = GL_LESS;

            // Culling info
            culling_enabled = true;
            culling_mode = GL_BACK;
            front_face = GL_CCW;

            // Blending
            blending_enabled = true;
            blend_func_x = GL_ONE;
            blend_func_y = GL_ONE_MINUS_SRC_ALPHA;
        }
    };


    // TODO: Parenting
    struct sDrawCall {
        sTransform transform;
        uint8_t mesh_id;
        uint8_t material_id;

        sGLState call_state;
    };

    struct sMeshBuffers {
        uint32_t VAO;
        uint32_t VBO;
        uint32_t EBO;

        uint32_t primitive_count;
        uint32_t primitive;

        bool is_indexed = false;

        void init_with_triangles(const float *geometry,
                                 const uint32_t geometry_size,
                                 const uint16_t *indices,
                                 const uint32_t indices_size);
    };
    
    struct sInstance {
        sGLState current_state;

        uint16_t material_count = 0;
        sMaterial materials[MATERIAL_TOTAL_COUNT];
        uint16_t meshes_count = 0;
        sMeshBuffers meshes[MESH_TOTAL_COUNT];

        uint16_t draw_stack_size = 0;
        sDrawCall draw_stack[DRAW_CALL_STACK_SIZE];

        void init();
        void change_graphic_state(const sGLState &new_state);
        void render_frame(const glm::mat4x4 &view_proj_mat,
                          const glm::vec3 &cam_pos);
    };

};

#endif // RENDER_H_
