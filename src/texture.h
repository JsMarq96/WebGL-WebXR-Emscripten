#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <string.h>
#include <cassert>
#include <cstdint>

#define DEFAULT_TEXT_FIDELITY 0

enum eTextureType : uint8_t {
   STANDART_2D = 0,
   CUBEMAP,
   VOLUME
};

struct sTexture {
    bool             store_on_RAM = false;
    eTextureType     type;

    // Raw data
    int             width     = 0;
    int             height    = 0;
    int             layers    = 0;
    char            *raw_data = NULL;

    // OpenGL id
    unsigned int     texture_id;

    void load(const eTextureType text_type,
              const bool store_on_RAM,
              const char *texture_name);

    void load3D(const char *texture_name,
                const uint16_t width,
                const uint16_t heigth,
                const uint16_t depth);

    void clean();

};



#endif // TEXTURE_H_
