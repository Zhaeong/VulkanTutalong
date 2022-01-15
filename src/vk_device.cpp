#include "vk_device.hpp"

namespace ve {
VkDevice::VkDevice() { createInstance(); }
VkDevice::~VkDevice() { vkDestroyInstance(instance, nullptr); }
void VkDevice::createInstance() {
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

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;

  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;

  createInfo.enabledLayerCount = 0;

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

  // for (const auto &extension : extensions) {
  //   std::cout << '\t' << extension.extensionName << '\n';
  // }
  for (int i = 0; i < extensions.size(); i++) {

    std::cout << '\t' << extensions[i].extensionName << '\n';
  }

  std::cout << "num glfw ext: " << glfwExtensionCount << "\n";
  for (int j = 0; j < glfwExtensionCount; j++) {
    std::cout << "ext: " << glfwExtensions[j] << "\n";
  }
}
} // namespace ve