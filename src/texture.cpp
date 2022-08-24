#include "texture.h"

#include <webgl/webgl2.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
void upload_simple_texture_to_GPU(sTexture *text);

#include <iostream>
#include <GLES3/gl3.h>

void sTexture::load(const eTextureType text_type,
                    const bool istore_on_RAM,
                    const char *texture_name) {
    store_on_RAM = istore_on_RAM;
    type = text_type;

    if (text_type == CUBEMAP) {
        const char *cubemap_terminations[] = {"right.jpg",
                                        "left.jpg",
                                        "top.jpg",
                                        "bottom.jpg",
                                        "front.jpg",
                                        "back.jpg" };

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

        // Generate buffer based on teh biggest possible size, the bottom terminating png
        char *name_buffer = (char*) malloc(strlen(texture_name) + sizeof("bottom.png") + 1);

        char* raw_data = NULL;
        for(int i = 0; i < 6; i++) {
            int w, h, l;
            memset(name_buffer, '\0', strlen(texture_name) + sizeof("bottom.png") + 1);
            strcat(name_buffer, texture_name);
            strcat(name_buffer, cubemap_terminations[i]);

#ifndef __EMSCRIPTEN__
            raw_data = stbi_load(name_buffer, &w, &h, &l, 0);
#else
            raw_data = emscripten_get_preloaded_image_data(name_buffer, &w, &h);
            l = 4;
#endif
            //info(" Load cubemap: %s %i %i %i", name_buffer, w, h, l);

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0,
                         GL_RGB,
                         w,
                         h,
                         0,
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         raw_data);
            //stbi_image_free(raw_data);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        free(name_buffer);

        return;
    }
    int w, h, l;
    //text->raw_data = stbi_load(texture_name, &w, &h, &l, 0);

#ifndef __EMSCRIPTEN__
    raw_data = stbi_load(name_buffer, &w, &h, &l, 0);
#else
    raw_data = emscripten_get_preloaded_image_data(texture_name, &w, &h);
    l = 4;
#endif

    width = w;
    height = h;

    assert(raw_data != NULL && "Uploading empty texture to GPU");

    glGenTextures(1, &texture_id);

    uint32_t texture_type = (text_type == VOLUME) ? GL_TEXTURE_3D : GL_TEXTURE_2D;

    glBindTexture(texture_type, texture_id);

    glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (text_type == STANDART_2D) {
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     w,
                     h,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     raw_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        glTexParameteri(texture_type, GL_TEXTURE_WRAP_R, GL_REPEAT);

        glTexImage3D(GL_TEXTURE_3D,
                     0,
                     GL_RGBA,
                     w,
                     h,
                     0,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     raw_data);
    }

    glBindTexture(texture_type, 0);

    //stbi_image_free(text->raw_data);
}


void sTexture::load3D(const char* texture_name,
                      const uint16_t width_i,
                      const uint16_t heigth_i,
                      const uint16_t depth_i) {
    store_on_RAM = false;
    type = VOLUME;
    int w = 0, h = 0, l = 0;
    //text->raw_data = stbi_load(texture_name, &w, &h, &l, 0);

#ifndef __EMSCRIPTEN__
    raw_data = stbi_load(name_buffer, &w, &h, &l, 0);
#else
    raw_data = emscripten_get_preloaded_image_data(texture_name, &w, &h);
    l = 4;
#endif

    width = w;
    height = h;

    assert(raw_data != NULL && "Uploading empty texture to GPU");

    glGenTextures(1, &texture_id);


    glBindTexture(GL_TEXTURE_3D, texture_id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glTexImage3D(GL_TEXTURE_3D,
                 0,
                 GL_RGBA,
                 width / width_i,
                 height / heigth_i,
                 width / width_i,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 raw_data);

    glBindTexture(GL_TEXTURE_3D, 0);

    //stbi_image_free(text->raw_data);
}

void sTexture::load_sphere_volume(const uint16_t size) {
        //free(raw_data);
}

void sTexture::clean() {
    if (store_on_RAM) {
        //stbi_image_free(text->raw_data);
    }

    glDeleteTextures(1, &texture_id);
}

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

void sTexture::load3D_async(const char *dir,
                            const uint16_t width_i,
                            const uint16_t heigth_i,
                            const uint16_t depth_i) {
    width = width_i;
    height = heigth_i;
    depth = depth_i;
    // Load empty texture
    glGenTextures(1, &texture_id);

    glBindTexture(GL_TEXTURE_3D, texture_id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_3D, 0);

    std::cout << "Start load of texture" << std::endl;
    emscripten_async_wget_data(dir,
                               this,
                               [](void *arg, void *buffer, int size) {
        sTexture* curr_text = (sTexture*) arg;
        std::cout << "recieved texture" << std::endl;

        // Check size

        glBindTexture(GL_TEXTURE_3D, curr_text->texture_id);

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

        std::cout << "recieved texture" << std::endl;

    }, NULL);
}

#endif
