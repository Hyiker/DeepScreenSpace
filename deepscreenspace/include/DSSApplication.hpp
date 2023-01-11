#ifndef DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP
#define DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP

#include <loo/Application.hpp>
#include <loo/Quad.hpp>
#include <loo/Scene.hpp>
#include <loo/Shader.hpp>
class DSSApplication : public loo::Application {
   public:
    DSSApplication(int width, int height);

   private:
    void loop() override;

    loo::ShaderProgram m_quad_shader;
    loo::Scene m_scene;
    GLuint VAO;
};

#endif /* DEEPSCREENSPACE_INCLUDE_DSSAPPLICATION_HPP */
