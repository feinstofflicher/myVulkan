#include "simplerenderer.h"
#include "vulkan/vulkanhelper.h"

bool SimpleRenderer::setup()
{
    m_pipelineLayout.init(m_device.getVkDevice());
    m_shader.createFromFiles(m_device.getVkDevice(), "shaders/simple.vert.spv", "shaders/simple.frag.spv");

    const float vertices[] = {
        0.0, -0.5,
        0.5,  0.5,
       -0.5,  0.5
    };

    const float colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    const std::vector<VertexBuffer::Description> descs =
    {
        { 0, 2, 3, vertices },
        { 1, 3, 3, colors }
    };

    m_vertexBuffer.init(&m_device, descs);


    PipelineSettings settings;

    m_pipeline.init(m_device.getVkDevice(),
        m_renderPass.getVkRenderPass(),
        m_pipelineLayout.getVkPipelineLayout(),
        settings,
        m_shader.getShaderStages(),
        &m_vertexBuffer);

    return true;
}

void SimpleRenderer::shutdown()
{
    m_shader.destory();
    m_vertexBuffer.destroy();
    m_pipeline.destroy();
    m_pipelineLayout.destroy();
}

void SimpleRenderer::fillCommandBuffers()
{
    for (size_t i = 0; i < m_commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        VK_CHECK_RESULT(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo));

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass.getVkRenderPass();
        renderPassInfo.framebuffer = m_framebuffers[i].getVkFramebuffer();
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChain.getImageExtent();
        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = { 0.0f, 0.0f, static_cast<float>(m_swapChain.getImageExtent().width), static_cast<float>(m_swapChain.getImageExtent().height), 0.0f, 1.0f };
        vkCmdSetViewport(m_commandBuffers[i], 0, 1, &viewport);

        VkRect2D scissor = { {0, 0}, m_swapChain.getImageExtent() };
        vkCmdSetScissor(m_commandBuffers[i], 0, 1, &scissor);

        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getVkPipeline());

        m_vertexBuffer.bind(m_commandBuffers[i]);

        vkCmdDraw(m_commandBuffers[i], m_vertexBuffer.numVertices(), 1, 0, 0);

        vkCmdEndRenderPass(m_commandBuffers[i]);

        VK_CHECK_RESULT(vkEndCommandBuffer(m_commandBuffers[i]));
    }
}