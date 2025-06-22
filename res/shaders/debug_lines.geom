#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

layout (location = 0) in vec3 uPosCamSpace[];
layout (location = 1) in vec3 vPosCamSpace[];
layout (location = 2) in vec3 inColour[];
layout (location = 0) out flat vec3 outColour;
layout (location = 1) out vec4 outClipPos;

layout (set = 0, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
} matrices;

#define LINE_WIDTH 0.002
#define OFFSET_EPS vec4(0,0,-0.001,0)

void main() {
    vec2 a = normalize(vPosCamSpace[0].xz - uPosCamSpace[0].xz);
    vec3 b = vec3(-a.y, 0, a.x);
    
    outColour = inColour[0];
    gl_Position = matrices.projection * vec4(uPosCamSpace[0] + LINE_WIDTH * uPosCamSpace[0].y * b, 1.0) + OFFSET_EPS;
    outClipPos = gl_Position;
    EmitVertex();
    gl_Position = matrices.projection * vec4(uPosCamSpace[0] - LINE_WIDTH * uPosCamSpace[0].y * b, 1.0) + OFFSET_EPS;
    outClipPos = gl_Position;
    EmitVertex();
    gl_Position = matrices.projection * vec4(vPosCamSpace[0] + LINE_WIDTH * vPosCamSpace[0].y * b, 1.0) + OFFSET_EPS;
    outClipPos = gl_Position;
    EmitVertex();
    EndPrimitive();
    
    gl_Position = matrices.projection * vec4(uPosCamSpace[0] - LINE_WIDTH * uPosCamSpace[0].y * b, 1.0) + OFFSET_EPS;
    outClipPos = gl_Position;
    EmitVertex();
    gl_Position = matrices.projection * vec4(vPosCamSpace[0] - LINE_WIDTH * vPosCamSpace[0].y * b, 1.0) + OFFSET_EPS;
    outClipPos = gl_Position;
    EmitVertex();
    gl_Position = matrices.projection * vec4(vPosCamSpace[0] + LINE_WIDTH * vPosCamSpace[0].y * b, 1.0) + OFFSET_EPS;
    outClipPos = gl_Position;
    EmitVertex();
    EndPrimitive();
}