////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_VULKAN_INSTANCE_HPP
#define ILLUSION_VULKAN_INSTANCE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../fwd.hpp"

struct GLFWwindow;

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class VulkanInstance {

 public:
  // -------------------------------------------------------------------------------- public methods
  VulkanInstance(std::string const& appName, bool debugMode = true);

  VkDevicePtr     createDevice() const;
  VkSurfaceKHRPtr createSurface(GLFWwindow* window) const;

  VulkanPhysicalDevicePtr const& getPhysicalDevice() const { return mPhysicalDevice; }
  int                            getGraphicsFamily() const { return mGraphicsFamily; }
  int                            getComputeFamily() const { return mComputeFamily; }
  int                            getPresentFamily() const { return mPresentFamily; }

 private:
  // ------------------------------------------------------------------------------- private methods
  void createInstance(std::string const& engineName, std::string const& appName);
  void setupDebugCallback();
  void pickPhysicalDevice();

  // ------------------------------------------------------------------------------- private members
  VkInstancePtr               mInstance;
  VkDebugReportCallbackEXTPtr mDebugCallback;
  VulkanPhysicalDevicePtr     mPhysicalDevice;

  int mGraphicsFamily{-1}, mComputeFamily{-1}, mPresentFamily{-1};

  bool mDebugMode{false};
};
}

#endif // ILLUSION_VULKAN_INSTANCE_HPP
