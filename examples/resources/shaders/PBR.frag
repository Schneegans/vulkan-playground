////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform Uniforms {
    vec3 color;
    float time;
} uniforms;

// layout(push_constant, std140) uniform PushConstants {
//     mat3  transform;
//     float depth;
//     layout(offset=64) vec4 color;
// } model;

void main() {
    outColor = vec4(uniforms.color, 1.0);
}
