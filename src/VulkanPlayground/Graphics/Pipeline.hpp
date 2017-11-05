////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_PIPELINE_HPP
#define ILLUSION_GRAPHICS_PIPELINE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../fwd.hpp"

namespace Illusion {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class Pipeline {

 public:
  // -------------------------------------------------------------------------------- public methods
  Pipeline(
    DevicePtr const&                device,
    VkRenderPassPtr const&          renderPass,
    std::vector<std::string> const& shaderFiles,
    uint32_t                        materialCount);
  virtual ~Pipeline();

  void use(FrameInfo const& info, vk::DescriptorSet const& descriptorSet);

  void setPushConstantData(FrameInfo const& info, uint32_t offset, uint32_t size, uint8_t* data);

  vk::DescriptorSet allocateDescriptorSet();
  void freeDescriptorSet(vk::DescriptorSet const& set);

  ShaderReflectionPtr const& getReflection() const { return mReflection; }

 private:
  // ------------------------------------------------------------------------------- private members
  DevicePtr           mDevice;
  ShaderReflectionPtr mReflection;

  VkRenderPassPtr          mVkRenderPass;
  VkDescriptorPoolPtr      mVkDescriptorPool;
  VkDescriptorSetLayoutPtr mVkDescriptorSetLayout;
  VkPipelineLayoutPtr      mVkPipelineLayout;
  VkPipelinePtr            mVkPipeline;
};
}
}

#endif // ILLUSION_GRAPHICS_PIPELINE_HPP
