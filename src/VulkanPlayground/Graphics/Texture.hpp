////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_TEXTURE_HPP
#define ILLUSION_GRAPHICS_TEXTURE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../fwd.hpp"

namespace Illusion {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Texture {

  // ------------------------------------------------------------------------------ public interface

 public:
  struct TextureLevel {
    int32_t  mWidth;
    int32_t  mHeight;
    uint64_t mSize;
  };

  // --------------------------------------------------------------------------------------- methods
  Texture(DevicePtr const& device, std::string const& fileName);

  Texture(
    DevicePtr const&          device,
    std::vector<TextureLevel> levels,
    vk::Format                format,
    vk::ImageTiling           tiling,
    vk::ImageUsageFlags       usage,
    vk::MemoryPropertyFlags   properties,
    size_t                    size,
    void*                     data);

  VkImagePtr const&        getImage() const { return mImage; }
  VkDeviceMemoryPtr const& getMemory() const { return mMemory; }
  VkImageViewPtr const&    getImageView() const { return mImageView; }
  VkSamplerPtr const&      getSampler() const { return mSampler; }

  // ----------------------------------------------------------------------------- private interface

 private:
  void InitData(
    DevicePtr const&          device,
    std::vector<TextureLevel> levels,
    vk::Format                format,
    vk::ImageTiling           tiling,
    vk::ImageUsageFlags       usage,
    vk::MemoryPropertyFlags   properties,
    size_t                    size,
    void*                     data);

  VkImagePtr        mImage;
  VkDeviceMemoryPtr mMemory;
  VkImageViewPtr    mImageView;
  VkSamplerPtr      mSampler;
};

// -------------------------------------------------------------------------------------------------
}
}

#endif // ILLUSION_GRAPHICS_TEXTURE_HPP
