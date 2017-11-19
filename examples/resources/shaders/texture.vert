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

struct Bar {
    float b;
};

struct Foo {
    Bar bar;
    float b;
};

layout(push_constant, std140) uniform PushConstants {
    vec2 pos;
    float important[3];
    Foo test;
    Foo bar1;
} pushConstants;

layout(binding = 0, std140) uniform Uniforms {
    vec3 color;
    float time;
} uniforms[2];

layout(location = 0) out vec2 fragTexCoord;

void main() {
    fragTexCoord = positions[gl_VertexIndex] + 0.5;
    gl_Position = vec4(positions[gl_VertexIndex] + pushConstants.pos, 0.0, 1.0);
}
