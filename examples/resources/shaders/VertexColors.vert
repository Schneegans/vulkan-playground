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

// inputs ------------------------------------------------------------------------------------------
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

// layout(location = 7, component = 0) in vec2 rectPos;
// layout(location = 7, component = 2) in vec2 rectSize;

// push constants ----------------------------------------------------------------------------------
layout(push_constant, std430) uniform PushConstants {
    vec2 pos; 
    float time;
} pushConstants;

// outputs -----------------------------------------------------------------------------------------
layout(location = 0) out vec3 outColor;

// methods -----------------------------------------------------------------------------------------
void main() {
    outColor = inColor;
    gl_Position = vec4(inPosition + pushConstants.pos + vec2(0, sin(pushConstants.time)), 0.0, 1.0);
}
