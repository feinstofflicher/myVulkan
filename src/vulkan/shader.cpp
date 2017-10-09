#include "shader.h"
#include "vulkanhelper.h"

#include <fstream>

bool Shader::createFromFiles(VkDevice device, const std::string& vertexFilename, const std::string& fragmentFilename)
{
    m_device = device;

    m_shaderModules.resize(2);
    m_shaderModules[0] = CreateShaderModule(device, vertexFilename);
    if (m_shaderModules[0] == VK_NULL_HANDLE)
        return false;

    m_shaderModules[1] = CreateShaderModule(device, fragmentFilename);
    if (m_shaderModules[1] == VK_NULL_HANDLE)
        return false;

    m_shaderStages.resize(2);
    m_shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    m_shaderStages[0].module = m_shaderModules[0];
    m_shaderStages[0].pName = "main";

    m_shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    m_shaderStages[1].module = m_shaderModules[1];
    m_shaderStages[1].pName = "main";

    return true;
}

void Shader::destory()
{
    for (auto shaderModule : m_shaderModules)
    {
        vkDestroyShaderModule(m_device, shaderModule, nullptr);
    }
}

VkShaderModule Shader::CreateShaderModule(VkDevice device, const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        return false;

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

    return shaderModule;
}