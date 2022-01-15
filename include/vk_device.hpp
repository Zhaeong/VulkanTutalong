#pragma once

#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#include <cstring>
#include <string>
#include <vector>

namespace ve {
class VkDevice {
public:
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;

  VkDevice();
  ~VkDevice();

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const bool enableValidationLayers = true;

  void createInstance();
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
};
} // namespace ve