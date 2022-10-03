#include "material.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <GLES3/gl3.h>
#else
#include <GL/gl3w.h>
#endif

#include "texture.h"
#include "volumetric_octree.h"
#include <cstddef>
#include <cstdint>


uint8_t sMaterialManager::add_shader(const char     *vertex_shader,
                           const char     *fragment_shader) {
    shaders[shader_count].load_file_shaders(vertex_shader,
                                            fragment_shader);

    return shader_count++;
}

 uint8_t sMaterialManager::add_raw_shader(const char     *vertex_shader,
                           const char     *fragment_shader) {
    shaders[shader_count].load_shaders(vertex_shader,
                                       fragment_shader);
    return shader_count++;
}

 uint8_t sMaterialManager::add_texture(const char*           text_dir) {
    textures[texture_count].load( eTextureType::STANDART_2D,
                                  false,
                                  text_dir);
    return texture_count++;
}

// TODO: this is kinda messy... refactor to sTexture??
void sMaterialManager::add_raw_texture(const char* raw_data,
                                const size_t width,
                                const size_t height,
                                const GLenum format,
                                const GLenum type,
                                const eTextureType text_type) {
   // enabled_textures[text_type] = true;
    assert(false && "TODO Add raw texture to material");

    assert(raw_data != NULL && "Uploading empty texture to GPU");

    sTexture *curr_texture = &textures[text_type];

    glGenTextures(1, &curr_texture->texture_id);

    glBindTexture(GL_TEXTURE_2D, curr_texture->texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 width, height,
                 0,
                 format,
                 type,
                 raw_data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void sMaterialManager::add_cubemap_texture(const char   *text_dir) {
    //enabled_textures[COLOR_MAP] = true;
    assert(false && "TODO add cubemap texture to the material");
    textures[COLOR_MAP].load(eTextureType::CUBEMAP,
                             false,
                             text_dir);
}

void sMaterialManager::add_volume_texture(const char* text_dir,
                                   const uint16_t tile_width,
                                   const uint16_t tile_heigth,
                                   const uint16_t tile_depth) {
    //enabled_textures[VOLUME_MAP] = true;
    assert(false && "TODO add volume texture from drive to material manager");
    textures[VOLUME_MAP].load3D(text_dir,
                                tile_width,
                                tile_heigth,
                                tile_depth);
}



#include <iostream>
 uint8_t sMaterialManager::load_async_texture3D(const char* dir,
                                      const uint16_t width,
                                      const uint16_t heigth,
                                      const uint16_t depth) {
#ifdef __EMSCRIPTEN__
    sTexture *text = &textures[texture_count];
    text->width = width;
    text->height = heigth;
    text->depth = depth;

    text->load_empty_volume();

    std::cout << "Start load of volume texture" << std::endl;
    emscripten_async_wget_data(dir,
                               text,
                               [](void *arg, void *buffer, int size) {
        sTexture* curr_text = (sTexture*) arg;

        // Check size
        std::cout << "End load of volume texture" << std::endl;

        glBindTexture(GL_TEXTURE_3D, curr_text->texture_id);

        // Re-fill the texture
        glTexStorage3D(GL_TEXTURE_3D,
                       1,
                       GL_R8,
                       curr_text->width,
                       curr_text->height,
                       curr_text->depth);

        glTexSubImage3D(GL_TEXTURE_3D,
                     0, 0, 0, 0,
                     curr_text->width,
                     curr_text->height,
                     curr_text->depth,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     buffer);

        glBindTexture(GL_TEXTURE_3D, 0);

    }, NULL);

#else
    assert(false && "There is async loading of volumes on this platform yet");
#endif
    return texture_count++;
 }

uint8_t sMaterialManager::load_async_octree_texture3D(const char* dir,
                                                      const uint16_t width,
                                                      const uint16_t heigth,
                                                      const uint16_t depth) {
    sTexture *text = &textures[texture_count];
    text->width = width;
    text->height = heigth;
    text->depth = depth;

    text->load_empty_volume();

    uint32_t test_octree[] = {
        1,  3, 6, 9,  12,   15, 18, 21,  24, 0, 0, 0,
        0,  3, 6, 9,  12,   15, 18, 21,  24, 0, 0, 0,
        0,  3, 6, 9,  12,   15, 18, 21,  24, 0, 0, 0,
        0,  3, 6, 9,  12,   15, 18, 21,  24, 0, 0, 0,
        0,  3, 6, 9,  12,   15, 18, 21,  24, 0, 0, 0,
        0,  3, 6, 9,  12,   15, 18, 21,  24, 0, 0, 0,
        0,  3, 6, 9,  12,   15, 18, 21,  24, 0, 0, 0,
        0,  3, 6, 9,  12,   15, 18, 21,  24, 0, 0, 0,
        0,  3, 6, 9,  12,   15, 18, 21,  24, 0, 0, 0,



        0,  27, 30, 39, 42, 75, 78, 87, 90, 0, 0, 0    };
glBindTexture(GL_TEXTURE_3D, text->texture_id);

    glTexStorage3D(GL_TEXTURE_3D,
                       1,
                       GL_RGBA32UI,
                       8,
                       7,
                       1);

        glTexSubImage3D(GL_TEXTURE_3D,
                     0, 0, 0, 0,
                     8,7,1,
                     GL_RGBA_INTEGER,
                     GL_UNSIGNED_INT,
                     test_octree);
        glBindTexture(GL_TEXTURE_3D, 0);
    return texture_count++;
    //glBindTexture(GL_TEXTURE_3D, text->texture_id);
#ifdef __EMSCRIPTEN__
    std::cout << "Start load of octree volume texture" << std::endl;
    emscripten_async_wget_data(dir,
                               text,
                               [](void *arg, void *buffer, int size) {
        sTexture* curr_text = (sTexture*) arg;

        // Check size
        std::cout << "End load of volume texture & start generation octree" << std::endl;

        OCTREE::sRawVolume raw_volume = {
                        .raw_volume = (uint8_t*) buffer,
                        .width = (uint16_t) curr_text->width,
                        .heigth = (uint16_t) curr_text->height,
                        .depth = (uint16_t) curr_text->depth
        };

        OCTREE::sVolumeOctree octree;

        double start = emscripten_get_now();
        OCTREE::octree_generation(raw_volume,
                                  &octree);
        double end = emscripten_get_now();
        std::cout << "Time on octree gen: "<< (end - start) << std::endl;

        glBindTexture(GL_TEXTURE_3D, curr_text->texture_id);

        uint32_t sizet = (pow(octree.get_size_on_pixels(), 1.0f/3.0f));

        std::cout << sizet << std::endl;

        // Re-fill the texture
        glTexStorage3D(GL_TEXTURE_3D,
                       1,
                       GL_RGBA32UI,
                       sizet,
                       sizet,
                       sizet);

        glTexSubImage3D(GL_TEXTURE_3D,
                     0, 0, 0, 0,
                     sizet, sizet, sizet,
                     GL_RGBA_INTEGER,
                     GL_UNSIGNED_INT,
                     octree.nodes);

        for(uint32_t i = 1; i < 9; i++) {
            octree.nodes[i].type = (i % 2) ? OCTREE::FULL_VOXEL : OCTREE::EMPTY_VOXEL;
        }
        std::cout << ((uint32_t*) octree.nodes)[0] << std::endl;

        for(uint32_t i = 0; i < 9; i++) {
            for(uint32_t j = 0; j < 4; j++) {
                 std::cout <<  i*4 + j << ": " << ((uint32_t*) octree.nodes)[i*4 + j]  << std::endl;
            }
            std::cout << "=====" << std::endl;
        }

        glBindTexture(GL_TEXTURE_3D, 0);

    }, NULL);

#else
    //assert(false && "There is async loading of volumes on this platform yet");
#endif
    return texture_count++;

}

/**
 * Binds the textures on Opengl
 *  COLOR - Texture 0
 *  NORMAL - Texture 1
 *  SPECULAR - TEXTURE 2
 *  VOLUME - Texture 3
 * */
void sMaterialManager::enable(const uint8_t material_id) const {
    const sMaterialInstance &material = materials[material_id];
    shaders[material.shader_id].activate();

    int curr_texture_spot = 0;
    for (int texture = 0; texture < TEXTURE_MAP_TYPE_COUNT; texture++) {
        if (!material.enabled_textures[texture]) {
            continue;
        }
        glActiveTexture(GL_TEXTURE0 + curr_texture_spot);

        glBindTexture((texture == VOLUME_MAP || texture == VOLUME_OCTREE) ? GL_TEXTURE_3D : GL_TEXTURE_2D,
                      textures[material.texture_ids[texture]].texture_id);

        shaders[material_id].set_uniform_texture(texture_uniform_LUT[texture], curr_texture_spot);
        curr_texture_spot++;
    }
}

void sMaterialManager::disable() const {
    //shader.disable();
}
