#include "device.h"
#include "vulkanhelper.h"
#include "debug.h"

#include <vector>

bool Device::init(VkInstance instance, VkSurfaceKHR surface, bool enableValidationLayers)
{
    uint32_t numDevices = 0;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &numDevices, nullptr));
    if (numDevices == 0)
    {
        std::cout << "Error occurred during physical devices enumeration!" << std::endl;
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices(numDevices);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &numDevices, &physicalDevices[0]));

    for (uint32_t i = 0; i < numDevices; ++i)
    {
        if (checkPhysicalDeviceProperties(physicalDevices[i], surface))
        {
            m_physicalDevice = physicalDevices[i];
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE)
    {
        std::cout << "Could not select physical device based on the chosen properties!" << std::endl;
        return false;
    }

    auto queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
        nullptr,                                        // const void                  *pNext
        0,                                              // VkDeviceQueueCreateFlags     flags
        m_graphicsQueueFamilyIndex,                     // uint32_t                     queueFamilyIndex
        1,                                              // uint32_t                     queueCount
        &queuePriority                                  // const float                 *pQueuePriorities
    };

    std::vector<const char*> extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,           // VkStructureType                    sType
        nullptr,                                        // const void                        *pNext
        0,                                              // VkDeviceCreateFlags                flags
        1,                                              // uint32_t                           queueCreateInfoCount
        &queueCreateInfo,                               // const VkDeviceQueueCreateInfo     *pQueueCreateInfos
        0,                                              // uint32_t                           enabledLayerCount
        nullptr,                                        // const char * const                *ppEnabledLayerNames
        static_cast<uint32_t>(extensions.size()),       // uint32_t                           enabledExtensionCount
        &extensions[0],                                 // const char * const                *ppEnabledExtensionNames
        nullptr                                         // const VkPhysicalDeviceFeatures    *pEnabledFeatures
    };

    if (enableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = debug::validationLayerCount;
        deviceCreateInfo.ppEnabledLayerNames = debug::validationLayerNames;
    }

    VK_CHECK_RESULT(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device));

    vkGetDeviceQueue(m_device, m_presentQueueFamilyIndex, 0, &m_presentQueue);
    vkGetDeviceQueue(m_device, m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);

    createCommandPool();

    return true;
}

bool Device::checkPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures   device_features;

    vkGetPhysicalDeviceProperties(physicalDevice, &device_properties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &device_features);

    uint32_t major_version = VK_VERSION_MAJOR(device_properties.apiVersion);
    uint32_t minor_version = VK_VERSION_MINOR(device_properties.apiVersion);
    uint32_t patch_version = VK_VERSION_PATCH(device_properties.apiVersion);

    if ((major_version < 1) || (device_properties.limits.maxImageDimension2D < 4096))
    {
        std::cout << "Physical device " << physicalDevice << " doesn't support required parameters!" << std::endl;
        return false;
    }

    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        std::cout << "Physical device " << physicalDevice << " doesn't have any queue families!" << std::endl;
        return false;
    }

    std::vector<VkQueueFamilyProperties>  queueProps(queueCount);
    std::vector<VkBool32>                 supportsPresent(queueCount);

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, &queueProps[0]);

    m_graphicsQueueFamilyIndex = UINT32_MAX;
    m_presentQueueFamilyIndex = UINT32_MAX;

    for (uint32_t i = 0; i < queueCount; ++i)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent[i]);

        if ((queueProps[i].queueCount > 0) &&
            (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            // Select first queue that supports graphics
            if (m_graphicsQueueFamilyIndex == UINT32_MAX)
            {
                m_graphicsQueueFamilyIndex = i;
            }

            // If there is queue that supports both graphics and present - prefer it
            if (supportsPresent[i])
            {
                m_graphicsQueueFamilyIndex = i;
                m_presentQueueFamilyIndex = i;
                return true;
            }
        }
    }

    // We don't have queue that supports both graphics and present so we have to use separate queues
    for (uint32_t i = 0; i < queueCount; ++i)
    {
        if (supportsPresent[i])
        {
            m_presentQueueFamilyIndex = i;
            break;
        }
    }

    // If this device doesn't support queues with graphics and present capabilities don't use it
    if ((m_graphicsQueueFamilyIndex == UINT32_MAX) ||
        (m_graphicsQueueFamilyIndex == UINT32_MAX))
    {
        std::cout << "Could not find queue family with required properties on physical device " << physicalDevice << "!" << std::endl;
        return false;
    }

    return true;
}

void Device::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;

    VK_CHECK_RESULT(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
}

void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void Device::createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, properties);

    VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory));
    VK_CHECK_RESULT(vkBindBufferMemory(m_device, buffer, bufferMemory, 0));
}

uint32_t Device::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

void Device::destroy()
{
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    m_commandPool = VK_NULL_HANDLE;

    vkDestroyDevice(m_device, nullptr);
    m_device = VK_NULL_HANDLE;
}
