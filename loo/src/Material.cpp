#include "loo/Material.hpp"

#include <glog/logging.h>

#include <memory>
#include <string>
namespace loo {
using namespace std;
using namespace tinyobj;
using namespace glm;
namespace fs = std::filesystem;
static unordered_map<string, shared_ptr<Texture>> uniqueTexture;

std::shared_ptr<Material> createSimpleMaterialFromObjFile(const material_t& mat,
                                                          fs::path objParent) {
    glm::vec3 ambient, diffuse, specular;
    for (int i = 0; i < 3; i++) {
        ambient[i] = mat.ambient[i];
        diffuse[i] = mat.diffuse[i];
        specular[i] = mat.specular[i];
    }
    float shininess = mat.shininess;
    float ior = mat.ior;
    int illum = mat.illum;

    auto material = make_shared<SimpleMaterial>(ambient, diffuse, specular,
                                                shininess, ior, illum);
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
void SimpleMaterial::bind() {
    SimpleMaterial::uniformBuffer->updateData(&m_shadermaterial);
}

shared_ptr<SimpleMaterial> SimpleMaterial::getDefault() {
    if (!defaultMaterial) {
        defaultMaterial = std::make_shared<SimpleMaterial>(
            glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), 0, 1.0,
            5);
    }
    return defaultMaterial;
}
shared_ptr<SimpleMaterial> SimpleMaterial::defaultMaterial = nullptr;
unique_ptr<UniformBuffer> SimpleMaterial::uniformBuffer = nullptr;

}  // namespace loo