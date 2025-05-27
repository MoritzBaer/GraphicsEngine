#version 450 core

layout (location = 0) out vec4 fragColour;

layout (location = 0) in flat vec3 colour;
layout (location = 1) in vec2 offset;

void main() {
    if (offset.x * offset.x + offset.y * offset.y > 1.0) {
        discard;
    }
    fragColour = vec4(colour,1.0);
}