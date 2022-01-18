#include "vk_swap_chain.hpp"
namespace ve {

VkEngineSwapChain::VkEngineSwapChain(VkEngineDevice &eDevice)
    : engineDevice{eDevice} {

  createSwapChain();
  createImageViews();
  createRenderPass();
  createFramebuffers();
  createSemaphores();
}

VkEngineSwapChain::~VkEngineSwapChain() {

  std::cout << "Cleaning up VkEngineSwapChain Init\n";
  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(engineDevice.logicalDevice, imageView, nullptr);
  }
  swapChainImageViews.clear();
  vkDestroySwapchainKHR(engineDevice.logicalDevice, swapChain, nullptr);

  vkDestroyRenderPass(engineDevice.logicalDevice, renderPass, nullptr);

  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(engineDevice.logicalDevice, framebuffer, nullptr);
  }
  swapChainImageViews.clear();

  vkDestroySemaphore(engineDevice.logicalDevice, renderFinishedSemaphore,
                     nullptr);
  vkDestroySemaphore(engineDevice.logicalDevice, imageAvailableSemaphore,
                     nullptr);
}

VkSurfaceFormatKHR VkEngineSwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {

  for (int i = 0; i < availableFormats.size(); i++) {
    if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormats[i];
    }
    std::cout << "format: " << availableFormats[i].format
              << " clrspace: " << availableFormats[i].colorSpace << "\n";
  }
  return availableFormats[0];
}

VkPresentModeKHR VkEngineSwapChain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {

  for (int i = 0; i < availablePresentModes.size(); i++) {
    if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentModes[i];
    }
    std::cout << "avail present modes " << availablePresentModes[i] << "\n";
  }
  // guaranteed to be avail
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VkEngineSwapChain::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR &capabilities) {

  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(engineDevice.vkWindow.window, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
    // clamp used to bound the values of width and height between the allowed
    // minimum and maximum extents that are supported by the implementation
    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

void VkEngineSwapChain::createSwapChain() {
  std::cout << "Creating Swap Chain\n";
  SwapChainSupportDetails swapChainSupport =
      engineDevice.querySwapChainSupport(engineDevice.physicalDevice);

  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);

  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);

  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;

  // sometimes have to wait on the driver to complete internal operations before
  // we can acquire another image to render to. Therefore it is recommended to
  // request at least one more image than the minimum:
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

  // so we don't go over the maximum
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = engineDevice.surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;

  // can remove this line to trigger validation layer error
  createInfo.imageExtent = extent;

  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // specify how to handle swap chain images that will be used across multiple
  // queue families
  // if the graphics queue family is different from the presentation queue.
  // We'll be drawing on the images in the swap chain from the graphics queue
  // and then submitting them on the presentation queue.
  QueueFamilyIndices indices =
      engineDevice.findQueueFamilies(engineDevice.physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;     // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  // can specify that a certain transform should be applied to images in the
  // swap chain if it is supported do not want any transformation, simply
  // specify the current transformation.
  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

  // specifies if the alpha channel should be used for blending with other
  // windows in the window system. You'll almost always want to simply ignore
  // the alpha channel
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  // we don't care about the color of pixels that are obscured, for example
  // because another window is in front of them.
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  // it's possible that your swap chain becomes invalid or unoptimized while
  // your application is running, for example because the window was resized. In
  // that case the swap chain actually needs to be recreated from scratch and a
  // reference to the old one must be specified in this field.
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(engineDevice.logicalDevice, &createInfo, nullptr,
                           &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(engineDevice.logicalDevice, swapChain, &imageCount,
                          nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(engineDevice.logicalDevice, swapChain, &imageCount,
                          swapChainImages.data());
}

void VkEngineSwapChain::createImageViews() {
  swapChainImageViews.resize(swapChainImages.size());

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapChainImages[i];

    // The viewType and format fields specify how the image data should be
    // interpreted. The viewType parameter allows you to treat images as 1D
    // textures, 2D textures, 3D textures and cube maps.
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = swapChainImageFormat;

    // allows you to swizzle the color channels around. For example, you can map
    // all of the channels to the red channel for a monochrome texture. You can
    // also map constant values of 0 and 1 to a channel.
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // subresourceRange field describes what the image's purpose is and which
    // part of the image should be accessed. Our images will be used as color
    // targets without any mipmapping levels or multiple layers.
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(engineDevice.logicalDevice, &createInfo, nullptr,
                          &swapChainImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image views!");
    }
  }
}

void VkEngineSwapChain::createRenderPass() {
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  // Create subpass dependency
  // https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(engineDevice.logicalDevice, &renderPassInfo, nullptr,
                         &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}
void VkEngineSwapChain::createFramebuffers() {
  swapChainFramebuffers.resize(swapChainImageViews.size());

  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {swapChainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(engineDevice.logicalDevice, &framebufferInfo,
                            nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void VkEngineSwapChain::createSemaphores() {
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  if (vkCreateSemaphore(engineDevice.logicalDevice, &semaphoreInfo, nullptr,
                        &imageAvailableSemaphore) != VK_SUCCESS ||
      vkCreateSemaphore(engineDevice.logicalDevice, &semaphoreInfo, nullptr,
                        &renderFinishedSemaphore) != VK_SUCCESS) {

    throw std::runtime_error("failed to create semaphores!");
  }
}

} // namespace ve