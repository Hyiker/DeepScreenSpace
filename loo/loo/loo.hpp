#ifndef LOO_LOO_LOO_HPP
#define LOO_LOO_LOO_HPP
#include "predefs.hpp"
namespace loo {
class Shader;
class ShaderProgram;
class Application;
class Camera;
class Scene;
class UniformBuffer;

static constexpr int DEFAULT_SHADER_MATERIAL_BINGDING_PORT = 1;
struct LooBasicConfigs {
    int shaderMaterialBindingPort;
    LooBasicConfigs(
        int shaderMaterialBindingPort = DEFAULT_SHADER_MATERIAL_BINGDING_PORT)
        : shaderMaterialBindingPort(shaderMaterialBindingPort) {}
};
extern int shaderMaterialBindingPort;
// Initialize libraries include:
// * glog
// * shaderMaterialBindingPort
LOO_EXPORT void initialize(const char* argv0, LooBasicConfigs* conf);

}  // namespace loo

#endif /* LOO_LOO_LOO_HPP */
