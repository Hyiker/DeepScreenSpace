
#include <glog/logging.h>

#include <filesystem>
#include <iostream>
#include <loo/loo.hpp>
#include <string>

#include "DSSApplication.hpp"
namespace fs = std::filesystem;

void loadScene(DSSApplication& app, const char* filename, float scaling) {
    using namespace std;
    fs::path p(filename);
    auto suffix = p.extension();
    if (suffix == ".obj") {
        LOG(INFO) << "Loading model from .obj file" << endl;
        app.loadModel(filename, scaling);
    } else if (suffix == ".gltf" || suffix == ".glb") {
        LOG(INFO) << "Loading scene from gltf file" << endl;
        app.loadGLTF(filename);
        exit(0);
    } else {
        LOG(FATAL) << "Unrecognizable file extension " << suffix << endl;
    }
}

int main(int argc, char* argv[]) {
    loo::LooBasicConfigs conf;
    loo::initialize(argv[0], &conf);
    DSSApplication app(1920, 1280);

    if (argc < 2) LOG(FATAL) << "Bad argument count\n";
    float scaling = 1.0;
    if (argc >= 3) {
        scaling = std::stof(argv[2]);
    }
    loadScene(app, argv[1], scaling);
    app.run();
}