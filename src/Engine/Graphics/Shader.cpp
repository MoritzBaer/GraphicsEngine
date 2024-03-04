#include "Shader.h"
#include "../Debug/Logging.h"
#include "InstanceManager.h"
#include <algorithm>

namespace Engine::Graphics
{
    inline shaderc_shader_kind ConvertToShaderKind(ShaderType const & type) {
        switch (type)
        {
            case ShaderType::COMPUTE:   return shaderc_compute_shader;
            case ShaderType::VERTEX:    return shaderc_vertex_shader;
            case ShaderType::GEOMETRY:  return shaderc_geometry_shader;
            case ShaderType::FRAGMENT:  return shaderc_fragment_shader;
            default:                    return static_cast<shaderc_shader_kind>(-1);
        }
    }

    inline VkShaderStageFlagBits ConvertToShaderStage(ShaderType const & type) {
        switch (type)
        {
            case ShaderType::COMPUTE:   return VK_SHADER_STAGE_COMPUTE_BIT;
            case ShaderType::VERTEX:    return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderType::GEOMETRY:  return VK_SHADER_STAGE_GEOMETRY_BIT;
            case ShaderType::FRAGMENT:  return VK_SHADER_STAGE_FRAGMENT_BIT;
            default:                    return static_cast<VkShaderStageFlagBits>(-1);
        }
    }

    ShaderCompiler::ShaderCompiler() { }
    ShaderCompiler::~ShaderCompiler() { }
    void ShaderCompiler::Init() { instance = new ShaderCompiler(); }
    void ShaderCompiler::Cleanup() { delete instance; }

    Shader ShaderCompiler::_CompileShaderCode(std::vector<char> const &shaderCode, ShaderType type)
    {
        Shader result;
        result.type = type;

        auto compilationResult = compiler.CompileGlslToSpv(shaderCode.data(),shaderCode.size(), ConvertToShaderKind(type), "Dummy String", options);

        ENGINE_ASSERT(compilationResult.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success, "Shader compilation error: {}", compilationResult.GetErrorMessage())

        std::vector<uint32_t> bytecode (compilationResult.cbegin(), compilationResult.cend());

        VkShaderModuleCreateInfo shaderModuleCreateInfo {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = static_cast<uint32_t>(bytecode.size()) * sizeof(uint32_t),
            .pCode = bytecode.data()
        };  

        InstanceManager::CreateShaderModule(&shaderModuleCreateInfo, &result.shaderModule);

        return result;
    }

    inline void Shader::Destroy()
    {
        InstanceManager::DestroyShaderModule(shaderModule);
    }

    VkPipelineShaderStageCreateInfo Shader::GetStageInfo()
    {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = ConvertToShaderStage(type),
            .module = shaderModule,
            .pName = "main"
        };
    }

    void DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type)
    {
        VkDescriptorSetLayoutBinding newBinding {
            .binding = binding,
            .descriptorType = type,
            .descriptorCount = 1,
        };

        bindings.push_back(newBinding);
    }

    void DescriptorLayoutBuilder::Clear()
    {
        bindings.clear();
    }

    VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkShaderStageFlags shaderStages)
    {
        for (auto & binding : bindings) { binding.stageFlags |= shaderStages; }

        VkDescriptorSetLayoutCreateInfo layoutInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
        };

        VkDescriptorSetLayout layout;

        InstanceManager::CreateDescriptorSetLayout(&layoutInfo, &layout);

        return layout;
    }

    void DescriptorAllocator::InitPool(uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
    {
        std::vector<VkDescriptorPoolSize> poolSizes(poolRatios.size());
        std::transform(poolRatios.begin(), poolRatios.end(), poolSizes.begin(), [maxSets](PoolSizeRatio ratio){ return VkDescriptorPoolSize { .type = ratio.type, .descriptorCount = static_cast<uint32_t>(ratio.ratio * maxSets) }; });
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = maxSets,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };

        InstanceManager::CreateDescriptorPool(&descriptorPoolCreateInfo, &descriptorPool);
    }

    void DescriptorAllocator::ClearDescriptors() { InstanceManager::ClearDescriptorPool(descriptorPool); }

    void DescriptorAllocator::DestroyPool() { InstanceManager::DestroyDescriptorPool(descriptorPool); }

    VkDescriptorSet DescriptorAllocator::Allocate(VkDescriptorSetLayout layout)
    {
        VkDescriptorSet result;
        InstanceManager::AllocateDescriptorSets(layout, descriptorPool, &result);
        return result;
    }

} // namespace Engine::Graphics