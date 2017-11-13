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
#include <vulkan/spirv_cpp.hpp>
#include <vulkan/spirv_glsl.hpp>

namespace Illusion {
namespace Graphics {
namespace {

class MyCompiler : public spirv_cross::Compiler {
 public:
  MyCompiler(std::vector<uint32_t> const& ir)
    : spirv_cross::Compiler(ir) {}
  std::vector<spirv_cross::Meta> const&    getMeta() const { return meta; }
  std::vector<spirv_cross::Variant> const& getIDs() const { return ids; }
};

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

ShaderReflection::Type::BaseType convert(spirv_cross::SPIRType type) {
  switch (type.basetype) {
  case spirv_cross::SPIRType::Int:
    return ShaderReflection::Type::BaseType::eInt;
  case spirv_cross::SPIRType::Boolean:
  case spirv_cross::SPIRType::UInt:
    return ShaderReflection::Type::BaseType::eUInt;
  case spirv_cross::SPIRType::Float:
    return ShaderReflection::Type::BaseType::eFloat;
  case spirv_cross::SPIRType::Double:
    return ShaderReflection::Type::BaseType::eDouble;
  case spirv_cross::SPIRType::Struct:
    return ShaderReflection::Type::BaseType::eStruct;
  default:
    return ShaderReflection::Type::BaseType::eUnknown;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::ShaderReflection(std::vector<uint32_t> const& code) {
  MyCompiler                   parser{code};
  spirv_cross::ShaderResources resources       = parser.get_shader_resources();
  auto                         activeVariables = parser.get_active_interface_variables();

  // spirv_cross::CompilerCPP compiler{code};
  // Illusion::ILLUSION_MESSAGE << compiler.compile() << std::endl;

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

  // for (auto& id : parser.getIDs()) {
  //   if (id.get_type() == spirv_cross::TypeType) {
  //     auto& type = id.get<spirv_cross::SPIRType>();
  //     auto& meta = parser.getMeta()[type.self];
  //     if (
  //       type.basetype == spirv_cross::SPIRType::Struct && type.array.empty() && !type.pointer &&
  //       (meta.decoration.decoration_flags &
  //        ((1ull << spv::DecorationBlock) | (1ull << spv::DecorationBufferBlock))) == 0) {

  //       Illusion::ILLUSION_MESSAGE << "-------- " << meta.decoration.alias << std::endl;
  //     }
  //   }
  // }

  // collect buffers -------------------------------------------------------------------------------
  auto getBufferRanges = [&](spirv_cross::Resource const& resource) {
    std::vector<BufferRange> result;
    auto                     type = parser.get_type(resource.type_id);
    auto                     activeMembers{parser.get_active_buffer_ranges(resource.id)};

    for (size_t i{0}; i < type.member_types.size(); ++i) {
      BufferRange range;
      auto        memberType = parser.get_type(type.member_types[i]);

      range.mName   = parser.get_member_name(resource.base_type_id, i);
      range.mSize   = parser.get_declared_struct_member_size(type, i);
      range.mOffset = parser.type_struct_member_offset(type, i);

      for (auto const& activeMember : activeMembers) {
        if (activeMember.index == i) { range.mActiveStages = mStages; }
      }

      range.mType.mBaseType = convert(memberType);
      range.mType.mBaseSize = memberType.width / 8;

      // vector types
      range.mType.mElements = memberType.vecsize;

      // matrix types
      if (parser.has_member_decoration(
            resource.base_type_id, i, spv::Decoration::DecorationMatrixStride)) {
        range.mType.mColumns      = memberType.columns;
        range.mType.mRows         = memberType.vecsize;
        range.mType.mMatrixStride = parser.type_struct_member_matrix_stride(type, i);
      }

      // array types
      if (!memberType.array.empty()) {
        range.mType.mArrayLengths = memberType.array;
        range.mType.mArrayStride  = parser.type_struct_member_array_stride(type, i);
      }

      // struct types
      if (range.mType.mBaseType == Type::BaseType::eStruct) {
        range.mType.mTypeName = parser.get_name(type.member_types[i]);
        Illusion::ILLUSION_MESSAGE << "FOOOOOOOOOO " << range.mType.mTypeName << std::endl;
        // range.mType.mRanges = getBufferRanges(structResource);
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

uint32_t ShaderReflection::Type::getBaseSize() const {
  if (mColumns > 1 && mRows > 1) return mColumns * mRows * mBaseSize;
  return mElements * mBaseSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Type::getTypePrefix() const {
  if (mElements == 1) return "";

  switch (mBaseType) {
  case BaseType::eDouble:
    return "d";
  case BaseType::eInt:
    return "i";
  case BaseType::eUInt:
    return "u";
  default:
    return "";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Type::getElementsPostfix() const {
  if (mColumns > 1 && mRows > 1) {
    if (mColumns == mRows) return std::to_string(mColumns);
    return std::to_string(mColumns) + "x" + std::to_string(mRows);
  }

  if (mElements > 1) return std::to_string(mElements);

  return "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Type::getArrayPostfix() const {
  std::string result;

  for (int i = mArrayLengths.size() - 1; i >= 0; --i) {
    if (mArrayLengths[i] > 0) result += "[" + std::to_string(mArrayLengths[i]) + "]";
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Type::getInfoType() const {
  if (mColumns > 1 && mRows > 1) return getTypePrefix() + "mat" + getElementsPostfix();
  if (mElements > 1) return getTypePrefix() + "vec" + getElementsPostfix();

  switch (mBaseType) {
  case BaseType::eInt:
    return "int";
  case BaseType::eUInt:
    return "uint";
  case BaseType::eFloat:
    return "float";
  case BaseType::eDouble:
    return "double";
  case BaseType::eStruct:
    return mTypeName;
  default:
    return "unknown";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Type::getCppType() const {

  // It can be necessary that the cpp is a bit larger than the spirv type when padding is required.
  // Therefore we create a copy and modify it in such a way that a padding rules are fullfilled.
  // Only modify base types. Structs need to be padded inside.
  if (mBaseType != BaseType::eUnknown && mBaseType != BaseType::eStruct) {

    // First modification can be neccessary when the matrix stride is larger than the row count. In
    // this case we should use the matrix stride value instead
    if (mColumns > 1 && mRows > 1 && mRows < mMatrixStride / mBaseSize) {
      Type copy{*this};
      copy.mRows = copy.mMatrixStride / copy.mBaseSize;
      return copy.getCppType();
    }

    // Next modification should occur when base type array elements are smaller than the array
    // stride. In this case we should use a larger glm type to fill the padding.
    if (getBaseSize() < mArrayStride) {
      Type copy{*this};

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
  switch (mBaseType) {
  case BaseType::eInt:
    return "int";
  case BaseType::eUInt:
    return "unsigned";
  case BaseType::eFloat:
    return "float";
  case BaseType::eDouble:
    return "double";
  case BaseType::eStruct:
    return mTypeName;
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
    // if (range.mType == BufferRange::Type::eStruct) {

    // } else {
    sstr << spaces << " - " << range.mType.getInfoType() << " " << range.mName
         << range.mType.getArrayPostfix() << " (Stages: " << toString(range.mActiveStages) << ")"
         << std::endl;
    sstr << spaces << "     Size:         " << range.mSize << std::endl;
    sstr << spaces << "     Offset:       " << range.mOffset << std::endl;

    if (range.mType.mArrayStride > 0)
      sstr << spaces << "     ArrayStride:  " << range.mType.mArrayStride << std::endl;
    if (range.mType.mMatrixStride > 0)
      sstr << spaces << "     MatrixStride: " << range.mType.mMatrixStride << std::endl;
    // }
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
    sstr << spaces << "  " << mRanges[i].mType.getCppType() << " " << mRanges[i].mName
         << mRanges[i].mType.getArrayPostfix() << ";" << std::endl;

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

std::string ShaderReflection::Struct::toInfoString(uint32_t indent) const {
  std::stringstream sstr;
  std::string       spaces(indent, ' ');
  sstr << spaces << " - struct " << mName << std::endl;
  sstr << spaces << "   Size:     " << mSize << std::endl;

  for (auto const& type : mMembers) {
    sstr << spaces << " - " << type.second.getInfoType() << " " << type.first
         << type.second.getArrayPostfix() << std::endl;

    if (type.second.mArrayStride > 0)
      sstr << spaces << "     ArrayStride:  " << type.second.mArrayStride << std::endl;
    if (type.second.mMatrixStride > 0)
      sstr << spaces << "     MatrixStride: " << type.second.mMatrixStride << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderReflection::Struct::toCppString(uint32_t indent) const {
  std::stringstream sstr;
  std::string       spaces(indent, ' ');
  sstr << spaces << "struct " << mName << " {" << std::endl;

  uint32_t paddingCounter{0};

  for (size_t i{0}; i < mRanges.size(); ++i) {
    sstr << spaces << "  " << mRanges[i].mType.getCppType() << " " << mRanges[i].mName
         << mRanges[i].mType.getArrayPostfix() << ";" << std::endl;

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
}
}
