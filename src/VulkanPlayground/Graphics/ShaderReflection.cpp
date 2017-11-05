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

std::string toString(spirv_cross::SPIRType type) {
  std::string result;

  switch (type.basetype) {
  case spirv_cross::SPIRType::Unknown:
    result = "unknown";
    break;
  case spirv_cross::SPIRType::Void:
    result = "void";
    break;
  case spirv_cross::SPIRType::Boolean:
    result = "boolean";
    break;
  case spirv_cross::SPIRType::Char:
    result = "char";
    break;
  case spirv_cross::SPIRType::Int:
    result = "int";
    break;
  case spirv_cross::SPIRType::UInt:
    result = "uint";
    break;
  case spirv_cross::SPIRType::Int64:
    result = "int64";
    break;
  case spirv_cross::SPIRType::UInt64:
    result = "uint64";
    break;
  case spirv_cross::SPIRType::AtomicCounter:
    result = "atomiccounter";
    break;
  case spirv_cross::SPIRType::Float:
    result = "float";
    break;
  case spirv_cross::SPIRType::Double:
    result = "double";
    break;
  case spirv_cross::SPIRType::Struct:
    result = "struct";
    break;
  case spirv_cross::SPIRType::Image:
    result = "image";
    break;
  case spirv_cross::SPIRType::SampledImage:
    result = "sampledimage";
    break;
  case spirv_cross::SPIRType::Sampler:
    result = "sampler";
    break;
  }

  if (type.vecsize > 1) {
    if (type.columns == 1)
      result = "vec" + std::to_string(type.vecsize);
    else if (type.vecsize == type.columns)
      result = "mat" + std::to_string(type.vecsize);
    else
      result = "mat" + std::to_string(type.columns) + "x" + std::to_string(type.vecsize);
  }

  return result;
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
  auto collectBuffers =
    [&](std::vector<spirv_cross::Resource> const& src, std::vector<Buffer>& dst) {
      for (auto const& resource : src) {

        Buffer buffer;
        auto   type     = parser.get_type(resource.type_id);
        buffer.mName    = parser.get_name(resource.id);
        buffer.mType    = parser.get_name(resource.base_type_id);
        buffer.mBinding = parser.get_decoration(resource.id, spv::DecorationBinding);
        buffer.mSize    = parser.get_declared_struct_size(type);
        if (activeVariables.find(resource.id) != activeVariables.end()) {
          buffer.mActiveStages = mStages;
        }

        auto activeMembers{parser.get_active_buffer_ranges(resource.id)};

        for (size_t i{0}; i < type.member_types.size(); ++i) {
          BufferRange range;
          auto        memberType = parser.get_type(type.member_types[i]);
          range.mName            = parser.get_member_name(resource.base_type_id, i);
          range.mType            = toString(memberType);
          range.mOffset          = parser.type_struct_member_offset(type, i);
          range.mSize            = parser.get_declared_struct_member_size(type, i);

          for (auto const& activeMember : activeMembers) {
            if (activeMember.index == i) { range.mActiveStages = mStages; }
          }

          buffer.mRanges.push_back(range);
        }

        dst.push_back(buffer);
      }
    };

  collectBuffers(resources.push_constant_buffers, mPushConstantBuffers);
  collectBuffers(resources.uniform_buffers, mUniformBuffers);

  // collect image samplers ------------------------------------------------------------------------
  auto collectSamplers =
    [&](std::vector<spirv_cross::Resource> const& src, std::vector<Sampler>& dst) {
      for (auto const& resource : src) {
        Sampler sampler;
        sampler.mName    = resource.name;
        sampler.mBinding = parser.get_decoration(resource.id, spv::DecorationBinding);

        if (activeVariables.find(resource.id) != activeVariables.end()) {
          sampler.mActiveStages = mStages;
        }

        dst.push_back(sampler);
      }
    };

  collectSamplers(resources.sampled_images, mSamplers);

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

void ShaderReflection::print() const {
  ILLUSION_MESSAGE << " Stage(s): " << toString(mStages) << std::endl;

  auto printBuffers = [](std::string const& name, std::vector<Buffer> const& buffers) {
    if (buffers.size() > 0) {
      ILLUSION_MESSAGE << std::endl;
      ILLUSION_MESSAGE << " " << name << ":" << std::endl;
      for (auto const& buffer : buffers) {
        ILLUSION_MESSAGE << Logger::PRINT_BOLD << " - " << buffer.mType << " " << buffer.mName
                         << Logger::PRINT_RESET << " (Stages: " << toString(buffer.mActiveStages)
                         << ")" << std::endl;
        ILLUSION_MESSAGE << "   Size:     " << buffer.mSize << std::endl;
        ILLUSION_MESSAGE << "   Binding:  " << buffer.mBinding << std::endl;

        for (auto range : buffer.mRanges) {
          ILLUSION_MESSAGE << Logger::PRINT_BOLD << "   - " << range.mType << " " << range.mName
                           << Logger::PRINT_RESET << " (Stages: " << toString(range.mActiveStages)
                           << ")" << std::endl;
          ILLUSION_MESSAGE << "     Size:   " << range.mSize << std::endl;
          ILLUSION_MESSAGE << "     Offset: " << range.mOffset << std::endl;
        }
      }
    }
  };

  auto printSamplers = [](std::string const& name, std::vector<Sampler> const& samplers) {
    if (samplers.size() > 0) {
      ILLUSION_MESSAGE << std::endl;
      ILLUSION_MESSAGE << " " << name << ":" << std::endl;
      for (auto const& sampler : samplers) {
        ILLUSION_MESSAGE << Logger::PRINT_BOLD << " - Name: " << sampler.mName
                         << Logger::PRINT_RESET << " (Stages: " << toString(sampler.mActiveStages)
                         << ")" << std::endl;
        ILLUSION_MESSAGE << "   Binding: " << sampler.mBinding << std::endl;
      }
    }
  };

  printBuffers("Push Constant Buffers", mPushConstantBuffers);
  printBuffers("Uniform Buffers", mUniformBuffers);
  printSamplers("Samplers", mSamplers);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
}
