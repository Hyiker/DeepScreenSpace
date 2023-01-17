#include "loo/Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <glog/logging.h>
#include <stb_image.h>

#include <format>
namespace loo {
using namespace std;
std::shared_ptr<Texture> createTextureFromFile(
    std::unordered_map<std::string, std::shared_ptr<Texture>>& uniqueTexture,
    const std::string& filename) {
    if (filename.length() == 0) return nullptr;
    if (uniqueTexture.count(filename)) return uniqueTexture[filename];
    shared_ptr<Texture> tex = make_shared<Texture>();
    LOG(INFO) << "Texture " << filename << " loaded.\n";
    stbi_set_flip_vertically_on_load(true);
    int ncomp;
    int width, height;
    unsigned char* data =
        stbi_load(filename.c_str(), &width, &height, &ncomp, 0);
    if (!data) {
        LOG(ERROR) << format("{}: bad filename", filename);
        return nullptr;
    }
    GLenum imgfmt;
    switch (ncomp) {
        case 1:
            imgfmt = GL_RED;
            break;
        case 3:
            imgfmt = GL_RGB;
            break;
        case 4:
            imgfmt = GL_RGBA;
            break;
        default:
            LOG(ERROR) << format("{}: unsupported tex format {}", filename,
                                 ncomp);
            return nullptr;
    }
    tex->init();
    tex->setup(data, width, height, GL_SRGB, imgfmt, GL_UNSIGNED_BYTE, 0);
    tex->setSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    tex->setWrapFilter(GL_REPEAT);
    tex->generateMipmap();

    stbi_image_free(data);
    uniqueTexture[filename] = tex;
    return tex;
}
}  // namespace loo