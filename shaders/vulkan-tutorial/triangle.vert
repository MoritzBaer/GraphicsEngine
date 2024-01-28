#version 450

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec2 _uv;

layout(location = 0) out vec3 fragColour;
layout(location = 1) out vec2 uv;

void main() {
    fragColour = colour;
    uv = _uv;
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(pos, 0.0, 1.0);
}