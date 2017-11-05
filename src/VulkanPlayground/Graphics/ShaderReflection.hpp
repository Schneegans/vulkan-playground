////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SHADER_REFLECTION_HPP
#define ILLUSION_GRAPHICS_SHADER_REFLECTION_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../Utils/Logger.hpp"
#include "../fwd.hpp"

#include <map>
#include <unordered_map>

namespace Illusion {
namespace Graphics {

struct FrameInfo;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class ShaderReflection {

 public:
  // -------------------------------------------------------------------------------- public classes
  struct BufferRange {
    std::string          mName;
    std::string          mType;
    uint32_t             mSize{0};
    uint32_t             mOffset{0};
    vk::ShaderStageFlags mActiveStages;
  };

  struct Buffer {
    std::string          mName;
    std::string          mType;
    uint32_t             mSize{0};
    uint32_t             mBinding{0};
    vk::ShaderStageFlags mActiveStages;

    std::vector<BufferRange> mRanges;
  };

  struct Sampler {
    std::string          mName;
    uint32_t             mBinding{0};
    vk::ShaderStageFlags mActiveStages;
  };

  // -------------------------------------------------------------------------------- public methods
  ShaderReflection(std::vector<uint32_t> const& code);
  ShaderReflection(std::vector<ShaderReflectionPtr> const& stages);

  void print() const;

  vk::ShaderStageFlags        getStages() const { return mStages; }
  std::vector<Buffer> const&  getPushConstantBuffers() const { return mPushConstantBuffers; }
  std::vector<Buffer> const&  getUniformBuffers() const { return mUniformBuffers; }
  std::vector<Sampler> const& getSamplers() const { return mSamplers; }

 private:
  // ------------------------------------------------------------------------------- private members
  vk::ShaderStageFlags mStages;

  std::vector<Buffer>  mPushConstantBuffers;
  std::vector<Buffer>  mUniformBuffers;
  std::vector<Sampler> mSamplers;
};
}
}

#endif // ILLUSION_GRAPHICS_SHADER_REFLECTION_HPP
