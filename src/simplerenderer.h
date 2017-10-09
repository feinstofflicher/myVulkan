#pragma once

#include "vulkan/basicrenderer.h"
#include "vulkan/shader.h"
#include "vulkan/pipeline.h"

class SimpleRenderer : public BasicRenderer
{
private:
    bool setup() override;
    void shutdown() override;
    void fillCommandBuffers() override;

    PipelineLayout m_pipelineLayout;
    Pipeline m_pipeline;
    Shader m_shader;
};