#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

namespace ve {
    class VkWindow{
        public:
        GLFWwindow *window;
        int width;
        int height;
        std::string windowName;

        VkWindow(int w, int h, std::string name);
        ~VkWindow();
        
        //deleting copy constructors
        VkWindow(const VkWindow &) = delete;
        VkWindow &operator=(const VkWindow &) = delete;

        void initWindow();

        bool shouldClose();


    };
}