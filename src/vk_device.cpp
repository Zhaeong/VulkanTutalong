#include "vk_device.hpp"

namespace ve {
VkDevice::VkDevice() {
  createInstance();
  setupDebugMessenger();
}
VkDevice::~VkDevice() {
  if (enableValidationLayers) {
    // Can remove this line to trigger validation layer error
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  }
  std::cout << "Cleaning up VkDevice Init\n";
  vkDestroyInstance(instance, nullptr);
}

void VkDevice::createInstance() {

  if (enableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("Validation Layer requested but not available\n");
  } else if (enableValidationLayers) {
    std::cout << "Enabling Validation Layers\n";
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  std::vector<const char *> requiredVkExtenstionsForGLFW =
      getRequiredVkExtensions();

  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(requiredVkExtenstionsForGLFW.size());
  createInfo.ppEnabledExtensionNames = requiredVkExtenstionsForGLFW.data();

  // The debugCreateInfo variable is placed outside the if statement
  // to ensure that it is not destroyed before the vkCreateInstance call
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         extensions.data());

  std::cout << "num extensions: " << extensionCount << "\n";
  std::cout << "available extensions:\n";
}

bool VkDevice::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (int i = 0; i < validationLayers.size(); i++) {

    bool layerFound = false;

    for (int j = 0; j < availableLayers.size(); j++) {

      //== compairs pointers
      // strcmp compairs actual string content
      if (strcmp(validationLayers[i], availableLayers[j].layerName)) {
        layerFound = true;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

std::vector<const char *> VkDevice::getRequiredVkExtensions() {

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;

  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  // vector(pointer_to_first_element, pointer_after_final_element)
  // glfwExtensions is pointer to first element
  // glfwExtensions + 1 is pointer to second element
  // std::cout << "tria:" << *(glfwExtensions + 1) << "\n";
  // glfwExtensions + glfwExtensionCount is pointer to last element
  std::vector<const char *> requiredVkExtensionsForGLFW(
      glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    requiredVkExtensionsForGLFW.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  // for (int i = 0; i < requiredVkExtensionsForGLFW.size(); i++) {
  //   std::cout << "ext: " << requiredVkExtensionsForGLFW[i] << "\n";
  // }

  return requiredVkExtensionsForGLFW;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkDevice::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {

  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  std::string *userData = static_cast<std::string *>(pUserData);
  std::cerr << "UserData: " << *userData << "\n";
  // std::cerr << "UserData: " << *userData << "\n";
  return VK_FALSE;
}

void VkDevice::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  // char *UserData = "Heyya\n";
  std::string UserData = "Heyya\n";
  void *userDataVoidPtr = static_cast<void *>(new std::string("ohh man\n"));
  // createInfo.pUserData = nullptr; // Optional
  createInfo.pUserData = userDataVoidPtr;
}
void VkDevice::setupDebugMessenger() {
  if (!enableValidationLayers) {
    return;
  }
  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  populateDebugMessengerCreateInfo(createInfo);

  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                   &debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

VkResult VkDevice::CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void VkDevice::DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}
} // namespace ve