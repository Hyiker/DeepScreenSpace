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

// light binding
constexpr int SHADER_BINDING_LIGHTS = 1;

// simple material binding
constexpr int SHADER_BINDING_PORT_SM_PARAMS = 2;
constexpr int SHADER_BINDING_PORT_SM_AMBIENT = 3;
constexpr int SHADER_BINDING_PORT_SM_DIFFUSE = 4;
constexpr int SHADER_BINDING_PORT_SM_SPECULAR = 5;
constexpr int SHADER_BINDING_PORT_SM_DISPLACEMENT = 6;
constexpr int SHADER_BINDING_PORT_SM_NORMAL = 7;

// pbr material binding
// TODO
constexpr int SHADER_BINDING_PORT_PBRM_PARAMS = 2;
constexpr int SHADER_BINDING_PORT_PBRM_AMBIENT = 3;
constexpr int SHADER_BINDING_PORT_PBRM_DIFFUSE = 4;
constexpr int SHADER_BINDING_PORT_PBRM_SPECULAR = 5;
constexpr int SHADER_BINDING_PORT_PBRM_DISPLACEMENT = 6;
constexpr int SHADER_BINDING_PORT_PBRM_NORMAL = 7;

constexpr int SHADER_LIGHTS_MAX = 12;

// clang-format on

extern int looBindingPortMax;

// Initialize libraries include:
// * glog
// * shaderMaterialBindingPort
LOO_EXPORT void initialize(const char* argv0);

}  // namespace loo

#endif /* LOO_LOO_LOO_HPP */
