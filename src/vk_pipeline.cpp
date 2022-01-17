#include "vk_pipeline.hpp"

namespace ve {

VkEnginePipeline::VkEnginePipeline(VkEngineDevice &eDevice,
                                   const PipelineConfigInfo &pipelineConfig,
                                   const std::string &vertFilepath,
                                   const std::string &fragFilepath)
    : engineDevice{eDevice} {
  createGraphicsPipeline(vertFilepath, fragFilepath, pipelineConfig);
}

VkEnginePipeline::~VkEnginePipeline() {

  vkDestroyShaderModule(engineDevice.logicalDevice, fragShaderModule, nullptr);
  vkDestroyShaderModule(engineDevice.logicalDevice, vertShaderModule, nullptr);
  vkDestroyPipeline(engineDevice.logicalDevice, graphicsPipeline, nullptr);
}

std::vector<char> VkEnginePipeline::readFile(const std::string &filePath) {

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

void VkEnginePipeline::createGraphicsPipeline(
    const std::string &vertFilepath, const std::string &fragFilepath,
    const PipelineConfigInfo &pipelineConfig) {

  assert(pipelineConfig.pipelineLayout != VK_NULL_HANDLE &&
         "Cannot create graphics pipeline: no pipelineLayout in config");
  assert(pipelineConfig.renderPass != VK_NULL_HANDLE &&
         "Cannot create graphics pipeline: no renderpass in config");
  auto vertCode = readFile(vertFilepath);
  auto fragCode = readFile(fragFilepath);

  std::cout << "Vertext shader Code Size:" << vertCode.size() << "\n";
  std::cout << "Fragment shader Code Size:" << fragCode.size() << "\n";

  vertShaderModule = createShaderModule(vertCode);
  fragShaderModule = createShaderModule(fragCode);

  // Create programmable shader creatInfo

  // Vert shader
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  // Frag shader
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  // Contains both
  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  // hardcoding vertext data in shader so fill struct to specify no vertex data
  // for now
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;

  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &pipelineConfig.inputAssemblyInfo;
  pipelineInfo.pViewportState = &pipelineConfig.viewportStateInfo;
  pipelineInfo.pRasterizationState = &pipelineConfig.rasterizationInfo;
  pipelineInfo.pMultisampleState = &pipelineConfig.multisampleInfo;
  pipelineInfo.pDepthStencilState =
      &pipelineConfig.depthStencilInfo; // Optional
  pipelineInfo.pColorBlendState = &pipelineConfig.colorBlendInfo;
  pipelineInfo.pDynamicState = nullptr; // Optional

  pipelineInfo.layout = pipelineConfig.pipelineLayout;
  pipelineInfo.renderPass = pipelineConfig.renderPass;
  pipelineInfo.subpass = pipelineConfig.subpass;

  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1;              // Optional

  if (vkCreateGraphicsPipelines(engineDevice.logicalDevice, VK_NULL_HANDLE, 1,
                                &pipelineInfo, nullptr,
                                &graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  /*
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;            // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(engineDevice.logicalDevice, &pipelineLayoutInfo,
                               nullptr,
                               pipelineConfig.pipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create pipeline layout!");
    }
    */
}

VkShaderModule
VkEnginePipeline::createShaderModule(const std::vector<char> &shaderCode) {

  VkShaderModule shaderModule;
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = shaderCode.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(shaderCode.data());

  if (vkCreateShaderModule(engineDevice.logicalDevice, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

PipelineConfigInfo
VkEnginePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {

  PipelineConfigInfo configInfo{};

  //***************************************************
  // Input Assembly
  configInfo.inputAssemblyInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  //***************************************************
  // Viewport
  configInfo.viewport.x = 0.0f;
  configInfo.viewport.y = 0.0f;
  configInfo.viewport.width = static_cast<float>(width);
  configInfo.viewport.height = static_cast<float>(height);

  // can't use member engineDevice due to static
  // configInfo.viewport.width = (float)engineDevice.swapChainExtent.width;
  // configInfo.viewport.height = (float)engineDevice.swapChainExtent.height;

  configInfo.viewport.minDepth = 0.0f;
  configInfo.viewport.maxDepth = 1.0f;

  //***************************************************
  // scissor
  configInfo.scissor.offset = {0, 0};
  configInfo.scissor.extent = {width, height};

  //***************************************************
  // Viewport info
  configInfo.viewportStateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  configInfo.viewportStateInfo.viewportCount = 1;
  configInfo.viewportStateInfo.pViewports = &configInfo.viewport;
  configInfo.viewportStateInfo.scissorCount = 1;
  configInfo.viewportStateInfo.pScissors = &configInfo.scissor;

  //***************************************************
  // Rasterizer
  configInfo.rasterizationInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
  // If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes
  // through the rasterizer stage. This basically disables any output to the
  // framebuffer.
  configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  // how fragments are generated for geometry
  // Using any mode other than fill requires enabling a GPU feature.
  // VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
  // VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
  // VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
  configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;

  // thickness of lines in terms of number of fragments.
  configInfo.rasterizationInfo.lineWidth = 1.0f;

  // type of face culling to use. You can disable culling, cull the front faces,
  // cull the back faces or both. The frontFace variable specifies the vertex
  // order for faces to be considered front-facing and can be clockwise or
  // counterclockwise.
  configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

  // can alter the depth values by adding a constant value or biasing them based
  // on a fragment's slope. This is sometimes used for shadow mapping, but we
  // won't be using it.
  configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
  configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
  configInfo.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
  configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional

  //***************************************************
  // Multisampling

  configInfo.multisampleInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
  configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  configInfo.multisampleInfo.minSampleShading = 1.0f;          // Optional
  configInfo.multisampleInfo.pSampleMask = nullptr;            // Optional
  configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
  configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional

  //***************************************************
  // Depth and stencil testing

  configInfo.depthStencilInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.minDepthBounds = 0.0f;
  configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
  configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.front = {};
  configInfo.depthStencilInfo.back = {};

  //***************************************************
  //  Color blend attachment
  // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions

  configInfo.colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
  configInfo.colorBlendAttachment.srcColorBlendFactor =
      VK_BLEND_FACTOR_ONE; // Optional
  configInfo.colorBlendAttachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                       // Optional
  configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  configInfo.colorBlendAttachment.srcAlphaBlendFactor =
      VK_BLEND_FACTOR_ONE; // Optional
  configInfo.colorBlendAttachment.dstAlphaBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                       // Optional
  configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  //***************************************************
  // color blend global

  configInfo.colorBlendInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
  configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
  configInfo.colorBlendInfo.attachmentCount = 1;
  configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
  configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

  //***************************************************
  // Dynamic state
  // limited amount of the state that we've specified in the previous structs
  // can actually be changed without recreating the pipeline. Examples are the
  // size of the viewport, line width and blend constants. If you want to do
  // that, then you'll have to fill in a VkPipelineDynamicStateCreateInfo
  // structure like this
  /*
  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                    VK_DYNAMIC_STATE_LINE_WIDTH};

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = dynamicStates;
  */

  //***************************************************

  return configInfo;
}

} // namespace ve