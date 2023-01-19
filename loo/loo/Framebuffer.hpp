#ifndef LOO_LOO_FRAMEBUFFER_HPP
#define LOO_LOO_FRAMEBUFFER_HPP
#include <glad/glad.h>

#include "Texture.hpp"
#include "predefs.hpp"
namespace loo {

class LOO_EXPORT Renderbuffer {
    GLuint m_rbo{GL_INVALID_INDEX};

   public:
    Renderbuffer() = default;
    Renderbuffer(const Renderbuffer&) = delete;
    Renderbuffer& operator=(const Renderbuffer&) = delete;
    Renderbuffer(Renderbuffer&& buffer) noexcept : m_rbo(buffer.m_rbo) {
        buffer.m_rbo = GL_INVALID_INDEX;
    }
    void bind() const { glBindRenderbuffer(GL_RENDERBUFFER, m_rbo); }
    void unbind() const { glBindRenderbuffer(GL_RENDERBUFFER, 0); }
    void init(GLenum internalformat, GLsizei width, GLsizei height) {
        glGenRenderbuffers(1, &m_rbo);
        bind();
        glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
        unbind();
    }
    GLuint getId() const { return m_rbo; }
    ~Renderbuffer() { glDeleteRenderbuffers(1, &m_rbo); }
};
class Framebuffer {
    GLuint m_fbo{GL_INVALID_INDEX};

   public:
    Framebuffer() = default;
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& buffer) noexcept : m_fbo(buffer.m_fbo) {
        buffer.m_fbo = GL_INVALID_INDEX;
    }
    void init() { glGenFramebuffers(1, &m_fbo); }
    GLuint getId() const { return m_fbo; }

    void bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_fbo); }
    void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
    void attachTexture(const Texture2D& tex, GLenum attachment, GLint level) {
        bind();
        tex.bind();
        glFramebufferTexture(GL_FRAMEBUFFER, attachment, tex.getId(), level);
        tex.unbind();
        unbind();
    }
    void attachRenderbuffer(const Renderbuffer& rb, GLenum attachment) {
        bind();
        rb.bind();
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER,
                                  rb.getId());
        rb.unbind();
        unbind();
    }
    ~Framebuffer() { glDeleteFramebuffers(1, &m_fbo); }
};
}  // namespace loo

#endif /* LOO_LOO_FRAMEBUFFER_HPP */
