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

  void bind(FrameInfo const& info) const;
  void useDescriptorSet(FrameInfo const& info, vk::DescriptorSet const& descriptorSet) const;

  void setPushConstant(
    FrameInfo const&     info,
    vk::ShaderStageFlags stages,
    uint32_t             size,
    uint8_t*             data,
    uint32_t             offset = 0) const;

  template <typename T>
  void setPushConstant(
    FrameInfo const& info, vk::ShaderStageFlags stages, T data, uint32_t offset = 0) const {

    setPushConstant(info, stages, sizeof(T), reinterpret_cast<uint8_t*>(&data), offset);
  }

  template <typename T>
  void setPushConstant(FrameInfo const& info, T data) const {
    auto stages(reinterpret_cast<const vk::ShaderStageFlags*>(&T::ACTIVE_STAGES));
    setPushConstant(info, *stages, sizeof(T), reinterpret_cast<uint8_t*>(&data));
  }

  vk::DescriptorSet allocateDescriptorSet() const;
  void freeDescriptorSet(vk::DescriptorSet const& set) const;

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
