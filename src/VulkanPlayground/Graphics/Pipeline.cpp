////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "Pipeline.hpp"

#include "../Utils/File.hpp"
#include "../Utils/Logger.hpp"
#include "Device.hpp"
#include "ShaderReflection.hpp"
#include "Surface.hpp"
#include "Window.hpp"

#include <vulkan/spirv_glsl.hpp>

namespace Illusion {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

Pipeline::Pipeline(
  DevicePtr const&                device,
  VkRenderPassPtr const&          renderPass,
  std::vector<std::string> const& shaderFiles,
  uint32_t                        materialCount)
  : mDevice(device)
  , mVkRenderPass(renderPass) {

  // create shader reflection ----------------------------------------------------------------------
  std::vector<ShaderReflectionPtr>   reflections;
  std::vector<std::vector<uint32_t>> shaderCodes;

  for (auto const& shaderFile : shaderFiles) {
    shaderCodes.push_back(File<uint32_t>(shaderFile).getContent());
    reflections.push_back(std::make_shared<ShaderReflection>(shaderFile, shaderCodes.back()));
  }

  mReflection = std::make_shared<ShaderReflection>(reflections);

  // create descriptor pool ------------------------------------------------------------------------
  {
    std::vector<vk::DescriptorPoolSize> pools;

    if (mReflection->getSamplers().size() > 0) {
      vk::DescriptorPoolSize pool;
      pool.type = vk::DescriptorType::eCombinedImageSampler;
      pool.descriptorCount =
        static_cast<uint32_t>(mReflection->getSamplers().size() * materialCount);
      pools.push_back(pool);
    }

    if (mReflection->getUniformBuffers().size() > 0) {
      vk::DescriptorPoolSize pool;
      pool.type = vk::DescriptorType::eUniformBuffer;
      pool.descriptorCount =
        static_cast<uint32_t>(mReflection->getUniformBuffers().size()) * materialCount;
      pools.push_back(pool);
    }

    vk::DescriptorPoolCreateInfo info;
    info.poolSizeCount = static_cast<uint32_t>(pools.size());
    info.pPoolSizes    = pools.data();
    info.maxSets       = materialCount;

    mVkDescriptorPool = mDevice->createVkDescriptorPool(info);
  }

  // vertex input ----------------------------------------------------------------------------------
  vk::PipelineVertexInputStateCreateInfo vertexInputState;
  vertexInputState.vertexBindingDescriptionCount   = 0;
  vertexInputState.vertexAttributeDescriptionCount = 0;

  // input assembly --------------------------------------------------------------------------------
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
  inputAssemblyState.topology               = vk::PrimitiveTopology::eTriangleStrip;
  inputAssemblyState.primitiveRestartEnable = false;

  // viewport state --------------------------------------------------------------------------------
  vk::PipelineViewportStateCreateInfo viewportState;
  viewportState.viewportCount = 1;
  viewportState.scissorCount  = 1;

  // dynamic state ---------------------------------------------------------------------------------
  std::vector<vk::DynamicState> dynamicStates;
  dynamicStates.push_back(vk::DynamicState::eViewport);
  dynamicStates.push_back(vk::DynamicState::eScissor);

  vk::PipelineDynamicStateCreateInfo dynamicState;
  dynamicState.pDynamicStates    = dynamicStates.data();
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());

  // rasterizer ------------------------------------------------------------------------------------
  vk::PipelineRasterizationStateCreateInfo rasterizerState;
  rasterizerState.depthClampEnable        = false;
  rasterizerState.rasterizerDiscardEnable = false;
  rasterizerState.polygonMode             = vk::PolygonMode::eFill;
  rasterizerState.lineWidth               = 1.0f;
  rasterizerState.cullMode                = vk::CullModeFlagBits::eNone;
  rasterizerState.frontFace               = vk::FrontFace::eCounterClockwise;
  rasterizerState.depthBiasEnable         = false;

  // multisampling ---------------------------------------------------------------------------------
  vk::PipelineMultisampleStateCreateInfo multisamplingState;
  multisamplingState.sampleShadingEnable  = false;
  multisamplingState.rasterizationSamples = vk::SampleCountFlagBits::e1;

  // color blending --------------------------------------------------------------------------------
  vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
  colorBlendAttachmentState.colorWriteMask =
    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
    vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachmentState.blendEnable         = true;
  colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
  colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  colorBlendAttachmentState.colorBlendOp        = vk::BlendOp::eAdd;
  colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
  colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
  colorBlendAttachmentState.alphaBlendOp        = vk::BlendOp::eAdd;

  vk::PipelineColorBlendStateCreateInfo colorBlendState;
  colorBlendState.logicOpEnable   = false;
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments    = &colorBlendAttachmentState;

  // pipeline layout -------------------------------------------------------------------------------
  std::vector<vk::DescriptorSetLayoutBinding> bindings;
  for (auto const& resource : mReflection->getUniformBuffers()) {
    bindings.push_back(
      {resource.mBinding, vk::DescriptorType::eUniformBuffer, 1, resource.mActiveStages});
  }

