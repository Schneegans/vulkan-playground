////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_TINYGLTF_HPP
#define ILLUSION_GRAPHICS_TINYGLTF_HPP

// ---------------------------------------------------------------------------------------- includes
#include "../fwd.hpp"

#include <tiny_gltf.h>

namespace Illusion {
namespace Graphics {
namespace TinyGLTF {

// -------------------------------------------------------------------------------------------------

TexturePtr createTexture(
  DevicePtr const& device, tinygltf::Sampler const& sampler, tinygltf::Image const& image);

// -------------------------------------------------------------------------------------------------
}
}
}

#endif // ILLUSION_GRAPHICS_TINYGLTF_HPP
