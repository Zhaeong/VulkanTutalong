#include "vk_pipeline.hpp"

namespace ve {

VkEnginePipeline::VkEnginePipeline(VkEngineDevice &eDevice,
                                   const PipelineConfigInfo &pipelineConfig,
                                   const std::string &vertFilepath,
                                   const std::string &fragFilepath)
    : engineDevice{eDevice} {
  createGraphicsPipeline(vertFilepath, fragFilepath, pipelineConfig);
}

VkEnginePipeline::~VkEnginePipeline() {}

std::vector<char> VkEnginePipeline::readFile(const std::string &filePath) {

  // std::ios::ate means seek the end immediatly
  // std::ios::binary read it in as a binary
  std::ifstream file{filePath, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filePath);
  }

  // tellg gets last position which is the filesize
  size_t fileSize = static_cast<size_t>(file.tellg());

  std::vector<char> buffer(fileSize);

  // Go to beginning
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

void VkEnginePipeline::createGraphicsPipeline(
    const std::string &vertFilepath, const std::string &fragFilepath,
    const PipelineConfigInfo &pipelineConfig) {

  auto vertCode = readFile(vertFilepath);
  auto fragCode = readFile(fragFilepath);

  std::cout << "Vertext shader Code Size:" << vertCode.size() << "\n";
  std::cout << "Fragment shader Code Size:" << fragCode.size() << "\n";
}

VkShaderModule
VkEnginePipeline::createShaderModule(const std::vector<char> &shaderCode) {

  VkShaderModule shaderModule;
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = shaderCode.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(shaderCode.data());

  if (vkCreateShaderModule(engineDevice.logicalDevice, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

PipelineConfigInfo
VkEnginePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {

  PipelineConfigInfo configInfo{};
  return configInfo;
}

} // namespace ve