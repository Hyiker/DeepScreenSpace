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

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"
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
      m_baseshader{Shader(BASE_VERT, ShaderType::Vertex),
                   Shader(BASE_FRAG, ShaderType::Fragment)},
      m_skyboxshader{Shader(SKYBOX_VERT, ShaderType::Vertex),
                     Shader(SKYBOX_FRAG, ShaderType::Fragment)},
      m_scene(),
      m_maincam(),
      m_mvpbuffer(0, sizeof(MVP)),
      m_lightsbuffer(SHADER_BINDING_LIGHTS,
                     sizeof(ShaderLight) * SHADER_LIGHTS_MAX),
      m_globalquad(make_shared<Quad>()),
      m_finalprocess(getWidth(), getHeight(), m_globalquad),
      m_dss(getWidth(), getHeight()) {
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
        m_skyboxtex = createTextureCubeMapFromFiles(
            skyboxFilenames,
            TEXTURE_OPTION_CONVERT_TO_LINEAR | TEXTURE_OPTION_MIPMAP);
    }
    // scene light
    {
        m_lights.push_back(
            createDirectionalLight(glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0)));
    }
    // final pass related
    {
        m_scenefb.init();
        m_scenetexture = make_shared<Texture2D>();
        m_scenetexture->init();
        m_scenetexture->setup(getWidth(), getHeight(), GL_RGB32F, GL_RGB,
                              GL_FLOAT, 1);
        m_scenetexture->setSizeFilter(GL_LINEAR, GL_LINEAR);
        panicPossibleGLError();
        m_scenedepthrb.init(GL_DEPTH_COMPONENT32, getWidth(), getHeight());

        m_scenefb.attachTexture(*m_scenetexture, GL_COLOR_ATTACHMENT0, 0);
        m_scenefb.attachRenderbuffer(m_scenedepthrb, GL_DEPTH_ATTACHMENT);
        m_finalprocess.init();
    }
    panicPossibleGLError();
}
void DSSApplication::gui() {
    auto& io = ImGui::GetIO();
    float h = io.DisplaySize.y;
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove;
    {
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::SetNextWindowSize(ImVec2(300, h * 0.3));
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        if (ImGui::Begin("Dashboard", nullptr, windowFlags)) {
            // OpenGL option
            if (ImGui::CollapsingHeader("General info",
                                        ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text(
                    "Frame generation delay: %.3f ms/frame\n"
                    "FPS: %.1f",
                    1000.0f / io.Framerate, io.Framerate);
                const GLubyte* renderer = glGetString(GL_RENDERER);
                const GLubyte* version = glGetString(GL_VERSION);
                ImGui::Text("Renderer: %s", renderer);
                ImGui::Text("OpenGL Version: %s", version);
                int triangleCount = m_scene.countTriangle();
                const char* base = "";
                if (triangleCount > 5000) {
                    triangleCount /= 1000;
                    base = "k";
                }
                if (triangleCount > 5000) {
                    triangleCount /= 1000;
                    base = "M";
                }
                ImGui::Text(
                    "Scene meshes: %d\n"
                    "Scene triangles: %d%s",
                    (int)m_scene.countMesh(), triangleCount, base);
            }
            if (ImGui::CollapsingHeader("Deep screen space info",
                                        ImGuiTreeNodeFlags_DefaultOpen)) {
                int surfelCount = m_dss.getSurfelCount();
                const char* base = "";
                if (surfelCount > 5000) {
                    surfelCount /= 1000;
                    base = "k";
                }
                if (surfelCount > 5000) {
                    surfelCount /= 1000;
                    base = "M";
                }
                ImGui::Text("Surfel count: %d%s", surfelCount, base);
            }
        }
    }

    {
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::SetNextWindowSize(ImVec2(300, h * 0.5));
        ImGui::SetNextWindowPos(ImVec2(0, h * 0.3), ImGuiCond_Always);
        if (ImGui::Begin("Options", nullptr, windowFlags)) {
            // Sun
            if (ImGui::CollapsingHeader("Sun",
                                        ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderFloat3("Direction", (float*)&m_lights[0].direction,
                                    -1, 1);
                ImGui::SliderFloat("Intensity", (float*)&m_lights[0].intensity,
                                   0.0, 100.0);
            }

            // OpenGL option
            if (ImGui::CollapsingHeader("OpenGL options",
                                        ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Wire frame mode", &m_wireframe);
                ImGui::Checkbox("Normal mapping", &m_enablenormal);
                ImGui::Checkbox("Parallax mapping", &m_enableparallax);
                ImGui::Checkbox("Visualize lod", &m_lodvisualize);
            }
        }
    }
    {
        float h_img = h * 0.2,
              w_img = h_img / io.DisplaySize.y * io.DisplaySize.x;
        ImGui::SetNextWindowBgAlpha(1.0f);
        vector<shared_ptr<Texture2D>> textures{
            m_scenetexture, m_dss.getSurfelVisualizationResult()};
        ImGui::SetNextWindowSize(ImVec2(w_img * textures.size(), h_img));
        ImGui::SetNextWindowPos(ImVec2(0, h * 0.8), ImGuiCond_Always);
        if (ImGui::Begin("Textures", nullptr,
                         windowFlags | ImGuiWindowFlags_NoDecoration)) {
            for (auto tex : textures) {
                ImGui::Image((void*)(intptr_t)tex->getId(),
                             ImVec2(w_img, h_img), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::SameLine();
            }
        }
    }
}

void DSSApplication::finalprocess() {
    m_finalprocess.render(*m_scenetexture, m_lodvisualize);
}
void DSSApplication::skybox() {
    if (m_skyboxtex) {
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
    m_baseshader.setUniform("enableLodVisualize", (int)m_lodvisualize);
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
// Deep screen space process:
// 1. shuffle scene position and normal onto each layer of  multiresolution
//    texture array
// 2. scene surfelization
// 3. apply splatting for each surfel and each multi-resolution texture layer
//    save the shading result in a multi-layer texture array
void DSSApplication::deepScreenSpace() {
    m_maincam.getViewMatrix(m_mvp.view);
    m_maincam.getProjectionMatrix(m_mvp.projection);
    m_mvpbuffer.updateData(0, sizeof(MVP), &m_mvp);
    {
        Framebuffer::bindDefault();
        // surfelization
        auto& surfelizeShader = m_dss.prepareSurfelization();
        surfelizeShader.use();
        surfelizeShader.setUniform("aspect", m_maincam.m_aspect);
        surfelizeShader.setUniform("scale", 1e-3f);
        surfelizeShader.setUniform("viewMatrix", m_maincam.getViewMatrix());
        surfelizeShader.setUniform("fov", m_maincam.m_fov);
        surfelizeShader.setUniform("cameraPosition", m_maincam.getPosition());
        m_scene.draw(
            surfelizeShader,
            [this](const auto& scene, const auto& mesh) {
                m_mvp.model = scene.getModelMatrix() * mesh.m_objmat;
                m_mvp.normalMatrix = glm::transpose(glm::inverse(m_mvp.model));
                m_mvpbuffer.updateData(0, sizeof(MVP), &m_mvp);
            },
            GL_FILL, DRAW_FLAG_TESSELLATION);
        panicPossibleGLError();
    }
    m_dss.copySurfelData();
    { m_dss.surfelVisualization(); }
    { m_dss.renderSplatting(); }
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

    {
        m_scenefb.bind();

        clear();

        scene();

        skybox();
        m_scenefb.unbind();
    }

    { deepScreenSpace(); }

    finalprocess();

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
