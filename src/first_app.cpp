#include "first_app.hpp"

namespace ve {

FirstApp::FirstApp() { createCommandBuffers(); }
FirstApp::~FirstApp() {}
void FirstApp::run() {
  std::cout << "In Run\n";

  while (!vkWindow.shouldClose()) {
    glfwPollEvents();
    drawFrame();
  }

  vkDeviceWaitIdle(vkEngineDevice.logicalDevice);
}

void FirstApp::createCommandBuffers() {
  commandBuffers.resize(vkEngineSwapChain.swapChainFramebuffers.size());

  VkCommandBufferAllocateInfo allocInfo{};

  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = vkEngineDevice.commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

  if (vkAllocateCommandBuffers(vkEngineDevice.logicalDevice, &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }

  for (size_t i = 0; i < commandBuffers.size(); i++) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }
    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkEngineSwapChain.renderPass;
    renderPassInfo.framebuffer = vkEngineSwapChain.swapChainFramebuffers[i];

    renderPassInfo.renderArea.offset = {0, 0};
    // make sure to use swapchainextent and not window extent due to high
    // density displays
    renderPassInfo.renderArea.extent = vkEngineSwapChain.swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkEnginePipeline.bindCommandBufferToGraphicsPipelilne(commandBuffers[i]);

    vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[i]);

    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }
}

void FirstApp::drawFrame() {
  // Wait for frame to be signalled before returning
  // Remember fences need to be initialzed since they are unsignaled state by
  // default
  vkWaitForFences(vkEngineDevice.logicalDevice, 1,
                  &vkEngineSwapChain.inFlightFences[currentFrame], VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex;

  // First get the next free image on the swapchain that is not being rendered
  // to The semaphore will let us know
  vkAcquireNextImageKHR(vkEngineDevice.logicalDevice,
                        vkEngineSwapChain.swapChain, UINT64_MAX,
                        vkEngineSwapChain.imageAvailableSemaphore[currentFrame],
                        VK_NULL_HANDLE, &imageIndex);

  // If a fence for this swap chain image has been created already, we have to
  // wait for it before rendering to it in case it's currently being rendered
  // to, since we might acquire the image out of order

  if (vkEngineSwapChain.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(vkEngineDevice.logicalDevice, 1,
                    &vkEngineSwapChain.imagesInFlight[imageIndex], VK_TRUE,
                    UINT64_MAX);
  }

  // After waiting set the current image's fence to the current frame fence
  vkEngineSwapChain.imagesInFlight[imageIndex] =
      vkEngineSwapChain.inFlightFences[currentFrame];

  // Create the command buffer to submit it to the queue
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // which semaphore to wait on before execution begins
  VkSemaphore waitSemaphores[] = {
      vkEngineSwapChain.imageAvailableSemaphore[currentFrame]};

  // which stage to wait on before execution beings
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  // which command buffer to submit for execution
  // should submit the command buffer that binds the swap chain image we just
  // acquired as color attachment the imageIndex we got from acquiring the next
  // free image
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

  // specify which sempahre to signal once command buffer has finished execution
  VkSemaphore signalSemaphores[] = {
      vkEngineSwapChain.renderFinishedSemaphore[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  // then we reset that fence to the unsignalled state for next loop
  // called here and not at the start of loop because we need to set image fence
  // to frame fence
  // Usually best to fall before actually using the fence
  vkResetFences(vkEngineDevice.logicalDevice, 1,
                &vkEngineSwapChain.inFlightFences[currentFrame]);

  // Can take array of submitinfo structs for efficieny
  if (vkQueueSubmit(vkEngineDevice.graphicsQueue, 1, &submitInfo,
                    vkEngineSwapChain.inFlightFences[currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  // Next we submit the result back to the swap chain to have it eventually show
  // up on screen

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  // which semaphore to wait on before presentation, same as specified above for
  // command buffer to signal when it has finished
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  // which swapChain to present images to and the index of the image for each
  // swap chain
  VkSwapchainKHR swapChains[] = {vkEngineSwapChain.swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  // Allows you to specify array of VkResult values to check every individual
  // swap chain not necessary for single swapchain, we can just check for reture
  // value of present function
  presentInfo.pResults = nullptr; // Optional

  // submits request to present and image to the swap chain
  vkQueuePresentKHR(vkEngineDevice.presentQueue, &presentInfo);

  // this waits for the present queue to be idle before submitting to it again
  vkQueueWaitIdle(vkEngineDevice.presentQueue);

  // update the current frame so it goes to the next one
  currentFrame = (currentFrame + 1) % vkEngineSwapChain.MAX_FRAMES_IN_FLIGHT;
}

} // namespace ve