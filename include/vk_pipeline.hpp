#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
namespace ve {
    class VkPipeline {
        public:
        VkPipeline(const std::string& vertFilepath, const std::string& fragFilepath);
        static std::vector<char> readFile(const std::string& filePath);

        void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath);

    };
}