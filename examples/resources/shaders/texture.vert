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

vec2 positions[4] = vec2[](
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5),
    vec2(-0.5, 0.5),
    vec2(0.5, 0.5)
);

struct Foo {
    float b;
};

layout(push_constant, std430) uniform PushConstants {
    Foo test[2];
    float important;
    Foo bar;
    dvec2 pos;
    // mat3 pos2[2];
    // float color2[3];
    // vec2 pos3;
} pushConstants;

layout(binding = 0, std140) uniform Uniforms {
    bool color;
    vec3 time2[13];
} uniforms[2];

layout(location = 0) out vec2 fragTexCoord;

void main() {
    fragTexCoord = positions[gl_VertexIndex] + 0.5;
    gl_Position = vec4(positions[gl_VertexIndex] + pushConstants.pos, 0.0, 1.0);
}
