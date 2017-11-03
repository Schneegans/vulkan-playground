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
#include "VulkanSwapChain.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <set>
#include <sstream>
#include <thread>

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanSurface::VulkanSurface(VulkanDevicePtr const& device, GLFWwindow* window)
  : mDevice(device) {

  mSurface = device->getInstance()->createSurface(window);

  createSwapChain();
  createSemaphores();
  createCommandBuffers();

  uint32_t bufferSize = sizeof(CameraUniforms);
  mCameraUniformBuffer    = mDevice->createBuffer(
    bufferSize,
    vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
    vk::MemoryPropertyFlagBits::eDeviceLocal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FrameInfo VulkanSurface::beginFrame(CameraUniforms const& camera) {
  uint32_t imageIndex;
  auto     result = mDevice->getDevice()->acquireNextImageKHR(
    *(VkSwapchainKHRPtr)*mSwapChain,
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
  viewport.height   = (float)mSwapChain->getExtent().height;
  viewport.width    = (float)mSwapChain->getExtent().width;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  buffer.setViewport(0, 1, &viewport);

  // Update dynamic scissor state
  vk::Rect2D scissor;
  scissor.extent.width  = mSwapChain->getExtent().width;
  scissor.extent.height = mSwapChain->getExtent().height;
  scissor.offset.x      = 0;
  scissor.offset.y      = 0;
  buffer.setScissor(0, 1, &scissor);

  // update camera uniforms
  buffer.updateBuffer(
    *mCameraUniformBuffer->mBuffer, 0, sizeof(CameraUniforms), (uint8_t*)&camera);

  return {buffer, imageIndex};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::beginRenderPass(FrameInfo const& info) const {
  vk::RenderPassBeginInfo passInfo;
  passInfo.renderPass  = *mSwapChain->getRenderPass();
  passInfo.framebuffer = *mSwapChain->getFramebuffers()[info.mSwapChainImageIndex].mFramebuffer;
  passInfo.renderArea.offset = vk::Offset2D(0, 0);
  passInfo.renderArea.extent = mSwapChain->getExtent();

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

  vk::SwapchainKHR swapChains[] = {*(VkSwapchainKHRPtr)*mSwapChain};

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
}

////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Extent2D const& VulkanSurface::getExtent() const { return mSwapChain->getExtent(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

VkRenderPassPtr const& VulkanSurface::getRenderPass() const { return mSwapChain->getRenderPass(); }

////////////////////////////////////////////////////////////////////////////////////////////////////

BufferPtr const& VulkanSurface::getCameraUniformBuffer() const { return mCameraUniformBuffer; }

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::createSwapChain() {
  // delete old swap chain first
  mSwapChain.reset();
  mSwapChain = std::make_shared<VulkanSwapChain>(mDevice, mSurface);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::createSemaphores() {
  vk::SemaphoreCreateInfo info;
  mImageAvailableSemaphore = VulkanFactory::createSemaphore(info, *mDevice);
  mRenderFinishedSemaphore = VulkanFactory::createSemaphore(info, *mDevice);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanSurface::createCommandBuffers() {
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.commandPool        = *mDevice->getCommandPool();
  allocInfo.level              = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = mSwapChain->getImageCount();

  mPrimaryCommandBuffers = mDevice->getDevice()->allocateCommandBuffers(allocInfo);

  for (uint32_t i = 0; i < mSwapChain->getImageCount(); ++i) {
    vk::FenceCreateInfo info;
    info.flags = vk::FenceCreateFlagBits::eSignaled;
    mFences.push_back(VulkanFactory::createFence(info, *mDevice));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
