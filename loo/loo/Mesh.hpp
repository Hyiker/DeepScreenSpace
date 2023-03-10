#ifndef LOO_LOO_MESH_HPP
#define LOO_LOO_MESH_HPP
#include <memory>
#include <utility>
#include <vector>

#include "Material.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "predefs.hpp"
namespace loo {

struct LOO_EXPORT Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    bool operator==(const Vertex& v) const;
    // Re-orthogonalization tangents, modifies tangent and bitangent
    void orthogonalizeTangent();
};

struct LOO_EXPORT Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> material;
    std::string name;
    glm::mat4 m_objmat;

    Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indicies,
         std::shared_ptr<Material> material, std::string name,
         const glm::mat4& transform, std::vector<unsigned int>&& lodOffsets)
        : vertices(vertices),
          indices(indicies),
          material(material),
          name(std::move(name)),
          m_objmat(transform),
          // TODO: controllable lod levels
          m_lodoffsets(lodOffsets) {}

    GLuint vao, vbo, ebo;
    void prepare();
    size_t countVertex() const;
    size_t countTriangles(bool lod = true) const;

    void draw(ShaderProgram& sp, GLenum drawMode = GL_FILL,
              bool tessellation = false) const;
    void updateLod(float screenProportion);
    int getLod() const { return m_lod; }

   private:
    // Level of detail
    // the larger the less detail
    // now using 0-1-2
    int m_lod{0};
    std::vector<unsigned int> m_lodoffsets;
};

LOO_EXPORT std::vector<std::shared_ptr<Mesh>> createMeshFromFile(
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
