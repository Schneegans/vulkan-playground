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
#include "VulkanSurface.hpp"

#include "../Utils/Logger.hpp"
#include "../Utils/ScopedTimer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanInstance.hpp"
#include "VulkanPhysicalDevice.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <set>
#include <sstream>
#include <thread>

namespace Illusion {

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::SurfaceFormatKHR chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& available) {
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

vk::PresentModeKHR choosePresentMode(std::vector<vk::PresentModeKHR> const& available) {
  for (auto const& mode : available) {
    if (mode == vk::PresentModeKHR::eMailbox) { return mode; }
  }

  return vk::PresentModeKHR::eImmediate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Extent2D chooseExtent(vk::SurfaceCapabilitiesKHR const& available) {
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

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanSurface::VulkanSurface(VulkanDevicePtr const& device, GLFWwindow* window)
  : mDevice(device) {

  mSurface = device->getInstance()->createSurface(window);

  createSwapChain();
  createRenderPass();
  createFramebuffers();
  createSemaphores();
  createCommandBuffers();

  uint32_t bufferSize  = sizeof(CameraUniforms);
  mCameraUniformBuffer = mDevice->createBuffer(
    bufferSize,
    vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
    vk::MemoryPropertyFlagBits::eDeviceLocal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameInfo VulkanSurface::beginFrame(CameraUniforms const& camera) {
  uint32_t imageIndex;
  auto     result = mDevice->getDevice()->acquireNextImageKHR(
    *mSwapChain,
    std::numeric_limits<uint64_t>::max(),
    *mImageAvailableSemaphore,
    nullptr,
    &imageIndex);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    recreate();
    return beginFrame(camera);
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    ILLUSION_ERROR << "Suboptimal swap chain!" << std::endl;
  }

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
  mDevice->getDevice()->waitForFences(*mFences[imageIndex], true, ~0);

  vk::CommandBuffer buffer = mPrimaryCommandBuffers[imageIndex];
  mDevice->getDevice()->resetFences(*mFences[imageIndex]);

  buffer.reset(vk::CommandBufferResetFlags());
  buffer.begin(beginInfo);

  // Update dynamic viewport state
  vk::Viewport viewport;
  viewport.height   = (float)mExtent.height;
  viewport.width    = (float)mExtent.width;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  buffer.setViewport(0, 1, &viewport);

  // Update dynamic scissor state
  vk::Rect2D scissor;
  scissor.extent.width  = mExtent.width;
  scissor.extent.height = mExtent.height;
  scissor.offset.x      = 0;
  scissor.offset.y      = 0;
  buffer.setScissor(0, 1, &scissor);

  // update camera uniforms
  buffer.updateBuffer(*mCameraUniformBuffer->mBuffer, 0, sizeof(CameraUniforms), (uint8_t*)&camera);

  return {buffer, imageIndex};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::beginRenderPass(FrameInfo const& info) const {
  vk::RenderPassBeginInfo passInfo;
  passInfo.renderPass        = *mRenderPass;
  passInfo.framebuffer       = *mFramebuffers[info.mSwapChainImageIndex].mFramebuffer;
  passInfo.renderArea.offset = vk::Offset2D(0, 0);
  passInfo.renderArea.extent = mExtent;

  std::array<float, 4> vals = {{0.f, 0.f, 0.f, 0.f}};
  vk::ClearValue clearColor(vals);
  passInfo.clearValueCount = 1;
  passInfo.pClearValues    = &clearColor;

  info.mPrimaryCommandBuffer.beginRenderPass(passInfo, vk::SubpassContents::eInline);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::endRenderPass(FrameInfo const& info) const {
  info.mPrimaryCommandBuffer.endRenderPass();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::endFrame(FrameInfo const& info) const {
  info.mPrimaryCommandBuffer.end();

  vk::PipelineStageFlags waitStages[]       = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  vk::Semaphore          waitSemaphores[]   = {*mImageAvailableSemaphore};
  vk::Semaphore          signalSemaphores[] = {*mRenderFinishedSemaphore};

  vk::SubmitInfo submitInfo;
  submitInfo.waitSemaphoreCount   = 1;
  submitInfo.pWaitSemaphores      = waitSemaphores;
  submitInfo.pWaitDstStageMask    = waitStages;
  submitInfo.commandBufferCount   = 1;
  submitInfo.pCommandBuffers      = &info.mPrimaryCommandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = signalSemaphores;

  mDevice->getGraphicsQueue().submit(submitInfo, *mFences[info.mSwapChainImageIndex]);

  vk::SwapchainKHR swapChains[] = {*mSwapChain};

  vk::PresentInfoKHR presentInfo;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = signalSemaphores;
  presentInfo.swapchainCount     = 1;
  presentInfo.pSwapchains        = swapChains;
  presentInfo.pImageIndices      = &info.mSwapChainImageIndex;

  auto result = mDevice->getPresentQueue().presentKHR(presentInfo);
  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
    ILLUSION_ERROR << "out of date 3!" << std::endl;
  } else if (result != vk::Result::eSuccess) {
    ILLUSION_ERROR << "out of date 4!" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::recreate() {
  mDevice->getDevice()->waitIdle();

  createSwapChain();
  createRenderPass();
  createFramebuffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::createSwapChain() {
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
  if (!mDevice->getInstance()->getPhysicalDevice()->getSurfaceSupportKHR(
        presentFamily, *mSurface)) {
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

  mSwapChain = mDevice->createSwapChainKhr(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::createFramebuffers() {

  // delete old frame buffers first
  mFramebuffers.clear();

  auto swapChainImages = mDevice->getDevice()->getSwapchainImagesKHR(*mSwapChain);

  for (auto const& image : swapChainImages) {
    mFramebuffers.push_back(VulkanFramebuffer(mDevice, mRenderPass, image, mExtent, mImageFormat));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::createRenderPass() {
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

  mRenderPass = mDevice->createRenderPass(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::createSemaphores() {
  vk::SemaphoreCreateInfo info;
  mImageAvailableSemaphore = mDevice->createSemaphore(info);
  mRenderFinishedSemaphore = mDevice->createSemaphore(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::createCommandBuffers() {
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.commandPool        = *mDevice->getCommandPool();
  allocInfo.level              = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = mImageCount;

  mPrimaryCommandBuffers = mDevice->getDevice()->allocateCommandBuffers(allocInfo);

  for (uint32_t i = 0; i < mImageCount; ++i) {
    vk::FenceCreateInfo info;
    info.flags = vk::FenceCreateFlagBits::eSignaled;
    mFences.push_back(mDevice->createFence(info));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
