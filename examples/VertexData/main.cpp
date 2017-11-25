////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <VulkanPlayground/Graphics/CombinedImageSampler.hpp>
#include <VulkanPlayground/Graphics/Device.hpp>
#include <VulkanPlayground/Graphics/Instance.hpp>
#include <VulkanPlayground/Graphics/Pipeline.hpp>
#include <VulkanPlayground/Graphics/ShaderReflection.hpp>
#include <VulkanPlayground/Graphics/Surface.hpp>
#include <VulkanPlayground/Graphics/Texture.hpp>
#include <VulkanPlayground/Graphics/Window.hpp>
#include <VulkanPlayground/Utils/Logger.hpp>

#include <iostream>
#include <thread>

#include "shaders/VertexColors.hpp"

int main(int argc, char* argv[]) {
  try {
    auto instance = std::make_shared<Illusion::Graphics::Instance>("SimpleWindow");
    auto device   = std::make_shared<Illusion::Graphics::Device>(instance);
    auto window   = std::make_shared<Illusion::Graphics::Window>(device);

    window->open(false);

    auto surface = window->getSurface();

    std::vector<std::string> shader{"data/shaders/VertexColors.vert.spv",
                                    "data/shaders/VertexColors.frag.spv"};
    auto pipeline =
      std::make_shared<Illusion::Graphics::Pipeline>(device, surface->getRenderPass(), shader, 10);

    Reflection::VertexColors::PushConstants pushConstants;
    pushConstants.pos = glm::vec2(0.2, 0.5);

    while (!window->shouldClose()) {
      window->processInput();

      auto frame = surface->beginFrame();
      surface->beginRenderPass(frame);

      pushConstants.time += 0.01;
      pipeline->bind(frame);
      pipeline->setPushConstant(frame, pushConstants);
      frame.mPrimaryCommandBuffer.draw(4, 1, 0, 0);

      surface->endRenderPass(frame);
      surface->endFrame(frame);

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    device->getVkDevice()->waitIdle();

  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
