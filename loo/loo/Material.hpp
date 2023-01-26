#ifndef LOO_LOO_MATERIAL_HPP
#define LOO_LOO_MATERIAL_HPP
#include <assimp/material.h>
#include <glog/logging.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <memory>

#include "Texture.hpp"
#include "UniformBuffer.hpp"
#include "loo.hpp"
namespace loo {
class LOO_EXPORT Material {
   public:
    // setup uniforms and textures for shader program
    virtual void bind(const ShaderProgram& sp) = 0;
};
struct LOO_EXPORT ShaderSimpleMaterial {
    // std140 pad vec3 to 4N(N = 4B)
    // using vec4 to save your day
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

    glm::vec4 ior;
    float shininess;
    ShaderSimpleMaterial(glm::vec3 ambient, glm::vec3 diffuse,
                         glm::vec3 specular, glm::vec3 ior, float shininess)
        : ambient(ambient, 1),
          diffuse(diffuse, 1),
          specular(specular, 1),
          ior(ior, 1),
          shininess(shininess) {}
};
class LOO_EXPORT SimpleMaterial : public Material {
    ShaderSimpleMaterial m_shadermaterial;
    static std::shared_ptr<SimpleMaterial> defaultMaterial;
    // global uniform buffer for SimpleMaterial
    static std::unique_ptr<UniformBuffer> uniformBuffer;

   public:
    ShaderSimpleMaterial& getShaderMaterial() { return m_shadermaterial; }
    SimpleMaterial(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,
                   glm::vec3 ior, float shininess)
        : m_shadermaterial(ambient, diffuse, specular, ior, shininess) {
        if (SimpleMaterial::uniformBuffer == nullptr) {
            SimpleMaterial::uniformBuffer = std::make_unique<UniformBuffer>(
                SHADER_BINDING_PORT_SM_PARAMS, sizeof(ShaderSimpleMaterial));
        }
    }
    static std::shared_ptr<SimpleMaterial> getDefault();
    void bind(const ShaderProgram& sp) override;
    std::shared_ptr<Texture2D> ambientTex{};
    std::shared_ptr<Texture2D> diffuseTex{};
    std::shared_ptr<Texture2D> displacementTex{};
    std::shared_ptr<Texture2D> normalTex{};
    std::shared_ptr<Texture2D> specularTex{};
    std::shared_ptr<Texture2D> opacityTex{};
    std::shared_ptr<Texture2D> heightTex{};
};
std::shared_ptr<Material> createSimpleMaterialFromAssimp(
    const aiMaterial* aMaterial, std::filesystem::path objParent);
}  // namespace loo
#endif /* LOO_LOO_MATERIAL_HPP */
