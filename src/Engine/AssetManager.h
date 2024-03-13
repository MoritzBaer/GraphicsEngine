#pragma once

#include "Util/Macros.h"
#include <unordered_map>
#include <string>
#include "Graphics/Shader.h"

namespace Engine
{
    class AssetManager
    {
        _SINGLETON(AssetManager)

        std::unordered_map<std::string, Graphics::Shader> loadedShaders;

    public:
        static Graphics::Shader LoadShader(char const * shaderName, Graphics::ShaderType shaderType);
        inline static Graphics::Shader LoadShader(std::string const & shaderName, Graphics::ShaderType shaderType) { return LoadShader(shaderName.c_str(), shaderType); }
        static Graphics::Shader LoadShaderWithInferredType(char const * shaderName);
        inline static Graphics::Shader LoadShaderWithInferredType(std::string const & shaderName) { LoadShaderWithInferredType(shaderName.c_str()); };

        inline static void UnloadShaders() { for(auto [_, shader] : instance->loadedShaders) { shader.Destroy(); } }
    };
    
} // namespace Engine
