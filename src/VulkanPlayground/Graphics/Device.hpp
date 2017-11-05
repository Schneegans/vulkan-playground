////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_DEVICE_HPP
#define ILLUSION_GRAPHICS_DEVICE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../fwd.hpp"

namespace Illusion {
namespace Graphics {

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
class Device {

 public:
  // -------------------------------------------------------------------------------- public methods
  Device(InstancePtr const& instance);
  ~Device();

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

  BufferPtr createBuffer(
    vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const;

  VkBufferPtr         createVkBuffer(vk::BufferCreateInfo const&) const;
  VkCommandPoolPtr    createVkCommandPool(vk::CommandPoolCreateInfo const&) const;
  VkDescriptorPoolPtr createVkDescriptorPool(vk::DescriptorPoolCreateInfo const&) const;
  VkDescriptorSetLayoutPtr
                      createVkDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo const&) const;
  VkDeviceMemoryPtr   allocateMemory(vk::MemoryAllocateInfo const&) const;
  VkFencePtr          createVkFence(vk::FenceCreateInfo const&) const;
  VkFramebufferPtr    createVkFramebuffer(vk::FramebufferCreateInfo const&) const;
  VkImagePtr          createVkImage(vk::ImageCreateInfo const&) const;
  VkImageViewPtr      createVkImageView(vk::ImageViewCreateInfo const&) const;
  VkPipelineLayoutPtr createVkPipelineLayout(vk::PipelineLayoutCreateInfo const&) const;
  VkPipelinePtr       createVkPipeline(vk::GraphicsPipelineCreateInfo const&) const;
  VkRenderPassPtr     createVkRenderPass(vk::RenderPassCreateInfo const&) const;
  VkSamplerPtr        createVkSampler(vk::SamplerCreateInfo const&) const;
  VkSemaphorePtr      createVkSemaphore(vk::SemaphoreCreateInfo const&) const;
  VkShaderModulePtr   createVkShaderModule(vk::ShaderModuleCreateInfo const&) const;
  VkSwapchainKHRPtr   createVkSwapChainKhr(vk::SwapchainCreateInfoKHR const&) const;

  void transitionImageLayout(
    VkImagePtr& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

  InstancePtr const& getInstance() const { return mInstance; }

  VkDevicePtr const&      getVkDevice() const { return mVkDevice; }
  VkCommandPoolPtr const& getVkCommandPool() const { return mVkCommandPool; }
  vk::Queue const&        getVkGraphicsQueue() const { return mVkGraphicsQueue; }
  vk::Queue const&        getVkComputeQueue() const { return mVkComputeQueue; }
  vk::Queue const&        getVkPresentQueue() const { return mVkPresentQueue; }

 private:
  // ------------------------------------------------------------------------------- private methods
  void copyImage(VkImagePtr& src, VkImagePtr& dst, uint32_t width, uint32_t height) const;

  // ------------------------------------------------------------------------------- private members
  InstancePtr mInstance;

  VkDevicePtr      mVkDevice;
  vk::Queue        mVkGraphicsQueue, mVkComputeQueue, mVkPresentQueue;
  VkCommandPoolPtr mVkCommandPool;
};
}
}

#endif // ILLUSION_GRAPHICS_DEVICE_HPP
