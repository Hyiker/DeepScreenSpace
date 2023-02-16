#include "DeepScreenSpace.hpp"

#include <algorithm>
#include <loo/Camera.hpp>
#include <memory>
#include <numeric>
#include <random>
#include <vector>

#include "glog/logging.h"
#include "shaders/positionNormalShuffler.frag.hpp"
#include "shaders/positionNormalShuffler.vert.hpp"
#include "shaders/splatting.frag.hpp"
#include "shaders/splatting.geom.hpp"
#include "shaders/splatting.vert.hpp"
#include "shaders/splattingUnshuffle.frag.hpp"
#include "shaders/splattingUnshuffle.vert.hpp"
#include "shaders/surfelize.tesc.hpp"
#include "shaders/surfelize.tese.hpp"
#include "shaders/surfelize.vert.hpp"
#include "shaders/surfelizeVisualize.frag.hpp"
#include "shaders/surfelizeVisualize.vert.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"
using namespace loo;
using namespace std;
using namespace glm;
DeepScreenSpace::DeepScreenSpace(int width, int height)
    : m_surfelizeshader{Shader(SURFELIZE_VERT, ShaderType::Vertex),
                        // add tessellation here
                        Shader(SURFELIZE_TESC, ShaderType::TessellationControl),
                        Shader(SURFELIZE_TESE,
                               ShaderType::TessellationEvaluation)},
      m_splattingshader{Shader(SPLATTING_VERT, ShaderType::Vertex),
                        Shader(SPLATTING_GEOM, ShaderType::Geometry),
                        Shader(SPLATTING_FRAG, ShaderType::Fragment)},
      m_surfelssbo(loo::SHADER_BINDING_PORT_MAX + 1,
                   N_SURFELS_MAX * sizeof(SurfelData)),
      m_surfelcounter(0),
      m_surfelvisshader{Shader(SURFELIZEVISUALIZE_VERT, ShaderType::Vertex),
                        Shader(SURFELIZEVISUALIZE_FRAG, ShaderType::Fragment)},
      m_surfeldata(new SurfelData[N_SURFELS_MAX]),
      m_shuffleshader(
          {Shader(POSITIONNORMALSHUFFLER_VERT, ShaderType::Vertex),
           Shader(POSITIONNORMALSHUFFLER_FRAG, ShaderType::Fragment)}),
      m_unshuffleshader(
          {Shader(SPLATTINGUNSHUFFLE_VERT, ShaderType::Vertex),
           Shader(SPLATTINGUNSHUFFLE_FRAG, ShaderType::Fragment)}),
      m_width(width),
      m_height(height) {
    {
        // splatting
        m_splattingfb.init();
        panicPossibleGLError();
        m_splattingresult = make_shared<Texture2DArray>();
        m_splattingresult->init();
        m_splattingresult->setupStorage(width, height, N_PARTITION_LAYERS,
                                        GL_RGB16F, 1);
        m_splattingresult->setSizeFilter(GL_LINEAR, GL_LINEAR);
        m_splattingfb.attachTexture(*m_splattingresult, GL_COLOR_ATTACHMENT0,
                                    0);

        m_splattingresultdebug = make_shared<Texture2D>();
        m_splattingresultdebug->init();
        m_splattingresultdebug->setupStorage(width, height, GL_RGB16F, 1);
        m_splattingresultdebug->setSizeFilter(GL_LINEAR, GL_LINEAR);

        panicPossibleGLError();
    }
    {
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferStorage(GL_ARRAY_BUFFER, sizeof(SurfelData) * N_SURFELS_MAX,
                        nullptr, GL_DYNAMIC_STORAGE_BIT);
        SurfelData sd{};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(SurfelData), &sd);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SurfelData),
                              (GLvoid*)offsetof(SurfelData, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SurfelData),
                              (GLvoid*)(offsetof(SurfelData, normal)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(SurfelData),
                              (GLvoid*)(offsetof(SurfelData, radius)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        m_surfelbuffer.vao = vao;
        m_surfelbuffer.vbo = vbo;
    }
    {
        // surfel visualize
        m_surfelvisfb.init();
        m_surfelvistex = make_shared<Texture2D>();
        m_surfelvistex->init();
        m_surfelvistex->setupStorage(width, height, GL_RGB32F, 1);
        m_surfelvistex->setSizeFilter(GL_LINEAR, GL_LINEAR);
        panicPossibleGLError();
        Renderbuffer surfelizerb;
        surfelizerb.init(GL_DEPTH_COMPONENT32, width, height);

        m_surfelvisfb.attachTexture(*m_surfelvistex, GL_COLOR_ATTACHMENT0, 0);
        m_surfelvisfb.attachRenderbuffer(surfelizerb, GL_DEPTH_ATTACHMENT);
    }
    { initPartition(); }
    {
        m_unshufflefb.init();
        m_unshuffleresult = make_shared<Texture2DArray>();
        m_unshuffleresult->init();
        m_unshuffleresult->setupStorage(width, height, N_PARTITION_LAYERS,
                                        GL_RGB16F, 1);
        m_unshuffleresult->setSizeFilter(GL_LINEAR, GL_LINEAR);
        m_unshufflefb.attachTexture(*m_unshuffleresult, GL_COLOR_ATTACHMENT0,
                                    0);
        m_unshufflefb.enableAttachments({GL_COLOR_ATTACHMENT0});
    }
}
void DeepScreenSpace::initPartition() {
    m_fbpartitiontex = make_shared<Texture2DArray>();
    m_fbpartitiontex->init();
    m_fbpartitiontex->setupStorage(
        m_width, m_height, N_PARTITION_LAYERS, GL_RGBA32I, 1  // no mipmap, plz
    );
    m_fbpartitiontex->setSizeFilter(GL_NEAREST, GL_NEAREST);

    panicPossibleGLError();
    // the partition indexing only need do once on the cpu-end
    vector<ivec4> basebuffer(m_width * m_height, ivec4(0)),
        buffer(m_width * m_height, ivec4(-1));
    for (int i = 0; i < basebuffer.size(); i++) {
        basebuffer[i] =
            ivec4(i % m_width, i / m_width, i % m_width, i / m_width);
    }
    m_fbpartitiontex->setupLayer(0, basebuffer.data(), GL_RGBA_INTEGER, GL_INT);
    std::random_device rd;
    std::mt19937 g(rd());
    for (int j = 1, s = 2; j < N_PARTITION_LAYERS; j++, s *= 2) {
        int width_s = m_width / s, height_s = m_height / s;
        vector<int> shuffleIndex(s * s);
        std::iota(shuffleIndex.begin(), shuffleIndex.end(), 0);
        for (int i = 0; i < basebuffer.size() / (s * s); i++) {
            ivec2 in_offset(i % width_s, i / width_s);
            ivec2 buffer_base(in_offset.x, in_offset.y);
            buffer_base *= s;
            // shuffle for each subblock
            std::shuffle(shuffleIndex.begin(), shuffleIndex.end(), g);
            for (int k = 0; k < s * s; k++) {
                ivec2 out_offset((k % s) * width_s, (k / s) * height_s);
                ivec2 sub_pos(out_offset + in_offset);
                int m = shuffleIndex[k];
                ivec2 buffer_offset(m % s, m / s);
                ivec2 buffer_pos = buffer_base + buffer_offset;
                auto base_pos =
                    basebuffer[buffer_pos.x + buffer_pos.y * m_width];
                buffer[sub_pos.x + sub_pos.y * m_width].x = base_pos.x;
                buffer[sub_pos.x + sub_pos.y * m_width].y = base_pos.y;
                buffer[base_pos.x + base_pos.y * m_width].z = sub_pos.x;
                buffer[base_pos.x + base_pos.y * m_width].w = sub_pos.y;
            }
        }
        m_fbpartitiontex->setupLayer(j, buffer.data(), GL_RGBA_INTEGER, GL_INT);
    }
    m_partitionfb.init();
    m_partitionednormal = make_shared<Texture2DArray>();
    m_partitionednormal->init();
    m_partitionednormal->setupStorage(m_width, m_height, N_PARTITION_LAYERS,
                                      GL_RGB8, 1);

    m_partitionedposition = make_shared<Texture2DArray>();
    m_partitionedposition->init();
    m_partitionedposition->setupStorage(m_width, m_height, N_PARTITION_LAYERS,
                                        GL_RGBA32F, 1);

    m_partitionednormaldebug = make_shared<Texture2D>();
    m_partitionednormaldebug->init();
    m_partitionednormaldebug->setupStorage(m_width, m_height, GL_RGB8, 1);

    m_partitionedpositiondebug = make_shared<Texture2D>();
    m_partitionedpositiondebug->init();
    m_partitionedpositiondebug->setupStorage(m_width, m_height, GL_RGB32F, 1);
}
void DeepScreenSpace::shufflePartition(
    const loo::Quad& quad,
    std::shared_ptr<loo::Texture2D> originalNormalTexture,
    std::shared_ptr<loo::Texture2D> originalPositionTexture) {
    m_partitionfb.bind();
    m_shuffleshader.use();
    for (int i = 0; i < N_PARTITION_LAYERS; i++) {
        m_partitionfb.attachTextureLayer(*m_partitionednormal,
                                         GL_COLOR_ATTACHMENT0, 0, i);
        m_partitionfb.attachTextureLayer(*m_partitionedposition,
                                         GL_COLOR_ATTACHMENT1, 0, i);
        m_partitionfb.enableAttachments(
            {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        m_shuffleshader.setTexture(0, *originalNormalTexture);
        m_shuffleshader.setTexture(1, *originalPositionTexture);
        m_shuffleshader.setTexture(2, *m_fbpartitiontex);
        m_shuffleshader.setUniform("currentLayer", i);
        quad.draw();
    }
}

void DeepScreenSpace::unshuffleSplattingResult(const loo::Quad& quad) {
    m_unshufflefb.bind();
    m_unshuffleshader.use();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    // unshuffle according to the original shuffle ordering
    m_unshuffleshader.setTexture(0, *m_fbpartitiontex);
    m_unshuffleshader.setTexture(1, *m_splattingresult);
    quad.drawInstances(N_PARTITION_LAYERS);
    m_unshufflefb.unbind();
}

void DeepScreenSpace::copySurfelData() {
    glCopyNamedBufferSubData(m_surfelssbo.getId(), m_surfelbuffer.vbo, 0, 0,
                             m_surfelssbo.getSize());
    panicPossibleGLError();
    // m_surfelssbo.getData(m_surfeldata.get(), 0, sizeof(SurfelData));
    // LOG(INFO) << m_surfeldata[0].radius;
}
int DeepScreenSpace::getSurfelCount() const {
    return (std::min)(m_surfelcounter.getCounter(), GLuint(N_SURFELS_MAX));
}

ShaderProgram& DeepScreenSpace::prepareSurfelization() {
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    m_surfelcounter.resetCounter();
    return m_surfelizeshader;
}
void DeepScreenSpace::surfelVisualization() {
    m_surfelvisfb.bind();
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    m_surfelvisshader.use();
    if (getSurfelCount()) {
        glBindVertexArray(m_surfelbuffer.vao);
        glDrawArrays(GL_POINTS, 0, getSurfelCount());
    }
    m_surfelvisfb.unbind();
}

void DeepScreenSpace::renderSplatting(const loo::Camera& camera,
                                      float splattingStrength) {
    m_splattingfb.bind();
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    m_splattingshader.use();
    if (getSurfelCount()) {
        m_splattingshader.setUniform("cameraPos", camera.getPosition());
        m_splattingshader.setUniform("viewMatrix", camera.getViewMatrix());
        m_splattingshader.setUniform("projectionMatrix",
                                     camera.getProjectionMatrix());
        m_splattingshader.setUniform("framebufferDeviceStep.resolution",
                                     ivec2(m_width, m_height));
        m_splattingshader.setUniform("strength", splattingStrength);
        m_splattingshader.setUniform("fov", camera.m_fov);
        m_splattingshader.setTexture(12, *m_partitionedposition);
        m_splattingshader.setTexture(13, *m_partitionednormal);
        glBindVertexArray(m_surfelbuffer.vao);
        glDrawArrays(GL_POINTS, 0, getSurfelCount());
        logPossibleGLError();
    }
    glDisable(GL_BLEND);
    m_splattingfb.unbind();
}

std::shared_ptr<loo::Texture2D> DeepScreenSpace::getPartitionedPosition(
    int layer) {
    glCopyImageSubData(m_partitionedposition->getId(), GL_TEXTURE_2D_ARRAY, 0,
                       0, 0, layer, m_partitionedpositiondebug->getId(),
                       GL_TEXTURE_2D, 0, 0, 0, 0, m_width, m_height, 1);
    logPossibleGLError();
    return m_partitionedpositiondebug;
}
std::shared_ptr<loo::Texture2D> DeepScreenSpace::getPartitionedNormal(
    int layer) {
    glCopyImageSubData(m_partitionednormal->getId(), GL_TEXTURE_2D_ARRAY, 0, 0,
                       0, layer, m_partitionednormaldebug->getId(),
                       GL_TEXTURE_2D, 0, 0, 0, 0, m_width, m_height, 1);
    logPossibleGLError();
    return m_partitionednormaldebug;
}
std::shared_ptr<loo::Texture2D> DeepScreenSpace::getSplattingResult(
    int layer, bool unshuffle) {
    glCopyImageSubData(
        unshuffle ? m_unshuffleresult->getId() : m_splattingresult->getId(),
        GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, m_splattingresultdebug->getId(),
        GL_TEXTURE_2D, 0, 0, 0, 0, m_width, m_height, 1);
    logPossibleGLError();
    return m_splattingresultdebug;
}
