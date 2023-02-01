#include "DeepScreenSpace.hpp"

#include <algorithm>
#include <memory>

#include "shaders/base.vert.hpp"
#include "shaders/splatting.vert.hpp"
#include "shaders/surfelize.tesc.hpp"
#include "shaders/surfelize.tese.hpp"
#include "shaders/surfelize.vert.hpp"
#include "shaders/surfelizeVisualize.frag.hpp"
#include "shaders/surfelizeVisualize.vert.hpp"
using namespace loo;
using namespace std;
DeepScreenSpace::DeepScreenSpace(int width, int height)
    : m_surfelizeshader{Shader(SURFELIZE_VERT, ShaderType::Vertex),
                        // add tessellation here
                        Shader(SURFELIZE_TESC, ShaderType::TessellationControl),
                        Shader(SURFELIZE_TESE,
                               ShaderType::TessellationEvaluation)},
      m_splattingshader{Shader(SPLATTING_VERT, ShaderType::Vertex)},
      m_surfelssbo(loo::SHADER_BINDING_PORT_MAX + 1,
                   N_SURFELS_MAX * sizeof(SurfelData)),
      m_surfelcounter(0),
      m_surfelvisshader{Shader(SURFELIZEVISUALIZE_VERT, ShaderType::Vertex),
                        Shader(SURFELIZEVISUALIZE_FRAG, ShaderType::Fragment)},
      m_surfeldata(new SurfelData[N_SURFELS_MAX]) {
    {
        // splatting
        m_splattingfb.init();
        panicPossibleGLError();
        m_splattingresult = make_shared<Texture2DArray>();
        m_splattingresult->init();
        // TODO modify depth
        m_splattingresult->setupStorage(width, height, 4, GL_RGBA16F, 1);
        m_splattingresult->setSizeFilter(GL_LINEAR, GL_LINEAR);
        m_splattingfb.attachTexture(*m_splattingresult, GL_COLOR_ATTACHMENT0,
                                    0);
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
        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SurfelData),
                              (GLvoid*)offsetof(SurfelData, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SurfelData),
                              (GLvoid*)(offsetof(SurfelData, normal)));
        glEnableVertexAttribArray(1);
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
        m_surfelvistex->setup(width, height, GL_RGB32F, GL_RGB, GL_FLOAT, 1);
        m_surfelvistex->setSizeFilter(GL_LINEAR, GL_LINEAR);
        panicPossibleGLError();

        m_surfelvisfb.attachTexture(*m_surfelvistex, GL_COLOR_ATTACHMENT0, 0);
    }
}
void DeepScreenSpace::copySurfelData() {
    // TODO
    glCopyNamedBufferSubData(m_surfelssbo.getId(), m_surfelbuffer.vbo, 0, 0,
                             m_surfelssbo.getSize());
    panicPossibleGLError();
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
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_PROGRAM_POINT_SIZE);
    m_surfelvisshader.use();
    if (getSurfelCount()) {
        glBindVertexArray(m_surfelbuffer.vao);
        glDrawArrays(GL_POINTS, 0, getSurfelCount());
    }
    m_surfelvisfb.unbind();
}

void DeepScreenSpace::renderSplatting() {
    m_splattingfb.bind();
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_splattingshader.use();
    if (getSurfelCount()) {
        glBindVertexArray(m_surfelbuffer.vao);
        glDrawArrays(GL_POINTS, 0, getSurfelCount());
    }
    m_splattingfb.unbind();
}