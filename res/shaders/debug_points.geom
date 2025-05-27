#version 450 core

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

layout (location = 0) in vec3 inPosCameraSpace[];
layout (location = 1) in vec3 inColour[];
layout (location = 0) out flat vec3 outColour;
layout (location = 1) out vec2 offset;

layout (set = 0, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
} matrices;

vec3 offset_in_cam_space(vec3 pos, vec2 offset) {
    return pos + 10 * vec3(offset, 0.0);// * pos.z;
}

#define CLIP_SPACE_POSITION_FROM_CAM_SPACE_AND_OFFSET matrices.projection * vec4(inPosCameraSpace[0] + vec3(0.01 * inPosCameraSpace[0].z * offset, 0.0), 1.0);
//#define CLIP_SPACE_POSITION_FROM_CAM_SPACE_AND_OFFSET gl_in[0].gl_Position + vec4(offset * 0.1, 0.0, 0.0) // For testing purposes, use a fixed centre;

void main() {
    // TODO: Test against depth buffer to avoid drawing if not visible
    
    outColour = inColour[0];
    offset = vec2(-1.0,-1.0);
    gl_Position = CLIP_SPACE_POSITION_FROM_CAM_SPACE_AND_OFFSET;
    EmitVertex();
    offset = vec2( 1.0, 1.0);
    gl_Position = CLIP_SPACE_POSITION_FROM_CAM_SPACE_AND_OFFSET;
    EmitVertex();
    offset = vec2(-1.0, 1.0);
    gl_Position = CLIP_SPACE_POSITION_FROM_CAM_SPACE_AND_OFFSET;
    EmitVertex();
    EndPrimitive();
    
    offset = vec2(-1.0,-1.0);
    gl_Position = CLIP_SPACE_POSITION_FROM_CAM_SPACE_AND_OFFSET;
    EmitVertex();
    offset = vec2( 1.0,-1.0);
    gl_Position = CLIP_SPACE_POSITION_FROM_CAM_SPACE_AND_OFFSET;
    EmitVertex();
    offset = vec2( 1.0, 1.0);
    gl_Position = CLIP_SPACE_POSITION_FROM_CAM_SPACE_AND_OFFSET;
    EmitVertex();
    EndPrimitive();
}