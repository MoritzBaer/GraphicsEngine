#include "AssetManager.h"

#include "Util/FileIO.h"
#include "Debug/Profiling.h"

#define RESOURCE_PATH "res/"
#define SHADER_PATH "shaders/"

#define CONSTRUCT_PATHS(a, b) a b

#define _RETURN_ASSET(key, table, constructor)                                                                  \
    if (instance->table.find(key) == instance->table.end()) { instance->table.insert({key, constructor}); }     \
    return instance->table.at(key);

namespace Engine
{
    AssetManager::AssetManager() : loadedShaders() { }
    AssetManager::~AssetManager() { }
    void AssetManager::Init() { instance = new AssetManager(); }
    void AssetManager::Cleanup() { delete instance; }

    Graphics::Shader AssetManager::LoadShader(char const *shaderName, Graphics::ShaderType shaderType)
    {
        PROFILE_FUNCTION()
        char dirPath[] = CONSTRUCT_PATHS(RESOURCE_PATH, SHADER_PATH);
        char * filePath = static_cast<char *>(malloc(strlen(shaderName) * sizeof(char) + sizeof(dirPath)));
        strcpy(filePath, dirPath);
        strcat(filePath, shaderName); 
        _RETURN_ASSET(shaderName, loadedShaders, Graphics::ShaderCompiler::CompileShaderCode(Util::FileIO::ReadFile(filePath), shaderType))
    }
    
    Graphics::Shader AssetManager::LoadShaderWithInferredType(char const * shaderName)
    {
        // TODO: Infer shader type from ending of file
        return LoadShader(shaderName, Graphics::ShaderType::COMPUTE);
    }
    
} // namespace Engine