#ifndef RENDER_H_
#define RENDER_H_

#include "transform.h"
#include "shader.h"

#define SHADER_TOTAL_COUNT 10
#define MESH_TOTAL_COUNT 20
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

    // TODO: Parenting
    struct sDrawCall {
        sTransform transform;
        uint8_t mesh_id;
        uint8_t shader_id;
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
        uint16_t shader_count = 0;
        sShader shaders[SHADER_TOTAL_COUNT];
        uint16_t meshes_count = 0;
        sMeshBuffers meshes[MESH_TOTAL_COUNT];

        uint16_t draw_stack_size = 0;
        sDrawCall draw_stack[DRAW_CALL_STACK_SIZE];

        void render_frame(const glm::mat4x4 &view_proj_mat);
    };

};

#endif // RENDER_H_
