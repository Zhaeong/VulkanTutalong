#pragma once

#include "vk_window.hpp"
#include <iostream>


namespace ve {
    class FirstApp{
        public:
            static const int WIDTH = 800;
            static const int HEIGHT = 600;
            VkWindow vkWindow{WIDTH, HEIGHT, "First Vulkan"};

            void run();
    };
}