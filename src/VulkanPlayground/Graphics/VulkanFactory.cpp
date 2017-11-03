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
#include "VulkanFactory.hpp"

#include "../Utils/Logger.hpp"
#include "../Utils/stl_helpers.hpp"

#include <functional>
#include <iostream>

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////

VkBufferPtr VulkanFactory::createBuffer(vk::BufferCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating buffer." << std::endl;
  return createManagedObject(device->createBuffer(info), [device](vk::Buffer* obj) {
    ILLUSION_DEBUG << "Deleting buffer." << std::endl;
    device->destroyBuffer(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkCommandPoolPtr
VulkanFactory::createCommandPool(vk::CommandPoolCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating command pool." << std::endl;
  return createManagedObject(device->createCommandPool(info), [device](vk::CommandPool* obj) {
    ILLUSION_DEBUG << "Deleting command pool." << std::endl;
    device->destroyCommandPool(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDebugReportCallbackEXTPtr VulkanFactory::createDebugReportCallbackExt(
  vk::DebugReportCallbackEXT const& callback, VkInstancePtr instance) {
  ILLUSION_DEBUG << "Creating debug callback." << std::endl;
  return createManagedObject(callback, [instance](vk::DebugReportCallbackEXT* obj) {
    auto destroyCallback =
      (PFN_vkDestroyDebugReportCallbackEXT)instance->getProcAddr("vkDestroyDebugReportCallbackEXT");
    ILLUSION_DEBUG << "Deleting debug callback." << std::endl;
    destroyCallback(*instance, *obj, nullptr);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDescriptorSetLayoutPtr VulkanFactory::createDescriptorSetLayout(
  vk::DescriptorSetLayoutCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating descriptor set layout." << std::endl;
  return createManagedObject(
    device->createDescriptorSetLayout(info), [device](vk::DescriptorSetLayout* obj) {
      ILLUSION_DEBUG << "Deleting descriptor set layout." << std::endl;
      device->destroyDescriptorSetLayout(*obj);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDescriptorPoolPtr
VulkanFactory::createDescriptorPool(vk::DescriptorPoolCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating descriptor pool." << std::endl;
  return createManagedObject(device->createDescriptorPool(info), [device](vk::DescriptorPool* obj) {
    ILLUSION_DEBUG << "Deleting descriptor pool." << std::endl;
    device->destroyDescriptorPool(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDeviceMemoryPtr
VulkanFactory::allocateMemory(vk::MemoryAllocateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Allocating memory." << std::endl;
  return createManagedObject(device->allocateMemory(info), [device](vk::DeviceMemory* obj) {
    ILLUSION_DEBUG << "Freeing memory." << std::endl;
    device->freeMemory(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkFramebufferPtr
VulkanFactory::createFramebuffer(vk::FramebufferCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating framebuffer." << std::endl;
  return createManagedObject(device->createFramebuffer(info), [device](vk::Framebuffer* obj) {
    ILLUSION_DEBUG << "Deleting framebuffer." << std::endl;
    device->destroyFramebuffer(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkFencePtr VulkanFactory::createFence(vk::FenceCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating fence." << std::endl;
  return createManagedObject(device->createFence(info), [device](vk::Fence* obj) {
    ILLUSION_DEBUG << "Deleting fence." << std::endl;
    device->destroyFence(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkImagePtr VulkanFactory::createImage(vk::ImageCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating image." << std::endl;
  return createManagedObject(device->createImage(info), [device](vk::Image* obj) {
    ILLUSION_DEBUG << "Deleting image." << std::endl;
    device->destroyImage(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkImageViewPtr
VulkanFactory::createImageView(vk::ImageViewCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating image view." << std::endl;
  return createManagedObject(device->createImageView(info), [device](vk::ImageView* obj) {
    ILLUSION_DEBUG << "Deleting image view." << std::endl;
    device->destroyImageView(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkInstancePtr VulkanFactory::createInstance(vk::InstanceCreateInfo const& info) {
  ILLUSION_DEBUG << "Creating instance." << std::endl;
  return createManagedObject(vk::createInstance(info), [](vk::Instance* obj) {
    ILLUSION_DEBUG << "Deleting instance." << std::endl;
    obj->destroy();
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkPipelinePtr
VulkanFactory::createPipeline(vk::GraphicsPipelineCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating pipeline." << std::endl;
  return createManagedObject(
    device->createGraphicsPipeline(nullptr, info), [device](vk::Pipeline* obj) {
      ILLUSION_DEBUG << "Deleting pipeline." << std::endl;
      device->destroyPipeline(*obj);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkPipelineLayoutPtr
VulkanFactory::createPipelineLayout(vk::PipelineLayoutCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating pipeline layout." << std::endl;
  return createManagedObject(device->createPipelineLayout(info), [device](vk::PipelineLayout* obj) {
    ILLUSION_DEBUG << "Deleting pipeline layout." << std::endl;
    device->destroyPipelineLayout(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkRenderPassPtr
VulkanFactory::createRenderPass(vk::RenderPassCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating render pass." << std::endl;
  return createManagedObject(device->createRenderPass(info), [device](vk::RenderPass* obj) {
    ILLUSION_DEBUG << "Deleting render pass." << std::endl;
    device->destroyRenderPass(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSamplerPtr VulkanFactory::createSampler(vk::SamplerCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating sampler." << std::endl;
  return createManagedObject(device->createSampler(info), [device](vk::Sampler* obj) {
    ILLUSION_DEBUG << "Deleting sampler." << std::endl;
    device->destroySampler(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSemaphorePtr
VulkanFactory::createSemaphore(vk::SemaphoreCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating semaphore." << std::endl;
  return createManagedObject(device->createSemaphore(info), [device](vk::Semaphore* obj) {
    ILLUSION_DEBUG << "Deleting semaphore." << std::endl;
    device->destroySemaphore(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkShaderModulePtr
VulkanFactory::createShaderModule(vk::ShaderModuleCreateInfo const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating shader module." << std::endl;
  return createManagedObject(device->createShaderModule(info), [device](vk::ShaderModule* obj) {
    ILLUSION_DEBUG << "Deleting shader module." << std::endl;
    device->destroyShaderModule(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSwapchainKHRPtr
VulkanFactory::createSwapChainKhr(vk::SwapchainCreateInfoKHR const& info, VkDevicePtr device) {
  ILLUSION_DEBUG << "Creating swap chain." << std::endl;
  return createManagedObject(device->createSwapchainKHR(info), [device](vk::SwapchainKHR* obj) {
    ILLUSION_DEBUG << "Deleting swap chain." << std::endl;
    device->destroySwapchainKHR(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
