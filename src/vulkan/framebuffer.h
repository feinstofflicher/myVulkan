#pragma once

#include <vulkan/vulkan.h>

class Framebuffer
{
public:
    bool init(VkDevice device, VkRenderPass renderPass, VkImageView attachment, VkExtent2D extent);
    void destroy();

    VkFramebuffer getVkFramebuffer() const { return m_framebuffer; }

private:
    VkDevice m_device;
    VkFramebuffer m_framebuffer;
};