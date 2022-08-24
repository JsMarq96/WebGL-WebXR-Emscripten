#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <cstddef>

#include <cstdint>
#include "texture.h"
#include "shader.h"

#define TEXTURE_SIZE 3

enum eTextureMapType : int {
    COLOR_MAP = 0,
    NORMAL_MAP,
    SPECULAR_MAP,
    METALLIC_ROUGHNESS_MAP,
    VOLUME_MAP,
    TEXTURE_MAP_TYPE_COUNT
};

const char texture_uniform_LUT[TEXTURE_MAP_TYPE_COUNT][25] = {
   "u_albedo_map",
   "u_normal_map",
   "u_metallic_rough_map",
   "u_metallic_rough_map",
   "u_volume_map"
};

struct sMaterial {
    sTexture        textures[TEXTURE_MAP_TYPE_COUNT];
    bool            enabled_textures[TEXTURE_MAP_TYPE_COUNT] = {false};

    sShader         shader;

    void add_shader(const char     *vertex_shader,
                    const char     *fragment_shader);
    void add_raw_shader(const char     *vertex_shader,
                    const char     *fragment_shader);

    void add_sphere_volume(const uint16_t size);

    void add_volume_texture(const char* text_dir,
                            const uint16_t tile_width,
                            const uint16_t tile_heigth,
                            const uint16_t tile_depth);

    void add_texture(const char*          text_dir,
                     const eTextureType   text_type);

    void add_raw_texture(const char* raw_data,
                         const size_t width,
                         const size_t height,
                         const GLenum format,
                         const GLenum type,
                         const eTextureType text_type);

    void add_cubemap_texture(const char  *text_dir);

    uint8_t get_used_textures() const {
        uint8_t tmp = 0b0;

        if (enabled_textures[COLOR_MAP]) {
            tmp |= 0b1;
        }
        if (enabled_textures[NORMAL_MAP]) {
            tmp |= 0b10;
        }
        if (enabled_textures[SPECULAR_MAP]) {
            tmp |= 0b100;
        }
        if (enabled_textures[METALLIC_ROUGHNESS_MAP]) {
            tmp |= 0b1000;
        }
        if (enabled_textures[VOLUME_MAP]) {
            tmp |= 0b10000;
        }

        return tmp;
    };

    /**
    * Binds the textures on Opengl
    *  COLOR - Texture 0
    *  NORMAL - Texture 1
    *  SPECULAR - TEXTURE 2
    * */
    void enable() const;

    void disable() const;
};

#endif // MATERIAL_H_
