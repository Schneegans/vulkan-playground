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

const std::vector<const char*> VALIDATION_LAYERS{"VK_LAYER_LUNARG_standard_validation"};
const std::vector<const char*> DEVICE_EXTENSIONS{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

////////////////////////////////////////////////////////////////////////////////////////////////////

bool glfwInitialized{false};

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

int chooseQueueFamily(vk::PhysicalDevice const& physicalDevice, vk::QueueFlagBits caps) {
  auto queueFamilies = physicalDevice.getQueueFamilyProperties();

  for (size_t i{0}; i < queueFamilies.size(); ++i) {
    if (queueFamilies[i].queueCount > 0 && (queueFamilies[i].queueFlags & caps) == caps) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int choosePresentQueueFamily(
  vk::PhysicalDevice const& physicalDevice, vk::Instance const& instance) {
  auto queueFamilies = physicalDevice.getQueueFamilyProperties();

  for (size_t i{0}; i < queueFamilies.size(); ++i) {
    if (
      queueFamilies[i].queueCount > 0 &&
      glfwGetPhysicalDevicePresentationSupport(instance, physicalDevice, i)) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool checkValidationLayerSupport() {
  for (auto const& layer : VALIDATION_LAYERS) {
    bool layerFound{false};

    for (auto const& property : vk::enumerateInstanceLayerProperties()) {
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

std::vector<const char*> getRequiredInstanceExtensions(bool debugMode) {
  unsigned int glfwExtensionCount{0};
  const char** glfwExtensions{glfwGetRequiredInstanceExtensions(&glfwExtensionCount)};

  std::vector<const char*> extensions;
  for (unsigned int i = 0; i < glfwExtensionCount; ++i) {
    extensions.push_back(glfwExtensions[i]);
  }

  if (debugMode) { extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); }

  return extensions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VulkanInstance::VulkanInstance(std::string const& appName, bool debugMode)
  : mDebugMode(debugMode) {

  if (!glfwInitialized) {
    if (!glfwInit()) { throw std::runtime_error{"Failed to initialize GLFW."}; }

    glfwSetErrorCallback([](int error, const char* description) {
      throw std::runtime_error{"GLFW: " + std::string(description)};
    });

    glfwInitialized = true;
  }

  if (mDebugMode && !checkValidationLayerSupport()) {
    throw std::runtime_error{"Requested validation layers are not available!"};
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
  auto extensions(getRequiredInstanceExtensions(mDebugMode));

  // create instance
  vk::InstanceCreateInfo info;
  info.pApplicationInfo        = &appInfo;
  info.enabledExtensionCount   = static_cast<int32_t>(extensions.size());
  info.ppEnabledExtensionNames = extensions.data();

  if (mDebugMode) {
    info.enabledLayerCount   = static_cast<int32_t>(VALIDATION_LAYERS.size());
    info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
  } else {
    info.enabledLayerCount = 0;
  }

  ILLUSION_DEBUG << "Creating instance." << std::endl;
  mInstance = makeVulkanPtr(vk::createInstance(info), [](vk::Instance* obj) {
    ILLUSION_DEBUG << "Deleting instance." << std::endl;
    obj->destroy();
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanInstance::setupDebugCallback() {
  if (!mDebugMode) { return; }

  auto createCallback{
    (PFN_vkCreateDebugReportCallbackEXT)mInstance->getProcAddr("vkCreateDebugReportCallbackEXT")};

  vk::DebugReportCallbackCreateInfoEXT info;
  info.flags = vk::DebugReportFlagBitsEXT::eInformation | vk::DebugReportFlagBitsEXT::eWarning |
               vk::DebugReportFlagBitsEXT::ePerformanceWarning |
               vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eDebug;
  info.pfnCallback = messageCallback;

  VkDebugReportCallbackEXT tmp;
  if (createCallback(*mInstance, (VkDebugReportCallbackCreateInfoEXT*)&info, nullptr, &tmp)) {
    throw std::runtime_error{"Failed to set up debug callback!"};
  }

  ILLUSION_DEBUG << "Creating debug callback." << std::endl;
  auto instance{mInstance};
  mDebugCallback =
    makeVulkanPtr(vk::DebugReportCallbackEXT(tmp), [instance](vk::DebugReportCallbackEXT* obj) {
      auto destroyCallback = (PFN_vkDestroyDebugReportCallbackEXT)instance->getProcAddr(
        "vkDestroyDebugReportCallbackEXT");
      ILLUSION_DEBUG << "Deleting debug callback." << std::endl;
      destroyCallback(*instance, *obj, nullptr);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void VulkanInstance::pickPhysicalDevice() {
  auto physicalDevices{mInstance->enumeratePhysicalDevices()};

  // loop through physical devices and choose a suitable one
  for (auto const& physicalDevice : physicalDevices) {

    // check whether the required queue families are supported
    int graphicsFamily{chooseQueueFamily(physicalDevice, vk::QueueFlagBits::eGraphics)};
    int computeFamily{chooseQueueFamily(physicalDevice, vk::QueueFlagBits::eCompute)};
    int presentFamily{choosePresentQueueFamily(physicalDevice, *mInstance)};

    // check whether all required queue types are supported
    if (graphicsFamily < 0 || presentFamily < 0 || computeFamily < 0) { continue; }

    // check whether all required extensions are supported
    auto                  availableExtensions{physicalDevice.enumerateDeviceExtensionProperties()};
    std::set<std::string> requiredExtensions{DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end()};

    for (auto const& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    if (!requiredExtensions.empty()) { continue; }

    // all regquired extensions are supported - take this device!
    mPhysicalDevice = std::make_shared<VulkanPhysicalDevice>(physicalDevice);
    mGraphicsFamily = graphicsFamily;
    mComputeFamily  = computeFamily;
    mPresentFamily  = presentFamily;

    if (mDebugMode) { mPhysicalDevice->printInfo(); }

    return;
  }

  throw std::runtime_error{"Failed to find a suitable vulkan device!"};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

VkDevicePtr VulkanInstance::createDevice() const {

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

  const float              queuePriority{1.0f};
  const std::set<uint32_t> uniqueQueueFamilies{(uint32_t)mGraphicsFamily,
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

VkSurfaceKHRPtr VulkanInstance::createSurface(GLFWwindow* window) const {
  VkSurfaceKHR tmp;
  if (glfwCreateWindowSurface(*mInstance, window, nullptr, &tmp) != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create window surface!"};
  }

  ILLUSION_DEBUG << "Creating window surface." << std::endl;

  // copying instance to keep reference counting up until the surface is destroyed
  auto instance{mInstance};
  return makeVulkanPtr(vk::SurfaceKHR(tmp), [instance](vk::SurfaceKHR* obj) {
    ILLUSION_DEBUG << "Deleting window surface." << std::endl;
    instance->destroySurfaceKHR(*obj);
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
