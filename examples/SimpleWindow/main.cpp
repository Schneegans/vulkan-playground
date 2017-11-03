////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// #include <Illusion/Engine/Engine.hpp>

// #include <Illusion/Graphics/Vulkan/VulkanDevice.hpp>
// #include <Illusion/Graphics/Vulkan/VulkanInstance.hpp>
// #include <Illusion/Graphics/Window.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
  std::cout << "huhu" << std::endl;
  // auto engine   = Illusion::Engine::create(argc, argv);
  // auto instance = Illusion::VulkanInstance::create("SimpleWindow");
  // auto device   = Illusion::VulkanDevice::create(instance);
  // auto window   = Illusion::Window::create(device);

  // window->open();
  // window->onKeyEvent.connect([&engine](Illusion::KeyEvent event) {
  //   if (event.mType == Illusion::KeyEvent::Type::PRESS && event.mKey == Illusion::Key::ESCAPE) {
  //     engine->stop();
  //     return false;
  //   }
  //   return true;
  // });

  // window->onClose.connect([&engine]() {
  //   engine->stop();
  //   return false;
  // });

  // while (engine->running()) {
  //   window->processInput();
  // }

  return 0;
}
