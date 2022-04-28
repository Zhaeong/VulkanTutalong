#include "vk_device.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>
namespace ve {

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static std::vector<VkVertexInputBindingDescription> getBindingDescription() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
  }

  static std::vector<VkVertexInputAttributeDescription>
  getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
  }
};
class VkModel {
public:
  VkEngineDevice &engineDevice;

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;

  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;

  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;

  VkModel(VkEngineDevice &eDevice);
  ~VkModel();

  void createVertexBuffer(std::vector<Vertex> vertices);

  void createIndexBuffer(std::vector<uint16_t> indices);

  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory);

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
};

} // namespace ve