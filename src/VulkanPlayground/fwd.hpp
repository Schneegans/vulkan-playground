////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_FWD_HPP
#define ILLUSION_FWD_HPP

// ---------------------------------------------------------------------------------------- includes
#include <memory>
#include <vulkan/vulkan.hpp>

#define ILLUSION_DECLARE_CLASS(TYPE_NAME)                                                          \
  class TYPE_NAME;                                                                                 \
  typedef std::shared_ptr<TYPE_NAME>       TYPE_NAME##Ptr;                                         \
  typedef std::shared_ptr<const TYPE_NAME> TYPE_NAME##ConstPtr;

#define ILLUSION_DECLARE_STRUCT(TYPE_NAME)                                                         \
  struct TYPE_NAME;                                                                                \
  typedef std::shared_ptr<TYPE_NAME>       TYPE_NAME##Ptr;                                         \
  typedef std::shared_ptr<const TYPE_NAME> TYPE_NAME##ConstPtr;

namespace Illusion {

namespace Graphics {

typedef std::shared_ptr<vk::Buffer>                 VkBufferPtr;
typedef std::shared_ptr<vk::CommandPool>            VkCommandPoolPtr;
typedef std::shared_ptr<vk::DebugReportCallbackEXT> VkDebugReportCallbackEXTPtr;
typedef std::shared_ptr<vk::DescriptorPool>         VkDescriptorPoolPtr;
typedef std::shared_ptr<vk::DescriptorSetLayout>    VkDescriptorSetLayoutPtr;
typedef std::shared_ptr<vk::Device>                 VkDevicePtr;
typedef std::shared_ptr<vk::DeviceMemory>           VkDeviceMemoryPtr;
typedef std::shared_ptr<vk::Fence>                  VkFencePtr;
typedef std::shared_ptr<vk::Framebuffer>            VkFramebufferPtr;
typedef std::shared_ptr<vk::Image>                  VkImagePtr;
typedef std::shared_ptr<vk::ImageView>              VkImageViewPtr;
typedef std::shared_ptr<vk::Instance>               VkInstancePtr;
typedef std::shared_ptr<vk::PhysicalDevice>         VkPhysicalDevicePtr;
typedef std::shared_ptr<vk::Pipeline>               VkPipelinePtr;
typedef std::shared_ptr<vk::PipelineLayout>         VkPipelineLayoutPtr;
typedef std::shared_ptr<vk::RenderPass>             VkRenderPassPtr;
typedef std::shared_ptr<vk::Sampler>                VkSamplerPtr;
typedef std::shared_ptr<vk::Semaphore>              VkSemaphorePtr;
typedef std::shared_ptr<vk::ShaderModule>           VkShaderModulePtr;
typedef std::shared_ptr<vk::SurfaceKHR>             VkSurfaceKHRPtr;
typedef std::shared_ptr<vk::SwapchainKHR>           VkSwapchainKHRPtr;

ILLUSION_DECLARE_STRUCT(Buffer);
ILLUSION_DECLARE_STRUCT(Image);
ILLUSION_DECLARE_STRUCT(FrameInfo);

ILLUSION_DECLARE_CLASS(Device);
ILLUSION_DECLARE_CLASS(Framebuffer);
ILLUSION_DECLARE_CLASS(Instance);
ILLUSION_DECLARE_CLASS(PhysicalDevice);
ILLUSION_DECLARE_CLASS(ShaderReflection);
ILLUSION_DECLARE_CLASS(Surface);
ILLUSION_DECLARE_CLASS(Texture);
ILLUSION_DECLARE_CLASS(Window);
}
}

#undef ILLUSION_DECLARE_CLASS
#undef ILLUSION_DECLARE_STRUCT

#endif // ILLUSION_FWD_HPP
