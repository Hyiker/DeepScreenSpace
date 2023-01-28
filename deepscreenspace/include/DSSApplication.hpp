#ifndef DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP
#define DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP

#include <glm/glm.hpp>
#include <loo/Application.hpp>
#include <loo/Camera.hpp>
#include <loo/Light.hpp>
#include <loo/Quad.hpp>
#include <loo/Scene.hpp>
#include <loo/Shader.hpp>
#include <loo/Skybox.hpp>
#include <loo/UniformBuffer.hpp>
#include <loo/loo.hpp>
#include <memory>
#include <string>
#include <vector>

#include "FinalProcess.hpp"

class DSSApplication : public loo::Application {
   public:
    DSSApplication(int width, int height, const char* skyBoxPrefix = nullptr);
    // only load model
    void loadModel(const std::string& filename, float scaling = 1.0);
    void loadGLTF(const std::string& filename, float scaling = 1.0);
    loo::Camera& getCamera() { return m_maincam; }
    void afterCleanup() override;

   private:
    void loop() override;
    void gui();
    void scene();
    void skybox();
    void finalprocess();
    void clear();
    void keyboard();
    void mouse();
    struct MVP {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 normalMatrix;
    };
    MVP m_mvp;

    loo::ShaderProgram m_baseshader, m_skyboxshader;
    loo::Scene m_scene;
    std::shared_ptr<loo::TextureCubeMap> m_skyboxtex{};
    loo::Skybox m_skybox;
    loo::Camera m_maincam;
    loo::UniformBuffer m_mvpbuffer;
    loo::UniformBuffer m_lightsbuffer;
    std::vector<loo::ShaderLight> m_lights;
    // scene output
    loo::Framebuffer m_scenefb;
    loo::Renderbuffer m_scenedepthrb;
    std::shared_ptr<loo::Texture2D> m_scenetexture;

    // screen quad
    std::shared_ptr<loo::Quad> m_globalquad;

    FinalProcess m_finalprocess;

    bool m_wireframe{};
    bool m_enablenormal{true};
    bool m_enableparallax{true};
    bool m_lodvisualize{false};
    // float m_displaceintensity{};
};

#endif /* DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP */
