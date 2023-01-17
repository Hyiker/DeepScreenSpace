#ifndef DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP
#define DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP

#include <glm/glm.hpp>
#include <loo/Application.hpp>
#include <loo/Camera.hpp>
#include <loo/Quad.hpp>
#include <loo/Scene.hpp>
#include <loo/Shader.hpp>
#include <loo/UniformBuffer.hpp>
#include <loo/loo.hpp>
#include <memory>
#include <string>
class DSSApplication : public loo::Application {
   public:
    DSSApplication(int width, int height);
    void loadObj(const std::string& filename, float scaling = 1.0);
    void loadGLTF(const std::string& filename);
    loo::Camera& getCamera() { return m_maincam; }

   private:
    void loop() override;
    void gui();
    struct MVP {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };
    MVP m_mvp;

    loo::ShaderProgram m_baseshader;
    loo::Scene m_scene;
    loo::Camera m_maincam;
    loo::UniformBuffer m_mvpbuffer;
};

#endif /* DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP */
