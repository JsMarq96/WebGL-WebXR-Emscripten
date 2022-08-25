#include "material.h"

#include "texture.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void sMaterial::add_shader(const char     *vertex_shader,
                           const char     *fragment_shader) {
    shader.load_file_shaders(vertex_shader,
                             fragment_shader);
}

void sMaterial::add_raw_shader(const char     *vertex_shader,
                           const char     *fragment_shader) {
    shader.load_shaders(vertex_shader,
                        fragment_shader);
}


void sMaterial::add_texture(const char*           text_dir,
                            const eTextureType   text_type) {
    enabled_textures[text_type] = true;
    textures[text_type].load( eTextureType::STANDART_2D,
                              false,
                              text_dir);
}

// TODO: this is kinda messy... refactor to sTexture??
void sMaterial::add_raw_texture(const char* raw_data,
                                const size_t width,
                                const size_t height,
                                const GLenum format,
                                const GLenum type,
                                const eTextureType text_type) {
    enabled_textures[text_type] = true;

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

void sMaterial::add_cubemap_texture(const char   *text_dir) {
    enabled_textures[COLOR_MAP] = true;
    textures[COLOR_MAP].load(eTextureType::CUBEMAP,
                             false,
                             text_dir);
}

void sMaterial::add_volume_texture(const char* text_dir,
                                   const uint16_t tile_width,
                                   const uint16_t tile_heigth,
                                   const uint16_t tile_depth) {
    enabled_textures[VOLUME_MAP] = true;
    textures[VOLUME_MAP].load3D(text_dir,
                                tile_width,
                                tile_heigth,
                                tile_depth);
}

void sMaterial::add_sphere_volume(const uint16_t size) {
    enabled_textures[VOLUME_MAP] = true;
    textures[VOLUME_MAP].load_sphere_volume(size);
}


#include <iostream>
 void sMaterial::load_async_texture3D(const char* dir,
                                      const uint16_t width,
                                      const uint16_t heigth,
                                      const uint16_t depth) {
#ifdef __EMSCRIPTEN__
    sTexture *text = &textures[VOLUME_MAP];
    text->width = width;
    text->height = heigth;
    text->depth = depth;

    text->load_empty_volume();

    enabled_textures[VOLUME_MAP] = true;

    std::cout << "Start load of volume texture" << std::endl;
    emscripten_async_wget_data(dir,
                               text,
                               [](void *arg, void *buffer, int size) {
        sTexture* curr_text = (sTexture*) arg;

        // Check size
        std::cout << "End load of volume texture" << std::endl;

        glBindTexture(GL_TEXTURE_3D, curr_text->texture_id);

        // Re-fill the texture
        glTexStorage3D(GL_TEXTURE_3D, 1, GL_R8, curr_text->width, 256, 256);
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
 }


/**
 * Binds the textures on Opengl
 *  COLOR - Texture 0
 *  NORMAL - Texture 1
 *  SPECULAR - TEXTURE 2
 *  VOLUME - Texture 3
 * */
void sMaterial::enable() const {
    shader.activate();

    for (int texture = 0; texture < TEXTURE_MAP_TYPE_COUNT; texture++) {
        if (!enabled_textures[texture]) {
            continue;
        }
        glActiveTexture(GL_TEXTURE0 + texture);

        if (texture == VOLUME_MAP) {
            glBindTexture(GL_TEXTURE_3D, textures[texture].texture_id);
        } else {
            glBindTexture(GL_TEXTURE_2D, textures[texture].texture_id);
        }

        shader.set_uniform_texture(texture_uniform_LUT[texture], texture);
    }
}

void sMaterial::disable() const {
    //shader.disable();
}
