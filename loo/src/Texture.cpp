#include "loo/Texture.hpp"

#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <glog/logging.h>
#include <stb_image.h>

#include <format>

#include "loo/glError.hpp"
namespace loo {
using namespace std;

static unsigned char* readImageFromFile(const std::string& filename, int* width,
                                        int* height, GLenum* imgfmt,
                                        GLenum* internalFmt) {
    int ncomp = 0;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(filename.c_str(), width, height, &ncomp, 0);
    if (!data) {
        LOG(ERROR) << format("Parse {} failed: {}", filename,
                             stbi_failure_reason());
        return nullptr;
    }
    switch (ncomp) {
        case 1:
            // grey
            *imgfmt = GL_RED;
            *internalFmt = GL_R8;
            break;
        case 2:
            // grey, alpha
            *imgfmt = GL_RG;
            *internalFmt = GL_RG8;
            break;
        case 3:
            // rgb
            *imgfmt = GL_RGB;
            *internalFmt = GL_RGB8;
            break;
        case 4:
            // rgba
            *imgfmt = GL_RGBA;
            *internalFmt = GL_RGBA8;
            break;
        default:
            LOG(ERROR) << format("{}: unsupported tex format {}", filename,
                                 ncomp);
            return nullptr;
    }
    return data;
}

std::shared_ptr<Texture2D> createTexture2DFromFile(
    std::unordered_map<std::string, std::shared_ptr<Texture2D>>& uniqueTexture,
    const std::string& filename, bool generateMipmap) {
    if (uniqueTexture.count(filename)) return uniqueTexture[filename];
    shared_ptr<Texture2D> tex = make_shared<Texture2D>();
    int width, height;
    GLenum imgfmt, internalFmt;

    auto data =
        readImageFromFile(filename, &width, &height, &imgfmt, &internalFmt);
    if (!data) {
        return nullptr;
    }
    tex->init();
    logPossibleGLError();
    // attention, mismatch between internalformat and format may casue
    // GL_INVALID_OPERATION
    tex->setup(data, width, height, internalFmt, imgfmt, GL_UNSIGNED_BYTE,
               generateMipmap ? -1 : 1);
    panicPossibleGLError();
    if (generateMipmap)
        tex->setSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    else
        tex->setSizeFilter(GL_LINEAR, GL_LINEAR);
    panicPossibleGLError();
    tex->setWrapFilter(GL_REPEAT);
    panicPossibleGLError();
    if (generateMipmap) tex->generateMipmap();

    stbi_image_free(data);
    uniqueTexture[filename] = tex;
    LOG(INFO) << "2D Texture " << filename << " loaded.";
    return tex;
}

void Texture2D::setup(unsigned char* data, GLsizei width, GLsizei height,
                      GLenum internalformat, GLenum format, GLenum type,
                      GLint maxLevel) {
    this->width = width;
    this->height = height;
#ifdef OGL_46
    // prepare storage
    glTextureStorage2D(m_id, maxLevel == -1 ? getMipmapLevels() : maxLevel,
                       internalformat, width, height);
    panicPossibleGLError();
    // store data
    glTextureSubImage2D(m_id, 0,              // level
                        0, 0, width, height,  // offset, size
                        format, type, data);
    panicPossibleGLError();
#else
    bind();
    glTexStorage2D(Target, level, internalformat, width, height);
    glTexSubImage2D(Target, level, 0, 0, width, height, format, type, data);
    // glTexImage2D(Target, level, internalformat, width, height,
    // 0,
    //              format, type, data);

    unbind();
#endif
}

Texture2D Texture2D::whiteTexture = Texture2D();
Texture2D Texture2D::blackTexture = Texture2D();

const Texture2D& Texture2D::getWhiteTexture() {
    if (whiteTexture.getId() == GL_INVALID_INDEX) {
        whiteTexture.init();
        unsigned char whiteData[] = {255, 255, 255};
        whiteTexture.setup(whiteData, 1, 1, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE,
                           1);
        whiteTexture.setSizeFilter(GL_LINEAR, GL_LINEAR);
        whiteTexture.setWrapFilter(GL_REPEAT);
    }
    return Texture2D::whiteTexture;
}
const Texture2D& Texture2D::getBlackTexture() {
    if (blackTexture.getId() == GL_INVALID_INDEX) {
        blackTexture.init();
        unsigned char blackData[] = {0, 0, 0};
        blackTexture.setup(blackData, 1, 1, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE,
                           1);
        blackTexture.setSizeFilter(GL_LINEAR, GL_LINEAR);
        blackTexture.setWrapFilter(GL_REPEAT);
    }
    return Texture2D::blackTexture;
}

vector<string> TextureCubeMap::TextureCubeMapBuilder::build() {
    vector<string> ret;
    for (int i = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
         i <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; i++) {
        std::string filename;
        if (auto fn = m_face2filename.find(i); fn != m_face2filename.end()) {
            filename = (m_prefix / fn->second).string() + m_ext;
        }
        // if no filename, leave blank
        ret.emplace_back(std::move(filename));
    }
    return ret;
}

void TextureCubeMap::setupStorage(GLsizei width, GLsizei height,
                                  GLenum internalformat, int maxLevel) {
    this->width = width;
    this->height = height;
    glTextureStorage2D(m_id, maxLevel == -1 ? getMipmapLevels() : maxLevel,
                       internalformat, width, height);
}
void TextureCubeMap::setupFace(GLenum face, unsigned char* data, GLenum format,
                               GLenum type) {
    CHECK_LT(face, 6);
    CHECK_GE(face, 0);
#ifdef OGL_46
    panicPossibleGLError();
    // store data

    // void glTextureSubImage3D(
    //    GLuint texture,
    //  	GLint level,
    //  	GLint xoffset,
    //  	GLint yoffset,
    //  	GLint zoffset,
    //  	GLsizei width,
    //  	GLsizei height,
    //  	GLsizei depth,
    //  	GLenum format,
    //  	GLenum type,
    //  	const void *pixels);
    glTextureSubImage3D(m_id, 0,           // level
                        0, 0, face,        // offset
                        width, height, 1,  // size
                        format, type, data);
    panicPossibleGLError();
#else
    bind();
    glTexStorage2D(Target, level, internalformat, width, height);
    glTexSubImage2D(Target, level, 0, 0, width, height, format, type, data);
    // glTexImage2D(Target, level, internalformat, width, height,
    // 0,
    //              format, type, data);

    unbind();
#endif
}

LOO_EXPORT std::shared_ptr<TextureCubeMap> createTextureCubeMapFromFiles(
    const std::vector<std::string>& filenames) {
    shared_ptr<TextureCubeMap> tex = make_shared<TextureCubeMap>();
    tex->init();
    for (int i = 0; i < 6; i++) {
        auto filename = filenames[i];
        int width, height;
        GLenum imgfmt, internalFmt;

        auto data =
            readImageFromFile(filename, &width, &height, &imgfmt, &internalFmt);
        if (!data) {
            return nullptr;
        }
        if (i == 0) {
            tex->setupStorage(width, height, internalFmt);
        }
        CHECK_EQ(width, tex->getWidth());
        CHECK_EQ(height, tex->getHeight());
        logPossibleGLError();
        // attention, mismatch between internalformat and format may casue
        // GL_INVALID_OPERATION
        tex->setupFace(i, data, imgfmt, GL_UNSIGNED_BYTE);
        panicPossibleGLError();
        tex->setSizeFilter(GL_LINEAR, GL_LINEAR);
        panicPossibleGLError();
        tex->setWrapFilter(GL_CLAMP_TO_EDGE);
        panicPossibleGLError();
        // TODO: cubemap mipmap
        // tex->generateMipmap();

        stbi_image_free(data);
        LOG(INFO) << "CubeMap Texture " << filename << " loaded.\n";
    }
    return tex;
}

}  // namespace loo