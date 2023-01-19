#ifndef LOO_LOO_GAUSSIAN_BLUR_HPP
#define LOO_LOO_GAUSSIAN_BLUR_HPP
#include <glad/glad.h>

#include "Framebuffer.hpp"
#include "Quad.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "predefs.hpp"
namespace loo {

class LOO_EXPORT GaussianBlur {
    Quad m_quad;
    ShaderProgram m_shader;
    Framebuffer m_fbs[2];
    Texture2D m_texs[2];
    GLsizei m_width, m_height;

   public:
    GaussianBlur(GLsizei width, GLsizei height);
    void init(GLenum internalformat, GLenum format, GLenum type);
    /**
     * @brief Blur a texture
     *
     * @param tex input Texture
     * @param blurKerSize 5, 9, 13
     */
    void blur(Texture2D& tex, int blurKerSize = 5);
};
}  // namespace loo

#endif /* LOO_LOO_GAUSSIAN_BLUR_HPP */
