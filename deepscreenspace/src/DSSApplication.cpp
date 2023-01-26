#include "DSSApplication.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <fstream>
#include <loo/glError.hpp>
#include <memory>
#include <vector>

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "imgui_impl_glfw.h"
#include "shaders/base.frag.hpp"
#include "shaders/base.vert.hpp"
#include "shaders/skybox.frag.hpp"
#include "shaders/skybox.vert.hpp"
using namespace loo;
using namespace std;

static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    ImGui_ImplGlfw_CursorPosCallback(window, xposIn, yposIn);
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

static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    auto myapp = static_cast<DSSApplication*>(glfwGetWindowUserPointer(window));
    myapp->getCamera().processMouseScroll(xOffset, yOffset);
}

void DSSApplication::loadModel(const std::string& filename, float scaling) {
    LOG(INFO) << "Loading model from " << filename << endl;
    glm::mat4 transform = glm::scale(glm::identity<glm::mat4>(),
                                     glm::vec3(scaling, scaling, scaling));
    auto meshes = createMeshFromFile(filename, transform);
    m_scene.addMeshes(std::move(meshes));

    m_scene.prepare();
    LOG(INFO) << "Load done" << endl;
}

void DSSApplication::loadGLTF(const std::string& filename, float scaling) {
    LOG(INFO) << "Loading scene from " << filename << endl;
    // TODO: m_scene = createSceneFromFile(filename);
    glm::mat4 transform = glm::scale(glm::identity<glm::mat4>(),
                                     glm::vec3(scaling, scaling, scaling));
    auto meshes = createMeshFromFile(filename, transform);
    m_scene.addMeshes(std::move(meshes));

    m_scene.prepare();
    LOG(INFO) << "Load done" << endl;
}

DSSApplication::DSSApplication(int width, int height, const char* skyBoxPrefix)
    : Application(width, height),
      m_baseshader{Shader(BASE_VERT, GL_VERTEX_SHADER),
                   Shader(BASE_FRAG, GL_FRAGMENT_SHADER)},
      m_skyboxshader{Shader(SKYBOX_VERT, GL_VERTEX_SHADER),
                     Shader(SKYBOX_FRAG, GL_FRAGMENT_SHADER)},
      m_scene(),
      m_maincam(),
      m_mvpbuffer(0, sizeof(MVP)),
      m_lightsbuffer(SHADER_BINDING_LIGHTS,
                     sizeof(ShaderLight) * SHADER_LIGHTS_MAX) {
    ifstream ifs("camera.txt");
    if (!ifs.fail()) {
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        float yaw = -90.0f;
        float pitch = 0.0f;
        float aspect = 4.f / 3.f;

        ifs >> position.x >> position.y >> position.z;
        // ifs >> up.x >> up.y >> up.z;
        ifs >> yaw >> pitch >> aspect;
        m_maincam = Camera(position, up, yaw, pitch, aspect);
    }
    ifs.close();
    if (skyBoxPrefix) {
        // skybox setup
        auto skyboxFilenames = TextureCubeMap::builder()
                                   .front("front")
                                   .back("back")
                                   .left("left")
                                   .right("right")
                                   .top("top")
                                   .bottom("bottom")
                                   .prefix(skyBoxPrefix)
                                   .build();
        m_skyboxtex = createTextureCubeMapFromFiles(skyboxFilenames);
    }
    // scene light
    {
        m_lights.push_back(
            createDirectionalLight(glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0)));
    }
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
    ImGui::NewLine();

    // Sun
    ImGui::Text("Sun");
    ImGui::SliderFloat3("Direction", (float*)&m_lights[0].direction, -1, 1);
    ImGui::SliderFloat("Intensity", (float*)&m_lights[0].intensity, 0.0, 100.0);
    ImGui::NewLine();

    // OpenGL option
    ImGui::Text("OpenGL options");
    ImGui::Checkbox("Wire frame mode", &m_wireframe);
    ImGui::Checkbox("Normal mapping", &m_enablenormal);
    ImGui::Checkbox("Parallax mapping", &m_enableparallax);
    ImGui::End();
}

void DSSApplication::skybox() {
    if (m_skyboxtex) {
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        glm::mat4 view;
        m_maincam.getViewMatrix(view);
        m_skyboxshader.use();
        m_mvp.view = glm::mat4(glm::mat3(view));
        m_mvpbuffer.updateData(0, sizeof(MVP), &m_mvp);
        m_skyboxshader.setTexture(SHADER_BINDING_PORT_SKYBOX, *m_skyboxtex);
        m_skybox.draw();
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
    }
}
void DSSApplication::scene() {
    m_maincam.getViewMatrix(m_mvp.view);
    m_maincam.getProjectionMatrix(m_mvp.projection);

    logPossibleGLError();
    m_baseshader.use();

    {
        m_lightsbuffer.updateData(0, sizeof(ShaderLight) * m_lights.size(),
                                  m_lights.data());
        m_baseshader.setUniform("nLights", (int)m_lights.size());
    }
    m_baseshader.setUniform("uCameraPosition", m_maincam.getPosition());
    m_baseshader.setUniform("enableNormal", m_enablenormal);
    m_baseshader.setUniform("enableParallax", m_enableparallax);
    logPossibleGLError();

    m_scene.draw(
        m_baseshader,
        [this](const auto& scene, const auto& mesh) {
            m_mvp.model = scene.getModelMatrix() * mesh.m_objmat;
            m_mvp.normalMatrix = glm::transpose(glm::inverse(m_mvp.model));
            m_mvpbuffer.updateData(0, sizeof(MVP), &m_mvp);
        },
        m_wireframe ? GL_LINE : GL_FILL);
    logPossibleGLError();
}
void DSSApplication::clear() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    logPossibleGLError();
}
void DSSApplication::keyboard() {
    if (keyForward())
        m_maincam.processKeyboard(CameraMovement::FORWARD, getFrameDeltaTime());
    if (keyBackward())
        m_maincam.processKeyboard(CameraMovement::BACKWARD,
                                  getFrameDeltaTime());
    if (keyLeft())
        m_maincam.processKeyboard(CameraMovement::LEFT, getFrameDeltaTime());
    if (keyRight())
        m_maincam.processKeyboard(CameraMovement::RIGHT, getFrameDeltaTime());
    if (glfwGetKey(getWindow(), GLFW_KEY_R)) {
        m_maincam = Camera();
    }
}
void DSSApplication::mouse() {
    glfwSetCursorPosCallback(getWindow(), mouseCallback);
    glfwSetScrollCallback(getWindow(), scrollCallback);
}
void DSSApplication::loop() {
    m_maincam.m_aspect = getWindowRatio();
    // render
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    clear();

    scene();

    skybox();

    keyboard();

    mouse();

    gui();
}

void DSSApplication::afterCleanup() {
    ofstream ofs("camera.txt");
    auto p = m_maincam.getPosition();
    auto up = m_maincam.up;
    auto yaw = m_maincam.yaw;
    auto pitch = m_maincam.pitch;
    auto aspect = m_maincam.m_aspect;
    ofs << p.x << " " << p.y << " " << p.z << endl;
    ofs << up.x << " " << up.y << " " << up.z << endl;
    ofs << yaw << " " << pitch << " " << aspect;
    ofs.close();
}
