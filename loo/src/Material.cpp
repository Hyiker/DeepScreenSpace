#include "loo/Material.hpp"

#include <glog/logging.h>

#include <memory>
#include <string>

#include "assimp/material.h"
#include "assimp/types.h"
#include "glm/fwd.hpp"
#include "loo/Shader.hpp"
namespace loo {
using namespace std;
using namespace glm;
namespace fs = std::filesystem;
static unordered_map<string, shared_ptr<Texture2D>> uniqueTexture;

static inline glm::vec3 aiColor3D2Glm(const aiColor3D& aColor) {
    return {aColor.r, aColor.g, aColor.b};
}

static shared_ptr<Texture2D> createMaterialTextures(
    const aiMaterial* mat, aiTextureType type, fs::path objParent,
    unsigned int options = TEXTURE_OPTION_MIPMAP |
                           TEXTURE_OPTION_CONVERT_TO_LINEAR) {
    vector<Texture2D> textures;
    if (mat->GetTextureCount(type)) {
        // TODO: support multilayer texture
        aiString str;
        mat->GetTexture(type, 0, &str);
        return createTexture2DFromFile(
            uniqueTexture, (objParent / str.C_Str()).string(), options);
    } else {
        return nullptr;
    }
}

std::shared_ptr<Material> createSimpleMaterialFromAssimp(
    const aiMaterial* aMaterial, fs::path objParent) {
    aiColor3D color(0, 0, 0);
    aMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
    glm::vec3 ambient = aiColor3D2Glm(color);
    aMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    glm::vec3 diffuse = aiColor3D2Glm(color);
    aMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
    glm::vec3 specular = aiColor3D2Glm(color);
    aMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);

    float shininess, _ior;
    aMaterial->Get(AI_MATKEY_SHININESS, shininess);
    aMaterial->Get(AI_MATKEY_REFRACTI, _ior);

    auto material = make_shared<SimpleMaterial>(ambient, diffuse, specular,
                                                glm::vec3(_ior), shininess);
    material->ambientTex =
        createMaterialTextures(aMaterial, aiTextureType_AMBIENT, objParent);

    material->diffuseTex =
        createMaterialTextures(aMaterial, aiTextureType_DIFFUSE, objParent);

    material->specularTex =
        createMaterialTextures(aMaterial, aiTextureType_SPECULAR, objParent);

    material->displacementTex = createMaterialTextures(
        aMaterial, aiTextureType_DISPLACEMENT, objParent, 0x0);
    // material->displacementTex->setSizeFilter(GL_NEAREST, GL_NEAREST);
    // obj file saves normal map as bump maps
    // FUCK YOU, wavefront obj
    material->normalTex = createMaterialTextures(
        aMaterial, aiTextureType_NORMALS, objParent, TEXTURE_OPTION_MIPMAP);
    material->opacityTex = createMaterialTextures(
        aMaterial, aiTextureType_OPACITY, objParent, TEXTURE_OPTION_MIPMAP);
    material->heightTex = createMaterialTextures(
        aMaterial, aiTextureType_HEIGHT, objParent, TEXTURE_OPTION_MIPMAP);
    return material;
}

void SimpleMaterial::bind(const ShaderProgram& sp) {
    SimpleMaterial::uniformBuffer->updateData(&m_shadermaterial);
    sp.setTexture(SHADER_BINDING_PORT_SM_AMBIENT,
                  ambientTex ? *ambientTex : Texture2D::getWhiteTexture());
    sp.setTexture(SHADER_BINDING_PORT_SM_DIFFUSE,
                  diffuseTex ? *diffuseTex : Texture2D::getWhiteTexture());
    sp.setTexture(SHADER_BINDING_PORT_SM_SPECULAR,
                  specularTex ? *specularTex : Texture2D::getWhiteTexture());
    sp.setTexture(
        SHADER_BINDING_PORT_SM_DISPLACEMENT,
        displacementTex ? *displacementTex : Texture2D::getBlackTexture());
    sp.setTexture(SHADER_BINDING_PORT_SM_NORMAL,
                  normalTex ? *normalTex : Texture2D::getBlackTexture());
    sp.setTexture(SHADER_BINDING_PORT_SM_OPACITY,
                  opacityTex ? *opacityTex : Texture2D::getBlackTexture());
    sp.setTexture(SHADER_BINDING_PORT_SM_HEIGHT,
                  heightTex ? *heightTex : Texture2D::getBlackTexture());
}

shared_ptr<SimpleMaterial> SimpleMaterial::getDefault() {
    if (!defaultMaterial) {
        defaultMaterial = std::make_shared<SimpleMaterial>(
            glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec3(0, 0, 0),
            glm::vec3(1.5), 1.0);
    }
    return defaultMaterial;
}
shared_ptr<SimpleMaterial> SimpleMaterial::defaultMaterial = nullptr;
unique_ptr<UniformBuffer> SimpleMaterial::uniformBuffer = nullptr;

}  // namespace loo