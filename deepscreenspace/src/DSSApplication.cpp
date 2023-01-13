#include "DSSApplication.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <loo/glError.hpp>
#include <vector>

#include "glm/fwd.hpp"
#include "shaders/base.frag.hpp"
#include "shaders/base.vert.hpp"
using namespace loo;
using namespace std;
void DSSApplication::loadObj(const std::string& filename) {
    LOG(INFO) << "Loading object from " << filename << endl;
    auto meshes = createMeshFromObjFile(filename);
    m_scene.addMeshes(std::move(meshes));

    m_scene.scale(glm::vec3(0.1, 0.1, 0.1));
    m_scene.prepare();
    LOG(INFO) << "Load done" << endl;
}

DSSApplication::DSSApplication(int width, int height)
    : Application(width, height),
      m_baseshader{Shader(BASE_VERT, GL_VERTEX_SHADER),
                   Shader(BASE_FRAG, GL_FRAGMENT_SHADER)},
      m_scene() {}
void DSSApplication::gui() {
    ImGui::Begin("Debug Panel");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    ImGui::Text("Renderer: %s", renderer);
    ImGui::Text("OpenGL Version: %s", version);

    ImGui::End();
}

static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    static bool firstMouse = true;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
        firstMouse = true;
        return;
    }
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    auto myapp = static_cast<DSSApplication*>(glfwGetWindowUserPointer(window));
    static float lastX = myapp->getWidth() / 2.0;
    static float lastY = myapp->getHeight() / 2.0;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;
    myapp->getCamera().processMouseMovement(xoffset, yoffset);
}

void DSSApplication::loop() {
    m_maincam.m_aspect = getWindowRatio();
    // render
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    logPossibleGLError();
    m_baseshader.use();
    m_baseshader.setUniform("uView", m_maincam.getViewMatrix());
    m_baseshader.setUniform("uProj", m_maincam.getProjectionMatrix());
    m_baseshader.setUniform("uModel", m_scene.getModelMatrix());
    m_baseshader.setUniform("uCameraPosition", m_maincam.getPosition());
    logPossibleGLError();

    m_scene.draw(m_baseshader);

    logPossibleGLError();
    // cam
    if (keyForward())
        m_maincam.processKeyboard(CameraMovement::FORWARD, getFrameDeltaTime());
    if (keyBackward())
        m_maincam.processKeyboard(CameraMovement::BACKWARD,
                                  getFrameDeltaTime());
    if (keyLeft())
        m_maincam.processKeyboard(CameraMovement::LEFT, getFrameDeltaTime());
    if (keyRight())
        m_maincam.processKeyboard(CameraMovement::RIGHT, getFrameDeltaTime());

    // gui
    this->gui();
    glfwSetCursorPosCallback(getWindow(), mouseCallback);
}
