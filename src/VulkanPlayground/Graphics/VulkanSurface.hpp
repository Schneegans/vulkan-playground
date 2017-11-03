////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_VULKAN_SURFACE_HPP
#define ILLUSION_GRAPHICS_VULKAN_SURFACE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "VulkanDevice.hpp"

#include <glm/glm.hpp>

struct GLFWwindow;

namespace Illusion {

struct FrameInfo {
  vk::CommandBuffer mPrimaryCommandBuffer;
  uint32_t          mSwapChainImageIndex;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class VulkanSurface {

 public:
  // -------------------------------------------------------------------------------- public classes
  struct CameraUniforms {
    glm::mat4 mTransform;
    float     mTime;
  };

  // -------------------------------------------------------------------------------- public methods
  VulkanSurface(VulkanDevicePtr const& device, GLFWwindow* window);

  FrameInfo beginFrame(CameraUniforms const& camera);
  void beginRenderPass(FrameInfo const& info) const;
  void endRenderPass(FrameInfo const& info) const;
  void endFrame(FrameInfo const& info) const;

  void recreate();

  vk::Extent2D const&    getExtent() const;
  VkRenderPassPtr const& getRenderPass() const;
  BufferPtr const&       getCameraUniformBuffer() const;

 private:
  // ------------------------------------------------------------------------------- private methods
  void createSwapChain();
  void createSemaphores();
  void createCommandBuffers();

  // ------------------------------------------------------------------------------- private members
  VulkanDevicePtr mDevice;

  VkSurfaceKHRPtr mSurface;
  VkSemaphorePtr  mImageAvailableSemaphore;
  VkSemaphorePtr  mRenderFinishedSemaphore;

  std::vector<vk::CommandBuffer> mPrimaryCommandBuffers;
  std::vector<VkFencePtr>        mFences;

  VulkanSwapChainPtr mSwapChain;

  BufferPtr mCameraUniformBuffer;
};
}

#endif // ILLUSION_GRAPHICS_VULKAN_SURFACE_HPP
