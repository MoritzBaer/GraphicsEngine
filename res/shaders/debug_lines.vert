#version 450 core

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

layout (location = 0) in vec3 uPos;
layout (location = 1) in vec3 vPos;
layout (location = 2) in vec3 lineColour;

layout (location = 0) out vec3 uPosCamSpace;
layout (location = 1) out vec3 vPosCamSpace;
layout (location = 2) out vec3 outColour;

layout (set = 0, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
} matrices;

void main() {
    uPosCamSpace = (matrices.view * vec4(uPos, 1.0)).xyz;
    vPosCamSpace = (matrices.view * vec4(vPos, 1.0)).xyz;
    outColour = lineColour;
        
    gl_Position = matrices.projection * vec4((uPos + vPos) / 2, 1.0);
}