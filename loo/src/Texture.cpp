#include "loo/Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <glog/logging.h>
#include <stb_image.h>

#include <format>

#include "loo/glError.hpp"
namespace loo {
using namespace std;
std::shared_ptr<Texture> createTextureFromFile(
    std::unordered_map<std::string, std::shared_ptr<Texture>>& uniqueTexture,
    const std::string& filename) {
    if (filename.length() == 0) return nullptr;
    if (uniqueTexture.count(filename)) return uniqueTexture[filename];
    shared_ptr<Texture> tex = make_shared<Texture>();
    LOG(INFO) << "Texture " << filename << " loaded.\n";
    int ncomp;
    int width, height;
    unsigned char* data =
        stbi_load(filename.c_str(), &width, &height, &ncomp, 0);
    if (!data) {
        LOG(ERROR) << format("{}: bad filename", filename);
        return nullptr;
    }
    GLenum imgfmt, internalFmt;
    switch (ncomp) {
        case 1:
            imgfmt = GL_RED;
            internalFmt = GL_R8;
            break;
        case 3:
            imgfmt = GL_RGB;
            internalFmt = GL_RGB8;
            break;
        case 4:
            imgfmt = GL_RGBA;
            internalFmt = GL_RGBA8;
            break;
        default:
            LOG(ERROR) << format("{}: unsupported tex format {}", filename,
                                 ncomp);
            return nullptr;
    }
    tex->init();
    logPossibleGLError();
    // attention, mismatch between internalformat and format may casue
    // GL_INVALID_OPERATION
    tex->setup(data, width, height, internalFmt, imgfmt, GL_UNSIGNED_BYTE, 0);
    panicPossibleGLError();
    tex->setSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    panicPossibleGLError();
    tex->setWrapFilter(GL_REPEAT);
    panicPossibleGLError();
    tex->generateMipmap();

    stbi_image_free(data);
    uniqueTexture[filename] = tex;
    return tex;
}
Texture Texture::blankTexture2D = Texture();

const Texture& Texture::getBlankTexture2D() {
    if (blankTexture2D.getId() == GL_INVALID_INDEX) {
        blankTexture2D.init();
        unsigned char whiteData[] = {255, 255, 255};
        blankTexture2D.setup(whiteData, 1, 1, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE,
                             0, 1);
        blankTexture2D.setSizeFilter(GL_LINEAR, GL_LINEAR);
        blankTexture2D.setWrapFilter(GL_REPEAT);
    }
    return blankTexture2D;
}
}  // namespace loo