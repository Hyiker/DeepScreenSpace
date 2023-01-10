#include "loo/ShadowMap.hpp"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

#include "loo/glError.hpp"
namespace loo {

using namespace glm;
void ShadowMap::init() {
    m_framebuffer.init();
    m_framebuffer.bind();
    m_framebuffer.unbind();

    m_renderbuffer.init(GL_DEPTH_COMPONENT, m_width, m_height);

    m_moments.init();
    m_moments.setup(m_width, m_height, GL_RGBA16, GL_RGBA, GL_FLOAT, 0);
    m_moments.setSizeFilter(GL_LINEAR, GL_LINEAR);

    m_framebuffer.attachTexture(m_moments, GL_COLOR_ATTACHMENT0, 0);
    m_framebuffer.attachRenderbuffer(m_renderbuffer, GL_DEPTH_ATTACHMENT);

    m_blurer.init(GL_RGBA16, GL_RGBA, GL_FLOAT);
}

void ShadowMap::render(const Scene& scene, glm::vec3 sunPosition) {
    m_framebuffer.bind();
    GLint m_viewport[4];
    glGetIntegerv(GL_VIEWPORT, m_viewport);

    glViewport(0, 0, m_width, m_height);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    m_shader.use();
    m_shader.setUniform("uModel", scene.getModelMatrix());
    m_shader.setUniform("uLightTransform", getLightSpaceTransform(sunPosition));
    scene.draw(m_shader);
    checkError();
    glCullFace(GL_BACK);

    m_framebuffer.unbind();

    blur();

    glViewport(0, 0, m_viewport[2], m_viewport[3]);
}

void ShadowMap::blur() {
    for (int i = 0; i < 3; i++) {
        m_blurer.blur(m_moments);
    }
}
}  // namespace loo
