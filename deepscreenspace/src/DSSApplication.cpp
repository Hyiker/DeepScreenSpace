#include "DSSApplication.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <loo/glError.hpp>
#include <memory>
#include <vector>

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "shaders/base.frag.hpp"
#include "shaders/base.vert.hpp"
using namespace loo;
using namespace std;

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

void DSSApplication::loadObj(const std::string& filename, float scaling) {
    LOG(INFO) << "Loading model from " << filename << endl;
    glm::mat4 transform = glm::scale(glm::identity<glm::mat4>(),
                                     glm::vec3(scaling, scaling, scaling));
    auto meshes = createMeshFromObjFile(filename, transform);
    m_scene.addMeshes(std::move(meshes));

    m_scene.prepare();
    LOG(INFO) << "Load done" << endl;
}

void DSSApplication::loadGLTF(const std::string& filename) {
    LOG(INFO) << "Loading scene from " << filename << endl;
    m_scene = createSceneFromGLTF(filename);
    // m_scene->prepare();
    LOG(INFO) << "Load done" << endl;
}

DSSApplication::DSSApplication(int width, int height)
    : Application(width, height),
      m_baseshader{Shader(BASE_VERT, GL_VERTEX_SHADER),
                   Shader(BASE_FRAG, GL_FRAGMENT_SHADER)},
      m_scene(),
      m_maincam(),
      m_mvpbuffer(0, sizeof(MVP)) {
    logPossibleGLError();
}
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

void DSSApplication::loop() {
    m_maincam.m_aspect = getWindowRatio();
    // render
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    logPossibleGLError();
    m_maincam.getViewMatrix(m_mvp.view);
    m_maincam.getProjectionMatrix(m_mvp.projection);
    m_mvp.model = glm::identity<glm::mat4>();

    m_baseshader.use();
    m_baseshader.setUniform("uCameraPosition", m_maincam.getPosition());
    logPossibleGLError();

    m_scene.draw(m_baseshader, [this](const auto& scene, const auto& mesh) {
        m_mvp.model = scene.getModelMatrix() * mesh.m_objmat;
        m_mvpbuffer.updateData(0, sizeof(MVP), &m_mvp);
    });
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
