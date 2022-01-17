#pragma once

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include <vk_device.hpp>
namespace ve {
struct PipelineConfigInfo {};
class VkEnginePipeline {
public:
  VkEngineDevice &engineDevice;
  VkPipeline graphicsPipeline;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;

  // deleting copy constructors
  VkEnginePipeline(const VkEnginePipeline &) = delete;
  void operator=(const VkEnginePipeline &) = delete;

  VkEnginePipeline(VkEngineDevice &eDevice,
                   const PipelineConfigInfo &pipelineConfig,
                   const std::string &vertFilepath,
                   const std::string &fragFilepath);
  ~VkEnginePipeline();

  static std::vector<char> readFile(const std::string &filePath);

  void createGraphicsPipeline(const std::string &vertFilepath,
                              const std::string &fragFilepath,
                              const PipelineConfigInfo &pipelineConfig);

  VkShaderModule createShaderModule(const std::vector<char> &shaderCode);

  static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width,
                                                      uint32_t height);
};

} // namespace ve