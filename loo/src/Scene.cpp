#include "loo/Scene.hpp"

#include <glad/glad.h>
#include <glog/logging.h>

#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <loo/glError.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <glm/gtx/hash.hpp>
#include <iostream>

namespace std {
size_t hash<loo::Vertex>::operator()(loo::Vertex const& v) const {
    return ((hash<glm::vec3>()(v.position) ^
             (hash<glm::vec3>()(v.normal) << 1)) >>
            1) ^
           (hash<glm::vec2>()(v.texCoord) << 1);
}
}  // namespace std
namespace loo {

using namespace std;
using namespace glm;
namespace fs = std::filesystem;

void Scene::scale(glm::vec3 ratio) {
    m_modelmat = glm::scale(m_modelmat, ratio);
}

void Scene::translate(glm::vec3 pos) {
    m_modelmat = glm::translate(m_modelmat, pos);
}

glm::mat4 Scene::getModelMatrix() const { return m_modelmat; }

size_t Scene::countMesh() const { return m_meshes.size(); }

size_t Scene::countVertex() const {
    size_t cnt = 0;
    for (const auto& mesh : m_meshes) {
        cnt += mesh->countVertex();
    }
    return cnt;
}
void Scene::addMeshes(vector<shared_ptr<Mesh>>&& meshes) {
    m_meshes.insert(m_meshes.end(), meshes.begin(), meshes.end());
}

// prepare the scene, move the mesh data into opengl side
void Scene::prepare() const {
    for (const auto& mesh : m_meshes) {
        mesh->prepare();
    }
}

void Scene::draw(
    ShaderProgram& sp,
    std::function<void(const Scene&, const Mesh&)> beforeDraw) const {
    for (const auto& mesh : m_meshes) {
        beforeDraw(*this, *mesh);
        mesh->draw(sp);
    }
}
void Scene::draw(ShaderProgram& sp) const {
    for (const auto& mesh : m_meshes) {
        mesh->draw(sp);
    }
}

Scene::Scene() = default;

Scene createSceneFromGLTF(const std::string& filename) {
    using namespace tinygltf;
    fs::path p(filename);
    Model model;
    TinyGLTF loader;
    std::string err;
    std::string warn;
    bool ret = p.extension() == ".gltf"
                   ? loader.LoadASCIIFromFile(&model, &err, &warn, filename)
                   : loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) LOG(WARNING) << warn << endl;
    if (!err.empty()) LOG(ERROR) << err << endl;
    if (!ret) LOG(FATAL) << "Failed to parse glTF" << endl;
    NOT_IMPLEMENTED();
    return std::move(Scene());
}

glm::mat4 getLightSpaceTransform(glm::vec3 lightPosition) {
    vec3 target(0, 0, 0);
    vec3 dir = normalize(target - lightPosition);
    vec3 up{0.0f, 1.0f, 0.0f};
    if (dir.x == 0.0f && dir.z == 0.0f) up = glm::vec3(1.0f, 0.0f, 0.0f);
    return ortho(-1.5f, 1.5f, -1.5f, 1.5f, -1.5f, 1.5f) *
           lookAt(vec3(0.0f), dir, up);
}

}  // namespace loo
