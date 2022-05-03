#pragma once

#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#include <cstring>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <vk_window.hpp>

#include <algorithm> // Necessary for std::clamp
#include <cstdint>   // Necessary for UINT32_MAX

namespace ve {

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class VkEngineDevice {
public:
  VkWindow &vkWindow;
  VkInstance instance;

  VkSurfaceKHR surface;

  static const int MAX_FRAMES_IN_FLIGHT = 3;
  // actual physical device
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  // logical device
  VkDevice logicalDevice;

  std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  VkDebugUtilsMessengerEXT debugMessenger;

  VkCommandPool commandPool;

  VkEngineDevice(VkWindow &window);
  ~VkEngineDevice();

  // deleting copy constructors
  VkEngineDevice(const VkEngineDevice &) = delete;
  void operator=(const VkEngineDevice &) = delete;

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const bool enableValidationLayers = true;

  void createInstance();
  void pickPhysicalDevice();
  void createLogicalDevice();

  bool checkValidationLayerSupport();
  std::vector<const char *> getRequiredVkExtensions();

  // Vulkan macros for different compilers in this form
  // VKAPI_ATTR <return_type> VKAPI_CALL <command_name>(<command_parameters>);
  // https://www.khronos.org/registry/vulkan/specs/1.0-extensions/html/vkspec.html#_platform_specific_calling_conventions
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData);

  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void setupDebugMessenger();

  VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator,
      VkDebugUtilsMessengerEXT *pDebugMessenger);

  static void
  DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                VkDebugUtilsMessengerEXT debugMessenger,
                                const VkAllocationCallbacks *pAllocator);

  bool isDeviceSuitable(VkPhysicalDevice device);

  bool checkDeviceExtensionSupport(VkPhysicalDevice device);

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

  void createSurface();

  void createCommandPool();
};
} // namespace ve