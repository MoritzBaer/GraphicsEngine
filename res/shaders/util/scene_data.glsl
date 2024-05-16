#ifndef SCENEDATA_BINDING
#define SCENEDATA_BINDING 0
#endif

#ifndef SCENEDATA_SET
#define SCENEDATA_SET 0
#endif

layout(set = SCENEDATA_SET, binding = SCENEDATA_BINDING) uniform SceneData {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 lightDir;
    vec3 lightColour;
 } sceneData;