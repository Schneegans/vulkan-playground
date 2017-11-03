////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_VULKAN_FACTORY_HPP
#define ILLUSION_VULKAN_FACTORY_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../fwd.hpp"

#include <memory>

namespace Illusion {

// -------------------------------------------------------------------------------------------------
class VulkanFactory {

 public:
  // ------------------------------------------------------------------------- public static methods
  static VkDebugReportCallbackEXTPtr
  createDebugReportCallbackExt(vk::DebugReportCallbackEXT const&, VkInstancePtr);
  static VkDescriptorSetLayoutPtr
  createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo const&, VkDevicePtr);

  static VkBufferPtr         createBuffer(vk::BufferCreateInfo const&, VkDevicePtr);
  static VkCommandPoolPtr    createCommandPool(vk::CommandPoolCreateInfo const&, VkDevicePtr);
  static VkDescriptorPoolPtr createDescriptorPool(vk::DescriptorPoolCreateInfo const&, VkDevicePtr);
  static VkDeviceMemoryPtr   allocateMemory(vk::MemoryAllocateInfo const&, VkDevicePtr);
  static VkFramebufferPtr    createFramebuffer(vk::FramebufferCreateInfo const&, VkDevicePtr);
  static VkFencePtr          createFence(vk::FenceCreateInfo const&, VkDevicePtr);
  static VkImagePtr          createImage(vk::ImageCreateInfo const&, VkDevicePtr);
  static VkImageViewPtr      createImageView(vk::ImageViewCreateInfo const&, VkDevicePtr);
  static VkInstancePtr       createInstance(vk::InstanceCreateInfo const&);
  static VkPipelinePtr       createPipeline(vk::GraphicsPipelineCreateInfo const&, VkDevicePtr);
  static VkPipelineLayoutPtr createPipelineLayout(vk::PipelineLayoutCreateInfo const&, VkDevicePtr);
  static VkRenderPassPtr     createRenderPass(vk::RenderPassCreateInfo const&, VkDevicePtr);
  static VkSamplerPtr        createSampler(vk::SamplerCreateInfo const&, VkDevicePtr);
  static VkSemaphorePtr      createSemaphore(vk::SemaphoreCreateInfo const&, VkDevicePtr);
  static VkShaderModulePtr   createShaderModule(vk::ShaderModuleCreateInfo const&, VkDevicePtr);
  static VkSwapchainKHRPtr   createSwapChainKhr(vk::SwapchainCreateInfoKHR const&, VkDevicePtr);
};
}

#endif // ILLUSION_VULKAN_FACTORY_HPP
