#pragma once

#include "vulkan/vulkan.h"
#include <string>

namespace Engine::Graphics
{
    template <typename T>
    struct ComputeEffect
    {
        std::string name;

        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        T constants;
    };
} // namespace Engine::Graphics
