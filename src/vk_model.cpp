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
  createTextureImage();
}

VkModel::~VkModel() {
  vkDestroyBuffer(engineDevice.logicalDevice, vertexBuffer, nullptr);
  vkFreeMemory(engineDevice.logicalDevice, vertexBufferMemory, nullptr);

  vkDestroyBuffer(engineDevice.logicalDevice, indexBuffer, nullptr);
  vkFreeMemory(engineDevice.logicalDevice, indexBufferMemory, nullptr);

  for (size_t i = 0; i < VkEngineDevice::MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(engineDevice.logicalDevice, uniformBuffers[i], nullptr);
    vkFreeMemory(engineDevice.logicalDevice, uniformBuffersMemory[i], nullptr);
  }

  vkDestroyDescriptorPool(engineDevice.logicalDevice, descriptorPool, nullptr);

  vkDestroyImage(engineDevice.logicalDevice, textureImage, nullptr);
  vkFreeMemory(engineDevice.logicalDevice, textureImageMemory, nullptr);
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

VkCommandBuffer VkModel::beginSingleTimeCommands() {
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

  return commandBuffer;
}

void VkModel::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
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

void VkModel::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                         VkDeviceSize size) {

  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer);
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
  uniformBuffers.resize(VkEngineDevice::MAX_FRAMES_IN_FLIGHT);
  uniformBuffersMemory.resize(VkEngineDevice::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < VkEngineDevice::MAX_FRAMES_IN_FLIGHT; i++) {
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
      static_cast<uint32_t>(VkEngineDevice::MAX_FRAMES_IN_FLIGHT);

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;

  poolInfo.maxSets =
      static_cast<uint32_t>(VkEngineDevice::MAX_FRAMES_IN_FLIGHT);

  if (vkCreateDescriptorPool(engineDevice.logicalDevice, &poolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void VkModel::createImage(uint32_t width, uint32_t height, VkFormat format,
                          VkImageTiling tiling, VkImageUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkImage &image,
                          VkDeviceMemory &imageMemory) {

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = static_cast<uint32_t>(width);
  imageInfo.extent.height = static_cast<uint32_t>(height);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;

  imageInfo.format = format;

  // If you want to be able to directly access texels in the memory of the
  // image, then you must use VK_IMAGE_TILING_LINEAR
  imageInfo.tiling = tiling;

  //    VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first
  //    transition will discard the texels.
  //   VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first
  //   transition will preserve the texels.
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  // We're copying info into this image so set DST_BIT
  // Also want to access this image from shader to color the mesh so use
  // Sampled Bit
  imageInfo.usage = usage;

  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  // for multisampling, relevant for images used as attachements
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  if (vkCreateImage(engineDevice.logicalDevice, &imageInfo, nullptr, &image) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(engineDevice.logicalDevice, image,
                               &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(engineDevice.logicalDevice, &allocInfo, nullptr,
                       &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(engineDevice.logicalDevice, image, imageMemory, 0);
}

// Loads an image and pushes the pixel values into a buffer
void VkModel::createTextureImage() {
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight,
                              &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(engineDevice.logicalDevice, stagingBufferMemory, 0, imageSize, 0,
              &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(engineDevice.logicalDevice, stagingBufferMemory);

  stbi_image_free(pixels);

  // Now that pixel info is in the buffer, we can create a VkImage
  // Note pixels inside VkImage is referrred as Texels

  createImage(
      texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

  transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  copyBufferToImage(stagingBuffer, textureImage,
                    static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));

  transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(engineDevice.logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(engineDevice.logicalDevice, stagingBufferMemory, nullptr);
}

void VkModel::transitionImageLayout(VkImage image, VkFormat format,
                                    VkImageLayout oldLayout,
                                    VkImageLayout newLayout) {

  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;

  // must be set to VK_QUEUE_FAMILY_IGNORED if you don't want to transfer queue
  // family ownership
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  barrier.srcAccessMask = 0; // TODO
  barrier.dstAccessMask = 0; // TODO

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);
  endSingleTimeCommands(commandBuffer);
}
void VkModel::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                                uint32_t height) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  endSingleTimeCommands(commandBuffer);
}
} // namespace ve