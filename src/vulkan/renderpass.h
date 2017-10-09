#pragma once

#include <vulkan/vulkan.h>

class RenderPass
{
public:
    bool init(VkDevice device, VkFormat colorAttachmentFormat);
    void destroy();

    VkRenderPass getVkRenderPass() const { return m_renderPass; }

private:
    VkDevice m_device;
    VkRenderPass m_renderPass;
};