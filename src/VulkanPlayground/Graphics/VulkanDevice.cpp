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
  : mInstance(instance) {

  mDevice = mInstance->createDevice();

  mGraphicsQueue = mDevice->getQueue(mInstance->getGraphicsFamily(), 0);
  mComputeQueue  = mDevice->getQueue(mInstance->getComputeFamily(), 0);
  mPresentQueue  = mDevice->getQueue(mInstance->getPresentFamily(), 0);

  createCommandPool();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanDevice::~VulkanDevice() { mDevice->waitIdle(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::CommandBuffer VulkanDevice::beginSingleTimeCommands() const {
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.level              = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandPool        = *mCommandPool;
  allocInfo.commandBufferCount = 1;

  vk::CommandBuffer commandBuffer = mDevice->allocateCommandBuffers(allocInfo)[0];

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

  auto result        = std::make_shared<Texture>();
  result->mImage     = image->mImage;
  result->mMemory    = image->mMemory;
  result->mImageView = createImageView(result->mImage);
  result->mSampler   = createSampler();

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

  result->mImage = VulkanFactory::createImage(imageInfo, mDevice);

  auto requirements = mDevice->getImageMemoryRequirements(*result->mImage);

  vk::MemoryAllocateInfo allocInfo;
  allocInfo.allocationSize = requirements.size;
  allocInfo.memoryTypeIndex =
    mInstance->getPhysicalDevice()->findMemoryType(requirements.memoryTypeBits, properties);

  result->mMemory = VulkanFactory::allocateMemory(allocInfo, mDevice);

  mDevice->bindImageMemory(*result->mImage, *result->mMemory, 0);

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkImageViewPtr VulkanDevice::createImageView(VkImagePtr const& image) const {
  vk::ImageViewCreateInfo info;
  info.image                           = *image;
  info.viewType                        = vk::ImageViewType::e2D;
  info.format                          = vk::Format::eR8G8B8A8Unorm;
  info.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
  info.subresourceRange.baseMipLevel   = 0;
  info.subresourceRange.levelCount     = 1;
  info.subresourceRange.baseArrayLayer = 0;
  info.subresourceRange.layerCount     = 1;

  return VulkanFactory::createImageView(info, mDevice);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSamplerPtr VulkanDevice::createSampler() const {
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

  return VulkanFactory::createSampler(info, mDevice);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BufferPtr VulkanDevice::createBuffer(
  vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const {

  auto result = std::make_shared<Buffer>();

  vk::BufferCreateInfo info;
  info.size        = size;
  info.usage       = usage;
  info.sharingMode = vk::SharingMode::eExclusive;

  result->mBuffer = VulkanFactory::createBuffer(info, mDevice);

  auto requirements = mDevice->getBufferMemoryRequirements(*result->mBuffer);

  vk::MemoryAllocateInfo allocInfo;
  allocInfo.allocationSize = requirements.size;
  allocInfo.memoryTypeIndex =
    mInstance->getPhysicalDevice()->findMemoryType(requirements.memoryTypeBits, properties);

  result->mMemory = VulkanFactory::allocateMemory(allocInfo, mDevice);

  mDevice->bindBufferMemory(*result->mBuffer, *result->mMemory, 0);

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanDevice::createCommandPool() {
  vk::CommandPoolCreateInfo info;
  info.queueFamilyIndex = mInstance->getGraphicsFamily();
  info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

  mCommandPool = VulkanFactory::createCommandPool(info, mDevice);
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
