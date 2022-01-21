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

  const int MAX_FRAMES_IN_FLIGHT = 2;
  std::vector<VkSemaphore> imageAvailableSemaphore;
  std::vector<VkSemaphore> renderFinishedSemaphore;
  std::vector<VkFence> inFlightFences;

  // For each image in the swap chain
  std::vector<VkFence> imagesInFlight;

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

  void createSyncObjects();
};
} // namespace ve