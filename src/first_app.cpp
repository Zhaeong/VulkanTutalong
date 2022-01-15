#include "first_app.hpp"

namespace ve {
void FirstApp::run() {
  std::cout << "In Run";

  while (!vkWindow.shouldClose()) {
    glfwPollEvents();
  }
}
} // namespace ve