#version 450 core

#include "util/scene_data.glsl"

layout (set = 1, binding = 0) uniform sampler2D albedo;
layout (set = 1, binding = 1) uniform sampler2D bump;

layout (location = 0) out vec4 fragColour;

layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec2 uv;
layout (location = 2) in mat3 TBN;

void main() {
    vec3 colour = texture(albedo, uv).xyz;
    fragColour = vec4(colour,1.0);
}