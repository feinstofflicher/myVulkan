#pragma once

#include <vulkan/vulkan.h>
#include <string>

class Device;

class Texture
{
public:
    bool loadFromFile(Device* device, const std::string& filename);
    void destroy();

private:
    VkImage m_image;
    VkDeviceMemory m_imageMemory;

};
