////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_VULKAN_SWAPCHAIN_HPP
#define ILLUSION_VULKAN_SWAPCHAIN_HPP

// ---------------------------------------------------------------------------------------- includes
#include "VulkanDevice.hpp"

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class VulkanSwapChain {

 public:
  // -------------------------------------------------------------------------------- public methods
  VulkanSwapChain(VulkanDevicePtr const& device, VkSurfaceKHRPtr const& surface);

  vk::Extent2D const&                   getExtent() const { return mExtent; }
  VkRenderPassPtr const&                getRenderPass() const { return mRenderPass; }
  uint32_t                              getImageCount() const { return mImageCount; }
  std::vector<VulkanFramebuffer> const& getFramebuffers() const { return mFramebuffers; }

  operator VkSwapchainKHRPtr() const { return mSwapChain; }

 private:
  // ------------------------------------------------------------------------------- private methods
  void createSwapChain();
  void createFramebuffers();
  void createRenderPass();

  vk::SurfaceFormatKHR chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& available);
  vk::PresentModeKHR choosePresentMode(std::vector<vk::PresentModeKHR> const& available);
  vk::Extent2D chooseExtent(vk::SurfaceCapabilitiesKHR const& available);

  // ------------------------------------------------------------------------------- private members
  VulkanDevicePtr mDevice;
  VkSurfaceKHRPtr mSurface;

  VkSwapchainKHRPtr mSwapChain;
  VkRenderPassPtr   mRenderPass;

  std::vector<VulkanFramebuffer> mFramebuffers;

  uint32_t     mImageCount;
  vk::Format   mImageFormat;
  vk::Extent2D mExtent;
};
}

#endif // ILLUSION_VULKAN_SWAPCHAIN_HPP
