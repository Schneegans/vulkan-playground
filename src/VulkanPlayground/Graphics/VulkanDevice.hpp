////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_VULKAN_DEVICE_HPP
#define ILLUSION_VULKAN_DEVICE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../fwd.hpp"

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Image {
  VkImagePtr        mImage;
  VkDeviceMemoryPtr mMemory;
};

struct Texture {
  VkImagePtr        mImage;
  VkDeviceMemoryPtr mMemory;
  VkImageViewPtr    mImageView;
  VkSamplerPtr      mSampler;
};

struct Buffer {
  VkBufferPtr       mBuffer;
  VkDeviceMemoryPtr mMemory;
};

// -------------------------------------------------------------------------------------------------
class VulkanDevice {

 public:
  // -------------------------------------------------------------------------------- public methods
  VulkanDevice(VulkanInstancePtr const& instance);
  ~VulkanDevice();

  vk::CommandBuffer beginSingleTimeCommands() const;
  void endSingleTimeCommands(vk::CommandBuffer commandBuffer) const;

  TexturePtr createTexture(std::string const& fileName) const;
  TexturePtr createTexture(
    uint32_t                width,
    uint32_t                height,
    vk::Format              format,
    vk::ImageTiling         tiling,
    vk::ImageUsageFlags     usage,
    vk::MemoryPropertyFlags properties) const;

  ImagePtr createImage(
    uint32_t                width,
    uint32_t                height,
    vk::Format              format,
    vk::ImageTiling         tiling,
    vk::ImageUsageFlags     usage,
    vk::MemoryPropertyFlags properties) const;

  VkBufferPtr         createBuffer(vk::BufferCreateInfo const&) const;
  VkCommandPoolPtr    createCommandPool(vk::CommandPoolCreateInfo const&) const;
  VkDescriptorPoolPtr createDescriptorPool(vk::DescriptorPoolCreateInfo const&) const;
  VkDescriptorSetLayoutPtr
                      createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo const&) const;
  VkDeviceMemoryPtr   allocateMemory(vk::MemoryAllocateInfo const&) const;
  VkFencePtr          createFence(vk::FenceCreateInfo const&) const;
  VkFramebufferPtr    createFramebuffer(vk::FramebufferCreateInfo const&) const;
  VkImagePtr          createImage(vk::ImageCreateInfo const&) const;
  VkImageViewPtr      createImageView(vk::ImageViewCreateInfo const&) const;
  VkPipelineLayoutPtr createPipelineLayout(vk::PipelineLayoutCreateInfo const&) const;
  VkPipelinePtr       createPipeline(vk::GraphicsPipelineCreateInfo const&) const;
  VkRenderPassPtr     createRenderPass(vk::RenderPassCreateInfo const&) const;
  VkSamplerPtr        createSampler(vk::SamplerCreateInfo const&) const;
  VkSemaphorePtr      createSemaphore(vk::SemaphoreCreateInfo const&) const;
  VkShaderModulePtr   createShaderModule(vk::ShaderModuleCreateInfo const&) const;
  VkSwapchainKHRPtr   createSwapChainKhr(vk::SwapchainCreateInfoKHR const&) const;

  BufferPtr createBuffer(
    vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const;

  void transitionImageLayout(
    VkImagePtr& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

  VulkanInstancePtr const& getInstance() const { return mInstance; }
  VkDevicePtr const&       getDevice() const { return mDevice; }
  VkCommandPoolPtr const&  getCommandPool() const { return mCommandPool; }
  vk::Queue const&         getGraphicsQueue() const { return mGraphicsQueue; }
  vk::Queue const&         getComputeQueue() const { return mComputeQueue; }
  vk::Queue const&         getPresentQueue() const { return mPresentQueue; }

 private:
  // ------------------------------------------------------------------------------- private methods
  void copyImage(VkImagePtr& src, VkImagePtr& dst, uint32_t width, uint32_t height) const;

  // ------------------------------------------------------------------------------- private members
  VulkanInstancePtr mInstance;
  VkDevicePtr       mDevice;
  vk::Queue         mGraphicsQueue, mComputeQueue, mPresentQueue;

  VkCommandPoolPtr mCommandPool;
};
}

#endif // ILLUSION_VULKAN_DEVICE_HPP
