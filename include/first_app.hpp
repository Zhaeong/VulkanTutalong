#pragma once

#include "vk_device.hpp"
#include "vk_pipeline.hpp"
#include "vk_swap_chain.hpp"
#include "vk_window.hpp"

#include <iostream>
#include <vulkan/vulkan.h>

namespace ve {
class FirstApp {
public:
  static const int WIDTH = 800;
  static const int HEIGHT = 600;

  VkWindow vkWindow{WIDTH, HEIGHT, "First Vulkan"};

  VkEngineDevice vkEngineDevice{vkWindow};

  VkEngineSwapChain vkEngineSwapChain{vkEngineDevice};

  VkEnginePipeline vkEnginePipeline{
      vkEngineDevice, vkEngineSwapChain,
      VkEnginePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT),
      "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv"};

  std::vector<VkCommandBuffer> commandBuffers;

  FirstApp();
  ~FirstApp();

  // deleting copy constructors
  FirstApp(const FirstApp &) = delete;
  void operator=(const FirstApp &) = delete;

  void run();

  void createCommandBuffers();

  void drawFrame();
};
} // namespace ve