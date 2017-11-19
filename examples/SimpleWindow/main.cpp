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

    std::cout << pipeline->getReflection()->toInfoString() << std::endl;

    std::cout << "sizeof(Uniforms) = " << sizeof(Reflection::SimpleTexture::Uniforms) << std::endl;
    std::cout << "sizeof(PushConstants) = " << sizeof(Reflection::SimpleTexture::PushConstants)
              << std::endl;

    auto texture = device->createTexture("data/textures/box.dds");

    auto descriptorSet{pipeline->allocateDescriptorSet()};

    Illusion::Graphics::UniformBuffer<Reflection::SimpleTexture::Uniforms> uniformBuffer(device);

    uniformBuffer.value.time = 0.f;

    uniformBuffer.bind(descriptorSet);

    {
      vk::DescriptorImageInfo imageInfo;
      imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      imageInfo.imageView   = *texture->mImageView;
      imageInfo.sampler     = *texture->mSampler;

      vk::WriteDescriptorSet info;
      info.dstSet          = descriptorSet;
      info.dstBinding      = 1;
      info.dstArrayElement = 0;
      info.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
      info.descriptorCount = 1;
      info.pImageInfo      = &imageInfo;

      device->getVkDevice()->updateDescriptorSets(info, nullptr);
    }

    Reflection::SimpleTexture::PushConstants pushConstants;
    pushConstants.pos = glm::vec2(0.2, 0.5);

    while (!window->shouldClose()) {
      window->processInput();

      auto frame = surface->beginFrame();

      uniformBuffer.value.time += 0.01;
      uniformBuffer.update(frame);

      pipeline->use(frame, descriptorSet);

      surface->beginRenderPass(frame);

      pipeline->setPushConstant(frame, vk::ShaderStageFlagBits::eVertex, 0, pushConstants);
      frame.mPrimaryCommandBuffer.draw(4, 1, 0, 0);

      surface->endRenderPass(frame);
      surface->endFrame(frame);

      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
