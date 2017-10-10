#include "vertexbuffer.h"
#include "vulkanhelper.h"
#include "device.h"

namespace
{
    VkFormat getAttributeFormat(uint32_t numVertices)
    {
        switch (numVertices)
        {
        case 1: return VK_FORMAT_R32_SFLOAT;
        case 2: return VK_FORMAT_R32G32_SFLOAT;
        case 3: return VK_FORMAT_R32G32B32_SFLOAT;
        default:
            assert(!"Unknown number of vertices");
            return VK_FORMAT_UNDEFINED;
        }
    }
}

void VertexBuffer::init(Device* device, const std::vector<Description>& descriptions)
{
    if (descriptions.size() == 0)
        return;

    m_device = device;
    m_numVertices = descriptions[0].vertexCount;

    auto totalSize = 0;
    m_attributesDescriptions.resize(descriptions.size());
    m_bindingDescriptions.resize(descriptions.size());
    for (auto i = 0; i < descriptions.size(); i++)
    {
        const auto& desc = descriptions[i];
        assert(m_numVertices == desc.vertexCount);

        VkVertexInputAttributeDescription& attribDesc = m_attributesDescriptions[i];
        attribDesc.binding = i;
        attribDesc.location = desc.location;
        attribDesc.format = getAttributeFormat(desc.componentCount);
        attribDesc.offset = totalSize;

        const auto attributeSize = desc.componentCount * 4;

        VkVertexInputBindingDescription& bindingDesc = m_bindingDescriptions[i];
        bindingDesc.binding = i;
        bindingDesc.stride = attributeSize;
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        totalSize += desc.vertexCount * attributeSize;
    }

    createVertexBuffer(totalSize);

    VK_CHECK_RESULT(vkBindBufferMemory(m_device->getVkDevice(), m_vertexBuffer, m_vertexBufferMemory, 0));

    void* mappedMemory;
    VK_CHECK_RESULT(vkMapMemory(m_device->getVkDevice(), m_vertexBufferMemory, 0, totalSize, 0, &mappedMemory));
    auto data = reinterpret_cast<float*>(mappedMemory);
    for (auto& desc : descriptions)
    {
        const auto attributeSize = desc.componentCount * desc.vertexCount;
        memcpy(data, desc.vertexData, attributeSize * 4);
        data += attributeSize;
    }
    vkUnmapMemory(m_device->getVkDevice(), m_vertexBufferMemory);
}

void VertexBuffer::destroy()
{
    vkDestroyBuffer(m_device->getVkDevice(), m_vertexBuffer, nullptr);
    vkFreeMemory(m_device->getVkDevice(), m_vertexBufferMemory, nullptr);
}

void VertexBuffer::createVertexBuffer(uint32_t size)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_RESULT(vkCreateBuffer(m_device->getVkDevice(), &bufferInfo, nullptr, &m_vertexBuffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device->getVkDevice(), m_vertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(m_device->getVkPysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(m_device->getVkDevice(), &allocInfo, nullptr, &m_vertexBufferMemory));
}

uint32_t VertexBuffer::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (auto i = 0u; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
        {
            return i;
        }
    }

    return ~0u;
}

void VertexBuffer::bind(VkCommandBuffer commandBuffer) const
{
    VkBuffer vertexBuffers[] = { m_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    for (auto i = 0; i < m_bindingDescriptions.size(); i++)
    {
        vkCmdBindVertexBuffers(commandBuffer, i, 1, vertexBuffers, offsets);
    }
}

uint32_t VertexBuffer::numVertices() const
{
    return m_numVertices;
}

const std::vector<VkVertexInputAttributeDescription>& VertexBuffer::getAttributeDescriptions() const
{
    return m_attributesDescriptions;
}

const std::vector<VkVertexInputBindingDescription>& VertexBuffer::getBindingDescriptions() const
{
    return m_bindingDescriptions;
}
