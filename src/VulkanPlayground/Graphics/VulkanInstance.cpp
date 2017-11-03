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
#include "VulkanInstance.hpp"

#include "../Utils/Logger.hpp"
#include "../Utils/stl_helpers.hpp"
#include "VulkanPhysicalDevice.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <set>

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////

VkBool32 messageCallback(
  VkDebugReportFlagsEXT      flags,
  VkDebugReportObjectTypeEXT type,
  uint64_t                   object,
  size_t                     location,
  int32_t                    code,
  const char*                layer,
  const char*                message,
  void*                      userData) {
  std::stringstream buf;
  buf << "[" << layer << "] " << message << " (code: " << code << ")" << std::endl;

  if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    ILLUSION_ERROR << buf.str();
  } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    ILLUSION_WARNING << buf.str();
  } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    ILLUSION_DEBUG << buf.str();
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_LUNARG_standard_validation"};

const std::vector<const char*> DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

////////////////////////////////////////////////////////////////////////////////////////////////////

bool glfwInitialized = false;

////////////////////////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanInstance::VulkanInstance(std::string const& appName, bool debugMode)
  : mDebugMode(debugMode) {

  if (!glfwInitialized) {
    if (!glfwInit()) { ILLUSION_ERROR << "Failed to initialize GLFW." << std::endl; }

    glfwSetErrorCallback([](int error, const char* description) {
      ILLUSION_ERROR << "GLFW: " << description << std::endl;
    });

    glfwInitialized = true;
  }

  if (mDebugMode && !checkValidationLayerSupport()) {
    ILLUSION_ERROR << "Requested validation layers are not available!" << std::endl;
  }

  createInstance("Illusion", appName);
  setupDebugCallback();
  pickPhysicalDevice();
  createDevice();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanInstance::createInstance(std::string const& engineName, std::string const& appName) {
  // app info
  vk::ApplicationInfo appInfo;
  appInfo.pApplicationName   = appName.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName        = engineName.c_str();
  appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion         = VK_API_VERSION_1_0;

  // find required extensions
  auto extensions(getRequiredInstanceExtensions());

  // create instance
  vk::InstanceCreateInfo createInfo;
  createInfo.pApplicationInfo        = &appInfo;
  createInfo.enabledExtensionCount   = static_cast<int32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  if (mDebugMode) {
    createInfo.enabledLayerCount   = static_cast<int32_t>(VALIDATION_LAYERS.size());
    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  mInstance = VulkanFactory::createInstance(createInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanInstance::setupDebugCallback() {
  if (!mDebugMode) return;

  auto createCallback =
    (PFN_vkCreateDebugReportCallbackEXT)mInstance->getProcAddr("vkCreateDebugReportCallbackEXT");

  vk::DebugReportCallbackCreateInfoEXT createInfo;
  createInfo.flags = vk::DebugReportFlagBitsEXT::eInformation |
                     vk::DebugReportFlagBitsEXT::eWarning |
                     vk::DebugReportFlagBitsEXT::ePerformanceWarning |
                     vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eDebug;
  createInfo.pfnCallback = messageCallback;

  VkDebugReportCallbackEXT tmp;
  if (createCallback(*mInstance, (VkDebugReportCallbackCreateInfoEXT*)&createInfo, nullptr, &tmp)) {
    ILLUSION_ERROR << "Failed to set up debug callback!" << std::endl;
  }

  mDebugCallback =
    VulkanFactory::createDebugReportCallbackExt(vk::DebugReportCallbackEXT(tmp), mInstance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanInstance::pickPhysicalDevice() {
  auto physicalDevices = mInstance->enumeratePhysicalDevices();

  // loop through physical devices and choose a suitable one
  for (auto const& physicalDevice : physicalDevices) {

    // check whether the required queue families are supported
    int graphicsFamily = chooseGraphicsQueueFamily(physicalDevice);
    int computeFamily  = chooseComputeQueueFamily(physicalDevice);
    int presentFamily  = choosePresentQueueFamily(physicalDevice);

    if (graphicsFamily < 0 || presentFamily < 0 || computeFamily < 0) { continue; }

    // check whether all required extensions are supported
    auto                  availableExtensions(physicalDevice.enumerateDeviceExtensionProperties());
    std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

    for (auto const& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    if (!requiredExtensions.empty()) { continue; }

    mPhysicalDevice = std::make_shared<VulkanPhysicalDevice>(physicalDevice);
    mGraphicsFamily = graphicsFamily;
    mComputeFamily  = computeFamily;
    mPresentFamily  = presentFamily;

    if (mDebugMode) { mPhysicalDevice->printInfo(); }

    return;
  }

  ILLUSION_ERROR << "Failed to find a suitable vulkan device!" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDevicePtr VulkanInstance::createDevice() const {
  float queuePriority = 1.0f;

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

  std::set<uint32_t> uniqueQueueFamilies = {(uint32_t)mGraphicsFamily,
                                            (uint32_t)mComputeFamily,
                                            (uint32_t)mPresentFamily};

  for (uint32_t queueFamily : uniqueQueueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures deviceFeatures;
  deviceFeatures.samplerAnisotropy = true;

  vk::DeviceCreateInfo createInfo;
  createInfo.pQueueCreateInfos       = queueCreateInfos.data();
  createInfo.queueCreateInfoCount    = (uint32_t)queueCreateInfos.size();
  createInfo.pEnabledFeatures        = &deviceFeatures;
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
  createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

  return mPhysicalDevice->createDevice(createInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkSurfaceKHRPtr VulkanInstance::createSurface(GLFWwindow* window) {
  VkSurfaceKHR tmp;
  if (glfwCreateWindowSurface(*mInstance, window, nullptr, &tmp) != VK_SUCCESS) {
    ILLUSION_ERROR << "Failed to create window surface!" << std::endl;
  }

  ILLUSION_DEBUG << "Creating window surface." << std::endl;
  auto instance = mInstance;
  return createManagedObject(vk::SurfaceKHR(tmp), [instance](vk::SurfaceKHR* obj) {
    ILLUSION_DEBUG << "Deleting window surface." << std::endl;
    instance->destroySurfaceKHR(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool VulkanInstance::checkValidationLayerSupport() {
  auto layerProperties = vk::enumerateInstanceLayerProperties();

  for (auto const& layer : VALIDATION_LAYERS) {
    bool layerFound = false;

    for (auto const& property : layerProperties) {
      if (std::strcmp(layer, property.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) { return false; }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<const char*> VulkanInstance::getRequiredInstanceExtensions() {
  std::vector<const char*> extensions;

  unsigned int glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  for (unsigned int i = 0; i < glfwExtensionCount; ++i) {
    extensions.push_back(glfwExtensions[i]);
  }

  if (mDebugMode) { extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); }

  return extensions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int VulkanInstance::chooseGraphicsQueueFamily(vk::PhysicalDevice const& physicalDevice) {
  auto queueFamilies = physicalDevice.getQueueFamilyProperties();

  for (int i(0); i < static_cast<int>(queueFamilies.size()); ++i) {
    if (
      queueFamilies[i].queueCount > 0 &&
      queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int VulkanInstance::chooseComputeQueueFamily(vk::PhysicalDevice const& physicalDevice) {
  auto queueFamilies = physicalDevice.getQueueFamilyProperties();

  for (int i(0); i < static_cast<int>(queueFamilies.size()); ++i) {
    if (
      queueFamilies[i].queueCount > 0 &&
      queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int VulkanInstance::choosePresentQueueFamily(vk::PhysicalDevice const& physicalDevice) {
  auto queueFamilies = physicalDevice.getQueueFamilyProperties();

  for (int i(0); i < static_cast<int>(queueFamilies.size()); ++i) {
    if (
      queueFamilies[i].queueCount > 0 &&
      glfwGetPhysicalDevicePresentationSupport(*mInstance, physicalDevice, i)) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
