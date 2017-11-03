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
#include "VulkanSwapChain.hpp"

#include "../Utils/Logger.hpp"
#include "../Utils/ScopedTimer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanInstance.hpp"
#include "VulkanPhysicalDevice.hpp"

#include <iostream>
#include <set>
#include <sstream>
#include <thread>

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanSwapChain::VulkanSwapChain(VulkanDevicePtr const& device, VkSurfaceKHRPtr const& surface)
  : mDevice(device)
  , mSurface(surface) {

  createSwapChain();
  createRenderPass();
  createFramebuffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSwapChain::createSwapChain() {
  // delete old swap chain first
  mSwapChain.reset();

  auto capabilities =
    mDevice->getInstance()->getPhysicalDevice()->getSurfaceCapabilitiesKHR(*mSurface);
  auto formats = mDevice->getInstance()->getPhysicalDevice()->getSurfaceFormatsKHR(*mSurface);
  auto presentModes =
    mDevice->getInstance()->getPhysicalDevice()->getSurfacePresentModesKHR(*mSurface);

  mExtent                            = chooseExtent(capabilities);
  vk::PresentModeKHR   presentMode   = choosePresentMode(presentModes);
  vk::SurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(formats);
  mImageFormat                       = surfaceFormat.format;

  mImageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && mImageCount > capabilities.maxImageCount) {
    mImageCount = capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR info;
  info.surface          = *mSurface;
  info.minImageCount    = mImageCount;
  info.imageFormat      = surfaceFormat.format;
  info.imageColorSpace  = surfaceFormat.colorSpace;
  info.imageExtent      = mExtent;
  info.imageArrayLayers = 1;
  info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
  info.preTransform     = capabilities.currentTransform;
  info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  info.presentMode      = presentMode;
  info.clipped          = true;
  info.oldSwapchain     = mSwapChain ? *mSwapChain : nullptr;

  uint32_t graphicsFamily = mDevice->getInstance()->getGraphicsFamily();
  uint32_t presentFamily  = mDevice->getInstance()->getPresentFamily();

  // this check should not be neccessary, but the validation layers complain
  // when only glfwGetPhysicalDevicePresentationSupport was used to check for
  // presentation support
  if (
    !mDevice->getInstance()->getPhysicalDevice()->getSurfaceSupportKHR(presentFamily, *mSurface)) {
    ILLUSION_ERROR << "The selected queue family does not "
                   << "support presentation!" << std::endl;
  }

  if (graphicsFamily != presentFamily) {
    uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};
    info.imageSharingMode         = vk::SharingMode::eConcurrent;
    info.queueFamilyIndexCount    = 2;
    info.pQueueFamilyIndices      = queueFamilyIndices;
  } else {
    info.imageSharingMode      = vk::SharingMode::eExclusive;
    info.queueFamilyIndexCount = 0;       // Optional
    info.pQueueFamilyIndices   = nullptr; // Optional
  }

  mSwapChain = VulkanFactory::createSwapChainKhr(info, *mDevice);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSwapChain::createFramebuffers() {

  // delete old frame buffers first
  mFramebuffers.clear();

  auto swapChainImages = mDevice->getDevice()->getSwapchainImagesKHR(*mSwapChain);

  for (auto const& image : swapChainImages) {
    mFramebuffers.push_back(VulkanFramebuffer(mDevice, mRenderPass, image, mExtent, mImageFormat));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSwapChain::createRenderPass() {
  // delete old render pass first
  mRenderPass.reset();

  vk::AttachmentDescription colorAttachment;
  colorAttachment.format         = mImageFormat;
  colorAttachment.samples        = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout    = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference colorAttachmentRef;
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout     = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subPass;
  subPass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
  subPass.colorAttachmentCount = 1;
  subPass.pColorAttachments    = &colorAttachmentRef;

  vk::SubpassDependency dependency;
  dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass    = 0;
  dependency.srcStageMask  = vk::PipelineStageFlagBits::eBottomOfPipe;
  dependency.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
  dependency.dstStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.dstAccessMask =
    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

  vk::RenderPassCreateInfo info;
  info.attachmentCount = 1;
  info.pAttachments    = &colorAttachment;
  info.subpassCount    = 1;
  info.pSubpasses      = &subPass;
  info.dependencyCount = 1;
  info.pDependencies   = &dependency;

  mRenderPass = VulkanFactory::createRenderPass(info, *mDevice);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SurfaceFormatKHR
VulkanSwapChain::chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& available) {
  if (available.size() == 1 && available[0].format == vk::Format::eUndefined) {
    return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
  }

  for (const auto& format : available) {
    if (
      format.format == vk::Format::eB8G8R8A8Unorm &&
      format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return format;
    }
  }

  return available[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::PresentModeKHR
VulkanSwapChain::choosePresentMode(std::vector<vk::PresentModeKHR> const& available) {
  for (auto const& mode : available) {
    if (mode == vk::PresentModeKHR::eMailbox) { return mode; }
  }

  return vk::PresentModeKHR::eImmediate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Extent2D VulkanSwapChain::chooseExtent(vk::SurfaceCapabilitiesKHR const& available) {
  if (available.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return available.currentExtent;
  } else {

    // when does this happen?
    ILLUSION_WARNING << "TODO" << std::endl;
    vk::Extent2D actualExtent = {500, 500};

    actualExtent.width = std::max(
      available.minImageExtent.width, std::min(available.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
      available.minImageExtent.height,
      std::min(available.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
