#include "vk_window.hpp"

int boyopoo = 1;
int boyopoo2;
// extern int boyopoo3;
namespace ve {
VkWindow::VkWindow(int w, int h, std::string name) {
  width = w;
  height = h;
  windowName = name;
  initWindow();
  std::cout << "Started VKWindow\n";
}

VkWindow::~VkWindow() {

  std::cout << "Cleaning up vkWindow Init\n";
  glfwDestroyWindow(window);
  glfwTerminate();
}

void VkWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

bool VkWindow::shouldClose() { return glfwWindowShouldClose(window); }
} // namespace ve