#version 450 core

#include "util/scene_data.glsl"

layout (set = 1, binding = 0) uniform sampler2D albedo;
layout (set = 1, binding = 1) uniform sampler2D bump;

layout (location = 0) out vec4 fragColour;

layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec2 uv;
layout (location = 2) in mat3 TBN;
layout (location = 6) in float vertexID;

void main() {
    vec3 viewDir = normalize(sceneData.cameraPos - worldPos);
    vec3 halfway = normalize(viewDir - sceneData.lightDir);

    mat3 normalizedTBN = mat3(normalize(TBN[0]), normalize(TBN[1]), normalize(TBN[2]));

    vec3 normal = normalize(normalizedTBN * (2 * texture(bump, uv).xyz - 1));
    //normal = normalize(normalizedTBN[2]);

    vec3 albedo = texture(albedo, uv).xyz;
    //albedo = vec3(1);
    vec3 diffuse = albedo * max(dot(normal, -sceneData.lightDir), 0.0) * sceneData.lightColour;
    vec3 specular = sceneData.lightColour * pow(max(dot(normal, halfway), 0.0), 8.0) * 0.5;
    vec3 ambient = 0.03 * albedo * sceneData.lightColour;

    vec3 colour = diffuse + specular + ambient;
    fragColour = vec4(colour, 1.0);
}