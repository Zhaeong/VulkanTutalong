#pragma once

#include "vk_window.hpp"
#include "vk_pipeline.hpp"
#include <iostream>


namespace ve {
    class FirstApp{
        public:
            static const int WIDTH = 800;
            static const int HEIGHT = 600;
            VkWindow vkWindow{WIDTH, HEIGHT, "First Vulkan"};

            VkPipeline vkPipeline{"../shaders/simple_shader.vert.spv", "../shaders/simple_shader.frag.spv"};

            void run();
    };
}