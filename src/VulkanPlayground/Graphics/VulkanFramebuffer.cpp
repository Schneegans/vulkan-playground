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
#include "VulkanFramebuffer.hpp"

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanFramebuffer::VulkanFramebuffer(
  VulkanDevicePtr const& device,
  VkRenderPassPtr const& renderPass,
  vk::Image const&       image,
  vk::Extent2D const&    extend,
  vk::Format             format) {

  {
    vk::ImageViewCreateInfo info;
    info.image                           = image;
    info.viewType                        = vk::ImageViewType::e2D;
    info.format                          = format;
    info.components.r                    = vk::ComponentSwizzle::eIdentity;
    info.components.g                    = vk::ComponentSwizzle::eIdentity;
    info.components.b                    = vk::ComponentSwizzle::eIdentity;
    info.components.a                    = vk::ComponentSwizzle::eIdentity;
    info.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = 1;

    mImageView = device->createImageView(info);
  }

  {
    vk::ImageView attachments[] = {*mImageView};

    vk::FramebufferCreateInfo info;
    info.renderPass      = *renderPass;
    info.attachmentCount = 1;
    info.pAttachments    = attachments;
    info.width           = extend.width;
    info.height          = extend.height;
    info.layers          = 1;

    mFramebuffer = device->createFramebuffer(info);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
