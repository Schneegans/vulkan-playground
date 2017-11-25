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

std::vector<std::string> splitString(std::string const& s, char delim) {
  std::vector<std::string> elems;

  std::stringstream ss(s);
  std::string       item;

  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }

  return elems;
}

int main(int argc, char* argv[]) {

  if (argc < 4) {
    Illusion::ILLUSION_MESSAGE << "Usage:" << std::endl;
    Illusion::ILLUSION_MESSAGE << "  ReflectionExtractor <SPIRV_FILE>"
                               << " [<ADDITIONAL_SPIRV_FILES>] <NAMESPACE> <OUTPUT_HPP> "
                               << std::endl;
    Illusion::ILLUSION_MESSAGE << std::endl;
    Illusion::ILLUSION_MESSAGE << "The ReflectionExtractor links together all provided spirv "
                               << std::endl;
    Illusion::ILLUSION_MESSAGE << "files and writes the resulting reflection header wrapped in a "
                               << std::endl;
    Illusion::ILLUSION_MESSAGE << "namespace <NAMESPACE> in the header file <OUTPUT_HPP>."
                               << std::endl;

    return 0;
  }

  try {
    std::string nameSpace{argv[argc - 2]};
    std::string outputName{argv[argc - 1]};

    std::vector<Illusion::Graphics::ShaderReflection> reflections;

    for (int i{1}; i < argc - 2; ++i) {
      auto code = Illusion::File<uint32_t>(argv[i]).getContent();
      reflections.push_back(Illusion::Graphics::ShaderReflection(code));
    }

    Illusion::Graphics::ShaderReflection reflection(reflections);

    // generate header guard
    size_t fileStart = outputName.find_last_of("/\\");
    if (fileStart == std::string::npos)
      fileStart = 0;
    else
      fileStart += 1;

    std::string guard{"ILLUSION_SHADER_REFLECTION_" + outputName.substr(fileStart)};
    std::transform(guard.begin(), guard.end(), guard.begin(), ::toupper);
    std::replace(guard.begin(), guard.end(), '.', '_');

    std::ofstream out(outputName);

    out << "#ifndef " + guard << std::endl;
    out << "#define " + guard << std::endl;
    out << std::endl;
    out << "// This file has been automatically created by the ReflectionExtractor." << std::endl;
    out << std::endl;
    out << "#include <glm/glm.hpp>" << std::endl;
    out << "#include <vulkan/vulkan.hpp>" << std::endl;
    out << std::endl;
    out << "namespace Reflection {" << std::endl;
    out << "namespace " << nameSpace << " {" << std::endl;
    out << std::endl;
    out << reflection.toCppString() << std::endl;
    out << std::endl;
    out << "} // " << nameSpace << std::endl;
    out << "} // Reflection" << std::endl;
    out << std::endl;
    out << "#endif // " + guard << std::endl;

  } catch (std::runtime_error const& e) { Illusion::ILLUSION_ERROR << e.what() << std::endl; }

  return 0;
}
