////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_WINDOW_HPP
#define ILLUSION_GRAPHICS_WINDOW_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../fwd.hpp"
#include <vector>

struct GLFWwindow;

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Window {

  // ------------------------------------------------------------------------------ public interface

 public:
  // --------------------------------------------------------------------------------------- methods
  Window(VulkanDevicePtr const& device);
  ~Window();

  void open();
  void close();

  bool shouldClose() const;

  void processInput();

  VulkanDevicePtr const&  getDevice() const;
  VulkanSurfacePtr const& getSurface() const;

  // ----------------------------------------------------------------------------- private interface

 private:
  VulkanDevicePtr  mDevice;
  VulkanSurfacePtr mSurface;
  GLFWwindow*      mWindow = nullptr;
};

// -------------------------------------------------------------------------------------------------
}

#endif // ILLUSION_GRAPHICS_WINDOW_HPP
