#include "framebuffer.h"
#include "vulkanhelper.h"


bool Framebuffer::init(VkDevice device, VkRenderPass renderPass, VkImageView attachment, VkExtent2D extent)
{
    m_device = device;

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &attachment;
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_framebuffer));

    return true;
}

void Framebuffer::destroy()
{ 
    vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
    m_framebuffer = VK_NULL_HANDLE;
}
