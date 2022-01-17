#pragma once

#include "vk_device.hpp"
#include "vk_pipeline.hpp"
#include "vk_swap_chain.hpp"
#include "vk_window.hpp"
#include <iostream>

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
      "../shaders/simple_shader.vert.spv", "../shaders/simple_shader.frag.spv"};

  void run();
};
} // namespace ve