#ifndef FBO_H_
#define FBO_H_

#include "texture.h"
#include <GLES3/gl3.h>
#include <webgl/webgl2.h>

enum eFBOAttachmentUse : uint32_t {
    JUST_COLOR = 0,
    JUST_DEPTH,
    COLOR_AND_DEPTH
};

struct sFBO {
    uint32_t id = 0;
    uint32_t width = 0;
    uint32_t height = 0;

    eFBOAttachmentUse attachment_use;

    sTexture color_attachment;
    sTexture depth_attachment;

    void init_with_color(const uint32_t width_i,
                         const uint32_t height_i) {
        attachment_use = JUST_COLOR;

        width = width_i;
        height = height_i;

        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        color_attachment.create_empty2D_with_size(width,
                                                  height);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                               color_attachment.texture_id,
                               0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void clean() {
        glDeleteTextures(1, &color_attachment.texture_id);
        glDeleteFramebuffers(1, &id);
    }

    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

#endif // FBO_H_
