#include "Shader.h"
#include "Debug/Logging.h"
#include "InstanceManager.h"
#include <algorithm>

namespace Engine::Graphics {

ShaderCompiler::ShaderCompiler() {}
ShaderCompiler::~ShaderCompiler() {}
void ShaderCompiler::Init() { instance = new ShaderCompiler(); }
void ShaderCompiler::Cleanup() { delete instance; }

Shader ShaderCompiler::_CompileShaderCode(std::vector<char> const &shaderCode, ShaderType type) {
  Shader result;
  result.type = type;

  auto compilationResult = compiler.CompileGlslToSpv(shaderCode.data(), shaderCode.size(), ConvertToShaderKind(type),
                                                     "Dummy String", options);

  ENGINE_ASSERT(compilationResult.GetCompilationStatus() ==
                    shaderc_compilation_status::shaderc_compilation_status_success,
                "Shader compilation error: {}", compilationResult.GetErrorMessage())

  std::vector<uint32_t> bytecode(compilationResult.cbegin(), compilationResult.cend());

  VkShaderModuleCreateInfo shaderModuleCreateInfo{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                                  .codeSize = static_cast<uint32_t>(bytecode.size()) * sizeof(uint32_t),
                                                  .pCode = bytecode.data()};

  InstanceManager::CreateShaderModule(&shaderModuleCreateInfo, &result.shaderModule);

  return result;
}

inline void Shader::Destroy() const { InstanceManager::DestroyShaderModule(shaderModule); }

VkPipelineShaderStageCreateInfo Shader::GetStageInfo() const {
  return {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = ConvertToShaderStage(type),
          .module = shaderModule,
          .pName = "main"};
}

} // namespace Engine::Graphics