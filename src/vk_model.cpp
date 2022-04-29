#include "vk_model.hpp"

namespace ve {

VkModel::VkModel(VkEngineDevice &eDevice) : engineDevice{eDevice} {

  // Triangle
  // vertices = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
  //             {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
  //             {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

  vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
              {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
              {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
              {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

  indices = {0, 1, 2, 2, 3, 0};

  createVertexBuffer(vertices);
  createIndexBuffer(indices);
  createUniformBuffers();
  createDescriptorPool();
}

VkModel::~VkModel() {
  vkDestroyBuffer(engineDevice.logicalDevice, vertexBuffer, nullptr);
  vkFreeMemory(engineDevice.logicalDevice, vertexBufferMemory, nullptr);

  vkDestroyBuffer(engineDevice.logicalDevice, indexBuffer, nullptr);
  vkFreeMemory(engineDevice.logicalDevice, indexBufferMemory, nullptr);

  for (size_t i = 0; i < VkEngineSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(engineDevice.logicalDevice, uniformBuffers[i], nullptr);
    vkFreeMemory(engineDevice.logicalDevice, uniformBuffersMemory[i], nullptr);
  }

  vkDestroyDescriptorPool(engineDevice.logicalDevice, descriptorPool, nullptr);
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
                     &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  // After this buffer has been created, but doesn't have memory inside
  // First step of allocating memory to buffer requires querying its memory
  // requirements
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(engineDevice.logicalDevice, buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(engineDevice.logicalDevice, &allocInfo, nullptr,
                       &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }
  vkBindBufferMemory(engineDevice.logicalDevice, buffer, bufferMemory, 0);
}

void VkModel::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                         VkDeviceSize size) {

  // First need to allocate a temporary command buffer
  //  Can create seperate command pool, but maybe at another time
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = engineDevice.commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(engineDevice.logicalDevice, &allocInfo,
                           &commandBuffer);

  // Begin recording to command buffer
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // telling driver about our onetime usage
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(engineDevice.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(engineDevice.graphicsQueue);
  vkFreeCommandBuffers(engineDevice.logicalDevice, engineDevice.commandPool, 1,
                       &commandBuffer);
}

void VkModel::createVertexBuffer(std::vector<Vertex> vertices) {

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  // Create a staging buffer as source for cpu accessible then copy over to
  // actual bufffer
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(engineDevice.logicalDevice, stagingBufferMemory, 0, bufferSize, 0,
              &data);
  memcpy(data, vertices.data(), (size_t)bufferSize);
  vkUnmapMemory(engineDevice.logicalDevice, stagingBufferMemory);

  createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

  copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
  vkDestroyBuffer(engineDevice.logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(engineDevice.logicalDevice, stagingBufferMemory, nullptr);
}

void VkModel::createIndexBuffer(std::vector<uint16_t> indices) {
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void *data;

  vkMapMemory(engineDevice.logicalDevice, stagingBufferMemory, 0, bufferSize, 0,
              &data);
  memcpy(data, indices.data(), (size_t)bufferSize);

  vkUnmapMemory(engineDevice.logicalDevice, stagingBufferMemory);
  createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

  copyBuffer(stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(engineDevice.logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(engineDevice.logicalDevice, stagingBufferMemory, nullptr);
}

void VkModel::createUniformBuffers() {
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);
  uniformBuffers.resize(VkEngineSwapChain::MAX_FRAMES_IN_FLIGHT);
  uniformBuffersMemory.resize(VkEngineSwapChain::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < VkEngineSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 uniformBuffers[i], uniformBuffersMemory[i]);
  }
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

void VkModel::createDescriptorPool() {
  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount =
      static_cast<uint32_t>(VkEngineSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;

  poolInfo.maxSets =
      static_cast<uint32_t>(VkEngineSwapChain::MAX_FRAMES_IN_FLIGHT);

  if (vkCreateDescriptorPool(engineDevice.logicalDevice, &poolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

} // namespace ve