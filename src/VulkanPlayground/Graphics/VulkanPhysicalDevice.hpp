////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_VULKAN_PHYSICAL_DEVICE_HPP
#define ILLUSION_GRAPHICS_VULKAN_PHYSICAL_DEVICE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "VulkanFactory.hpp"

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class VulkanPhysicalDevice : public vk::PhysicalDevice {
 public:
  VulkanPhysicalDevice(vk::PhysicalDevice const& device);

  VkDevicePtr createDevice(vk::DeviceCreateInfo const& info);

  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

  void printInfo();
};
}

#endif // ILLUSION_GRAPHICS_VULKAN_PHYSICAL_DEVICE_HPP
