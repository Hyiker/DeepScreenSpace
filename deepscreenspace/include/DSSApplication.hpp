#ifndef DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP
#define DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP

#include <loo/Application.hpp>
#include <loo/Camera.hpp>
#include <loo/Quad.hpp>
#include <loo/Scene.hpp>
#include <loo/Shader.hpp>
class DSSApplication : public loo::Application {
   public:
    DSSApplication(int width, int height);
    void loadObj(const std::string& filename);
    loo::Camera& getCamera() { return m_maincam; }

   private:
    void loop() override;
    void gui();

    loo::ShaderProgram m_baseshader;
    loo::Scene m_scene;
    loo::Camera m_maincam;
};

#endif /* DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP */
