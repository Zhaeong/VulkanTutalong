#pragma once

#include "vk_device.hpp"
#include "vk_model.hpp"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

namespace ve {

class VkEngineSwapChain {
public:
  VkEngineDevice &engineDevice;
  VkModel &inputModel;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;

  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  // image views
  std::vector<VkImageView> swapChainImageViews;

  VkImageView textureImageView;

  // sampler
  VkSampler textureSampler;

  // renderpass
  VkRenderPass renderPass;

  std::vector<VkFramebuffer> swapChainFramebuffers;

  std::vector<VkSemaphore> imageAvailableSemaphore;
  std::vector<VkSemaphore> renderFinishedSemaphore;
  std::vector<VkFence> inFlightFences;

  // For each image in the swap chain
  std::vector<VkFence> imagesInFlight;

  VkEngineSwapChain(VkEngineDevice &eDevice, VkModel &model);
  ~VkEngineSwapChain();

  void createSwapChain();

  VkImageView createImageView(VkImage image, VkFormat format);
  void createImageViews();
  void createTextureImageView();

  void createTextureSampler();

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