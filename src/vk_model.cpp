#include "vk_model.hpp"

namespace ve {

VkModel::VkModel(VkEngineDevice &eDevice) : engineDevice{eDevice} {
  vertices = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
              {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
              {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
  createVertexBuffer(vertices);
}

VkModel::~VkModel() {
  vkDestroyBuffer(engineDevice.logicalDevice, vertexBuffer, nullptr);
  vkFreeMemory(engineDevice.logicalDevice, vertexBufferMemory, nullptr);
}

void VkModel::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties, VkBuffer &buffer,
                           VkDeviceMemory &bufferMemory) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(engineDevice.logicalDevice, &bufferInfo, nullptr,
                     &vertexBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  // After this buffer has been created, but doesn't have memory inside
  // First step of allocating memory to buffer requires querying its memory
  // requirements
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(engineDevice.logicalDevice, vertexBuffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(engineDevice.logicalDevice, &allocInfo, nullptr,
                       &vertexBufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }
  vkBindBufferMemory(engineDevice.logicalDevice, vertexBuffer,
                     vertexBufferMemory, 0);
}
void VkModel::createVertexBuffer(std::vector<Vertex> vertices) {

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
  createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               vertexBuffer, vertexBufferMemory);

  void *data;
  vkMapMemory(engineDevice.logicalDevice, vertexBufferMemory, 0, bufferSize, 0,
              &data);
  memcpy(data, vertices.data(), (size_t)bufferSize);
  vkUnmapMemory(engineDevice.logicalDevice, vertexBufferMemory);
}

uint32_t VkModel::findMemoryType(uint32_t typeFilter,
                                 VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;

  vkGetPhysicalDeviceMemoryProperties(engineDevice.physicalDevice,
                                      &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    // typeFiler specifies the bit field of memory types that are suitable
    // can find index of suitable memory by iterating over all memoryTypes and
    // checking if the bit is set to 1

    // need to also look at special features of the memory, like being able to
    // map so we can write to it from CPU so look for a bitwise match

    if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags &
                                  properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("Failed to find memory type!");
}
} // namespace ve