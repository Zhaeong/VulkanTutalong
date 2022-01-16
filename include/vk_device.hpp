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

namespace ve {

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
};

class VkEngineDevice {
public:
  VkWindow &vkWindow;
  VkInstance instance;

  VkSurfaceKHR surface;
  // actual physical device
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  // logical device
  VkDevice logicalDevice;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  VkDebugUtilsMessengerEXT debugMessenger;

  VkEngineDevice(VkWindow &window);
  ~VkEngineDevice();

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

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

  void createSurface();
};
} // namespace ve