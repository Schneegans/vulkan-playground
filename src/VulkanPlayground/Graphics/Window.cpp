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
#include "Window.hpp"

#include "../Utils/Logger.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanSurface.hpp"

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>
#include <iostream>

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////

Window::Window(VulkanDevicePtr const& device)
  : mDevice(device) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

Window::~Window() {
  close();
  glfwDestroyWindow(mWindow);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::open() {
  if (!mWindow) {

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if (false) {
      auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
      mWindow   = glfwCreateWindow(
        mode->width, mode->height, "VulkanPlayground", glfwGetPrimaryMonitor(), nullptr);
    } else {
      mWindow = glfwCreateWindow(800, 600, "VulkanPlayground", nullptr, nullptr);
    }

    mSurface = std::make_shared<VulkanSurface>(mDevice, mWindow);

    glfwSetWindowUserPointer(mWindow, this);

    glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* w) {
      // auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      // window->onClose.emit();
    });

    glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* w, int width, int height) {
      auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      window->getSurface()->recreate();
    });

    glfwSetKeyCallback(mWindow, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
      // auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      // window->onKeyEvent.emit(KeyEvent(key, scancode, action, mods));
    });

    glfwSetCursorPosCallback(mWindow, [](GLFWwindow* w, double x, double y) {
      // auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      // window->onMouseEvent.emit(MouseEvent(static_cast<int>(x), static_cast<int>(y)));
    });

    glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* w, int button, int action, int mods) {
      // auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      // window->onMouseEvent.emit(MouseEvent(button, action == GLFW_PRESS));
    });

    glfwSetScrollCallback(mWindow, [](GLFWwindow* w, double x, double y) {
      // auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      // window->onMouseEvent.emit(MouseEvent(static_cast<int>(y * 10.0)));
    });

    glfwSetCharModsCallback(mWindow, [](GLFWwindow* w, unsigned c, int mods) {
      // auto window(static_cast<Window*>(glfwGetWindowUserPointer(w)));
      // window->onKeyEvent.emit(KeyEvent(c, mods));
    });
  } else {
    ILLUSION_WARNING << "Attempting to open an already opened window!" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::close() {
  if (mWindow) {
    glfwDestroyWindow(mWindow);
    mWindow = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Window::shouldClose() const { return glfwWindowShouldClose(mWindow); }

////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::processInput() {
  if (mWindow) { glfwPollEvents(); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanDevicePtr const& Window::getDevice() const { return mDevice; }

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanSurfacePtr const& Window::getSurface() const { return mSurface; }

////////////////////////////////////////////////////////////////////////////////////////////////////
}
