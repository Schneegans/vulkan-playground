////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <VulkanPlayground/Graphics/ShaderReflection.hpp>
#include <VulkanPlayground/Utils/File.hpp>
#include <VulkanPlayground/Utils/Logger.hpp>

#include <glm/glm.hpp>

#include <iostream>
#include <thread>

int main(int argc, char* argv[]) {

  std::string fileName{"texture.vert.spv"};

  try {
    auto code = Illusion::File<uint32_t>("data/shaders/" + fileName).getContent();
    auto reflection{std::make_shared<Illusion::Graphics::ShaderReflection>(code)};

    std::cout << reflection->toInfoString() << std::endl;
    std::cout << std::endl;
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << std::endl;

    std::string guard{"ILLUSION_SHADER_REFLECTION_" + fileName + "_HPP"};
    std::transform(guard.begin(), guard.end(), guard.begin(), ::toupper);
    std::replace(guard.begin(), guard.end(), '.', '_');

    std::cout << "#ifndef " + guard << std::endl;
    std::cout << "#define " + guard << std::endl;
    std::cout << std::endl;
    std::cout << "#include <glm/glm.hpp>" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << reflection->toCppString() << std::endl;
    std::cout << std::endl;
    std::cout << "#endif // " + guard << std::endl;

  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
