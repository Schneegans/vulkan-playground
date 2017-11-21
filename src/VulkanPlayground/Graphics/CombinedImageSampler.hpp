////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_COMBINED_IMAGE_SAMPLER_HPP
#define ILLUSION_GRAPHICS_COMBINED_IMAGE_SAMPLER_HPP

// ---------------------------------------------------------------------------------------- includes
#include "Device.hpp"
#include "Texture.hpp"

namespace Illusion {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
template <typename T>
class CombinedImageSampler : public T {

 public:
  // -------------------------------------------------------------------------------- public members
  TexturePtr mTexture;

  // -------------------------------------------------------------------------------- public methods
  CombinedImageSampler(DevicePtr const& device)
    : mDevice(device) {}

  void bind(vk::DescriptorSet const& descriptorSet) const {
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView   = *mTexture->getImageView();
    imageInfo.sampler     = *mTexture->getSampler();

    vk::WriteDescriptorSet info;
    info.dstSet          = descriptorSet;
    info.dstBinding      = T::BINDING_POINT;
    info.dstArrayElement = 0;
    info.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
    info.descriptorCount = 1;
    info.pImageInfo      = &imageInfo;

    mDevice->getVkDevice()->updateDescriptorSets(info, nullptr);
  }

 private:
  // ------------------------------------------------------------------------------- private members
  DevicePtr mDevice;
};
}
}

#endif // ILLUSION_GRAPHICS_COMBINED_IMAGE_SAMPLER_HPP
