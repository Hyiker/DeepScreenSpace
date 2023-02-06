#ifndef DEEPSCREENSPACE_INCLUDE_DEEP_SCREEN_SPACE_HPP
#define DEEPSCREENSPACE_INCLUDE_DEEP_SCREEN_SPACE_HPP
#include <loo/AtomicCounter.hpp>
#include <loo/Framebuffer.hpp>
#include <loo/Quad.hpp>
#include <loo/Shader.hpp>
#include <loo/ShaderStorageBuffer.hpp>
#include <loo/Texture.hpp>
#include <memory>
struct SurfelData {
    // use vec4 to get it aligned
    glm::vec4 position;
    glm::vec3 normal;
    float radius;
};
constexpr int N_SURFELS_MAX = 2000000;
constexpr int N_PARTITION_LAYERS = 3;
class DeepScreenSpace {
    // mesh surfelize shader
    // only contains vertex & tessellation stages
    loo::ShaderProgram m_surfelizeshader;

    // TODO: add position normal shuffler

    // splatting related
    loo::Framebuffer m_splattingfb;
    // make use of surfelize result
    loo::ShaderProgram m_splattingshader;

    std::shared_ptr<loo::Texture2DArray> m_splattingresult;

    // surfel storage
    loo::ShaderStorageBuffer m_surfelssbo;
    loo::AtomicCounter m_surfelcounter;

    loo::Framebuffer m_surfelvisfb;
    std::shared_ptr<loo::Texture2D> m_surfelvistex;
    loo::ShaderProgram m_surfelvisshader;

    // only for cpu-end debug use
    std::unique_ptr<SurfelData[]> m_surfeldata;
    struct SurfelBuffer {
        GLuint vao;
        GLuint vbo;
    } m_surfelbuffer;
    // this texture describes how framebuffer is partitioned(shuffled)
    // on the different layers
    std::shared_ptr<loo::Texture2DArray> m_fbpartitiontex;
    std::shared_ptr<loo::Texture2DArray> m_partitionedposition,
        m_partitionednormal;
    // debug display
    std::shared_ptr<loo::Texture2D> m_partitionedpositiondebug,
        m_partitionednormaldebug;

    // framebuffer for
    loo::Framebuffer m_partitionfb;
    loo::ShaderProgram m_shuffleshader;
    int m_width, m_height;

   public:
    DeepScreenSpace(int width, int height);

    void initPartition();

    void shufflePartition(
        const loo::Quad& quad,
        std::shared_ptr<loo::Texture2D> originalNormalTexture,
        std::shared_ptr<loo::Texture2D> originalPositionTexture);

    int getSurfelCount() const;
    // prepare surfelization shader
    // no framebuffer is needed in this pass
    loo::ShaderProgram& prepareSurfelization();

    // rendering splatting result with surfelized scene
    void renderSplatting();

    void surfelVisualization();

    // copy surfeldata from ssbo to vbo
    void copySurfelData();

    auto getSplattingResult() const { return m_splattingresult; }
    auto getSurfelVisualizationResult() const { return m_surfelvistex; }
    auto getPartitionedPosition() const { return m_partitionedposition; }
    auto getPartitionedNormal() const { return m_partitionednormal; }
    std::shared_ptr<loo::Texture2D> getPartitionedPosition(int layer);
    std::shared_ptr<loo::Texture2D> getPartitionedNormal(int layer);
};

#endif /* DEEPSCREENSPACE_INCLUDE_DEEP_SCREEN_SPACE_HPP */