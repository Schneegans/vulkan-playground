////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <VulkanPlayground/Utils/Logger.hpp>

#include <VulkanPlayground/Graphics/VulkanDevice.hpp>
#include <VulkanPlayground/Graphics/VulkanInstance.hpp>
#include <VulkanPlayground/Graphics/Window.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
  try {
    auto instance = std::make_shared<Illusion::VulkanInstance>("SimpleWindow");
    auto device   = std::make_shared<Illusion::VulkanDevice>(instance);
    auto window   = std::make_shared<Illusion::Window>(device);

    window->open();

    while (!window->shouldClose()) {
      window->processInput();
    }
  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
