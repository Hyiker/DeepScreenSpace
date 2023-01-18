#ifndef LOO_LOO_TEXTURE_HPP
#define LOO_LOO_TEXTURE_HPP
#include <glad/glad.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "loo/glError.hpp"
#include "predefs.hpp"

namespace loo {

class LOO_EXPORT Texture {
    GLuint m_id{GL_INVALID_INDEX};
    GLsizei width{-1}, height{-1};
    static Texture blankTexture2D;

   public:
    void init() {
#ifdef OGL_46
        glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
#else
        glGenTextures(1, &m_id);
#endif
    }
    // for opengl 4.5+ use glBindTextureUnit
    void bind() const {
#ifndef OGL_46
        glBindTexture(GL_TEXTURE_2D, m_id);
#endif
    }
    void unbind() const {
#ifndef OGL_46
        glBindTexture(GL_TEXTURE_2D, 0);
#endif
    }
    GLuint getId() const { return m_id; };
    GLenum getType() const { return GL_TEXTURE_2D; }
    int getMipmapLevels() const {
        unsigned int lvl = 0;
        int width = this->width, height = this->height;
        while ((width | height) >> 1) {
            width >>= 1;
            height >>= 1;
            lvl++;
        }
        return lvl;
    }
    void generateMipmap() {
#ifdef OGL_46
        glGenerateTextureMipmap(m_id);
#else
        bind();
        glGenerateMipmap(GL_TEXTURE_2D);
        unbind();
#endif
    }
    void setSizeFilter(GLenum min_filter, GLenum mag_filter) {
#ifdef OGL_46
        glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, min_filter);
        glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, mag_filter);
#else
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
        unbind();
#endif
    }
    void setWrapFilter(GLenum filter) {
#ifdef OGL_46
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, filter);
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, filter);
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, filter);
#else
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, filter);
        unbind();
#endif
    }
    void setClampToBorderFilter(GLfloat* borderColor) {
#ifdef OGL_46
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameterfv(m_id, GL_TEXTURE_BORDER_COLOR, borderColor);
#else
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        unbind();
#endif
    }
    void setup(GLsizei width, GLsizei height, GLenum internalformat,
               GLenum format, GLenum type, GLsizei level) {
        setup(nullptr, width, height, internalformat, format, type, level);
    }
    void setup(unsigned char* data, GLsizei width, GLsizei height,
               GLenum internalformat, GLenum format, GLenum type, GLint level,
               GLint maxLevel = -1) {
        this->width = width;
        this->height = height;
#ifdef OGL_46
        // prepare storage
        glTextureStorage2D(m_id, maxLevel == -1 ? getMipmapLevels() : maxLevel,
                           internalformat, width, height);
        panicPossibleGLError();
        // store data
        glTextureSubImage2D(m_id, level,          // level
                            0, 0, width, height,  // offset, size
                            format, type, data);
        panicPossibleGLError();
#else
        bind();
        glTexStorage2D(GL_TEXTURE_2D, level, internalformat, width, height);
        glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, width, height, format, type,
                        data);
        // glTexImage2D(GL_TEXTURE_2D, level, internalformat, width, height,
        // 0,
        //              format, type, data);

        unbind();
#endif
    }
    static const Texture& getBlankTexture2D();
};

LOO_EXPORT std::shared_ptr<Texture> createTextureFromFile(
    std::unordered_map<std::string, std::shared_ptr<Texture>>& uniqueTexture,
    const std::string& filename);
}  // namespace loo
#endif /* LOO_LOO_TEXTURE_HPP */
