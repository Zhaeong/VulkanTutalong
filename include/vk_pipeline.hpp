#pragma once

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ve {
class VkPipeline {
public:
  VkPipeline(const std::string &vertFilepath, const std::string &fragFilepath);
  static std::vector<char> readFile(const std::string &filePath);

  void createGraphicsPipeline(const std::string &vertFilepath,
                              const std::string &fragFilepath);
};

} // namespace ve