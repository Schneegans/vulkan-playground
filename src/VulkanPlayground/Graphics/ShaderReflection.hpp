////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_SHADER_REFLECTION_HPP
#define ILLUSION_SHADER_REFLECTION_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../Utils/Logger.hpp"
#include "../fwd.hpp"
#include "VulkanDevice.hpp"

#include <map>
#include <unordered_map>

namespace Illusion {

struct FrameInfo;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class ShaderReflection {

 public:
  // -------------------------------------------------------------------------------- public classes
  enum class Stage : uint32_t { Unknown = 0, Vertex = 1 << 0, Fragment = 1 << 1 };

  struct BufferRange {
    std::string mName;
    uint32_t    mSize         = 0;
    uint32_t    mActiveStages = 0;
  };

  struct Buffer {
    std::string mName;
    uint32_t    mSize         = 0;
    uint32_t    mBinding      = 0;
    uint32_t    mActiveStages = 0;

    std::map<uint32_t, BufferRange> mMembers;
  };

  struct Sampler {
    std::string mName;
    uint32_t    mBinding      = 0;
    uint32_t    mActiveStages = 0;
  };

  // -------------------------------------------------------------------------------- static methods
  static vk::ShaderStageFlags toVk(uint32_t stages);
  static vk::ShaderStageFlagBits toVk(Stage stage);

  // -------------------------------------------------------------------------------- public methods
  ShaderReflection(std::string const& name, std::vector<uint32_t> const& code);
  ShaderReflection(std::vector<ShaderReflectionPtr> const& stages);

  void print() const;

  std::string const& getName() const;
  uint32_t           getStages() const;
  std::map<Stage, std::vector<uint32_t>> const& getCode() const;

  std::vector<Buffer> const&  getPushConstantBuffers() const;
  std::vector<Buffer> const&  getUniformBuffers() const;
  std::vector<Sampler> const& getSamplers() const;

 private:
  // ------------------------------------------------------------------------------- private members
  std::string mName;
  uint32_t    mStages = static_cast<uint32_t>(Stage::Unknown);

  std::vector<Buffer>  mPushConstantBuffers;
  std::vector<Buffer>  mUniformBuffers;
  std::vector<Sampler> mSamplers;

  std::map<Stage, std::vector<uint32_t>> mCode;
};
}

#endif // ILLUSION_SHADER_REFLECTION_HPP
