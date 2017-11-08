////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SURFACE_HPP
#define ILLUSION_GRAPHICS_SURFACE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "Device.hpp"

struct GLFWwindow;

namespace Illusion {
namespace Graphics {

struct FrameInfo {
  vk::CommandBuffer mPrimaryCommandBuffer;
  uint32_t          mSwapChainImageIndex;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Surface {

 public:
  // -------------------------------------------------------------------------------- public methods
  Surface(DevicePtr const& device, GLFWwindow* window);

  FrameInfo beginFrame();
  void beginRenderPass(FrameInfo const& info) const;
  void endRenderPass(FrameInfo const& info) const;
  void endFrame(FrameInfo const& info) const;

  void recreate();

  vk::Extent2D const&             getExtent() const { return mExtent; }
  VkRenderPassPtr const&          getRenderPass() const { return mRenderPass; }
  uint32_t                        getImageCount() const { return mImageCount; }
  std::vector<Framebuffer> const& getFramebuffers() const { return mFramebuffers; }

 private:
  // ------------------------------------------------------------------------------- private methods
  void createSwapChain();
  void createFramebuffers();
  void createRenderPass();
  void createSemaphores();
  void createCommandBuffers();

  // ------------------------------------------------------------------------------- private members
  DevicePtr mDevice;

  VkSurfaceKHRPtr mSurface;
  VkSemaphorePtr  mImageAvailableSemaphore;
  VkSemaphorePtr  mRenderFinishedSemaphore;

  std::vector<vk::CommandBuffer> mPrimaryCommandBuffers;
  std::vector<VkFencePtr>        mFences;

  VkSwapchainKHRPtr        mSwapChain;
  VkRenderPassPtr          mRenderPass;
  std::vector<Framebuffer> mFramebuffers;
  uint32_t                 mImageCount;
  vk::Format               mImageFormat;
  vk::Extent2D             mExtent;
};
}
}

#endif // ILLUSION_GRAPHICS_SURFACE_HPP
