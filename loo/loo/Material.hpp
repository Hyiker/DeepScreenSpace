#ifndef LOO_LOO_MATERIAL_HPP
#define LOO_LOO_MATERIAL_HPP
#include <glog/logging.h>
#include <tiny_obj_loader.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <memory>

#include "Texture.hpp"
#include "UniformBuffer.hpp"
#include "loo.hpp"
namespace loo {
class LOO_EXPORT Material {
   public:
    virtual void bind() = 0;
};
struct LOO_EXPORT ShaderSimpleMaterial {
    // std140 pad vec3 to 4N(N = 4B)
    // using vec4 to save your day
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

    float shininess;
    float ior;
    int illum;
    ShaderSimpleMaterial(glm::vec3 ambient, glm::vec3 diffuse,
                         glm::vec3 specular, float shininess, float ior,
                         int illum)
        : ambient(ambient, 1),
          diffuse(diffuse, 1),
          specular(specular, 1),
          shininess(shininess),
          ior(ior),
          illum(illum) {}
};
class LOO_EXPORT SimpleMaterial : public Material {
    ShaderSimpleMaterial m_shadermaterial;
    static std::shared_ptr<SimpleMaterial> defaultMaterial;
    // global uniform buffer for SimpleMaterial
    static std::unique_ptr<UniformBuffer> uniformBuffer;

   public:
    ShaderSimpleMaterial& getShaderMaterial() { return m_shadermaterial; }
    SimpleMaterial(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,
                   float shininess, float ior, int illum)
        : m_shadermaterial(ambient, diffuse, specular, shininess, ior, illum) {
        if (SimpleMaterial::uniformBuffer == nullptr) {
            SimpleMaterial::uniformBuffer = std::make_unique<UniformBuffer>(
                shaderMaterialBindingPort, sizeof(ShaderSimpleMaterial));
        }
    }
    static std::shared_ptr<SimpleMaterial> getDefault();
    void bind() override;
    std::shared_ptr<Texture> ambientTex{};
    std::shared_ptr<Texture> diffuseTex{};
    std::shared_ptr<Texture> displacementTex{};
    std::shared_ptr<Texture> normalTex{};
    std::shared_ptr<Texture> specularTex{};
    std::shared_ptr<Texture> alphaTex{};
};
LOO_EXPORT std::shared_ptr<Material> createSimpleMaterialFromObjFile(
    const tinyobj::material_t& mat, std::filesystem::path objParent);
}  // namespace loo
#endif /* LOO_LOO_MATERIAL_HPP */