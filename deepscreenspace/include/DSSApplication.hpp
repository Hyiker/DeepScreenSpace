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

#include "DeepScreenSpace.hpp"
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
    void deepScreenSpace();
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
    // scene render output
    loo::Framebuffer m_scenefb;
    loo::Renderbuffer m_scenedepthrb;
    std::shared_ptr<loo::Texture2D> m_scenetexture;
    std::shared_ptr<loo::Texture2D> m_sceneposition, m_scenenormal;

    // scene surfelize
    loo::Framebuffer m_surfelizefb;
    float m_surfelizescale{0.002f}, m_splattingstrength{20.f};

    // screen quad
    std::shared_ptr<loo::Quad> m_globalquad;

    // process
    FinalProcess m_finalprocess;

    // Deep screen space
    int m_dss_partitiondebuglayer{1};
    DeepScreenSpace m_dss;

    bool m_wireframe{};
    bool m_enablenormal{true};
    bool m_enableparallax{true};
    bool m_lodvisualize{false};
    bool m_showunshuffleresult{true};
    // float m_displaceintensity{};
};

#endif /* DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP */
