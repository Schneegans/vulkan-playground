////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------- includes
#include "ShaderReflection.hpp"

#include <iomanip>
#include <vulkan/spirv_glsl.hpp>

namespace Illusion {
namespace Graphics {
namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string toString(vk::ShaderStageFlags stages) {
  std::string result;

  auto addStage = [&](std::string const& name, vk::ShaderStageFlags stage) {
    if ((VkShaderStageFlags)(stages & stage) > 0) {
      if (result.size() > 0) { result += " | "; }
      result += name;
    }
  };

  addStage("Compute", vk::ShaderStageFlagBits::eCompute);
  addStage("Fragment", vk::ShaderStageFlagBits::eFragment);
  addStage("Geometry", vk::ShaderStageFlagBits::eGeometry);
  addStage("TessellationControl", vk::ShaderStageFlagBits::eTessellationControl);
  addStage("TessellationEvaluation", vk::ShaderStageFlagBits::eTessellationEvaluation);
  addStage("Vertex", vk::ShaderStageFlagBits::eVertex);

  if (result.size() == 0) { result = "None"; }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::BufferRange::Type convert(spirv_cross::SPIRType type) {
  switch (type.basetype) {
  case spirv_cross::SPIRType::Int:
    return ShaderReflection::BufferRange::Type::eInt;
  case spirv_cross::SPIRType::Boolean:
  case spirv_cross::SPIRType::UInt:
    return ShaderReflection::BufferRange::Type::eUInt;
  case spirv_cross::SPIRType::Float:
    return ShaderReflection::BufferRange::Type::eFloat;
  case spirv_cross::SPIRType::Double:
    return ShaderReflection::BufferRange::Type::eDouble;
  case spirv_cross::SPIRType::Struct:
    return ShaderReflection::BufferRange::Type::eStruct;
  default:
    return ShaderReflection::BufferRange::Type::eUnknown;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::ShaderReflection(std::vector<uint32_t> const& code) {
  spirv_cross::Compiler        parser{code};
  spirv_cross::ShaderResources resources       = parser.get_shader_resources();
  auto                         activeVariables = parser.get_active_interface_variables();

  // collect basic information ---------------------------------------------------------------------
  auto stage = parser.get_execution_model();

  switch (stage) {
  case spv::ExecutionModelVertex:
    mStages = vk::ShaderStageFlagBits::eVertex;
    break;
  case spv::ExecutionModelFragment:
    mStages = vk::ShaderStageFlagBits::eFragment;
    break;
  default:
    throw std::runtime_error{"Shader stage is not supported!"};
    break;
  }

  // collect buffers -------------------------------------------------------------------------------
  auto getBufferRanges = [&](spirv_cross::Resource const& resource) {
    std::vector<BufferRange> result;
    auto                     type = parser.get_type(resource.type_id);
    auto                     activeMembers{parser.get_active_buffer_ranges(resource.id)};

    for (size_t i{0}; i < type.member_types.size(); ++i) {
      BufferRange range;
      auto        memberType = parser.get_type(type.member_types[i]);

      range.mType     = convert(memberType);
      range.mName     = parser.get_member_name(resource.base_type_id, i);
      range.mSize     = parser.get_declared_struct_member_size(type, i);
      range.mOffset   = parser.type_struct_member_offset(type, i);
      range.mBaseSize = memberType.width / 8;

      for (auto const& activeMember : activeMembers) {
        if (activeMember.index == i) { range.mActiveStages = mStages; }
      }

      // vector types
      range.mElements = memberType.vecsize;

      // matrix types
      if (parser.has_member_decoration(
            resource.base_type_id, i, spv::Decoration::DecorationMatrixStride)) {
        range.mColumns      = memberType.columns;
        range.mRows         = memberType.vecsize;
        range.mMatrixStride = parser.type_struct_member_matrix_stride(type, i);
      }

      // array types
      if (!memberType.array.empty()) {
        range.mArrayLengths = memberType.array;
        range.mArrayStride  = parser.type_struct_member_array_stride(type, i);
      }

      // struct types
      if (range.mType == BufferRange::Type::eStruct) {

        uint32_t structBaseTypeId{type.member_types[i]};

        Illusion::ILLUSION_MESSAGE << parser.get_name(structBaseTypeId) << std::endl;

        // range.mRanges = getBufferRanges(structResource);
      }

      result.push_back(range);
    }

    return result;
  };

  auto getBuffers = [&](std::vector<spirv_cross::Resource> const& resources) {
    std::vector<Buffer> result;

    for (auto const& resource : resources) {
      Buffer buffer;
      auto   type = parser.get_type(resource.type_id);

      buffer.mName    = parser.get_name(resource.id);
      buffer.mType    = parser.get_name(resource.base_type_id);
      buffer.mSize    = parser.get_declared_struct_size(type);
      buffer.mBinding = parser.get_decoration(resource.id, spv::DecorationBinding);
      if (activeVariables.find(resource.id) != activeVariables.end()) {
        buffer.mActiveStages = mStages;
      }

      buffer.mRanges = getBufferRanges(resource);

      result.push_back(buffer);
    }

    return result;
  };

  mPushConstantBuffers = getBuffers(resources.push_constant_buffers);
  mUniformBuffers      = getBuffers(resources.uniform_buffers);

  // collect image samplers ------------------------------------------------------------------------
  auto getSamplers = [&](std::vector<spirv_cross::Resource> const& resources) {
    std::vector<Sampler> result;
    for (auto const& resource : resources) {
      Sampler sampler;
      sampler.mName    = resource.name;
      sampler.mBinding = parser.get_decoration(resource.id, spv::DecorationBinding);

      if (activeVariables.find(resource.id) != activeVariables.end()) {
        sampler.mActiveStages = mStages;
      }

      result.push_back(sampler);
    }
    return result;
  };

  mSamplers = getSamplers(resources.sampled_images);

  // warn if not supported features are used -------------------------------------------------------
  auto errorNotSupported =
    [this](std::string const& name, std::vector<spirv_cross::Resource> const& resources) {
      if (resources.size() > 0) {
        throw std::runtime_error{"Support for " + name + " is not implemented yet."};
      }
    };

  errorNotSupported("Atomic counters", resources.atomic_counters);
  errorNotSupported("Separate images", resources.separate_images);
  errorNotSupported("Separate samplers", resources.separate_samplers);
  errorNotSupported("Storage buffers", resources.storage_buffers);
  errorNotSupported("Storage images", resources.storage_images);
  errorNotSupported("Subpass inputs", resources.subpass_inputs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::ShaderReflection(std::vector<ShaderReflectionPtr> const& stages) {
  for (auto const& stage : stages) {

    // check that we do not have such a stage already
    if ((VkShaderStageFlags)(mStages & stage->mStages) > 0) {
      throw std::runtime_error{toString(stage->mStages) + " shader stage is already present!"};
    }

    // concatenate stages
    mStages |= stage->mStages;

    // combine buffers
    auto mergeBuffers =
      [this](std::vector<Buffer> const& srcBuffers, std::vector<Buffer>& dstBuffers) {

        for (auto const& srcBuffer : srcBuffers) {
          bool merged = false;

          for (auto& dstBuffer : dstBuffers) {

            // there is already a buffer at this binding point
            if (srcBuffer.mBinding == dstBuffer.mBinding) {

              // check if they have the same type
              if (srcBuffer.mType != dstBuffer.mType) {
                throw std::runtime_error{"Types of Buffers at binding point " +
                                         std::to_string(dstBuffer.mBinding) + " do not match!"};
              }

              // check if they have the same size
              if (srcBuffer.mSize != dstBuffer.mSize) {
                throw std::runtime_error{"Sizes of Buffers at binding point " +
                                         std::to_string(dstBuffer.mBinding) + " do not match!"};
              }

              // check if they have the same ranges
              if (srcBuffer.mRanges.size() != dstBuffer.mRanges.size()) {
                throw std::runtime_error{"Ranges of Buffers at binding point " +
                                         std::to_string(dstBuffer.mBinding) + " do not match!"};
              }

              for (size_t i{0}; i < srcBuffer.mRanges.size(); ++i) {
                // check if they have the same type
                if (srcBuffer.mRanges[i].mType != dstBuffer.mRanges[i].mType) {
                  throw std::runtime_error{"Types of Range #" + std::to_string(i) +
                                           " of Buffer at binding point " +
                                           std::to_string(dstBuffer.mBinding) + " do not match!"};
                }

                // check if they have the same size
                if (srcBuffer.mRanges[i].mSize != dstBuffer.mRanges[i].mSize) {
                  throw std::runtime_error{"Sizes of Range #" + std::to_string(i) + " of Buffer " +
                                           dstBuffer.mType + " at binding point " +
                                           std::to_string(dstBuffer.mBinding) + " do not match!"};
                }

                // check if they have the same offsets
                if (srcBuffer.mRanges[i].mOffset != dstBuffer.mRanges[i].mOffset) {
                  throw std::runtime_error{"Offsets of Range #" + std::to_string(i) +
                                           " of Buffer " + dstBuffer.mType + " at binding point " +
                                           std::to_string(dstBuffer.mBinding) + " do not match!"};
                }

                dstBuffer.mRanges[i].mActiveStages |= srcBuffer.mRanges[i].mActiveStages;
              }

              dstBuffer.mActiveStages |= srcBuffer.mActiveStages;

              merged = true;
              break;
            }
          }

          // this buffer is not part of the combined module yet
          if (!merged) dstBuffers.push_back(srcBuffer);
        }
      };

    // combine samplers
    auto mergeSamplers = [](std::vector<Sampler> const& src, std::vector<Sampler>& dst) {
      for (auto const& srcSampler : src) {
        bool merged = false;
        for (auto& dstSampler : dst) {
          if (srcSampler.mBinding == dstSampler.mBinding) {
            dstSampler.mActiveStages |= srcSampler.mActiveStages;
            merged = true;
            break;
          }
        }

        // this buffer is not part of the combined module yet
        if (!merged) { dst.push_back(srcSampler); }
      }
    };

    mergeBuffers(stage->mPushConstantBuffers, mPushConstantBuffers);
    mergeBuffers(stage->mUniformBuffers, mUniformBuffers);
    mergeSamplers(stage->mSamplers, mSamplers);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::toInfoString() const {
  std::stringstream sstr;

  if (!mUniformBuffers.empty()) {
    sstr << "Uniform Buffers:" << std::endl;
    for (auto const& resource : mUniformBuffers) {
      sstr << resource.toInfoString() << std::endl;
    }
  }

  if (!mPushConstantBuffers.empty()) {
    sstr << "PushConstant Buffers:" << std::endl;
    for (auto const& resource : mPushConstantBuffers) {
      sstr << resource.toInfoString() << std::endl;
    }
  }

  if (!mSamplers.empty()) {
    sstr << "Samplers:" << std::endl;
    for (auto const& resource : mSamplers) {
      sstr << resource.toInfoString() << std::endl;
    }
  }

  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::toCppString() const {
  std::stringstream sstr;

  if (!mUniformBuffers.empty()) {
    for (auto const& resource : mUniformBuffers) {
      sstr << resource.toCppString() << std::endl;
    }
  }

  if (!mPushConstantBuffers.empty()) {
    for (auto const& resource : mPushConstantBuffers) {
      sstr << resource.toCppString() << std::endl;
    }
  }

  if (!mSamplers.empty()) {
    for (auto const& resource : mSamplers) {
      sstr << resource.toCppString() << std::endl;
    }
  }

  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ShaderReflection::Buffer> const&
ShaderReflection::getBuffers(ShaderReflection::BufferType type) const {
  if (type == BufferType::ePushConstant) return mPushConstantBuffers;
  return mUniformBuffers;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t ShaderReflection::BufferRange::getBaseSize() const {
  if (mColumns > 1 && mRows > 1) return mColumns * mRows * mBaseSize;
  return mElements * mBaseSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::BufferRange::getTypePrefix() const {
  if (mElements == 1) return "";

  switch (mType) {
  case ShaderReflection::BufferRange::Type::eDouble:
    return "d";
  case ShaderReflection::BufferRange::Type::eInt:
    return "i";
  case ShaderReflection::BufferRange::Type::eUInt:
    return "u";
  default:
    return "";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::BufferRange::getElementsPostfix() const {
  if (mColumns > 1 && mRows > 1) {
    if (mColumns == mRows) return std::to_string(mColumns);
    return std::to_string(mColumns) + "x" + std::to_string(mRows);
  }

  if (mElements > 1) return std::to_string(mElements);

  return "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::BufferRange::getArrayPostfix() const {
  std::string result;

  for (int i = mArrayLengths.size() - 1; i >= 0; --i) {
    if (mArrayLengths[i] > 0) result += "[" + std::to_string(mArrayLengths[i]) + "]";
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::BufferRange::getInfoType() const {
  if (mColumns > 1 && mRows > 1) return getTypePrefix() + "mat" + getElementsPostfix();
  if (mElements > 1) return getTypePrefix() + "vec" + getElementsPostfix();

  switch (mType) {
  case Type::eInt:
    return "int";
  case Type::eUInt:
    return "uint";
  case Type::eFloat:
    return "float";
  case Type::eDouble:
    return "double";
  case Type::eStruct:
    return "struct";
  default:
    return "unknown";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::BufferRange::getCppType() const {

  // It can be necessary that the cpp is a bit larger than the spirv type when padding is required.
  // Therefore we create a copy and modify it in such a way that a padding rules are fullfilled.
  // Only modify base types. Structs need to be padded inside.
  if (mType != Type::eUnknown && mType != Type::eStruct) {

    // First modification can be neccessary when the matrix stride is larger than the row count. In
    // this case we should use the matrix stride value instead
    if (mColumns > 1 && mRows > 1 && mRows < mMatrixStride / mBaseSize) {
      BufferRange copy{*this};
      copy.mRows = copy.mMatrixStride / copy.mBaseSize;
      return copy.getCppType();
    }

    // Next modification should occur when base type array elements are smaller than the array
    // stride. In this case we should use a larger glm type to fill the padding.
    if (getBaseSize() < mArrayStride) {
      BufferRange copy{*this};

      // Matrix types should increase the column count accordingly
      if (mColumns > 1 && mRows > 1) {
        copy.mColumns = copy.mArrayStride / copy.mBaseSize / copy.mRows;
        return copy.getCppType();
      }

      // Scalar or vector types should increase the amount of elements
      copy.mElements = copy.mArrayStride / copy.mBaseSize;
      return copy.getCppType();
    }
  }

  // All required padding / stride issues should be resolved now. For matrix types return glm
  // matrices, for vector types glm vectors
  if (mColumns > 1 && mRows > 1) {
    return "glm::" + getTypePrefix() + "mat" + getElementsPostfix();
  }
  if (mElements > 1) { return "glm::" + getTypePrefix() + "vec" + getElementsPostfix(); }

  // For structs, continue recursively.

  // For base types, return the C++ equivavlent.
  switch (mType) {
  case Type::eInt:
    return "int";
  case Type::eUInt:
    return "unsigned";
  case Type::eFloat:
    return "float";
  case Type::eDouble:
    return "double";
  case Type::eStruct:
    return "struct";
  default:
    return "unknown";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Buffer::toInfoString(uint32_t indent) const {
  std::stringstream sstr;
  std::string       spaces(indent, ' ');
  sstr << spaces << " - " << mType << " " << mName << " (Stages: " << toString(mActiveStages) << ")"
       << std::endl;
  sstr << spaces << "   Size:     " << mSize << std::endl;
  sstr << spaces << "   Binding:  " << mBinding << std::endl;

  for (auto const& range : mRanges) {
    if (range.mType == BufferRange::Type::eStruct) {

    } else {
      sstr << spaces << " - " << range.getInfoType() << " " << range.mName
           << range.getArrayPostfix() << " (Stages: " << toString(range.mActiveStages) << ")"
           << std::endl;
      sstr << spaces << "     Size:         " << range.mSize << std::endl;
      sstr << spaces << "     Offset:       " << range.mOffset << std::endl;
      sstr << spaces << "     BaseBytes:    " << range.mBaseSize << std::endl;

      if (range.mArrayStride > 0)
        sstr << spaces << "     ArrayStride:  " << range.mArrayStride << std::endl;
      if (range.mMatrixStride > 0)
        sstr << spaces << "     MatrixStride: " << range.mMatrixStride << std::endl;
    }
  }

  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Buffer::toCppString(uint32_t indent) const {
  std::stringstream sstr;
  std::string       spaces(indent, ' ');
  sstr << spaces << "struct " << mType << " {" << std::endl;

  uint32_t paddingCounter{0};

  for (size_t i{0}; i < mRanges.size(); ++i) {
    sstr << spaces << "  " << mRanges[i].getCppType() << " " << mRanges[i].mName
         << mRanges[i].getArrayPostfix() << ";" << std::endl;

    uint32_t nextOffset{i < mRanges.size() - 1 ? mRanges[i + 1].mOffset : mSize};
    uint64_t requiredPadding{(nextOffset - mRanges[i].mOffset - mRanges[i].mSize) / sizeof(float)};

    while (requiredPadding > 0) {
      sstr << spaces << "  float _padding" << ++paddingCounter << ";" << std::endl;
      --requiredPadding;
    }
  }

  sstr << spaces << "};" << std::endl;
  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Sampler::toInfoString(uint32_t indent) const {
  std::stringstream sstr;
  std::string       spaces(indent, ' ');
  sstr << spaces << " - Name: " << mName << " (Stages: " << toString(mActiveStages) << ")"
       << std::endl;
  sstr << spaces << "   Binding: " << mBinding << std::endl;
  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Sampler::toCppString(uint32_t indent) const {
  std::stringstream sstr;
  std::string       spaces(indent, ' ');
  sstr << spaces << "const uint32_t binding_" << mName << " = " << mBinding << ";";
  return sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
}
