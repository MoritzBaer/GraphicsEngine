#pragma once

#include "InstanceManager.h"
#include "Util/Macros.h"
#include "shaderc/shaderc.hpp"
#include <deque>
#include <span>

namespace Engine::Graphics {
class ShaderCompiler;

enum class ShaderType { VERTEX, FRAGMENT, COMPUTE, GEOMETRY, NUMBER_OF_TYPES };

template <ShaderType Type> class Shader {
private:
  friend class ShaderCompiler;

  // TODO: store entry point
  VkShaderModule shaderModule;

public:
  Shader() {}
  VkPipelineShaderStageCreateInfo GetStageInfo() const;
};

class ShaderCompiler {
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  InstanceManager const *instanceManager;

  void AssertPreprocessingWorked(shaderc_compilation_status status, const char *shaderName, const char *message) const;
  void AssertCompilationWorked(shaderc_compilation_status status, const char *shaderName, const char *message) const;

public:
  ShaderCompiler(InstanceManager const *instanceManager);

  template <ShaderType Type>
  inline Shader<Type> *CompileShaderCode(const char *shaderName, std::string const &shaderCode) const;
  template <ShaderType Type>
  inline void RecompileShaderCode(const char *shaderName, std::string const &shaderCode, Shader<Type> &shader) const;
  template <ShaderType Type> void DestroyShader(Shader<Type> *&shader) const;
};

template <ShaderType Type> struct StageConstants {
  inline static constexpr VkShaderStageFlagBits stageFlags = 0;
  inline static constexpr shaderc_shader_kind kind = static_cast<shaderc_shader_kind>(-1);
};

template <> struct StageConstants<ShaderType::COMPUTE> {
  inline static constexpr VkShaderStageFlagBits stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  inline static constexpr shaderc_shader_kind kind = shaderc_compute_shader;
};
template <> struct StageConstants<ShaderType::VERTEX> {
  inline static constexpr VkShaderStageFlagBits stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  inline static constexpr shaderc_shader_kind kind = shaderc_vertex_shader;
};
template <> struct StageConstants<ShaderType::GEOMETRY> {
  inline static constexpr VkShaderStageFlagBits stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;
  inline static constexpr shaderc_shader_kind kind = shaderc_geometry_shader;
};
template <> struct StageConstants<ShaderType::FRAGMENT> {
  inline static constexpr VkShaderStageFlagBits stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  inline static constexpr shaderc_shader_kind kind = shaderc_fragment_shader;
};

template <ShaderType Type> inline VkPipelineShaderStageCreateInfo Shader<Type>::GetStageInfo() const {
  return {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = StageConstants<Type>::stageFlags,
          .module = shaderModule,
          .pName = "main"};
}

template <ShaderType Type> inline void ShaderCompiler::DestroyShader(Shader<Type> *&shader) const {
  instanceManager->DestroyShaderModule(shader->shaderModule);
}

template <ShaderType Type>
inline Shader<Type> *ShaderCompiler::CompileShaderCode(const char *shaderName, std::string const &shaderCode) const {
  auto result = new Shader<Type>();

  auto preprocessedCode = compiler.PreprocessGlsl(shaderCode, StageConstants<Type>::kind, shaderName, options);
  AssertPreprocessingWorked(preprocessedCode.GetCompilationStatus(), shaderName,
                            preprocessedCode.GetErrorMessage().c_str());

  auto compilationResult = compiler.CompileGlslToSpv(std::string(preprocessedCode.begin(), preprocessedCode.end()),
                                                     StageConstants<Type>::kind, shaderName, options);

  AssertCompilationWorked(compilationResult.GetCompilationStatus(), shaderName,
                          compilationResult.GetErrorMessage().c_str());

  std::vector<uint32_t> bytecode(compilationResult.cbegin(), compilationResult.cend());

  VkShaderModuleCreateInfo shaderModuleCreateInfo{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                                  .codeSize = static_cast<uint32_t>(bytecode.size()) * sizeof(uint32_t),
                                                  .pCode = bytecode.data()};

  instanceManager->CreateShaderModule(&shaderModuleCreateInfo, &result->shaderModule);

  return result;
}

template <ShaderType Type>
inline void ShaderCompiler::RecompileShaderCode(const char *shaderName, std::string const &shaderCode,
                                                Shader<Type> &shader) const {
  auto temp = CompileShaderCode<Type>(shaderName, shaderCode);
  DestroyShader(shader);
  shader = temp;
}

} // namespace Engine::Graphics
