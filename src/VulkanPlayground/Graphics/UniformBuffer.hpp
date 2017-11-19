////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_UNIFORM_BUFFER_HPP
#define ILLUSION_GRAPHICS_UNIFORM_BUFFER_HPP

// ---------------------------------------------------------------------------------------- includes
#include "Surface.hpp"

namespace Illusion {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
template <typename T>
class UniformBuffer {

 public:
  // -------------------------------------------------------------------------------- public methods
  T value;

  UniformBuffer(DevicePtr const& device)
    : mDevice(device) {
    mBuffer = device->createBuffer(
      sizeof(T),
      vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal);
  }

  void update(Illusion::Graphics::FrameInfo const& info) const {
    info.mPrimaryCommandBuffer.updateBuffer(*mBuffer->mBuffer, 0, sizeof(T), (uint8_t*)&value);
  }

  void bind(vk::DescriptorSet const& descriptorSet) const {
    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = *mBuffer->mBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range  = sizeof(T);

    vk::WriteDescriptorSet info;
    info.dstSet          = descriptorSet;
    info.dstBinding      = T::BINDING_POINT;
    info.dstArrayElement = 0;
    info.descriptorType  = vk::DescriptorType::eUniformBuffer;
    info.descriptorCount = 1;
    info.pBufferInfo     = &bufferInfo;

    mDevice->getVkDevice()->updateDescriptorSets(info, nullptr);
  }

 private:
  // ------------------------------------------------------------------------------- private members
  DevicePtr mDevice;
  BufferPtr mBuffer;
};
}
}

#endif // ILLUSION_GRAPHICS_UNIFORM_BUFFER_HPP
