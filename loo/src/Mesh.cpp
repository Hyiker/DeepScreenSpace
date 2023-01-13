#include "loo/Mesh.hpp"

#include <glog/logging.h>

#include <format>
#include <memory>
#include <vector>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <filesystem>
#include <string>
#include <unordered_map>
namespace loo {
using namespace std;
using namespace tinyobj;
namespace fs = std::filesystem;
void Mesh::prepare() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid*)(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

size_t Mesh::countVertex() const { return vertices.size(); }

void Mesh::draw(ShaderProgram& sp) const {
    glBindVertexArray(vao);

    sp.setUniform("material.ambient", material->ambient);
    sp.setUniform("material.diffuse", material->diffuse);
    sp.setUniform("material.specular", material->specular);
    // sp.setUniform("material.shininess", material.shininess);
    // sp.setUniform("material.ior", material.ior);
    // sp.setUniform("material.illum", material.illum);

    // if (material.ambientTex)
    //     sp.setTexture("material.ambientTex", 0,
    //     material.ambientTex->getId());
    // if (material.diffuseTex)
    //     sp.setTexture("material.diffuseTex", 1,
    //     material.diffuseTex->getId());
    // if (material.specularTex)
    //     sp.setTexture("material.specularTex", 2,
    //     material.specularTex->getId());
    // if (material.displacementTex)
    //     sp.setTexture("material.displacementTex", 3,
    //                   material.displacementTex->getId());
    // ;
    // if (material.normalTex)
    //     sp.setTexture("material.normalTex", 4, material.normalTex->getId());

    glDrawElements(GL_TRIANGLES, static_cast<GLuint>(indices.size()),
                   GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

static unordered_map<string, shared_ptr<Texture>> uniqueTexture;

static std::shared_ptr<Material> createMaterialFromObjFile(
    const material_t& mat, fs::path objParent) {
    auto material = make_shared<Material>();
    for (int i = 0; i < 3; i++) {
        material->ambient[i] = mat.ambient[i];
        material->diffuse[i] = mat.diffuse[i];
        material->specular[i] = mat.specular[i];
    }
    material->shininess = mat.shininess;
    material->ior = mat.ior;
    material->illum = mat.illum;

    if (mat.ambient_texname.length())
        material->ambientTex = createTextureFromFile(
            uniqueTexture, (objParent / mat.ambient_texname).string());
    if (mat.diffuse_texname.length())
        material->diffuseTex = createTextureFromFile(
            uniqueTexture, (objParent / mat.diffuse_texname).string());
    if (mat.specular_texname.length())
        material->specularTex = createTextureFromFile(
            uniqueTexture, (objParent / mat.specular_texname).string());
    if (mat.displacement_texname.length())
        material->displacementTex = createTextureFromFile(
            uniqueTexture, (objParent / mat.displacement_texname).string());
    if (mat.normal_texname.length())
        material->normalTex = createTextureFromFile(
            uniqueTexture, (objParent / mat.normal_texname).string());
    return material;
}
std::vector<std::shared_ptr<Mesh>> createMeshFromObjFile(
    const std::string& filename) {
    static shared_ptr<Material> defaultMaterial{nullptr};
    if (!defaultMaterial) {
        defaultMaterial = make_shared<Material>();
        defaultMaterial->ambient = glm::vec3(1.0, 0.0, 1.0);
        defaultMaterial->diffuse = glm::vec3(1.0, 0.0, 1.0);
        defaultMaterial->specular = glm::vec3(1.0, 0.0, 1.0);

        defaultMaterial->shininess = 0;
        defaultMaterial->ior = 1.0;
        defaultMaterial->illum = 1.0;
    }
    ObjReader reader;
    ObjReaderConfig config;

    if (!reader.ParseFromFile(filename, config)) {
        if (!reader.Error().empty()) {
            LOG(ERROR) << format("{}: failed parsing object file, {}", filename,
                                 reader.Error())
                       << endl;
            return {};
        }
    }
    fs::path objRoot(filename);
    fs::path objParent = objRoot.parent_path();

    if (!reader.Warning().empty()) {
        LOG(WARNING) << "TinyObjReader: " << reader.Warning() << endl;
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();
    vector<shared_ptr<Mesh>> meshes;

    unordered_map<string, std::shared_ptr<Texture>> uniqueTexture;
    unordered_map<Vertex, unsigned int> uniqueVertices;
    for (auto& shape : shapes) {
        vector<unsigned int> indices;
        vector<Vertex> vertices;

        std::shared_ptr<Material> material{};

        // one material for one mesh
        if (shape.mesh.material_ids.size()) {
            int n = 0;
            n++;
            while (n < shape.mesh.material_ids.size() &&
                   shape.mesh.material_ids[n] == -1) {
                n++;
            }
            if (shape.mesh.material_ids[0] != -1) {
                material = createMaterialFromObjFile(
                    materials[shape.mesh.material_ids[0]], objParent);
            } else {
                material = defaultMaterial;
            }
        }
        for (const auto& idx : shape.mesh.indices) {
            Vertex vertex;
            // access to vertex
            vertex.position.x =
                attrib.vertices[3 * size_t(idx.vertex_index) + 0];
            vertex.position.y =
                attrib.vertices[3 * size_t(idx.vertex_index) + 1];
            vertex.position.z =
                attrib.vertices[3 * size_t(idx.vertex_index) + 2];

            // Check if `normal_index` is zero or positive. negative = no
            // normal data
            if (idx.normal_index >= 0) {
                vertex.normal.x =
                    attrib.normals[3 * size_t(idx.normal_index) + 0];
                vertex.normal.y =
                    attrib.normals[3 * size_t(idx.normal_index) + 1];
                vertex.normal.z =
                    attrib.normals[3 * size_t(idx.normal_index) + 2];
            }

            // Check if `texcoord_index` is zero or positive. negative = no
            // texcoord data
            if (idx.texcoord_index >= 0) {
                vertex.texCoord.x =
                    attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                vertex.texCoord.y =
                    attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
            }
            if (!uniqueVertices.count(vertex)) {
                uniqueVertices[vertex] = vertices.size();
                vertices.emplace_back(std::move(vertex));
            }
            indices.push_back(uniqueVertices[vertex]);
        }
        auto mesh = make_shared<Mesh>(std::move(vertices), std::move(indices),
                                      material, shape.name);
        meshes.push_back(mesh);
    }
    return meshes;
}
bool Vertex::operator==(const Vertex& v) const {
    return position == v.position && normal == v.normal &&
           texCoord == v.texCoord;
}
}  // namespace loo
