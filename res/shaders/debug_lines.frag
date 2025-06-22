#version 450 core

layout (location = 0) out vec4 fragColour;

layout (location = 0) in flat vec3 colour;
layout (location = 1) in vec4 clipPos;

layout (set = 1, binding = 0) uniform sampler2D sceneDepth;

#define EPS 0.0001

void main() {
    // Manual depth test because we test against a second depth buffer
    vec3 homogenizedCoord = clipPos.xyz / clipPos.w;
    float depth = texture(sceneDepth, (homogenizedCoord.xy + 1) / 2).r;
    if (depth < homogenizedCoord.z - EPS) discard;

    fragColour = vec4(colour,1.0);
}