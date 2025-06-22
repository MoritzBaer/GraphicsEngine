{
    "vertexStage": {
        "shaderName": "debug_lines",
        "boundDescriptorSets": [ 0 ]
    },
    "geometryStage": {
        "shaderName": "debug_lines",
        "boundDescriptorSets": [ 0 ]
    },
    "fragmentStage": {
        "shaderName": "debug_lines",
        "boundDescriptorSets": [ 1 ]
    },
    "topology": "VK_PRIMITIVE_TOPOLOGY_POINT_LIST",
    "polygonMode": "VK_POLYGON_MODE_FILL",
    "cullMode": 2,
    "frontFace": "VK_FRONT_FACE_COUNTER_CLOCKWISE",
    "colourAttachmentFormat": "VK_FORMAT_R16G16B16A16_SFLOAT",
    "depthFormat": "VK_FORMAT_D32_SFLOAT",
    "depthTestEnabled": true,
    "depthWriteEnabled": true,
    "depthCompareOperation": "VK_COMPARE_OP_LESS",
    "blendingEnabled": true,
    "vertexInputBindings": [
        {
            "binding": 0,
            "stride": 48,
            "inputRate": "VK_VERTEX_INPUT_RATE_VERTEX",
            "attributes": [
                {
                    "location": 0,
                    "binding": 0,
                    "format": "VK_FORMAT_R32G32B32_SFLOAT",
                    "offset": 0
                },
                {
                    "location": 1,
                    "binding": 0,
                    "format": "VK_FORMAT_R32G32B32_SFLOAT",
                    "offset": 16
                },
                {
                    "location": 2,
                    "binding": 0,
                    "format": "VK_FORMAT_R32G32B32_SFLOAT",
                    "offset": 32
                }
            ]
        }
    ],
    "descriptorBindings": [
        {
            "set": 0,
            "binding": 0,
            "descriptorType": "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER"
        },
        {
            "set": 1,
            "binding": 0,
            "descriptorType": "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER"
        }
    ]
}