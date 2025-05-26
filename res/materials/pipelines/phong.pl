{
    "vertexStage": {
        "shaderName": "phong",
        "pushConstants": [
            {
                "size": 72,
                "offset": 0
            }
        ],
        "boundDescriptorSets": [0]
    },
    "fragmentStage": {
        "shaderName": "phong",
        "boundDescriptorSets": [0,1]
    },
    "topology": "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST",
    "polygonMode": "VK_POLYGON_MODE_FILL",
    "cullMode": 2,
    "frontFace": "VK_FRONT_FACE_COUNTER_CLOCKWISE",
    "colourAttachmentFormat": "VK_FORMAT_R16G16B16A16_SFLOAT",
    "depthFormat": "VK_FORMAT_D32_SFLOAT",
    "depthTestEnabled": true,
    "depthWriteEnabled": true,
    "depthCompareOperation": "VK_COMPARE_OP_LESS",
    "blendingEnabled":true,
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
        },
        {
            "set": 1,
            "binding": 1,
            "descriptorType": "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER"
        }
    ]
}

# ColourAttachmentFormat should be SFLOAT