#version 450 core

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexColour;

layout (location = 0) out vec3 outPosCameraSpace;
layout (location = 1) out vec3 outColour;

layout (set = 0, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
} matrices;

void main() {
    outPosCameraSpace = (matrices.view * vec4(vertexPosition, 1.0)).xyz;
    outColour = vertexColour;
        
    gl_Position = matrices.projection * vec4(outPosCameraSpace,1.0);
}