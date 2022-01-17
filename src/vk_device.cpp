#include "vk_device.hpp"

namespace ve {
VkEngineDevice::VkEngineDevice(VkWindow &window) : vkWindow{window} {
  // vkWindow = window;
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
}
VkEngineDevice::~VkEngineDevice() {
  if (enableValidationLayers) {
    // Can remove this line to trigger validation layer error
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  }
  std::cout << "Cleaning up VkEngineDevice Init\n";

  vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
  // have to destroy logical device first it seems
  vkDestroyDevice(logicalDevice, nullptr);
  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyInstance(instance, nullptr);
}

void VkEngineDevice::createInstance() {

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

bool VkEngineDevice::checkValidationLayerSupport() {
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

std::vector<const char *> VkEngineDevice::getRequiredVkExtensions() {

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

VKAPI_ATTR VkBool32 VKAPI_CALL VkEngineDevice::debugCallback(
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

void VkEngineDevice::populateDebugMessengerCreateInfo(
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
void VkEngineDevice::setupDebugMessenger() {
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

VkResult VkEngineDevice::CreateDebugUtilsMessengerEXT(
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

void VkEngineDevice::DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

void VkEngineDevice::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  } else {
    std::cout << "Num Physical Devices: " << deviceCount << "\n";
  }

  std::vector<VkPhysicalDevice> vPhysicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, vPhysicalDevices.data());

  for (int i = 0; i < vPhysicalDevices.size(); i++) {
    if (isDeviceSuitable(vPhysicalDevices[i])) {
      physicalDevice = vPhysicalDevices[i];
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("Failed to find Physical Device");
  }
}

bool VkEngineDevice::isDeviceSuitable(VkPhysicalDevice device) {

  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceFeatures deviceFeatures;

  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  std::cout << "Device Name: " << deviceProperties.deviceName
            << " Device ID: " << deviceProperties.deviceID << "\n";

  QueueFamilyIndices indices = findQueueFamilies(device);

  bool swapChainAdequate = false;
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
  swapChainAdequate = !swapChainSupport.formats.empty() &&
                      !swapChainSupport.presentModes.empty();

  return indices.graphicsFamily.has_value() &&
         indices.presentFamily.has_value() &&
         checkDeviceExtensionSupport(device) && swapChainAdequate;
}

bool VkEngineDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (int i = 0; i < availableExtensions.size(); i++) {
    // std::cout << "Avail: " << availableExtensions[i].extensionName << "\n";
    requiredExtensions.erase(availableExtensions[i].extensionName);
  }
  if (requiredExtensions.empty()) {
    std::cout << "Physical device contains all required extensions\n";
  }

  return requiredExtensions.empty();
}

QueueFamilyIndices VkEngineDevice::findQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  for (int i = 0; i < queueFamilies.size(); i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
      std::cout << "Graphics Index: " << i << " Queuecount"
                << queueFamilies[i].queueCount << "\n";
    }
    // Find if the device supports window system and present images to the
    // surface we created
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    if (presentSupport) {
      indices.presentFamily = i;
      std::cout << "Present Queue family Index: " << i << " Queuecount"
                << queueFamilies[i].queueCount << "\n";
    }
  }
  return indices;
}

void VkEngineDevice::createLogicalDevice() {

  // Specifying queues to be created
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

  // set will only contain unique values
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                            indices.presentFamily.value()};

  // Assigns priorty to queues to influence scheduling of comand buffer
  // execution
  float queuePriority = 1.0f;
  for (int i = 0; i < uniqueQueueFamilies.size(); i++) {

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = i;
    queueCreateInfo.queueCount = 1;

    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // Specifying used device features
  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());

  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());

  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0,
                   &graphicsQueue);

  // Now create the present queue
  vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0,
                   &presentQueue);
}

void VkEngineDevice::createSurface() {
  if (glfwCreateWindowSurface(instance, vkWindow.window, nullptr, &surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
}

SwapChainSupportDetails
VkEngineDevice::querySwapChainSupport(VkPhysicalDevice device) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                            nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount, details.presentModes.data());
  }
  return details;
}

VkSurfaceFormatKHR VkEngineDevice::chooseSwapSurfaceFormat(
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

VkPresentModeKHR VkEngineDevice::chooseSwapPresentMode(
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

VkExtent2D
VkEngineDevice::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {

  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(vkWindow.window, &width, &height);

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

void VkEngineDevice::createSwapChain() {
  SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(physicalDevice);

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
  createInfo.surface = surface;

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
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
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

  if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount,
                          swapChainImages.data());
}
} // namespace ve
