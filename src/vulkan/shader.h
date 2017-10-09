#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class Shader
{
public:
    bool createFromFiles(VkDevice device, const std::string& vertexFilename, const std::string& fragmentFilename);
    void destory();

    std::vector<VkPipelineShaderStageCreateInfo> getShaderStages() const { return m_shaderStages; }

private:
    static VkShaderModule CreateShaderModule(VkDevice device, const std::string& filename);

    VkDevice m_device;
    std::vector<VkShaderModule> m_shaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
};