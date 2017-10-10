#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Device;


class VertexBuffer
{
public:
    struct Description
    {
        Description(uint32_t _location, uint32_t _componentCount, uint32_t _vertexCount, const float* _vertexData)
            : location(_location)
            , componentCount(_componentCount)
            , vertexCount(_vertexCount)
            , vertexData(_vertexData)
        {}

        uint32_t location = 0;
        uint32_t componentCount = 0;
        uint32_t vertexCount = 0;
        const float* vertexData = nullptr;
    };

    void init(Device* device, const std::vector<Description>& descriptions);
    void destroy();

    void bind(VkCommandBuffer commandBuffer) const;

    uint32_t numVertices() const;
    const std::vector<VkVertexInputBindingDescription>& getBindingDescriptions() const;
    const std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions() const;

private:
    void createVertexBuffer(uint32_t size);
    static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    Device* m_device = nullptr;
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

    uint32_t m_numVertices = 0;
    std::vector<VkVertexInputAttributeDescription> m_attributesDescriptions;
    std::vector<VkVertexInputBindingDescription> m_bindingDescriptions;
};
