#ifndef RENDER_H_
#define RENDER_H_

#include <webgl/webgl2.h>
#include <emscripten/html5_webgl.h>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

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
 * */
namespace Render {

    struct sRenderCall {

    };
    
    struct sInstance {

    };


    void render_frame();
};

#endif // RENDER_H_
