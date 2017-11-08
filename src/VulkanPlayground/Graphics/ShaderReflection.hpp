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
    enum class Type { eUnknown, eInt, eUInt, eFloat, eDouble, eStruct };

    Type                 mType;
    std::string          mName;
    uint32_t             mSize{0};
    uint32_t             mOffset{0};
    uint32_t             mBaseSize{0};
    vk::ShaderStageFlags mActiveStages;

    // if larger than one, its a vec type
    uint32_t mElements{1};

    // only set if it's an matrix type
    uint32_t mColumns{0};
    uint32_t mRows{0};
    uint32_t mMatrixStride{0};

    // only set if it's an array type
    std::vector<uint32_t> mArrayLengths{0};
    uint32_t              mArrayStride{0};

    // only set if mType is eStruct
    std::string              mTypeName;
    std::vector<BufferRange> mRanges;

    uint32_t    getBaseSize() const;
    std::string getTypePrefix() const;
    std::string getElementsPostfix() const;
    std::string getArrayPostfix() const;
    std::string getInfoType() const;
    std::string getCppType() const;
  };

  struct Buffer {
    std::string          mName;
    std::string          mType;
    uint32_t             mSize{0};
    uint32_t             mBinding{0};
    vk::ShaderStageFlags mActiveStages;

    std::vector<BufferRange> mRanges;

    std::string toInfoString(uint32_t indent = 0) const;
    std::string toCppString(uint32_t indent = 0) const;
  };

  struct Sampler {
    std::string          mName;
    uint32_t             mBinding{0};
    vk::ShaderStageFlags mActiveStages;

    std::string toInfoString(uint32_t indent = 0) const;
    std::string toCppString(uint32_t indent = 0) const;
  };

  enum class BufferType { ePushConstant, eUniform };

  // -------------------------------------------------------------------------------- public methods
  ShaderReflection(std::vector<uint32_t> const& code);
  ShaderReflection(std::vector<ShaderReflectionPtr> const& stages);

  std::string toInfoString() const;
  std::string toCppString() const;
  std::string toGlslString() const;

  vk::ShaderStageFlags       getStages() const { return mStages; }
  std::vector<Buffer> const& getBuffers(BufferType type) const;
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
