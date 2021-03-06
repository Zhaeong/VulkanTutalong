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

  int currentFrame = 0;

  VkWindow vkWindow{WIDTH, HEIGHT, "First Vulkan"};

  VkEngineDevice vkEngineDevice{vkWindow};

  VkModel vkModel{vkEngineDevice};

  VkEngineSwapChain vkEngineSwapChain{vkEngineDevice, vkModel};

  VkEnginePipeline vkEnginePipeline{
      vkEngineDevice,
      vkEngineSwapChain,
      VkEnginePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT),
      "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv",
      vkModel};

  bool frameBufferResized = false;

  FirstApp();
  ~FirstApp();

  // deleting copy constructors
  FirstApp(const FirstApp &) = delete;
  void operator=(const FirstApp &) = delete;

  void run();

  void drawFrame();

  void updateUniformBuffer(uint32_t currentImage);
  static void framebufferResizeCallback(GLFWwindow *window, int width,
                                        int height);
};
} // namespace ve