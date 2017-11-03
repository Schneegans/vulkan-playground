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

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out vec2 fragTexCoord;

vec2 positions[4] = vec2[](
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5),
    vec2(-0.5, 0.5),
    vec2(0.5, 0.5)
);

layout(binding = 0) uniform CameraUniforms {
    mat3 transform;
    float parallax;
    float time;
} view;

layout(push_constant, std140) uniform PushConstants {
    mat3  transform;
    float depth;
} model;

void main() {
    vec3 pos = view.transform * model.transform * vec3(positions[gl_VertexIndex], 1.0);
    pos.xy *= pow(view.parallax, model.depth);
    fragTexCoord = positions[gl_VertexIndex].xy + 0.5;
    gl_Position = vec4(pos.xy, 0.0, 1.0);
}
