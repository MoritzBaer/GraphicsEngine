#version 450

layout(location = 0) out vec4 colour;
layout(location = 0) in vec3 fragColour;
layout(location = 1) in vec2 uv;
layout(binding = 1) uniform sampler2D textureSampler;

void main() {
    //colour = vec4(vec3(gl_FragCoord.y) / 2, 1);
    colour = vec4(fragColour * texture(textureSampler, uv).rgb, 1);
}