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

#include <vulkan/spirv_glsl.hpp>

namespace Illusion {

namespace {
std::string convertToString(uint32_t stages) {
  std::string result;

  auto addStage = [&](std::string const& name, ShaderReflection::Stage stage) {
    if ((stages & static_cast<uint32_t>(stage)) > 0) {
      if (result.size() > 0) { result += " | "; }
      result += name;
    }
  };

  addStage("Vertex", ShaderReflection::Stage::Vertex);
  addStage("Fragment", ShaderReflection::Stage::Fragment);

  if (result.size() == 0) { result = "None"; }

  return result;
}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ShaderStageFlags ShaderReflection::toVk(uint32_t stages) {
  vk::ShaderStageFlags result;

  if ((stages & static_cast<uint32_t>(ShaderReflection::Stage::Vertex)) > 0) {
    result |= vk::ShaderStageFlagBits::eVertex;
  }

  if ((stages & static_cast<uint32_t>(ShaderReflection::Stage::Fragment)) > 0) {
    result |= vk::ShaderStageFlagBits::eFragment;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::ShaderStageFlagBits ShaderReflection::toVk(ShaderReflection::Stage stage) {
  switch (stage) {
  case ShaderReflection::Stage::Vertex:
    return vk::ShaderStageFlagBits::eVertex;
  case ShaderReflection::Stage::Fragment:
    return vk::ShaderStageFlagBits::eFragment;
  default:
    return vk::ShaderStageFlagBits();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::ShaderReflection(std::string const& name, std::vector<uint32_t> const& code)
  : mName(name) {
  spirv_cross::Compiler        parser(code);
  spirv_cross::ShaderResources resources       = parser.get_shader_resources();
  auto                         activeVariables = parser.get_active_interface_variables();

  // collect basic information ---------------------------------------------------------------------
  auto stage = parser.get_execution_model();

  switch (stage) {
  case spv::ExecutionModelVertex:
    mStages              = static_cast<uint32_t>(Stage::Vertex);
    mCode[Stage::Vertex] = code;
    break;
  case spv::ExecutionModelFragment:
    mStages                = static_cast<uint32_t>(Stage::Fragment);
    mCode[Stage::Fragment] = code;
    break;
  default:
    ILLUSION_WARNING << "Shader stage of module " << mName << " is not supported!" << std::endl;
    break;
  }

  auto collectBuffers =
    [&](std::vector<spirv_cross::Resource> const& src, std::vector<Buffer>& dst) {
      for (auto const& resource : src) {

        auto type = parser.get_type(resource.base_type_id);

        Buffer buffer;
        buffer.mName    = resource.name;
        buffer.mBinding = parser.get_decoration(resource.id, spv::DecorationBinding);
        buffer.mSize    = static_cast<uint32_t>(parser.get_declared_struct_size(type));

        if (activeVariables.find(resource.id) != activeVariables.end()) {
          buffer.mActiveStages = mStages;
        }

        if (buffer.mName == "model")
          Illusion::ILLUSION_MESSAGE << parser.get_active_buffer_ranges(resource.id).size() << std::endl;

        for (auto const& activeRange : parser.get_active_buffer_ranges(resource.id)) {
          buffer.mMembers[static_cast<uint32_t>(activeRange.offset)].mSize =
            static_cast<uint32_t>(activeRange.range);
          buffer.mMembers[static_cast<uint32_t>(activeRange.offset)].mActiveStages = mStages;
        }

        dst.push_back(buffer);
      }
    };

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

  // collect push constant ranges ------------------------------------------------------------------
  collectBuffers(resources.push_constant_buffers, mPushConstantBuffers);

  // collect uniform buffers -----------------------------------------------------------------------
  collectBuffers(resources.uniform_buffers, mUniformBuffers);

  // collect image samplers
  collectSamplers(resources.sampled_images, mSamplers);

  // warn if not supported features are used -------------------------------------------------------
  auto printNotSupported =
    [this](std::string const& name, std::vector<spirv_cross::Resource> const& resources) {
      if (resources.size() > 0) {
        ILLUSION_WARNING << "Shader module " << mName << " constains " << name
                         << " which are not supported by the Material yet!" << std::endl;
      }
    };

  printNotSupported("atomic counters", resources.atomic_counters);
  printNotSupported("separate images", resources.separate_images);
  printNotSupported("separate samplers", resources.separate_samplers);
  printNotSupported("storage buffers", resources.storage_buffers);
  printNotSupported("storage images", resources.storage_images);
  printNotSupported("subpass inputs", resources.subpass_inputs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderReflection::ShaderReflection(std::vector<ShaderReflectionPtr> const& stages) {
  for (auto const& stage : stages) {

    // check that we do not have such a stage already - this should not happen!
    if ((mStages & stage->mStages) > 0) {
      ILLUSION_WARNING << convertToString(stage->mStages) << " shader stage is already part of "
                       << "the ShaderReflection!" << std::endl;
    }

    // concatenate code
    for (auto const& code : stage->mCode) {
      mCode[code.first] = code.second;
    }

    // concatenate name and stage
    if (mName.size() > 0) { mName += " | "; }

    mName += stage->mName;
    mStages |= stage->mStages;

    // combine buffers
    auto mergeBuffers = [this](std::vector<Buffer> const& src, std::vector<Buffer>& dst) {
      for (auto const& srcBuffer : src) {
        bool merged = false;
        for (auto& dstBuffer : dst) {
          if (srcBuffer.mBinding == dstBuffer.mBinding) {

            dstBuffer.mActiveStages |= srcBuffer.mActiveStages;

            for (auto const& src : srcBuffer.mMembers) {
              auto dst = dstBuffer.mMembers.find(src.first);
              if (dst == dstBuffer.mMembers.end()) {
                dstBuffer.mMembers.insert(src);
              } else {
                if (src.second.mSize == dst->second.mSize) {
                  dst->second.mActiveStages |= src.second.mActiveStages;
                } else {
                  ILLUSION_WARNING << "Failed to merge shader stages " << mName << "! Offsets and "
                                   << "sizes for buffer at binding " << srcBuffer.mBinding
                                   << " do not match." << std::endl;
                }
              }
            }

            merged = true;
            break;
          }
        }

        // this buffer is not part of the combined module yet
        if (!merged) dst.push_back(srcBuffer);
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
  ILLUSION_MESSAGE << Logger::PRINT_BOLD << "Reflection information for shader " << mName
                   << Logger::PRINT_RESET << std::endl;

  ILLUSION_MESSAGE << " Stage: " << convertToString(mStages) << std::endl;

  auto printBuffers = [](std::string const& name, std::vector<Buffer> const& buffers) {
    if (buffers.size() > 0) {
      ILLUSION_MESSAGE << " " << name << ":" << std::endl;
      for (auto const& buffer : buffers) {
        ILLUSION_MESSAGE << " - Name: " << buffer.mName << std::endl;
        ILLUSION_MESSAGE << "   Size: " << buffer.mSize << std::endl;
        ILLUSION_MESSAGE << "   Binding: " << buffer.mBinding << std::endl;
        ILLUSION_MESSAGE << "   Active in: " << convertToString(buffer.mActiveStages) << std::endl;
        ILLUSION_MESSAGE << "   Members: " << std::endl;

        for (auto member : buffer.mMembers) {
          ILLUSION_MESSAGE << "   - Name: " << member.second.mName << std::endl;
          ILLUSION_MESSAGE << "     Offset: " << member.first << std::endl;
          ILLUSION_MESSAGE << "     Size: " << member.second.mSize << std::endl;
          ILLUSION_MESSAGE << "     Active in: " << convertToString(member.second.mActiveStages)
                           << std::endl;
        }
      }
    }
  };

  auto printSamplers = [](std::string const& name, std::vector<Sampler> const& samplers) {
    if (samplers.size() > 0) {
      ILLUSION_MESSAGE << " " << name << ":" << std::endl;
      for (auto const& sampler : samplers) {
        ILLUSION_MESSAGE << " - Name: " << sampler.mName << std::endl;
        ILLUSION_MESSAGE << "   Binding: " << sampler.mBinding << std::endl;
        ILLUSION_MESSAGE << "   Active in: " << convertToString(sampler.mActiveStages) << std::endl;
      }
    }
  };

  printBuffers("Push Constant Buffers", mPushConstantBuffers);
  printBuffers("Uniform Buffers", mUniformBuffers);
  printSamplers("Samplers", mSamplers);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string const& ShaderReflection::getName() const { return mName; }

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t ShaderReflection::getStages() const { return mStages; }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<ShaderReflection::Stage, std::vector<uint32_t>> const&
ShaderReflection::getCode() const {
  return mCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ShaderReflection::Buffer> const& ShaderReflection::getPushConstantBuffers() const {
  return mPushConstantBuffers;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ShaderReflection::Buffer> const& ShaderReflection::getUniformBuffers() const {
  return mUniformBuffers;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ShaderReflection::Sampler> const& ShaderReflection::getSamplers() const {
  return mSamplers;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
