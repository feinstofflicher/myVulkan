#pragma once

#include "vulkan/basicrenderer.h"
#include "vulkan/shader.h"
#include "vulkan/pipeline.h"
#include "vulkan/vertexbuffer.h"

class SimpleRenderer : public BasicRenderer
{
private:
    bool setup() override;
    void shutdown() override;
    void fillCommandBuffers() override;

    PipelineLayout m_pipelineLayout;
    Pipeline m_pipeline;
    VertexBuffer m_vertexBuffer;
    Shader m_shader;
};