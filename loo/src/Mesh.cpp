#include "loo/Mesh.hpp"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glog/logging.h>

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
    // bind material uniforms
    material->bind();

    // sp.setUniform("material.ambient", material->ambient);
    // sp.setUniform("material.diffuse", material->diffuse);
    // sp.setUniform("material.specular", material->specular);
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
                   GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}

using namespace Assimp;

// https://learnopengl-cn.github.io/03%20Model%20Loading/03%20Model/
static std::shared_ptr<Mesh> processAssimpMesh(
    aiMesh* mesh, const aiScene* scene, fs::path objParent,
    const glm::mat4& basicTransform) {
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unordered_map<Vertex, unsigned int> uniqueVertices;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
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
        if (mesh->mTextureCoords[0])  // does the mesh contain texture
                                      // coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We
            // thus make the assumption that we won't use models where a vertex
            // can have multiple texture coordinates so we always take the first
            // set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoord = vec;
            // tangent
            // vector.x = mesh->mTangents[i].x;
            // vector.y = mesh->mTangents[i].y;
            // vector.z = mesh->mTangents[i].z;
            // vertex.Tangent = vector;
            // bitangent
            // vector.x = mesh->mBitangents[i].x;
            // vector.y = mesh->mBitangents[i].y;
            // vector.z = mesh->mBitangents[i].z;
            // vertex.Bitangent = vector;
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
    // return a mesh object created from the extracted mesh data
    return make_shared<Mesh>(std::move(vertices), std::move(indices), mat,
                             mesh->mName.C_Str(), basicTransform);
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
    const auto scene =
        importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs |
                                        aiProcess_GenNormals);
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
           texCoord == v.texCoord;
}
}  // namespace loo
