#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include <vk_device.hpp>
#include <vk_model.hpp>
#include <vk_swap_chain.hpp>
namespace ve {
struct PipelineConfigInfo {
  VkViewport viewport;
  VkRect2D scissor;
  // VkPipelineViewportStateCreateInfo viewportStateInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  // VkPipelineLayout pipelineLayout = nullptr;
  // VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};
class VkEnginePipeline {
public:
  VkEngineDevice &engineDevice;
  VkEngineSwapChain &engineSwapChain;
  VkModel &engineInputModel;

  VkPipeline graphicsPipeline;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;

  VkDescriptorSetLayout descriptorSetLayout = nullptr;
  std::vector<VkDescriptorSet> descriptorSets;

  VkPipelineLayout pipelineLayout = nullptr;

  std::string vertexCodeFilePath;
  std::string fragmentCodeFilePath;

  std::vector<VkCommandBuffer> commandBuffers;

  // deleting copy constructors
  VkEnginePipeline(const VkEnginePipeline &) = delete;
  void operator=(const VkEnginePipeline &) = delete;

  VkEnginePipeline(VkEngineDevice &eDevice, VkEngineSwapChain &eSwapChain,
                   const PipelineConfigInfo &pipelineConfig,
                   std::string vertFilepath, std::string fragFilepath,
                   VkModel &inputModel);
  ~VkEnginePipeline();

  static std::vector<char> readFile(std::string filePath);

  void createCommandBuffers();
  // for window resizes
  void recreateSwapChain();
  void cleanupSwapChain();

  void createGraphicsPipeline(const PipelineConfigInfo &pipelineConfig);

  VkShaderModule createShaderModule(const std::vector<char> &shaderCode);

  static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width,
                                                      uint32_t height);

  void bindCommandBufferToGraphicsPipelilne(VkCommandBuffer commandBuffer);

  void createDescriptorSetLayout();
  void createDescriptorSets();
};

} // namespace ve