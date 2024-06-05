#version 450 core

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "util/scene_data.glsl"

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec2 outUV;
layout (location = 2) out mat3 outTBN;

struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

//push constants block
layout( push_constant ) uniform PushConstants
{	
	mat4 model;
	VertexBuffer vertexBuffer;
} pushConstants;

void main() {
        Vertex vertex = pushConstants.vertexBuffer.vertices[gl_VertexIndex];
        vec4 worldPos = pushConstants.model * vec4(vertex.position, 1.0);
        outWorldPos = worldPos.xyz;
        outUV = vec2(vertex.uv_x, 1.0 - vertex.uv_y);

        mat4 normalTransform = transpose(inverse(pushConstants.model));
        
        vec3 N = normalize((normalTransform * vec4(vertex.normal, 0)).xyz);
        vec3 T = normalize((N != vec3(1, 0, 0)) ? cross(vec3(1, 0, 0), N) : cross(vec3(0, 1, 0), N));
        vec3 B = normalize(cross(N, T));
        
        outTBN = mat3(T, B, N);
        
        gl_Position = sceneData.viewProjection * worldPos;
}