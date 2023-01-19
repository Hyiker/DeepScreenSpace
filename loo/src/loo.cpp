#include "loo/loo.hpp"

#include <glog/logging.h>

namespace loo {
int shaderMaterialBindingPort = -1;
int skyBoxBindingPort = -1;
int looBindingPortMax = -1;

void initialize(const char* argv0) {
    FLAGS_logtostderr = 1;
    google::InitGoogleLogging(argv0);
}
}  // namespace loo