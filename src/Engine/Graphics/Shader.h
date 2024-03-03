#pragma once

#include "shaderc/shaderc.hpp"
#include "../Util/Macros.h"
#include "../Util/DeletionQueue.h"

namespace Engine::Graphics
{
    class ShaderCompiler;

    class Shader : public Destroyable
    {
    private:
        friend class ShaderCompiler;
        VkShaderModule shaderModule;
    public:
        void Destroy();
    };

    enum class ShaderType {
        VERTEX = shaderc_vertex_shader,
        GEOMETRY = shaderc_geometry_shader,
        FRAGMENT = shaderc_fragment_shader,
        COMPUTE = shaderc_compute_shader
    };

    class ShaderCompiler {
        _SINGLETON(ShaderCompiler)
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        // TODO: Write includer

        Shader _CompileShaderCode(std::vector<char> const &shaderCode, ShaderType type);

        public:
        static inline Shader CompileShaderCode(std::vector<char> const &shaderCode, ShaderType type) { return instance->_CompileShaderCode(shaderCode, type); };
    };

    enum class DescriptorType {

    };

    class DescriptorLayoutBuilder {
        std::vector<VkDescriptorSetLayoutBinding> bindings { };

    public: 
        void AddBinding(uint32_t binding, VkDescriptorType type);
        void Clear();
        VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderStages);
    };
} // namespace Engine::Graphics
