#ifndef LOO_LOO_SHADOW_MAP_HPP
#define LOO_LOO_SHADOW_MAP_HPP

#include <glm/vec3.hpp>

#include "Framebuffer.hpp"
#include "GaussianBlur.hpp"
#include "Scene.hpp"
#include "Texture.hpp"
#include "predefs.hpp"
namespace loo {

class LOO_EXPORT ShadowMap {
    // MSM
    Texture m_moments;
    Framebuffer m_framebuffer;
    Renderbuffer m_renderbuffer;
    ShaderProgram m_shader;
    int m_width, m_height;
    GaussianBlur m_blurer;

   public:
    ShadowMap(ShaderProgram&& shaderProgram, int width = 2048,
              int height = 2048)
        : m_shader{std::move(shaderProgram)},
          m_width{width},
          m_height{height},
          m_blurer(width, height) {}
    void init();
    void render(const Scene& scene, glm::vec3 sunPosition);
    void blur();
    const Texture& getDepthTexture() const { return m_moments; }
};

}  // namespace loo
#endif /* LOO_LOO_SHADOW_MAP_HPP */
