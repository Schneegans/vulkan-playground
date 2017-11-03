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
#include "Events/Property.hpp"

#include <memory>
#include <vulkan/vulkan.hpp>

#define ILLUSION_DECLARE_CLASS(TYPE_NAME)                                                          \
  class TYPE_NAME;                                                                                 \
  typedef std::shared_ptr<TYPE_NAME>       TYPE_NAME##Ptr;                                         \
  typedef std::shared_ptr<const TYPE_NAME> TYPE_NAME##ConstPtr;                                    \
  typedef Property<TYPE_NAME##Ptr>         TYPE_NAME##Property;

#define ILLUSION_DECLARE_STRUCT(TYPE_NAME)                                                         \
  struct TYPE_NAME;                                                                                \
  typedef std::shared_ptr<TYPE_NAME>       TYPE_NAME##Ptr;                                         \
  typedef std::shared_ptr<const TYPE_NAME> TYPE_NAME##ConstPtr;                                    \
  typedef Property<TYPE_NAME##Ptr>         TYPE_NAME##Property;

namespace Illusion {

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
ILLUSION_DECLARE_STRUCT(Texture);

ILLUSION_DECLARE_CLASS(AudioBuffer);
ILLUSION_DECLARE_CLASS(AudioComponent);
ILLUSION_DECLARE_CLASS(AudioContext);
ILLUSION_DECLARE_CLASS(BoxCollisionShape);
ILLUSION_DECLARE_CLASS(CameraComponent);
ILLUSION_DECLARE_CLASS(CircleCollisionShape);
ILLUSION_DECLARE_CLASS(ColorMaterial);
ILLUSION_DECLARE_CLASS(Cursor);
ILLUSION_DECLARE_CLASS(Engine);
ILLUSION_DECLARE_CLASS(EngineSettings);
ILLUSION_DECLARE_CLASS(GuiComponent);
ILLUSION_DECLARE_CLASS(GuiContext);
ILLUSION_DECLARE_CLASS(GuiItem);
ILLUSION_DECLARE_CLASS(ICollisionShape);
ILLUSION_DECLARE_CLASS(IComponent);
ILLUSION_DECLARE_CLASS(ICullableComponent);
ILLUSION_DECLARE_CLASS(IDepthComponent);
ILLUSION_DECLARE_CLASS(IDrawableComponent);
ILLUSION_DECLARE_CLASS(IObject);
ILLUSION_DECLARE_CLASS(ISavableObject);
ILLUSION_DECLARE_CLASS(ITransformableComponent);
ILLUSION_DECLARE_CLASS(Jamendo);
ILLUSION_DECLARE_CLASS(Material);
ILLUSION_DECLARE_CLASS(MaterialShader);
ILLUSION_DECLARE_CLASS(Music);
ILLUSION_DECLARE_CLASS(NetworkClient);
ILLUSION_DECLARE_CLASS(NetworkContext);
ILLUSION_DECLARE_CLASS(NetworkServer);
ILLUSION_DECLARE_CLASS(ParticleEmitterComponent);
ILLUSION_DECLARE_CLASS(ParticleOnceEmitterComponent);
ILLUSION_DECLARE_CLASS(ParticleSystem);
ILLUSION_DECLARE_CLASS(ParticleSystemComponent);
ILLUSION_DECLARE_CLASS(Physics);
ILLUSION_DECLARE_CLASS(PhysicsBodyComponent);
ILLUSION_DECLARE_CLASS(PolygonCollisionShape);
ILLUSION_DECLARE_CLASS(ResourceManager);
ILLUSION_DECLARE_CLASS(SavableObjectVisitor);
ILLUSION_DECLARE_CLASS(Scene);
ILLUSION_DECLARE_CLASS(SceneObject);
ILLUSION_DECLARE_CLASS(Scheduler);
ILLUSION_DECLARE_CLASS(ShaderCompiler);
ILLUSION_DECLARE_CLASS(ShaderReflection);
ILLUSION_DECLARE_CLASS(Sound);
ILLUSION_DECLARE_CLASS(SpriteComponent);
ILLUSION_DECLARE_CLASS(Ticker);
ILLUSION_DECLARE_CLASS(Timer);
ILLUSION_DECLARE_CLASS(VulkanDevice);
ILLUSION_DECLARE_CLASS(VulkanInstance);
ILLUSION_DECLARE_CLASS(VulkanSurface);
ILLUSION_DECLARE_CLASS(VulkanFramebuffer);
ILLUSION_DECLARE_CLASS(VulkanSwapChain);
ILLUSION_DECLARE_CLASS(VulkanPhysicalDevice);
ILLUSION_DECLARE_CLASS(WebView);
ILLUSION_DECLARE_CLASS(Window);
}

#undef ILLUSION_DECLARE_CLASS
#undef ILLUSION_DECLARE_STRUCT

#endif // ILLUSION_FWD_HPP
