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

  if (argc == 1) {
    Illusion::ILLUSION_MESSAGE
      << "Please specifiy at least an input file and optionally an output file." << std::endl;
    return 0;
  }

  std::string inputName{argv[1]};

  try {
    auto code = Illusion::File<uint32_t>(inputName).getContent();
    auto reflection{std::make_shared<Illusion::Graphics::ShaderReflection>(code)};

    if (argc == 2) {

      std::cout << reflection->toInfoString() << std::endl;
      std::cout << std::endl;
      std::cout << "-------------------------------------------" << std::endl;
      std::cout << std::endl;

    } else {
      std::string outputName{argv[2]};

      size_t fileStart = outputName.find_last_of("/\\");
      if (fileStart == std::string::npos)
        fileStart = 0;
      else
        fileStart += 1;

      std::string guard{"ILLUSION_SHADER_REFLECTION_" + inputName.substr(fileStart) + "_HPP"};
      std::transform(guard.begin(), guard.end(), guard.begin(), ::toupper);
      std::replace(guard.begin(), guard.end(), '.', '_');

      std::ofstream out(outputName);

      out << "#ifndef " + guard << std::endl;
      out << "#define " + guard << std::endl;
      out << std::endl;
      out << "#include <glm/glm.hpp>" << std::endl;
      out << std::endl;
      out << std::endl;
      out << reflection->toCppString() << std::endl;
      out << std::endl;
      out << "#endif // " + guard << std::endl;
    }

  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
