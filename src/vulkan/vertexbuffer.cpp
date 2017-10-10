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

    const bool useStaging = true;
    if (useStaging)
    {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        m_device->createBuffer(totalSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        mapMemory(descriptions, stagingBufferMemory);

        m_device->createBuffer(totalSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_vertexBuffer, m_vertexBufferMemory);

        m_device->copyBuffer(stagingBuffer, m_vertexBuffer, totalSize);

        vkDestroyBuffer(m_device->getVkDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_device->getVkDevice(), stagingBufferMemory, nullptr);
    }
    else
    {
        m_device->createBuffer(totalSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_vertexBuffer, m_vertexBufferMemory);

        mapMemory(descriptions, m_vertexBufferMemory);
    }
}

void VertexBuffer::destroy()
{
    vkDestroyBuffer(m_device->getVkDevice(), m_vertexBuffer, nullptr);
    m_vertexBuffer = VK_NULL_HANDLE;

    vkFreeMemory(m_device->getVkDevice(), m_vertexBufferMemory, nullptr);
    m_vertexBufferMemory = VK_NULL_HANDLE;
}

void VertexBuffer::mapMemory(const std::vector<Description>& descriptions, VkDeviceMemory bufferMemory)
{
    void* mappedMemory;
    VK_CHECK_RESULT(vkMapMemory(m_device->getVkDevice(), bufferMemory, 0, VK_WHOLE_SIZE, 0, &mappedMemory));
    auto data = reinterpret_cast<float*>(mappedMemory);
    for (auto& desc : descriptions)
    {
        const auto attributeSize = desc.componentCount * desc.vertexCount;
        memcpy(data, desc.vertexData, attributeSize * 4);
        data += attributeSize;
    }
    vkUnmapMemory(m_device->getVkDevice(), bufferMemory);
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
