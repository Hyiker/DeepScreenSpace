#ifndef LOO_LOO_SCENE_HPP
#define LOO_LOO_SCENE_HPP
#include <glad/glad.h>

#include <functional>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Shader.hpp"
#include "Texture.hpp"
#include "predefs.hpp"

namespace loo {

struct LOO_EXPORT Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    bool operator==(const Vertex& v) const;
};

struct LOO_EXPORT Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float shininess;
    float ior;
    int illum;

    ::std::shared_ptr<Texture> ambientTex;
    ::std::shared_ptr<Texture> diffuseTex;
    ::std::shared_ptr<Texture> displacementTex;
    ::std::shared_ptr<Texture> normalTex;
    ::std::shared_ptr<Texture> specularTex;
    ::std::shared_ptr<Texture> alphaTex;
};
struct LOO_EXPORT Mesh {
    ::std::vector<Vertex> vertices;
    ::std::vector<unsigned int> indices;
    Material material;

    GLuint vao, vbo, ebo;
    void prepare();
    size_t countVertex() const;
    void draw(ShaderProgram& sp) const;
};
class LOO_EXPORT Scene {
    ::std::vector<Mesh> m_meshes;
    glm::mat4 m_modelmat{1.0};

   public:
    void scale(glm::vec3 ratio);
    void translate(glm::vec3 pos);
    glm::mat4 getModelMatrix() const;
    size_t countMesh() const;
    size_t countVertex() const;
    void draw(ShaderProgram& sp) const;
    Scene(const ::std::string& path);
};

LOO_EXPORT glm::mat4 getLightSpaceTransform(glm::vec3 lightPosition);
}  // namespace loo

namespace std {
template <>
struct LOO_EXPORT hash<loo::Vertex> {
    size_t operator()(loo::Vertex const& v) const;
};
}  // namespace std
#endif /* LOO_LOO_SCENE_HPP */
