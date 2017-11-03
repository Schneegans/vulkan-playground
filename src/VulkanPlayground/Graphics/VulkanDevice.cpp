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
#include "VulkanDevice.hpp"

#include "../Utils/Logger.hpp"
#include "../Utils/stl_helpers.hpp"
#include "VulkanInstance.hpp"
#include "VulkanPhysicalDevice.hpp"

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <set>

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanDevice::VulkanDevice(VulkanInstancePtr const& instance)
  : mInstance(instance)
  , mDevice(instance->createDevice())
  , mGraphicsQueue(mDevice->getQueue(mInstance->getGraphicsFamily(), 0))
  , mComputeQueue(mDevice->getQueue(mInstance->getComputeFamily(), 0))
  , mPresentQueue(mDevice->getQueue(mInstance->getPresentFamily(), 0)) {

  vk::CommandPoolCreateInfo info;
  info.queueFamilyIndex = mInstance->getGraphicsFamily();
  info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

  mCommandPool = createCommandPool(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanDevice::~VulkanDevice() { mDevice->waitIdle(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::CommandBuffer VulkanDevice::beginSingleTimeCommands() const {
  vk::CommandBufferAllocateInfo info;
  info.level              = vk::CommandBufferLevel::ePrimary;
  info.commandPool        = *mCommandPool;
  info.commandBufferCount = 1;

  vk::CommandBuffer commandBuffer = mDevice->allocateCommandBuffers(info)[0];

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

  commandBuffer.begin(beginInfo);

  return commandBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanDevice::endSingleTimeCommands(vk::CommandBuffer commandBuffer) const {
  commandBuffer.end();

  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer;

  mGraphicsQueue.submit(submitInfo, nullptr);
  mGraphicsQueue.waitIdle();

  mDevice->freeCommandBuffers(*mCommandPool, commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr VulkanDevice::createTexture(std::string const& fileName) const {

  int      texWidth, texHeight, texChannels;
  stbi_uc* pixels =
    stbi_load(fileName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  vk::DeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) ILLUSION_ERROR << "Failed to load texture " << fileName << "!" << std::endl;

  auto stagingBuffer = createBuffer(
    imageSize,
    vk::BufferUsageFlagBits::eTransferSrc,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

  void* data = mDevice->mapMemory(*stagingBuffer->mMemory, 0, imageSize);
  std::memcpy(data, pixels, (size_t)imageSize);
  mDevice->unmapMemory(*stagingBuffer->mMemory);

  stbi_image_free(pixels);

  auto result = createTexture(
    texWidth,
    texHeight,
    vk::Format::eR8G8B8A8Unorm,
    vk::ImageTiling::eOptimal,
    vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
    vk::MemoryPropertyFlagBits::eDeviceLocal);

  transitionImageLayout(
    result->mImage, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eTransferDstOptimal);

  auto buffer = beginSingleTimeCommands();

  vk::BufferImageCopy bufferCopyRegion;
  bufferCopyRegion.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
  bufferCopyRegion.imageSubresource.mipLevel       = 0;
  bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
  bufferCopyRegion.imageSubresource.layerCount     = 1;
  bufferCopyRegion.imageExtent.width               = texWidth;
  bufferCopyRegion.imageExtent.height              = texHeight;
  bufferCopyRegion.imageExtent.depth               = 1;
  bufferCopyRegion.imageOffset.z                   = 0;

  buffer.copyBufferToImage(
    *stagingBuffer->mBuffer,
    *result->mImage,
    vk::ImageLayout::eTransferDstOptimal,
    bufferCopyRegion);

  endSingleTimeCommands(buffer);

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TexturePtr VulkanDevice::createTexture(
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

    result->mImageView = createImageView(info);
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

    result->mSampler = createSampler(info);
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ImagePtr VulkanDevice::createImage(
  uint32_t                width,
  uint32_t                height,
  vk::Format              format,
  vk::ImageTiling         tiling,
  vk::ImageUsageFlags     usage,
  vk::MemoryPropertyFlags properties) const {

  vk::ImageCreateInfo imageInfo;
  imageInfo.imageType     = vk::ImageType::e2D;
  imageInfo.extent.width  = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth  = 1;
  imageInfo.mipLevels     = 1;
  imageInfo.arrayLayers   = 1;
  imageInfo.format        = format;
  imageInfo.tiling        = tiling;
  imageInfo.initialLayout = vk::ImageLayout::ePreinitialized;
  imageInfo.usage         = usage;
  imageInfo.sharingMode   = vk::SharingMode::eExclusive;
  imageInfo.samples       = vk::SampleCountFlagBits::e1;

  auto result = std::make_shared<Image>();

  result->mImage = createImage(imageInfo);

  auto requirements = mDevice->getImageMemoryRequirements(*result->mImage);

  vk::MemoryAllocateInfo allocInfo;
  allocInfo.allocationSize = requirements.size;
  allocInfo.memoryTypeIndex =
    mInstance->getPhysicalDevice()->findMemoryType(requirements.memoryTypeBits, properties);

  result->mMemory = allocateMemory(allocInfo);

  mDevice->bindImageMemory(*result->mImage, *result->mMemory, 0);

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkBufferPtr VulkanDevice::createBuffer(vk::BufferCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating buffer." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createBuffer(info), [device](vk::Buffer* obj) {
    ILLUSION_DEBUG << "Deleting buffer." << std::endl;
    device->destroyBuffer(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkCommandPoolPtr VulkanDevice::createCommandPool(vk::CommandPoolCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating command pool." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createCommandPool(info), [device](vk::CommandPool* obj) {
    ILLUSION_DEBUG << "Deleting command pool." << std::endl;
    device->destroyCommandPool(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDescriptorSetLayoutPtr
VulkanDevice::createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating descriptor set layout." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(
    device->createDescriptorSetLayout(info), [device](vk::DescriptorSetLayout* obj) {
      ILLUSION_DEBUG << "Deleting descriptor set layout." << std::endl;
      device->destroyDescriptorSetLayout(*obj);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDescriptorPoolPtr
VulkanDevice::createDescriptorPool(vk::DescriptorPoolCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating descriptor pool." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createDescriptorPool(info), [device](vk::DescriptorPool* obj) {
    ILLUSION_DEBUG << "Deleting descriptor pool." << std::endl;
    device->destroyDescriptorPool(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDeviceMemoryPtr VulkanDevice::allocateMemory(vk::MemoryAllocateInfo const& info) const {
  ILLUSION_DEBUG << "Allocating memory." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->allocateMemory(info), [device](vk::DeviceMemory* obj) {
    ILLUSION_DEBUG << "Freeing memory." << std::endl;
    device->freeMemory(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkFramebufferPtr VulkanDevice::createFramebuffer(vk::FramebufferCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating framebuffer." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createFramebuffer(info), [device](vk::Framebuffer* obj) {
    ILLUSION_DEBUG << "Deleting framebuffer." << std::endl;
    device->destroyFramebuffer(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkFencePtr VulkanDevice::createFence(vk::FenceCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating fence." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createFence(info), [device](vk::Fence* obj) {
    ILLUSION_DEBUG << "Deleting fence." << std::endl;
    device->destroyFence(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkImagePtr VulkanDevice::createImage(vk::ImageCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating image." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createImage(info), [device](vk::Image* obj) {
    ILLUSION_DEBUG << "Deleting image." << std::endl;
    device->destroyImage(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkImageViewPtr VulkanDevice::createImageView(vk::ImageViewCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating image view." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createImageView(info), [device](vk::ImageView* obj) {
    ILLUSION_DEBUG << "Deleting image view." << std::endl;
    device->destroyImageView(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkPipelinePtr VulkanDevice::createPipeline(vk::GraphicsPipelineCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating pipeline." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createGraphicsPipeline(nullptr, info), [device](vk::Pipeline* obj) {
    ILLUSION_DEBUG << "Deleting pipeline." << std::endl;
    device->destroyPipeline(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkPipelineLayoutPtr
VulkanDevice::createPipelineLayout(vk::PipelineLayoutCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating pipeline layout." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createPipelineLayout(info), [device](vk::PipelineLayout* obj) {
    ILLUSION_DEBUG << "Deleting pipeline layout." << std::endl;
    device->destroyPipelineLayout(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkRenderPassPtr VulkanDevice::createRenderPass(vk::RenderPassCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating render pass." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createRenderPass(info), [device](vk::RenderPass* obj) {
    ILLUSION_DEBUG << "Deleting render pass." << std::endl;
    device->destroyRenderPass(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSamplerPtr VulkanDevice::createSampler(vk::SamplerCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating sampler." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createSampler(info), [device](vk::Sampler* obj) {
    ILLUSION_DEBUG << "Deleting sampler." << std::endl;
    device->destroySampler(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSemaphorePtr VulkanDevice::createSemaphore(vk::SemaphoreCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating semaphore." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createSemaphore(info), [device](vk::Semaphore* obj) {
    ILLUSION_DEBUG << "Deleting semaphore." << std::endl;
    device->destroySemaphore(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkShaderModulePtr VulkanDevice::createShaderModule(vk::ShaderModuleCreateInfo const& info) const {
  ILLUSION_DEBUG << "Creating shader module." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createShaderModule(info), [device](vk::ShaderModule* obj) {
    ILLUSION_DEBUG << "Deleting shader module." << std::endl;
    device->destroyShaderModule(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSwapchainKHRPtr VulkanDevice::createSwapChainKhr(vk::SwapchainCreateInfoKHR const& info) const {
  ILLUSION_DEBUG << "Creating swap chain." << std::endl;
  auto device{mDevice};
  return makeVulkanPtr(device->createSwapchainKHR(info), [device](vk::SwapchainKHR* obj) {
    ILLUSION_DEBUG << "Deleting swap chain." << std::endl;
    device->destroySwapchainKHR(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BufferPtr VulkanDevice::createBuffer(
  vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const {

  auto result = std::make_shared<Buffer>();

  {
    vk::BufferCreateInfo info;
    info.size        = size;
    info.usage       = usage;
    info.sharingMode = vk::SharingMode::eExclusive;

    result->mBuffer = createBuffer(info);
  }

  {
    auto requirements = mDevice->getBufferMemoryRequirements(*result->mBuffer);

    vk::MemoryAllocateInfo info;
    info.allocationSize = requirements.size;
    info.memoryTypeIndex =
      mInstance->getPhysicalDevice()->findMemoryType(requirements.memoryTypeBits, properties);

    result->mMemory = allocateMemory(info);
  }

  mDevice->bindBufferMemory(*result->mBuffer, *result->mMemory, 0);

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanDevice::transitionImageLayout(
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

  if (
    oldLayout == vk::ImageLayout::ePreinitialized &&
    newLayout == vk::ImageLayout::eTransferSrcOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
  } else if (
    oldLayout == vk::ImageLayout::ePreinitialized &&
    newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
  } else if (
    oldLayout == vk::ImageLayout::eTransferDstOptimal &&
    newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
  } else if (
    oldLayout == vk::ImageLayout::ePreinitialized && newLayout == vk::ImageLayout::eGeneral) {
    barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
  } else {
    ILLUSION_ERROR << "Requested an unsupported layout transition!" << std::endl;
  }

  commandBuffer.pipelineBarrier(
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::PipelineStageFlagBits::eTopOfPipe,
    vk::DependencyFlagBits(),
    nullptr,
    nullptr,
    barrier);

  endSingleTimeCommands(commandBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanDevice::copyImage(
  VkImagePtr& src, VkImagePtr& dst, uint32_t width, uint32_t height) const {

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
