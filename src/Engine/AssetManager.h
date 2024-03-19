#pragma once

#include "Util/Macros.h"
#include <unordered_map>
#include <string>
#include "Graphics/Shader.h"
#include "Graphics/MeshRenderer.h"
#include "Core/ECS.h"

namespace Engine
{
    class AssetManager
    {
        _SINGLETON(AssetManager)

        std::unordered_map<std::string, Graphics::Shader> loadedShaders;
        std::unordered_map<std::string, Graphics::Material const *> loadedMaterials;
        std::unordered_map<std::string, Graphics::Mesh> loadedMeshes;

    public:
        static Graphics::Shader LoadShader(char const * shaderName, Graphics::ShaderType shaderType);
        inline static Graphics::Shader LoadShader(std::string const & shaderName, Graphics::ShaderType shaderType) { return LoadShader(shaderName.c_str(), shaderType); }
        static Graphics::Shader LoadShaderWithInferredType(char const * shaderName);
        inline static Graphics::Shader LoadShaderWithInferredType(std::string const & shaderName) { LoadShaderWithInferredType(shaderName.c_str()); };
        
        static Graphics::Mesh LoadMeshFromOBJ(char const * meshName);
        static Graphics::Material * LoadMaterial(char const * materialName);
        static Core::Entity LoadPrefab(char const * prefabName);
    };
    
} // namespace Engine
