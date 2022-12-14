#ifndef FBO_H_
#define FBO_H_
#ifdef __EMSCRIPTEN__
#include <webgl/webgl2.h>
#include <GLES3/gl3.h>
#else
#include <GL/gl3w.h>
#endif

#include "texture.h"

enum eFBOAttachmentUse : uint32_t {
    JUST_COLOR = 0,
    JUST_DUAL_COLOR,
    JUST_DEPTH,
    COLOR_AND_DEPTH
};

struct sFBO {
    uint32_t id = 0;
    uint32_t width = 0;
    uint32_t height = 0;

    eFBOAttachmentUse attachment_use;

    sTexture color_attachment0;
    sTexture color_attachment1;
    sTexture depth_attachment;

    void init_with_single_color(const uint32_t width_i,
                                const uint32_t height_i) {
        attachment_use = JUST_COLOR;

        width = width_i;
        height = height_i;

        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        color_attachment0.create_empty2D_with_size(width,
                                                  height);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                               color_attachment0.texture_id,
                               0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void init_with_dual_color(const uint32_t width_i,
                              const uint32_t height_i) {
        attachment_use = JUST_DUAL_COLOR;

        width = width_i;
        height = height_i;

        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

         color_attachment0.create_empty2D_with_size(width,
                                                  height);

        color_attachment1.create_empty2D_with_size(width,
                                                  height);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                               color_attachment0.texture_id,
                               0);


        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT1,
                               GL_TEXTURE_2D,
                               color_attachment1.texture_id,
                               0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    void clean() {
        glDeleteTextures(1, &color_attachment0.texture_id);
        glDeleteTextures(1, &color_attachment1.texture_id);
        glDeleteFramebuffers(1, &id);
    }

    void reinit(const uint32_t width_i,
                const uint32_t height_i) {
        clean();

        switch (attachment_use) {
            case JUST_COLOR:
                init_with_single_color(width_i,
                                       height_i);
                break;
            case JUST_DUAL_COLOR:
                init_with_dual_color(width_i,
                                     height_i);
                break;
            case JUST_DEPTH:
                // TODO
                break;
            case COLOR_AND_DEPTH:
                // TODO
                break;
        }
    }

    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

#endif // FBO_H_
