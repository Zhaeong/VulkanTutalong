#include "vk_pipeline.hpp"

namespace ve {

VkPipeline::VkPipeline(const std::string &vertFilepath,
                       const std::string &fragFilepath) {
  createGraphicsPipeline(vertFilepath, fragFilepath);
}

std::vector<char> VkPipeline::readFile(const std::string &filePath) {

  // std::ios::ate means seek the end immediatly
  // std::ios::binary read it in as a binary
  std::ifstream file{filePath, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filePath);
  }

  // tellg gets last position which is the filesize
  size_t fileSize = static_cast<size_t>(file.tellg());

  std::vector<char> buffer(fileSize);

  // Go to beginning
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

void VkPipeline::createGraphicsPipeline(const std::string &vertFilepath,
                                        const std::string &fragFilepath) {
  auto vertCode = readFile(vertFilepath);
  auto fragCode = readFile(fragFilepath);

  std::cout << "Vertext shader Code Size:" << vertCode.size() << "\n";
  std::cout << "Fragment shader Code Size:" << fragCode.size() << "\n";
}

} // namespace ve