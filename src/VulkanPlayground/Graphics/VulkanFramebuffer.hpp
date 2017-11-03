////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_VULKAN_FRAMEBUFFER_HPP
#define ILLUSION_VULKAN_FRAMEBUFFER_HPP

// ---------------------------------------------------------------------------------------- includes
#include "VulkanDevice.hpp"

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class VulkanFramebuffer {

 public:
  // -------------------------------------------------------------------------------- public methods
  VulkanFramebuffer(
    VulkanDevicePtr const& device,
    VkRenderPassPtr const& renderPass,
    vk::Image const&       image,
    vk::Extent2D const&    extend,
    vk::Format             format);

  vk::Image        mImage;
  VkImageViewPtr   mImageView;
  VkFramebufferPtr mFramebuffer;
};
}

#endif // ILLUSION_VULKAN_FRAMEBUFFER_HPP
