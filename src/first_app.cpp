#include "first_app.hpp"

namespace ve{
    void FirstApp::run(){
        std::cout << "in herel";

        while(!vkWindow.shouldClose())
        {
            glfwPollEvents();
        }
    }
}