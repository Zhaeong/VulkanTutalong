#pragma once

#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#include <vector>

namespace ve {
class VkDevice {
public:
  VkInstance instance;

  VkDevice();
  ~VkDevice();
  void createInstance();
};
} // namespace ve