  for (auto const& resource : mReflection->getSamplers()) {
    bindings.push_back(
      {resource.mBinding, vk::DescriptorType::eCombinedImageSampler, 1, resource.mActiveStages});
  }

  vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  descriptorSetLayoutInfo.pBindings    = bindings.data();

  mVkDescriptorSetLayout = mDevice->createVkDescriptorSetLayout(descriptorSetLayoutInfo);
  vk::DescriptorSetLayout descriptorSetLayouts[] = {*mVkDescriptorSetLayout};

  std::vector<vk::PushConstantRange> pushConstantRanges;
  for (auto const& pushConstant : mReflection->getPushConstantBuffers()) {
    pushConstantRanges.push_back({pushConstant.mActiveStages, 0, pushConstant.mSize});
    // for (auto const& range : pushConstant.mMembers) {
    //   Illusion::ILLUSION_MESSAGE << range.second.mActiveStages << std::endl;
    //   pushConstantRanges.push_back(
    //     {ShaderReflection::toVk(range.second.mActiveStages), range.first, range.second.mSize});
    // }
  }

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
  pipelineLayoutInfo.setLayoutCount         = 1;
  pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts;
  pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
  pipelineLayoutInfo.pPushConstantRanges    = pushConstantRanges.data();

  mVkPipelineLayout = mDevice->createVkPipelineLayout(pipelineLayoutInfo);

  // shader state ----------------------------------------------------------------------------------
  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
  std::vector<VkShaderModulePtr>                 shaderModules;

  for (size_t i{0}; i < reflections.size(); ++i) {
    vk::ShaderModuleCreateInfo moduleInfo;
    moduleInfo.codeSize = shaderCodes[i].size() * 4;
    moduleInfo.pCode    = shaderCodes[i].data();

    auto module = mDevice->createVkShaderModule(moduleInfo);

    vk::PipelineShaderStageCreateInfo stageInfo;
    stageInfo.stage  = vk::ShaderStageFlagBits((VkShaderStageFlags)reflections[i]->getStages());
    stageInfo.module = *module;
    stageInfo.pName  = "main";

    shaderStages.push_back(stageInfo);

    // prevent destruction in this scope
    shaderModules.push_back(module);
  }

  // create pipeline -------------------------------------------------------------------------------
  vk::GraphicsPipelineCreateInfo info;
  info.stageCount          = static_cast<uint32_t>(shaderStages.size());
  info.pStages             = shaderStages.data();
  info.pVertexInputState   = &vertexInputState;
  info.pInputAssemblyState = &inputAssemblyState;
  info.pViewportState      = &viewportState;
  info.pRasterizationState = &rasterizerState;
  info.pMultisampleState   = &multisamplingState;
  info.pColorBlendState    = &colorBlendState;
  info.pDynamicState       = &dynamicState;
  info.layout              = *mVkPipelineLayout;
  info.renderPass          = *mVkRenderPass;
  info.subpass             = 0;
  info.basePipelineHandle  = nullptr;

  mVkPipeline = mDevice->createVkPipeline(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Pipeline::~Pipeline() { mDevice->getVkDevice()->waitIdle(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

void Pipeline::use(FrameInfo const& info, vk::DescriptorSet const& descriptorSet) {
  info.mPrimaryCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *mVkPipeline);
  info.mPrimaryCommandBuffer.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, *mVkPipelineLayout, 0, descriptorSet, nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Pipeline::setPushConstantData(
  FrameInfo const& info, uint32_t offset, uint32_t size, uint8_t* data) {
  // for (auto const& pushConstant : mReflection->getPushConstantBuffers()) {
  //   auto range = pushConstant.mMembers.find(offset);
  //   if (range != pushConstant.mMembers.end()) {
  //     if (range->second.mActiveStages) {
  //       info.mPrimaryCommandBuffer.pushConstants(
  //         *mVkPipelineLayout, range->second.mActiveStages, offset, size, data);
  //       return;
  //     } else {
  //       throw std::runtime_error{"Failed to set push constant: Size does not match! (should be "
  //       +
  //                                std::to_string(range->second.mSize) + " but got " +
  //                                std::to_string(size) + ")"};
  //     }
  //   }
  // }
  // ILLUSION_WARNING << "Failed to set push constant: There is no push constant at offset " <<
  // offset
  //                  << "!" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorSet Pipeline::allocateDescriptorSet() {
  vk::DescriptorSetLayout       descriptorSetLayouts[] = {*mVkDescriptorSetLayout};
  vk::DescriptorSetAllocateInfo allocInfo;
  allocInfo.descriptorPool     = *mVkDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts        = descriptorSetLayouts;

  return mDevice->getVkDevice()->allocateDescriptorSets(allocInfo)[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Pipeline::freeDescriptorSet(vk::DescriptorSet const& set) {
  mDevice->getVkDevice()->freeDescriptorSets(*mVkDescriptorPool, set);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
}
