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
#include <VulkanPlayground/Graphics/TinyGLTF.hpp>
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

    // load the model ------------------------------------------------------------------------------
    tinygltf::Model model;
    {
      if (argc <= 1) {
        Illusion::ILLUSION_ERROR << "Please provide a GLTF file." << std::endl;
        return -1;
      }

      std::string        file{argv[1]};
      std::string        extension{file.substr(file.find_last_of('.'))};
      std::string        error;
      bool               success = false;
      tinygltf::TinyGLTF loader;

      if (extension == ".glb" || extension == ".bin") {
        Illusion::ILLUSION_MESSAGE << "Loading binary file " << file << "..." << std::endl;
        success = loader.LoadBinaryFromFile(&model, &error, file);

      } else if (extension == ".gltf") {
        Illusion::ILLUSION_MESSAGE << "Loading ascii file " << file << "..." << std::endl;
        success = loader.LoadASCIIFromFile(&model, &error, file);
      } else {
        Illusion::ILLUSION_ERROR << "Unknown extension " << extension << std::endl;
        return -1;
      }

      if (!error.empty()) {
        Illusion::ILLUSION_ERROR << "Error loading file " << file << ": " << error << std::endl;
      }

      if (!success) { return -1; }
    }

    Illusion::ILLUSION_MESSAGE << "accessors: " << model.accessors.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "animations: " << model.animations.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "buffers: " << model.buffers.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "bufferViews: " << model.bufferViews.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "materials: " << model.materials.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "meshes: " << model.meshes.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "nodes: " << model.nodes.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "textures: " << model.textures.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "images: " << model.images.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "skins: " << model.skins.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "samplers: " << model.samplers.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "cameras: " << model.cameras.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "scenes: " << model.scenes.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "lights: " << model.lights.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "extensionsUsed: " << model.extensionsUsed.size() << std::endl;
    Illusion::ILLUSION_MESSAGE << "extensionsRequired: " << model.extensionsRequired.size()
                               << std::endl;
    Illusion::ILLUSION_MESSAGE << "defaultScene: " << model.defaultScene << std::endl;

    for (auto material : model.materials) {
      Illusion::ILLUSION_MESSAGE << "Material: " << material.name << std::endl;
      auto getTextureIndex = [](tinygltf::ParameterMap const& matParams, std::string const& name) {
        auto texIt = matParams.find(name);
        if (texIt == matParams.end()) return -1;
        auto idxIt = texIt->second.json_double_value.find("index");
        if (idxIt == texIt->second.json_double_value.end()) return -1;
        return static_cast<int>(idxIt->second);
      };

      int baseColorTexture         = getTextureIndex(material.values, "baseColorTexture");
      int metallicRoughnessTexture = getTextureIndex(material.values, "metallicRoughnessTexture");
      int normalTexture            = getTextureIndex(material.additionalValues, "normalTexture");
      int occlusionTexture         = getTextureIndex(material.additionalValues, "occlusionTexture");

      auto sampler = Illusion::Graphics::TinyGLTF::createSampler(
        device, model.samplers[model.textures[metallicRoughnessTexture].sampler]);

      auto texture = Illusion::Graphics::TinyGLTF::createTexture(
        device,
        model.samplers[model.textures[metallicRoughnessTexture].sampler],
        model.images[model.textures[metallicRoughnessTexture].source]);
    }

    // create the pipeline -------------------------------------------------------------------------
    window->open(false);

    auto surface{window->getSurface()};

    std::vector<std::string> shaderModules{"data/shaders/texture.vert.spv",
                                           "data/shaders/texture.frag.spv"};

    auto pipeline{std::make_shared<Illusion::Graphics::Pipeline>(
      device, surface->getRenderPass(), shaderModules, 10)};

    // while (!window->shouldClose()) {
    //   window->processInput();

    //   auto frame = surface->beginFrame();

    //   surface->beginRenderPass(frame);

    //   surface->endRenderPass(frame);
    //   surface->endFrame(frame);

    //   std::this_thread::sleep_for(std::chrono::milliseconds(5));
    // }
  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
