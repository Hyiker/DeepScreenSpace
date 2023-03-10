#include "loo/Mesh.hpp"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glog/logging.h>
#include <meshoptimizer.h>

#include <assimp/Importer.hpp>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"
namespace loo {
using namespace std;
using namespace glm;
namespace fs = std::filesystem;

void Vertex::orthogonalizeTangent() {
    tangent = normalize(tangent);
    normal = normalize(normal);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    auto B = cross(normal, tangent);
    if (dot(B, bitangent) < 0) {
        // flip bitangent if necessary
        // THIS IS IMPORTANT
        B = -B;
    }
    bitangent = B;
}

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

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid*)(offsetof(Vertex, tangent)));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid*)offsetof(Vertex, bitangent));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
}

size_t Mesh::countVertex() const { return vertices.size(); }
size_t Mesh::countTriangles(bool lod) const {
    if (!lod) return indices.size() / 3;
    GLuint indexOffset = m_lodoffsets[m_lod],
           indexCount =
               (m_lod == m_lodoffsets.size() - 1 ? indices.size()
                                                 : m_lodoffsets[m_lod + 1]) -
               indexOffset;
    return indexCount / 3;
}

void Mesh::draw(ShaderProgram& sp, GLenum drawMode, bool tessellation) const {
    glPolygonMode(GL_FRONT_AND_BACK, drawMode);
    glBindVertexArray(vao);
    // bind material uniforms
    material->bind(sp);
    sp.setUniform("meshLod", m_lod);
    GLuint indexOffset = m_lodoffsets[m_lod],
           indexCount =
               (m_lod == m_lodoffsets.size() - 1 ? indices.size()
                                                 : m_lodoffsets[m_lod + 1]) -
               indexOffset;
    logPossibleGLError();
    glDrawElements(tessellation ? GL_PATCHES : GL_TRIANGLES,
                   static_cast<GLuint>(indexCount), GL_UNSIGNED_INT,
                   (void*)(indexOffset * sizeof(GLuint)));

    glBindVertexArray(0);
}

void Mesh::updateLod(float screenProportion) {
    if (screenProportion < 0.05) {
        m_lod = 2;
    } else if (screenProportion < 0.2) {
        m_lod = 1;
    } else {
        m_lod = 0;
    }
}

using namespace Assimp;

// https://learnopengl-cn.github.io/03%20Model%20Loading/03%20Model/
static std::shared_ptr<Mesh> processAssimpMesh(
    aiMesh* mesh, const aiScene* scene, fs::path objParent,
    const glm::mat4& basicTransform) {
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture2D> textures;
    unordered_map<Vertex, unsigned int> uniqueVertices;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};
        glm::vec3 vector;  // we declare a placeholder vector since assimp uses
                           // its own vector class that doesn't directly convert
                           // to glm's vec3 class so we transfer the data to
                           // this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        // normals
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0]) [[likely]] {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We
            // thus make the assumption that we won't use models where a vertex
            // can have multiple texture coordinates so we always take the first
            // set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoord = vec;
            if (mesh->mTangents) [[likely]] {
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent = vector;

                // re-smoothing tangents
                // https://github.com/assimp/assimp/issues/3191
                vertex.orthogonalizeTangent();
            }
        } else
            vertex.texCoord = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle)
    // and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse
    // texture should be named as 'texture_diffuseN' where N is a sequential
    // number ranging from 1 to MAX_SAMPLER_NUMBER. Same applies to other
    // texture as the following list summarizes: diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN
    auto mat = createSimpleMaterialFromAssimp(material, objParent);
    vector<unsigned int> lodOffsets(3, 0);
    {
        vector<unsigned int> remap(indices.size());
        // optimization
        auto vertex_count = meshopt_generateVertexRemap(
            remap.data(), indices.data(), indices.size(), vertices.data(),
            vertices.size(), sizeof(Vertex));
        vector<unsigned int> rmpIndices(indices.size());
        vector<Vertex> rmpVertices(vertex_count);
        meshopt_remapIndexBuffer(rmpIndices.data(), indices.data(),
                                 remap.size(), remap.data());
        meshopt_remapVertexBuffer(rmpVertices.data(), vertices.data(),
                                  indices.size(), sizeof(Vertex), remap.data());
        indices = rmpIndices;
        vertices = rmpVertices;
        // lod simplification ratio
        float thresholds[2]{0.25f, 0.05f};
        int index_count = indices.size();
        meshopt_optimizeVertexCache(indices.data(), indices.data(),
                                    indices.size(), vertex_count);
        for (int i = 0; i < 2; i++) {
            float threshold = thresholds[i];
            size_t target_index_count = size_t(index_count * threshold);
            float target_error = 1e-2f;

            std::vector<unsigned int> lod(index_count);
            float lod_error = 0.f;
            lod.resize(meshopt_simplify(lod.data(), indices.data(), index_count,
                                        &vertices[0].position.x, vertex_count,
                                        sizeof(Vertex), target_index_count,
                                        target_error, 0, &lod_error));
            meshopt_optimizeVertexCache(lod.data(), lod.data(), lod.size(),
                                        vertex_count);
            lodOffsets[i + 1] = indices.size();
            // LOG(INFO) << "Lod simplification ratio: "
            //           << lod.size() / (float)index_count * 100.0 << "%";
            indices.insert(indices.end(), lod.begin(), lod.end());
        }
    }

    // return a mesh object created from the extracted mesh data
    return make_shared<Mesh>(std::move(vertices), std::move(indices), mat,
                             mesh->mName.C_Str(), basicTransform,
                             std::move(lodOffsets));
}

static void processAssimpNode(aiNode* node, const aiScene* scene,
                              vector<shared_ptr<Mesh>>& meshes,
                              fs::path objParent,
                              const glm::mat4& basicTransform) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(
            processAssimpMesh(mesh, scene, objParent, basicTransform));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processAssimpNode(node->mChildren[i], scene, meshes, objParent,
                          basicTransform);
    }
}

vector<shared_ptr<Mesh>> createMeshFromFile(const string& filename,
                                            const glm::mat4& basicTransform) {
    Importer importer;
    vector<shared_ptr<Mesh>> meshes;
    fs::path filePath(filename);
    fs::path fileParent = filePath.parent_path();
    const auto scene = importer.ReadFile(
        filename, aiProcess_Triangulate | aiProcess_FlipUVs |
                      aiProcess_GenNormals | aiProcess_GenNormals |
                      aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        LOG(ERROR) << "Assimp: " << importer.GetErrorString() << endl;
        return {};
    }
    processAssimpNode(scene->mRootNode, scene, meshes, fileParent,
                      basicTransform);
    return std::move(meshes);
}

bool Vertex::operator==(const Vertex& v) const {
    return position == v.position && normal == v.normal &&
           texCoord == v.texCoord && tangent == v.tangent &&
           bitangent == v.bitangent;
}
}  // namespace loo
