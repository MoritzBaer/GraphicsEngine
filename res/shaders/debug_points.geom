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
layout (set = 1, binding = 0) uniform sampler2D sceneDepth;

#define CLIP_SPACE_POSITION_FROM_CAM_SPACE_AND_OFFSET matrices.projection * vec4(inPosCameraSpace[0] + vec3(0.01 * inPosCameraSpace[0].y * offset, 0).xzy, 1.0) + vec4(0,0,-0.002,0);
#define EPS 0.0001

void main() {
    vec4 homogenizedInPosition = gl_in[0].gl_Position / gl_in[0].gl_Position.w;
    vec2 depthRead = (vec2(homogenizedInPosition.x, homogenizedInPosition.y) + 1) / 2;

    // Take multiple samples to reduce flickering
    float depth = 0;
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            depth = max(depth, texture(sceneDepth, depthRead + vec2(i, j) * 0.0005).r);
        }
    }

    if (depth < homogenizedInPosition.z - EPS) return;
    
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