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

#include <glm/glm.hpp>

#include <iostream>
#include <thread>

struct Uniforms {
  glm::vec3 mColor;
  float     mTime;
};

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

    auto texture = device->createTexture("data/textures/box.dds");

    auto uniformBuffer = device->createBuffer(
      sizeof(Uniforms),
      vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

    auto descriptorSet{pipeline->allocateDescriptorSet()};

    {
      vk::DescriptorBufferInfo bufferInfo;
      bufferInfo.buffer = *uniformBuffer->mBuffer;
      bufferInfo.offset = 0;
      bufferInfo.range  = sizeof(Uniforms);

      vk::WriteDescriptorSet info;
      info.dstSet          = descriptorSet;
      info.dstBinding      = 0;
      info.dstArrayElement = 0;
      info.descriptorType  = vk::DescriptorType::eUniformBuffer;
      info.descriptorCount = 1;
      info.pBufferInfo     = &bufferInfo;

      device->getVkDevice()->updateDescriptorSets(info, nullptr);
    }
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

    Uniforms uniforms;
    uniforms.mColor = glm::vec3(1, 0, 0);
    uniforms.mTime  = 0.f;

    while (!window->shouldClose()) {
      window->processInput();

      auto frame = surface->beginFrame();

      frame.mPrimaryCommandBuffer.updateBuffer(
        *uniformBuffer->mBuffer, 0, sizeof(Uniforms), (uint8_t*)&uniforms);

      pipeline->use(frame, descriptorSet);

      surface->beginRenderPass(frame);

      pipeline->setPushConstant(frame, vk::ShaderStageFlagBits::eVertex, 0, glm::vec2(0.2, 0.5));
      frame.mPrimaryCommandBuffer.draw(4, 1, 0, 0);

      surface->endRenderPass(frame);
      surface->endFrame(frame);

      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
