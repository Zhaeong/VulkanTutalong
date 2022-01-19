#pragma once

#include "vk_device.hpp"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

namespace ve {
class VkEngineSwapChain {
public:
  VkEngineDevice &engineDevice;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;

  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  // image views
  std::vector<VkImageView> swapChainImageViews;

  // renderpass
  VkRenderPass renderPass;
  VkCommandPool commandPool;

  std::vector<VkFramebuffer> swapChainFramebuffers;

  std::vector<VkCommandBuffer> commandBuffers;

  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;

  VkEngineSwapChain(VkEngineDevice &eDevice);
  ~VkEngineSwapChain();

  void createSwapChain();
  void createImageViews();

  void createRenderPass();
  void createFramebuffers();

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);

  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  void createSemaphores();
};
} // namespace ve