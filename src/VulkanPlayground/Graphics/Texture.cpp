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
#include "Texture.hpp"

#include "Device.hpp"

#include <gli/gli.hpp>
#include <stb_image.h>

namespace Illusion {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

Texture::Texture(DevicePtr const& device, std::string const& fileName) {

  // first try loading with gli
  gli::texture texture = gli::load(fileName);
  if (!texture.empty()) {
    std::vector<TextureLevel> levels;
    for (uint32_t i{0}; i < texture.levels(); ++i) {
      levels.push_back({texture.extent(i).x, texture.extent(i).y, texture.size(i)});
    }

    InitData(
      device,
      levels,
      (vk::Format)texture.format(),
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal,
      texture.size(),
      texture.data());

    return;
  }

  // then try stb_image
  int   width, height, components, bytes;
  void* data;

  if (stbi_is_hdr(fileName.c_str())) {
    data  = stbi_loadf(fileName.c_str(), &width, &height, &components, 0);
    bytes = 4;
  } else {
    data  = stbi_load(fileName.c_str(), &width, &height, &components, 0);
    bytes = 1;
  }

  if (data) {
    uint64_t                  size = width * height * bytes * components;
    std::vector<TextureLevel> levels;
    levels.push_back({width, height, size});

    vk::Format format;
    if (components == 1) {
      if (bytes == 1)
        format = vk::Format::eR8Unorm;
      else
        format = vk::Format::eR32Sfloat;
    } else if (components == 2) {
      if (bytes == 1)
        format = vk::Format::eR8G8Unorm;
      else
        format = vk::Format::eR32G32Sfloat;
    } else if (components == 3) {
      if (bytes == 1)
        format = vk::Format::eR8G8B8Unorm;
      else
        format = vk::Format::eR32G32B32Sfloat;
    } else {
      if (bytes == 1)
        format = vk::Format::eR8G8B8A8Unorm;
      else
        format = vk::Format::eR32G32B32A32Sfloat;
    }

    InitData(
      device,
      levels,
      format,
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal,
      size,
      data);

    stbi_image_free(data);

    return;
  }

  std::string error(stbi_failure_reason());

  throw std::runtime_error{"Failed to load texture " + fileName + ": " + error};
}

Texture::Texture(
  DevicePtr const&          device,
  std::vector<TextureLevel> levels,
  vk::Format                format,
  vk::ImageTiling           tiling,
  vk::ImageUsageFlags       usage,
  vk::MemoryPropertyFlags   properties,
  size_t                    size,
  void*                     data) {

  InitData(device, levels, format, tiling, usage, properties, size, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Texture::InitData(
  DevicePtr const&          device,
  std::vector<TextureLevel> levels,
  vk::Format                format,
  vk::ImageTiling           tiling,
  vk::ImageUsageFlags       usage,
  vk::MemoryPropertyFlags   properties,
  size_t                    size,
  void*                     data) {

  auto stagingBuffer = device->createBuffer(
    size,
    vk::BufferUsageFlagBits::eTransferSrc,
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
    data);

  auto image = device->createImage(
    levels[0].mWidth, levels[0].mHeight, levels.size(), format, tiling, usage, properties);

  mImage  = image->mImage;
  mMemory = image->mMemory;

  {
    vk::ImageViewCreateInfo info;
    info.image                           = *image->mImage;
    info.viewType                        = vk::ImageViewType::e2D;
    info.format                          = format;
    info.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = levels.size();
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = 1;

    mImageView = device->createVkImageView(info);
  }

  {
    vk::SamplerCreateInfo info;
    info.magFilter               = vk::Filter::eLinear;
    info.minFilter               = vk::Filter::eLinear;
    info.addressModeU            = vk::SamplerAddressMode::eRepeat;
    info.addressModeV            = vk::SamplerAddressMode::eRepeat;
    info.addressModeW            = vk::SamplerAddressMode::eRepeat;
    info.anisotropyEnable        = true;
    info.maxAnisotropy           = 16;
    info.borderColor             = vk::BorderColor::eIntOpaqueBlack;
    info.unnormalizedCoordinates = false;
    info.compareEnable           = false;
    info.compareOp               = vk::CompareOp::eAlways;
    info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
    info.mipLodBias              = 0.0f;
    info.minLod                  = 0.0f;
    info.maxLod                  = levels.size();

    mSampler = device->createVkSampler(info);
  }

  vk::ImageSubresourceRange subresourceRange;
  subresourceRange.aspectMask   = vk::ImageAspectFlagBits::eColor;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount   = levels.size();
  subresourceRange.layerCount   = 1;

  device->transitionImageLayout(
    mImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresourceRange);

  auto buffer = device->beginSingleTimeCommands();

  std::vector<vk::BufferImageCopy> infos;
  uint64_t                         offset = 0;

  for (uint32_t i = 0; i < levels.size(); ++i) {
    vk::BufferImageCopy info;
    info.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
    info.imageSubresource.mipLevel       = i;
    info.imageSubresource.baseArrayLayer = 0;
    info.imageSubresource.layerCount     = 1;
    info.imageExtent.width               = levels[i].mWidth;
    info.imageExtent.height              = levels[i].mHeight;
    info.imageExtent.depth               = 1;
    info.bufferOffset                    = offset;

    infos.push_back(info);

    offset += levels[i].mSize;
  }

  buffer.copyBufferToImage(
    *stagingBuffer->mBuffer, *mImage, vk::ImageLayout::eTransferDstOptimal, infos);

  device->endSingleTimeCommands(buffer);

  device->transitionImageLayout(
    mImage,
    vk::ImageLayout::eTransferDstOptimal,
    vk::ImageLayout::eShaderReadOnlyOptimal,
    subresourceRange);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
}
