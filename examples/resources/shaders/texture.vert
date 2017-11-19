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

struct A {
    mat2 b;
}; 

struct B {
    A a[2];
    // float b[5];
    float c;
};

layout(push_constant, std430) uniform PushConstants {
    B test1;
    vec2 pos; 
} pushConstants;

layout(binding = 0, std140) uniform Uniforms {
    mat4 fuu[123];
    mat2 bar;
    B b[3];
    A a[17];
    vec2 c;
    vec3 d;
    vec4 f;
    float g;
    float time;
} uniforms;

layout(location = 0) out vec2 fragTexCoord;

void main() {
    fragTexCoord = positions[gl_VertexIndex] + 0.5;
    gl_Position = vec4(positions[gl_VertexIndex] + pushConstants.pos + vec2(0, sin(uniforms.time)), 0.0, 1.0);
}
