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
#include "Device.hpp"

#include "../Utils/Logger.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "VulkanPtr.hpp"

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <set>

namespace Illusion {
namespace Graphics {
////////////////////////////////////////////////////////////////////////////////////////////////////

Device::Device(InstancePtr const& instance)
  : mInstance(instance)
  , mVkDevice(instance->createVkDevice())
  , mVkGraphicsQueue(mVkDevice->getQueue(mInstance->getGraphicsFamily(), 0))
  , mVkComputeQueue(mVkDevice->getQueue(mInstance->getComputeFamily(), 0))
  , mVkPresentQueue(mVkDevice->getQueue(mInstance->getPresentFamily(), 0)) {

  vk::CommandPoolCreateInfo info;
  info.queueFamilyIndex = mInstance->getGraphicsFamily();
  info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

  mVkCommandPool = createVkCommandPool(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Device::~Device() { mVkDevice->waitIdle(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::CommandBuffer Device::beginSingleTimeCommands() const {
  vk::CommandBufferAllocateInfo info;
  info.level              = vk::CommandBufferLevel::ePrimary;
  info.commandPool        = *mVkCommandPool;
  info.commandBufferCount = 1;

  vk::CommandBuffer commandBuffer{mVkDevice->allocateCommandBuffers(info)[0]};

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

  commandBuffer.begin(beginInfo);

  return commandBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Device::endSingleTimeCommands(vk::CommandBuffer commandBuffer) const {
  commandBuffer.end();

  vk::SubmitInfo info;
  info.commandBufferCount = 1;
  info.pCommandBuffers    = &commandBuffer;

  mVkGraphicsQueue.submit(info, nullptr);
  mVkGraphicsQueue.waitIdle();

  mVkDevice->freeCommandBuffers(*mVkCommandPool, commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Device::createTexture(std::string const& fileName) const {

  int      texWidth, texHeight, texChannels;
  stbi_uc* pixels =
    stbi_load(fileName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  vk::DeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) ILLUSION_ERROR << "Failed to load texture " << fileName << "!" << std::endl;

  auto stagingBuffer = createBuffer(
    imageSize,
    vk::BufferUsageFlagBits::eTransferSrc,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

  void* data = mVkDevice->mapMemory(*stagingBuffer->mMemory, 0, imageSize);
  std::memcpy(data, pixels, (size_t)imageSize);
  mVkDevice->unmapMemory(*stagingBuffer->mMemory);

  stbi_image_free(pixels);

  auto result = createTexture(
    texWidth,
    texHeight,
    vk::Format::eR8G8B8A8Unorm,
    vk::ImageTiling::eOptimal,
    vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
    vk::MemoryPropertyFlagBits::eDeviceLocal);

  transitionImageLayout(
    result->mImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

  auto buffer = beginSingleTimeCommands();

  vk::BufferImageCopy info;
  info.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
  info.imageSubresource.mipLevel       = 0;
  info.imageSubresource.baseArrayLayer = 0;
  info.imageSubresource.layerCount     = 1;
  info.imageExtent.width               = texWidth;
  info.imageExtent.height              = texHeight;
  info.imageExtent.depth               = 1;
  info.imageOffset.z                   = 0;

  buffer.copyBufferToImage(
    *stagingBuffer->mBuffer, *result->mImage, vk::ImageLayout::eTransferDstOptimal, info);

  endSingleTimeCommands(buffer);

  transitionImageLayout(
    result->mImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr Device::createTexture(
  uint32_t                width,
  uint32_t                height,
  vk::Format              format,
  vk::ImageTiling         tiling,
  vk::ImageUsageFlags     usage,
  vk::MemoryPropertyFlags properties) const {
  auto image = createImage(width, height, format, tiling, usage, properties);

  auto result     = std::make_shared<Texture>();
  result->mImage  = image->mImage;
  result->mMemory = image->mMemory;

  {
    vk::ImageViewCreateInfo info;
    info.image                           = *image->mImage;
    info.viewType                        = vk::ImageViewType::e2D;
    info.format                          = vk::Format::eR8G8B8A8Unorm;
    info.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = 1;

    result->mImageView = createVkImageView(info);
  }

  {
    vk::SamplerCreateInfo info;
    info.magFilter               = vk::Filter::eLinear;
    info.minFilter               = vk::Filter::eLinear;
    info.addressModeU            = vk::SamplerAddressMode::eRepeat;
    info.addressModeV            = vk::SamplerAddressMode::eRepeat;
    info.addressModeW            = vk::SamplerAddressMode::eRepeat;
    info.anisotropyEnable        = true;
    info.maxAnisotropy           = 16;
    info.borderColor             = vk::BorderColor::eIntOpaqueBlack;
    info.unnormalizedCoordinates = false;
    info.compareEnable           = false;
    info.compareOp               = vk::CompareOp::eAlways;
    info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
    info.mipLodBias              = 0.0f;
    info.minLod                  = 0.0f;
    info.maxLod                  = 0.0f;

    result->mSampler = createVkSampler(info);
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ImagePtr Device::createImage(
  uint32_t                width,
  uint32_t                height,
  vk::Format              format,
  vk::ImageTiling         tiling,
  vk::ImageUsageFlags     usage,
  vk::MemoryPropertyFlags properties) const {

  vk::ImageCreateInfo info;
  info.imageType     = vk::ImageType::e2D;
  info.extent.width  = width;
  info.extent.height = height;
  info.extent.depth  = 1;
  info.mipLevels     = 1;
  info.arrayLayers   = 1;
  info.format        = format;
  info.tiling        = tiling;
  info.initialLayout = vk::ImageLayout::ePreinitialized;
  info.usage         = usage;
  info.sharingMode   = vk::SharingMode::eExclusive;
  info.samples       = vk::SampleCountFlagBits::e1;

  auto result = std::make_shared<Image>();

  result->mImage = createVkImage(info);

  auto requirements = mVkDevice->getImageMemoryRequirements(*result->mImage);

  vk::MemoryAllocateInfo allocInfo;
  allocInfo.allocationSize = requirements.size;
  allocInfo.memoryTypeIndex =
    mInstance->getPhysicalDevice()->findMemoryType(requirements.memoryTypeBits, properties);

  result->mMemory = allocateMemory(allocInfo);

  mVkDevice->bindImageMemory(*result->mImage, *result->mMemory, 0);

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkBufferPtr Device::createVkBuffer(vk::BufferCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating buffer." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createBuffer(info), [device](vk::Buffer* obj) {
    ILLUSION_DEBUG << "Deleting buffer." << std::endl;
    device->waitIdle(); // is that really necessary?
    device->destroyBuffer(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkCommandPoolPtr Device::createVkCommandPool(vk::CommandPoolCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating command pool." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createCommandPool(info), [device](vk::CommandPool* obj) {
    ILLUSION_DEBUG << "Deleting command pool." << std::endl;
    device->destroyCommandPool(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDescriptorSetLayoutPtr
Device::createVkDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating descriptor set layout." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(
    device->createDescriptorSetLayout(info), [device](vk::DescriptorSetLayout* obj) {
      ILLUSION_DEBUG << "Deleting descriptor set layout." << std::endl;
      device->destroyDescriptorSetLayout(*obj);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDescriptorPoolPtr Device::createVkDescriptorPool(vk::DescriptorPoolCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating descriptor pool." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createDescriptorPool(info), [device](vk::DescriptorPool* obj) {
    ILLUSION_DEBUG << "Deleting descriptor pool." << std::endl;
    device->destroyDescriptorPool(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDeviceMemoryPtr Device::allocateMemory(vk::MemoryAllocateInfo const& info) const {
  ILLUSION_DEBUG << "Allocating memory." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->allocateMemory(info), [device](vk::DeviceMemory* obj) {
    ILLUSION_DEBUG << "Freeing memory." << std::endl;
    device->freeMemory(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkFramebufferPtr Device::createVkFramebuffer(vk::FramebufferCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating framebuffer." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createFramebuffer(info), [device](vk::Framebuffer* obj) {
    ILLUSION_DEBUG << "Deleting framebuffer." << std::endl;
    device->destroyFramebuffer(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkFencePtr Device::createVkFence(vk::FenceCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating fence." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createFence(info), [device](vk::Fence* obj) {
    ILLUSION_DEBUG << "Deleting fence." << std::endl;
    device->destroyFence(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkImagePtr Device::createVkImage(vk::ImageCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating image." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createImage(info), [device](vk::Image* obj) {
    ILLUSION_DEBUG << "Deleting image." << std::endl;
    device->destroyImage(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkImageViewPtr Device::createVkImageView(vk::ImageViewCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating image view." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createImageView(info), [device](vk::ImageView* obj) {
    ILLUSION_DEBUG << "Deleting image view." << std::endl;
    device->destroyImageView(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkPipelinePtr Device::createVkPipeline(vk::GraphicsPipelineCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating pipeline." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createGraphicsPipeline(nullptr, info), [device](vk::Pipeline* obj) {
    ILLUSION_DEBUG << "Deleting pipeline." << std::endl;
    device->destroyPipeline(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkPipelineLayoutPtr Device::createVkPipelineLayout(vk::PipelineLayoutCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating pipeline layout." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createPipelineLayout(info), [device](vk::PipelineLayout* obj) {
    ILLUSION_DEBUG << "Deleting pipeline layout." << std::endl;
    device->destroyPipelineLayout(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkRenderPassPtr Device::createVkRenderPass(vk::RenderPassCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating render pass." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createRenderPass(info), [device](vk::RenderPass* obj) {
    ILLUSION_DEBUG << "Deleting render pass." << std::endl;
    device->destroyRenderPass(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSamplerPtr Device::createVkSampler(vk::SamplerCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating sampler." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createSampler(info), [device](vk::Sampler* obj) {
    ILLUSION_DEBUG << "Deleting sampler." << std::endl;
    device->destroySampler(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSemaphorePtr Device::createVkSemaphore(vk::SemaphoreCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating semaphore." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createSemaphore(info), [device](vk::Semaphore* obj) {
    ILLUSION_DEBUG << "Deleting semaphore." << std::endl;
    device->destroySemaphore(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkShaderModulePtr Device::createVkShaderModule(vk::ShaderModuleCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating shader module." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createShaderModule(info), [device](vk::ShaderModule* obj) {
    ILLUSION_DEBUG << "Deleting shader module." << std::endl;
    device->destroyShaderModule(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSwapchainKHRPtr Device::createVkSwapChainKhr(vk::SwapchainCreateInfoKHR const& info) const {
  ILLUSION_DEBUG << "Creating swap chain." << std::endl;
  auto device{mVkDevice};
  return makeVulkanPtr(device->createSwapchainKHR(info), [device](vk::SwapchainKHR* obj) {
    ILLUSION_DEBUG << "Deleting swap chain." << std::endl;
    device->destroySwapchainKHR(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BufferPtr Device::createBuffer(
  vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const {

  auto result = std::make_shared<Buffer>();

  {
    vk::BufferCreateInfo info;
    info.size        = size;
    info.usage       = usage;
    info.sharingMode = vk::SharingMode::eExclusive;

    result->mBuffer = createVkBuffer(info);
  }

  {
    auto requirements = mVkDevice->getBufferMemoryRequirements(*result->mBuffer);

    vk::MemoryAllocateInfo info;
    info.allocationSize = requirements.size;
    info.memoryTypeIndex =
      mInstance->getPhysicalDevice()->findMemoryType(requirements.memoryTypeBits, properties);

    result->mMemory = allocateMemory(info);
  }

  mVkDevice->bindBufferMemory(*result->mBuffer, *result->mMemory, 0);

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Device::transitionImageLayout(
  VkImagePtr& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const {

  vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

  vk::ImageMemoryBarrier barrier;
  barrier.oldLayout                       = oldLayout;
  barrier.newLayout                       = newLayout;
  barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.image                           = *image;
  barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
  barrier.subresourceRange.baseMipLevel   = 0;
  barrier.subresourceRange.levelCount     = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount     = 1;

  vk::PipelineStageFlags sourceStage;
  vk::PipelineStageFlags destinationStage;

  if (
    oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    sourceStage           = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage      = vk::PipelineStageFlagBits::eTransfer;
  } else if (
    oldLayout == vk::ImageLayout::eTransferDstOptimal &&
    newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    sourceStage           = vk::PipelineStageFlagBits::eTransfer;
    destinationStage      = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    ILLUSION_ERROR << "Requested an unsupported layout transition!" << std::endl;
  }

  commandBuffer.pipelineBarrier(
    sourceStage, destinationStage, vk::DependencyFlagBits(), nullptr, nullptr, barrier);

  endSingleTimeCommands(commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Device::copyImage(VkImagePtr& src, VkImagePtr& dst, uint32_t width, uint32_t height) const {

  vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

  vk::ImageSubresourceLayers subResource;
  subResource.aspectMask     = vk::ImageAspectFlagBits::eColor;
  subResource.baseArrayLayer = 0;
  subResource.mipLevel       = 0;
  subResource.layerCount     = 1;

  vk::ImageCopy region;
  region.srcSubresource = subResource;
  region.dstSubresource = subResource;
  region.srcOffset      = vk::Offset3D(0, 0, 0);
  region.dstOffset      = vk::Offset3D(0, 0, 0);
  region.extent.width   = width;
  region.extent.height  = height;
  region.extent.depth   = 1;

  commandBuffer.copyImage(
    *src, vk::ImageLayout::eTransferSrcOptimal, *dst, vk::ImageLayout::eTransferDstOptimal, region);

  endSingleTimeCommands(commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
}
