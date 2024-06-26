#version 450 core

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "util/scene_data.glsl"

#define ACCESS_TBNP(i) vec3(vertex.TBNP_0[i], vertex.TBNP_1[i], vertex.TBNP_2[i])

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec2 outUV;
layout (location = 2) out mat3 outTBN;

struct Vertex {
        vec3 pos;
        float uv_x;
        vec3 normal;
        float uv_y;
        vec2 uv;
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

        vec4 worldPos = pushConstants.model * vec4(vertex.pos, 1);//vec4(ACCESS_TBNP(3), 1.0);
        outWorldPos = worldPos.xyz;
        //outUV = vec2(vertex.uv.x, 1.0 - vertex.uv.y);
        outUV = vec2(vertex.uv.x, 1.0 - vertex.uv.y);

        mat4 normalTransform = transpose(inverse(pushConstants.model));
        
        //vec3 T = normalize(normalTransform * vec4(ACCESS_TBNP(0), 0)).xyz;
        //vec3 B = normalize(normalTransform * vec4(ACCESS_TBNP(1), 0)).xyz;
        //vec3 N = normalize(normalTransform * vec4(ACCESS_TBNP(2), 0)).xyz;

        outTBN = mat3(vertex.normal, vertex.normal, vertex.normal);//mat3(T, B, ACCESS_TBNP(2));
        
        gl_Position = sceneData.viewProjection * worldPos;
}