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
#include <VulkanPlayground/Graphics/UniformBuffer.hpp>
#include <VulkanPlayground/Graphics/Window.hpp>

#include <glm/glm.hpp>
#include <tiny_gltf.h>

#include <iostream>
#include <thread>

#include "shaders/SimpleTexture.hpp"

int main(int argc, char* argv[]) {
  try {
    auto instance{std::make_shared<Illusion::Graphics::Instance>("SimpleWindow")};
    auto device{std::make_shared<Illusion::Graphics::Device>(instance)};
    auto window{std::make_shared<Illusion::Graphics::Window>(device)};

    window->open(false);

    auto surface{window->getSurface()};

    std::vector<std::string> shaderModules{"data/shaders/texture.vert.spv",
                                           "data/shaders/texture.frag.spv"};

    auto pipeline{std::make_shared<Illusion::Graphics::Pipeline>(
      device, surface->getRenderPass(), shaderModules, 10)};

    tinygltf::Model    model;
    tinygltf::TinyGLTF loader;

    while (!window->shouldClose()) {
      window->processInput();

      auto frame = surface->beginFrame();

      surface->beginRenderPass(frame);

      surface->endRenderPass(frame);
      surface->endFrame(frame);

      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
