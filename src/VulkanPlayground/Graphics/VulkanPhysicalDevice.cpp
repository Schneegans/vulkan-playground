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
#include "VulkanPhysicalDevice.hpp"
#include "../Utils/Logger.hpp"
#include "../Utils/stl_helpers.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

void printCap(std::string const& name, vk::Bool32 cap) {
  ILLUSION_DEBUG << std::left << std::setw(50) << std::setfill('.') << (name + " ")
                 << (cap ? Logger::PRINT_GREEN + " yes" : Logger::PRINT_RED + " no")
                 << Logger::PRINT_RESET << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void printVal(std::string const& name, std::vector<std::string> const& vals) {
  std::stringstream sstr;

  for (size_t i(0); i < vals.size(); ++i) {
    sstr << vals[i];
    if (i < vals.size() - 1) { sstr << " | "; }
  }

  ILLUSION_DEBUG << std::left << std::setw(50) << std::setfill('.') << (name + " ") << " "
                 << sstr.str() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename S, typename T>
std::string printMin(S val, T ref) {
  std::string color = Logger::PRINT_RED;

  if (val == ref)
    color = Logger::PRINT_YELLOW;
  else if (val > ref)
    color = Logger::PRINT_GREEN;

  return color + toString(val) + Logger::PRINT_RESET + " (" + toString(ref) + ")";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename S, typename T>
std::string printMax(S val, T ref) {
  std::string color = Logger::PRINT_RED;

  if (val == ref)
    color = Logger::PRINT_YELLOW;
  else if (val < ref)
    color = Logger::PRINT_GREEN;

  return color + toString(val) + Logger::PRINT_RESET + " (" + toString(ref) + ")";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanPhysicalDevice::VulkanPhysicalDevice(vk::PhysicalDevice const& device)
  : vk::PhysicalDevice(device) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDevicePtr VulkanPhysicalDevice::createDevice(vk::DeviceCreateInfo const& info) {
  ILLUSION_DEBUG << "Creating device." << std::endl;
  return createManagedObject(vk::PhysicalDevice::createDevice(info), [](vk::Device* obj) {
    ILLUSION_DEBUG << "Deleting device." << std::endl;
    obj->destroy();
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t VulkanPhysicalDevice::findMemoryType(
  uint32_t typeFilter, vk::MemoryPropertyFlags properties) const {
  auto memProperties = getMemoryProperties();

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if (
      (typeFilter & (1 << i)) &&
      (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  ILLUSION_ERROR << "Failed to find suitable memory type." << std::endl;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanPhysicalDevice::printInfo() {
  // basic information
  vk::PhysicalDeviceProperties properties = getProperties();
  ILLUSION_DEBUG << Logger::PRINT_BOLD << "Physical Device Information " << Logger::PRINT_RESET
                 << std::endl;
  printVal("apiVersion", {toString(properties.apiVersion)});
  printVal("driverVersion", {toString(properties.driverVersion)});
  printVal("vendorID", {toString(properties.vendorID)});
  printVal("deviceID", {toString(properties.deviceID)});
  printVal("deviceType", {vk::to_string(properties.deviceType)});
  printVal("deviceName", {properties.deviceName});

  // memory information
  vk::PhysicalDeviceMemoryProperties memoryProperties = getMemoryProperties();
  ILLUSION_DEBUG << Logger::PRINT_BOLD << "Memory Information " << Logger::PRINT_RESET << std::endl;
  for (unsigned i(0); i < memoryProperties.memoryTypeCount; ++i) {
    printVal(
      "Memory type " + toString(i), {vk::to_string(memoryProperties.memoryTypes[i].propertyFlags)});
  }

  for (unsigned i(0); i < memoryProperties.memoryHeapCount; ++i) {
    printVal(
      "Memory heap " + toString(i),
      {toString(memoryProperties.memoryHeaps[i].size / (1024 * 1024)) + " MB " +
       vk::to_string(memoryProperties.memoryHeaps[i].flags)});
  }

  // features
  vk::PhysicalDeviceFeatures features = getFeatures();
  ILLUSION_DEBUG << Logger::PRINT_BOLD << "Features " << Logger::PRINT_RESET << std::endl;
  printCap("robustBufferAccess", features.robustBufferAccess);
  printCap("fullDrawIndexUint32", features.fullDrawIndexUint32);
  printCap("imageCubeArray", features.imageCubeArray);
  printCap("independentBlend", features.independentBlend);
  printCap("geometryShader", features.geometryShader);
  printCap("tessellationShader", features.tessellationShader);
  printCap("sampleRateShading", features.sampleRateShading);
  printCap("dualSrcBlend", features.dualSrcBlend);
  printCap("logicOp", features.logicOp);
  printCap("multiDrawIndirect", features.multiDrawIndirect);
  printCap("drawIndirectFirstInstance", features.drawIndirectFirstInstance);
  printCap("depthClamp", features.depthClamp);
  printCap("depthBiasClamp", features.depthBiasClamp);
  printCap("fillModeNonSolid", features.fillModeNonSolid);
  printCap("depthBounds", features.depthBounds);
  printCap("wideLines", features.wideLines);
  printCap("largePoints", features.largePoints);
  printCap("alphaToOne", features.alphaToOne);
  printCap("multiViewport", features.multiViewport);
  printCap("samplerAnisotropy", features.samplerAnisotropy);
  printCap("textureCompressionETC2", features.textureCompressionETC2);
  printCap("textureCompressionASTC_LDR", features.textureCompressionASTC_LDR);
  printCap("textureCompressionBC", features.textureCompressionBC);
  printCap("occlusionQueryPrecise", features.occlusionQueryPrecise);
  printCap("pipelineStatisticsQuery", features.pipelineStatisticsQuery);
  printCap("vertexPipelineStoresAndAtomics", features.vertexPipelineStoresAndAtomics);
  printCap("fragmentStoresAndAtomics", features.fragmentStoresAndAtomics);
  printCap(
    "shaderTessellationAndGeometryPointSize", features.shaderTessellationAndGeometryPointSize);
  printCap("shaderImageGatherExtended", features.shaderImageGatherExtended);
  printCap("shaderStorageImageExtendedFormats", features.shaderStorageImageExtendedFormats);
  printCap("shaderStorageImageMultisample", features.shaderStorageImageMultisample);
  printCap("shaderStorageImageReadWithoutFormat", features.shaderStorageImageReadWithoutFormat);
  printCap("shaderStorageImageWriteWithoutFormat", features.shaderStorageImageWriteWithoutFormat);
  printCap(
    "shaderUniformBufferArrayDynamicIndexing", features.shaderUniformBufferArrayDynamicIndexing);
  printCap(
    "shaderSampledImageArrayDynamicIndexing", features.shaderSampledImageArrayDynamicIndexing);
  printCap(
    "shaderStorageBufferArrayDynamicIndexing", features.shaderStorageBufferArrayDynamicIndexing);
  printCap(
    "shaderStorageImageArrayDynamicIndexing", features.shaderStorageImageArrayDynamicIndexing);
  printCap("shaderClipDistance", features.shaderClipDistance);
  printCap("shaderCullDistance", features.shaderCullDistance);
  printCap("shaderFloat64", features.shaderFloat64);
  printCap("shaderInt64", features.shaderInt64);
  printCap("shaderInt16", features.shaderInt16);
  printCap("shaderResourceResidency", features.shaderResourceResidency);
  printCap("shaderResourceMinLod", features.shaderResourceMinLod);
  printCap("sparseBinding", features.sparseBinding);
  printCap("sparseResidencyBuffer", features.sparseResidencyBuffer);
  printCap("sparseResidencyImage2D", features.sparseResidencyImage2D);
  printCap("sparseResidencyImage3D", features.sparseResidencyImage3D);
  printCap("sparseResidency2Samples", features.sparseResidency2Samples);
  printCap("sparseResidency4Samples", features.sparseResidency4Samples);
  printCap("sparseResidency8Samples", features.sparseResidency8Samples);
  printCap("sparseResidency16Samples", features.sparseResidency16Samples);
  printCap("sparseResidencyAliased", features.sparseResidencyAliased);
  printCap("variableMultisampleRate", features.variableMultisampleRate);
  printCap("inheritedQueries", features.inheritedQueries);

  // limits
  vk::PhysicalDeviceLimits limits = properties.limits;
  ILLUSION_DEBUG << Logger::PRINT_BOLD << "Limits " << Logger::PRINT_RESET << std::endl;
  printVal("maxImageDimension1D", {printMin(limits.maxImageDimension1D, 4096u)});
  printVal("maxImageDimension2D", {printMin(limits.maxImageDimension2D, 4096u)});
  printVal("maxImageDimension3D", {printMin(limits.maxImageDimension3D, 256u)});
  printVal("maxImageDimensionCube", {printMin(limits.maxImageDimensionCube, 4096u)});
  printVal("maxImageArrayLayers", {printMin(limits.maxImageArrayLayers, 256u)});
  printVal("maxTexelBufferElements", {printMin(limits.maxTexelBufferElements, 65536u)});
  printVal("maxUniformBufferRange", {printMin(limits.maxUniformBufferRange, 16384u)});
  printVal("maxStorageBufferRange", {printMin(limits.maxStorageBufferRange, std::pow(2, 27))});
  printVal("maxPushConstantsSize", {printMin(limits.maxPushConstantsSize, 128u)});
  printVal("maxMemoryAllocationCount", {printMin(limits.maxMemoryAllocationCount, 4096u)});
  printVal("maxSamplerAllocationCount", {printMin(limits.maxSamplerAllocationCount, 4000u)});
  printVal("bufferImageGranularity", {printMax(limits.bufferImageGranularity, 131072u)});
  printVal("sparseAddressSpaceSize", {printMin(limits.sparseAddressSpaceSize, std::pow(2, 31))});
  printVal("maxBoundDescriptorSets", {printMin(limits.maxBoundDescriptorSets, 4u)});
  printVal("maxPerStageDescriptorSamplers", {printMin(limits.maxPerStageDescriptorSamplers, 16u)});
  printVal(
    "maxPerStageDescriptorUniformBuffers",
    {printMin(limits.maxPerStageDescriptorUniformBuffers, 12u)});
  printVal(
    "maxPerStageDescriptorStorageBuffers",
    {printMin(limits.maxPerStageDescriptorStorageBuffers, 4u)});
  printVal(
    "maxPerStageDescriptorSampledImages",
    {printMin(limits.maxPerStageDescriptorSampledImages, 16u)});
  printVal(
    "maxPerStageDescriptorStorageImages",
    {printMin(limits.maxPerStageDescriptorStorageImages, 4u)});
  printVal(
    "maxPerStageDescriptorInputAttachments",
    {printMin(limits.maxPerStageDescriptorInputAttachments, 4u)});
  printVal("maxPerStageResources", {printMin(limits.maxPerStageResources, 128u)});
  printVal("maxDescriptorSetSamplers", {printMin(limits.maxDescriptorSetSamplers, 96u)});
  printVal(
    "maxDescriptorSetUniformBuffers", {printMin(limits.maxDescriptorSetUniformBuffers, 72u)});
  printVal(
    "maxDescriptorSetUniformBuffersDynamic",
    {printMin(limits.maxDescriptorSetUniformBuffersDynamic, 8u)});
  printVal(
    "maxDescriptorSetStorageBuffers", {printMin(limits.maxDescriptorSetStorageBuffers, 24u)});
  printVal(
    "maxDescriptorSetStorageBuffersDynamic",
    {printMin(limits.maxDescriptorSetStorageBuffersDynamic, 4u)});
  printVal("maxDescriptorSetSampledImages", {printMin(limits.maxDescriptorSetSampledImages, 96u)});
  printVal("maxDescriptorSetStorageImages", {printMin(limits.maxDescriptorSetStorageImages, 24u)});
  printVal(
    "maxDescriptorSetInputAttachments", {printMin(limits.maxDescriptorSetInputAttachments, 4u)});
  printVal("maxVertexInputAttributes", {printMin(limits.maxVertexInputAttributes, 16u)});
  printVal("maxVertexInputBindings", {printMin(limits.maxVertexInputBindings, 16u)});
  printVal(
    "maxVertexInputAttributeOffset", {printMin(limits.maxVertexInputAttributeOffset, 2047u)});
  printVal("maxVertexInputBindingStride", {printMin(limits.maxVertexInputBindingStride, 2048u)});
  printVal("maxVertexOutputComponents", {printMin(limits.maxVertexOutputComponents, 64u)});
  printVal(
    "maxTessellationGenerationLevel", {printMin(limits.maxTessellationGenerationLevel, 64u)});
  printVal("maxTessellationPatchSize", {printMin(limits.maxTessellationPatchSize, 32u)});
  printVal(
    "maxTessellationControlPerVertexInputComponents",
    {printMin(limits.maxTessellationControlPerVertexInputComponents, 64u)});
  printVal(
    "maxTessellationControlPerVertexOutputComponents",
    {printMin(limits.maxTessellationControlPerVertexOutputComponents, 64u)});
  printVal(
    "maxTessellationControlPerPatchOutputComponents",
    {printMin(limits.maxTessellationControlPerPatchOutputComponents, 120u)});
  printVal(
    "maxTessellationControlTotalOutputComponents",
    {printMin(limits.maxTessellationControlTotalOutputComponents, 2048u)});
  printVal(
    "maxTessellationEvaluationInputComponents",
    {printMin(limits.maxTessellationEvaluationInputComponents, 64u)});
  printVal(
    "maxTessellationEvaluationOutputComponents",
    {printMin(limits.maxTessellationEvaluationOutputComponents, 64u)});
  printVal("maxGeometryShaderInvocations", {printMin(limits.maxGeometryShaderInvocations, 32u)});
  printVal("maxGeometryInputComponents", {printMin(limits.maxGeometryInputComponents, 64u)});
  printVal("maxGeometryOutputComponents", {printMin(limits.maxGeometryOutputComponents, 64u)});
  printVal("maxGeometryOutputVertices", {printMin(limits.maxGeometryOutputVertices, 256u)});
  printVal(
    "maxGeometryTotalOutputComponents", {printMin(limits.maxGeometryTotalOutputComponents, 1024u)});
  printVal("maxFragmentInputComponents", {printMin(limits.maxFragmentInputComponents, 64u)});
  printVal("maxFragmentOutputAttachments", {printMin(limits.maxFragmentOutputAttachments, 4u)});
  printVal("maxFragmentDualSrcAttachments", {printMin(limits.maxFragmentDualSrcAttachments, 1u)});
  printVal(
    "maxFragmentCombinedOutputResources",
    {printMin(limits.maxFragmentCombinedOutputResources, 4u)});
  printVal("maxComputeSharedMemorySize", {printMin(limits.maxComputeSharedMemorySize, 16384u)});
  printVal(
    "maxComputeWorkGroupCount",
    {printMin(limits.maxComputeWorkGroupCount[0], 65535u),
     printMin(limits.maxComputeWorkGroupCount[1], 65535u),
     printMin(limits.maxComputeWorkGroupCount[2], 65535u)});
  printVal(
    "maxComputeWorkGroupInvocations", {printMin(limits.maxComputeWorkGroupInvocations, 128u)});
  printVal(
    "maxComputeWorkGroupSize",
    {printMin(limits.maxComputeWorkGroupSize[0], 128u),
     printMin(limits.maxComputeWorkGroupSize[1], 128u),
     printMin(limits.maxComputeWorkGroupSize[2], 64u)});
  printVal("subPixelPrecisionBits", {printMin(limits.subPixelPrecisionBits, 4u)});
  printVal("subTexelPrecisionBits", {printMin(limits.subTexelPrecisionBits, 4u)});
  printVal("mipmapPrecisionBits", {printMin(limits.mipmapPrecisionBits, 4u)});
  printVal(
    "maxDrawIndexedIndexValue", {printMin(limits.maxDrawIndexedIndexValue, std::pow(2, 32) - 1)});
  printVal("maxDrawIndirectCount", {printMin(limits.maxDrawIndirectCount, std::pow(2, 16) - 1)});
  printVal("maxSamplerLodBias", {printMin(limits.maxSamplerLodBias, 2)});
  printVal("maxSamplerAnisotropy", {printMin(limits.maxSamplerAnisotropy, 16)});
  printVal("maxViewports", {printMin(limits.maxViewports, 16u)});
  printVal(
    "maxViewportDimensions",
    {printMin(limits.maxViewportDimensions[0], 4096u),
     printMin(limits.maxViewportDimensions[1], 4096u)});
  printVal(
    "viewportBoundsRange",
    {printMax(limits.viewportBoundsRange[0], -8192),
     printMin(limits.viewportBoundsRange[1], 8191)});
  printVal("viewportSubPixelBits", {printMin(limits.viewportSubPixelBits, 0u)});
  printVal("minMemoryMapAlignment", {printMin(limits.minMemoryMapAlignment, 64u)});
  printVal("minTexelBufferOffsetAlignment", {printMax(limits.minTexelBufferOffsetAlignment, 256u)});
  printVal(
    "minUniformBufferOffsetAlignment", {printMax(limits.minUniformBufferOffsetAlignment, 256u)});
  printVal(
    "minStorageBufferOffsetAlignment", {printMax(limits.minStorageBufferOffsetAlignment, 256u)});
  printVal("minTexelOffset", {printMax(limits.minTexelOffset, -8)});
  printVal("maxTexelOffset", {printMin(limits.maxTexelOffset, 7u)});
  printVal("minTexelGatherOffset", {printMax(limits.minTexelGatherOffset, -8)});
  printVal("maxTexelGatherOffset", {printMin(limits.maxTexelGatherOffset, 7u)});
  printVal("minInterpolationOffset", {printMax(limits.minInterpolationOffset, 0.5)});
  printVal(
    "maxInterpolationOffset",
    {printMin(
      limits.maxInterpolationOffset, 0.5 - std::pow(0.5, limits.subPixelInterpolationOffsetBits))});
  printVal(
    "subPixelInterpolationOffsetBits", {printMin(limits.subPixelInterpolationOffsetBits, 4u)});
  printVal("maxFramebufferWidth", {printMin(limits.maxFramebufferWidth, 4096u)});
  printVal("maxFramebufferHeight", {printMin(limits.maxFramebufferHeight, 4096u)});
  printVal("maxFramebufferLayers", {printMin(limits.maxFramebufferLayers, 256u)});
  printVal(
    "framebufferColorSampleCounts",
    {vk::to_string(limits.framebufferColorSampleCounts) + " ({1 | 4})"});
  printVal(
    "framebufferDepthSampleCounts",
    {vk::to_string(limits.framebufferDepthSampleCounts) + " ({1 | 4})"});
  printVal(
    "framebufferStencilSampleCounts",
    {vk::to_string(limits.framebufferStencilSampleCounts) + " ({1 | 4})"});
  printVal(
    "framebufferNoAttachmentsSampleCounts",
    {vk::to_string(limits.framebufferNoAttachmentsSampleCounts) + " ({1 | 4})"});
  printVal("maxColorAttachments", {printMin(limits.maxColorAttachments, 4u)});
  printVal(
    "sampledImageColorSampleCounts",
    {vk::to_string(limits.sampledImageColorSampleCounts) + " ({1 | 4})"});
  printVal(
    "sampledImageIntegerSampleCounts",
    {vk::to_string(limits.sampledImageIntegerSampleCounts) + " ({1})"});
  printVal(
    "sampledImageDepthSampleCounts",
    {vk::to_string(limits.sampledImageDepthSampleCounts) + " ({1 | 4})"});
  printVal(
    "sampledImageStencilSampleCounts",
    {vk::to_string(limits.sampledImageStencilSampleCounts) + " ({1 | 4})"});
  printVal(
    "storageImageSampleCounts", {vk::to_string(limits.storageImageSampleCounts) + " ({1 | 4})"});
  printVal("maxSampleMaskWords", {printMin(limits.maxSampleMaskWords, 1u)});
  printVal("timestampComputeAndGraphics", {toString(limits.timestampComputeAndGraphics)});
  printVal("timestampPeriod", {toString(limits.timestampPeriod)});
  printVal("maxClipDistances", {printMin(limits.maxClipDistances, 8u)});
  printVal("maxCullDistances", {printMin(limits.maxCullDistances, 8u)});
  printVal(
    "maxCombinedClipAndCullDistances", {printMin(limits.maxCombinedClipAndCullDistances, 8u)});
  printVal("discreteQueuePriorities", {printMin(limits.discreteQueuePriorities, 2u)});
  printVal(
    "pointSizeRange",
    {printMax(limits.pointSizeRange[0], 1.0),
     printMin(limits.pointSizeRange[1], 64.0 - limits.pointSizeGranularity)});
  printVal(
    "lineWidthRange",
    {printMax(limits.lineWidthRange[0], 1.0),
     printMin(limits.lineWidthRange[1], 8.0 - limits.lineWidthGranularity)});
  printVal("pointSizeGranularity", {printMax(limits.pointSizeGranularity, 1)});
  printVal("lineWidthGranularity", {printMax(limits.lineWidthGranularity, 1)});
  printVal("strictLines", {toString(limits.strictLines)});
  printVal("standardSampleLocations", {toString(limits.standardSampleLocations)});
  printVal("optimalBufferCopyOffsetAlignment", {toString(limits.optimalBufferCopyOffsetAlignment)});
  printVal(
    "optimalBufferCopyRowPitchAlignment", {toString(limits.optimalBufferCopyRowPitchAlignment)});
  printVal("nonCoherentAtomSize", {printMax(limits.nonCoherentAtomSize, 256u)});
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
