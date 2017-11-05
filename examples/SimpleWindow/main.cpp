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

#include <VulkanPlayground/Graphics/Device.hpp>
#include <VulkanPlayground/Graphics/Instance.hpp>
#include <VulkanPlayground/Graphics/Pipeline.hpp>
#include <VulkanPlayground/Graphics/ShaderReflection.hpp>
#include <VulkanPlayground/Graphics/Surface.hpp>
#include <VulkanPlayground/Graphics/Window.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
  try {
    auto instance{std::make_shared<Illusion::Graphics::Instance>("SimpleWindow")};
    auto device{std::make_shared<Illusion::Graphics::Device>(instance)};
    auto window{std::make_shared<Illusion::Graphics::Window>(device)};

    window->open(false);

    std::vector<std::string> shaderModules{"data/shaders/texture.vert.spv",
                                           "data/shaders/texture.frag.spv"};

    auto pipeline{std::make_shared<Illusion::Graphics::Pipeline>(
      device, window->getSurface()->getRenderPass(), shaderModules, 10)};

    pipeline->getReflection()->print();

    while (!window->shouldClose()) {
      window->processInput();
    }
  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
