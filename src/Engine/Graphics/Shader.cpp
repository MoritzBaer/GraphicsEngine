#include "Shader.h"
#include "../Debug/Logging.h"
#include "InstanceManager.h"

namespace Engine::Graphics
{
    template <ShaderType T> 
    const shaderc_shader_kind KIND_CONVERSION;

    ShaderCompiler::ShaderCompiler() { }
    ShaderCompiler::~ShaderCompiler() { }
    void ShaderCompiler::Init() { instance = new ShaderCompiler(); }
    void ShaderCompiler::Cleanup() { delete instance; }

    Shader ShaderCompiler::_CompileShaderCode(std::vector<char> const &shaderCode, ShaderType type)
    {
        Shader result;

        auto compilationResult = compiler.CompileGlslToSpv(shaderCode.data(),shaderCode.size(), static_cast<shaderc_shader_kind>(type), "Dummy String", options);

        ENGINE_ASSERT(compilationResult.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success, "Shader compilation error: {}", compilationResult.GetErrorMessage())

        std::vector<uint32_t> bytecode (compilationResult.cbegin(), compilationResult.cend());

        VkShaderModuleCreateInfo shaderModuleCreateInfo {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = static_cast<uint32_t>(bytecode.size()) * sizeof(uint32_t),
            .pCode = bytecode.data()
        };  

        InstanceManager::CreateShaderModule(&shaderModuleCreateInfo, &result.shaderModule);

        mainDeletionQueue.Push(&result);   

        return result;
    }

    inline void Shader::Destroy()
    {
        InstanceManager::DestroyShaderModule(shaderModule);
    }

    DescriptorLayoutBuilder::DescriptorLayoutBuilder(std::initializer_list<DescriptorType> const &descriptors)
    {
        
    }
} // namespace Engine::Graphics