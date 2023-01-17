#ifndef LOO_LOO_MESH_HPP
#define LOO_LOO_MESH_HPP
#include <memory>
#include <utility>
#include <vector>

#include "Shader.hpp"
#include "Texture.hpp"
#include "glm/ext/matrix_transform.hpp"
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

    std::shared_ptr<Texture> ambientTex{};
    std::shared_ptr<Texture> diffuseTex{};
    std::shared_ptr<Texture> displacementTex{};
    std::shared_ptr<Texture> normalTex{};
    std::shared_ptr<Texture> specularTex{};
    std::shared_ptr<Texture> alphaTex{};
};
struct LOO_EXPORT Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> material;
    std::string name;
    glm::mat4 m_objmat;

    Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indicies,
         std::shared_ptr<Material> material, std::string name,
         const glm::mat4& transform)
        : vertices(vertices),
          indices(indicies),
          material(material),
          name(std::move(name)),
          m_objmat(transform) {}

    GLuint vao, vbo, ebo;
    void prepare();
    size_t countVertex() const;
    void draw(ShaderProgram& sp) const;
};

LOO_EXPORT std::vector<std::shared_ptr<Mesh>> createMeshFromObjFile(
    const std::string& filename,
    const glm::mat4& basicTransform = glm::identity<glm::mat4>());

}  // namespace loo

namespace std {
template <>
struct LOO_EXPORT hash<loo::Vertex> {
    size_t operator()(loo::Vertex const& v) const;
};
}  // namespace std
#endif /* LOO_LOO_MESH_HPP */
