#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class PipelineLayout
{
public:
    void init(VkDevice device);
    void destroy();

    VkPipelineLayout getVkPipelineLayout() const { return m_pipelineLayout; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};

struct PipelineSettings
{
public:
    PipelineSettings();

    VkPipelineViewportStateCreateInfo viewportState = {};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    VkPipelineDynamicStateCreateInfo dynamicState = {};

    static VkDynamicState dynamicStates[2];
};

class VertexBuffer;

class Pipeline
{
public:
    bool init(VkDevice device,
        VkRenderPass renderPass,
        VkPipelineLayout layout,
        const PipelineSettings& settings,
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages,
        VertexBuffer* vertexbuffer = nullptr);

    void destroy();

    VkPipeline getVkPipeline() const { return m_pipeline; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
};