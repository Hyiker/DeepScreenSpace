#include "loo/loo.hpp"

#include <glog/logging.h>
namespace loo {
int shaderMaterialBindingPort = -1;

void initialize(const char* argv0, LooBasicConfigs* conf) {
    FLAGS_logtostderr = 1;
    google::InitGoogleLogging(argv0);
    shaderMaterialBindingPort = conf->shaderMaterialBindingPort;
}
}  // namespace loo