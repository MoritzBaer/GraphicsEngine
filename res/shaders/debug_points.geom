#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

layout (location = 0) in vec3 inColour[];
layout (location = 1) in vec3 inPosCameraSpace[];
layout (location = 0) out vec3 outColour;
layout (location = 1) out vec2 offset;

layout (set = 0, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
} matrices;

vec3 offset_in_cam_space(vec3 pos, vec2 offset) {
    return pos + 0.01 * vec3(offset, 0.0) * pos.z;
}

void main() {
    // TODO: Test against depth buffer to avoid drawing if not visible
    outColour = inColour[0];
    
    offset = vec2(-1.0,-1.0);
    gl_Position = matrices.projection * vec4(offset_in_cam_space(inPosCameraSpace[0], offset), 1.0);
    EmitVertex();
    offset = vec2(-1.0, 1.0);
    gl_Position = matrices.projection * vec4(offset_in_cam_space(inPosCameraSpace[0], offset), 1.0);
    EmitVertex();
    offset = vec2( 1.0, 1.0);
    gl_Position = matrices.projection * vec4(offset_in_cam_space(inPosCameraSpace[0], offset), 1.0);
    EmitVertex();
    EndPrimitive();

    offset = vec2(-1.0,-1.0);
    gl_Position = matrices.projection * vec4(offset_in_cam_space(inPosCameraSpace[0], offset), 1.0);
    EmitVertex();
    offset = vec2( 1.0, 1.0);
    gl_Position = matrices.projection * vec4(offset_in_cam_space(inPosCameraSpace[0], offset), 1.0);
    EmitVertex();
    offset = vec2( 1.0,-1.0);
    gl_Position = matrices.projection * vec4(offset_in_cam_space(inPosCameraSpace[0], offset), 1.0);
    EmitVertex();
    EndPrimitive();
}