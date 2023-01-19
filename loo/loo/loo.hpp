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

// clang-format off

constexpr int SHADER_BINDING_PORT_SKYBOX = 0;
// simple material binding
constexpr int SHADER_BINDING_PORT_SM_PARAMS = 1;
constexpr int SHADER_BINDING_PORT_SM_AMBIENT = 2;
constexpr int SHADER_BINDING_PORT_SM_DIFFUSE = 3;
constexpr int SHADER_BINDING_PORT_SM_SPECULAR = 4;
constexpr int SHADER_BINDING_PORT_SM_DISPLACEMENT = 5;
constexpr int SHADER_BINDING_PORT_SM_NORMAL = 6;
// pbr material binding
// TODO
constexpr int SHADER_BINDING_PORT_PBRM_PARAMS = 1;
constexpr int SHADER_BINDING_PORT_PBRM_AMBIENT = 2;
constexpr int SHADER_BINDING_PORT_PBRM_DIFFUSE = 3;
constexpr int SHADER_BINDING_PORT_PBRM_SPECULAR = 4;
constexpr int SHADER_BINDING_PORT_PBRM_DISPLACEMENT = 5;
constexpr int SHADER_BINDING_PORT_PBRM_NORMAL = 6;
// clang-format on

extern int looBindingPortMax;

// Initialize libraries include:
// * glog
// * shaderMaterialBindingPort
LOO_EXPORT void initialize(const char* argv0);

}  // namespace loo

#endif /* LOO_LOO_LOO_HPP */
