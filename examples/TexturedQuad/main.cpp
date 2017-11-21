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

#include "shaders/TexturedQuad.hpp"

int main(int argc, char* argv[]) {
  try {
    auto instance = std::make_shared<Illusion::Graphics::Instance>("SimpleWindow");
    auto device   = std::make_shared<Illusion::Graphics::Device>(instance);
    auto window   = std::make_shared<Illusion::Graphics::Window>(device);

    window->open(false);

    auto surface = window->getSurface();

    std::vector<std::string> shader{"data/shaders/TexturedQuad.vert.spv",
                                    "data/shaders/TexturedQuad.frag.spv"};
    auto pipeline =
      std::make_shared<Illusion::Graphics::Pipeline>(device, surface->getRenderPass(), shader, 10);

    auto texture = std::make_shared<Illusion::Graphics::Texture>(
      device, "data/textures/box.dds", vk::SamplerCreateInfo());

    auto descriptorSet = pipeline->allocateDescriptorSet();

    Illusion::Graphics::CombinedImageSampler<Reflection::TexturedQuad::texSampler> sampler(device);
    sampler.mTexture = texture;
    sampler.bind(descriptorSet);

    Reflection::TexturedQuad::PushConstants pushConstants;
    pushConstants.pos = glm::vec2(0.2, 0.5);

    while (!window->shouldClose()) {
      window->processInput();

      auto frame = surface->beginFrame();
      surface->beginRenderPass(frame);

      pushConstants.time += 0.01;
      pipeline->use(frame, descriptorSet);
      pipeline->setPushConstant(frame, vk::ShaderStageFlagBits::eVertex, 0, pushConstants);
      frame.mPrimaryCommandBuffer.draw(4, 1, 0, 0);

      surface->endRenderPass(frame);
      surface->endFrame(frame);

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    device->getVkDevice()->waitIdle();

  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
