#pragma once

#include "Image.h"
#include "Util/DeletionQueue.h"
#include "Util/Macros.h"
#include "shaderc/shaderc.hpp"
#include <deque>
#include <span>

namespace Engine::Graphics {
class ShaderCompiler;

enum class ShaderType { VERTEX, FRAGMENT, COMPUTE, GEOMETRY, NUMBER_OF_TYPES };

class Shader : public ConstDestroyable {
private:
  friend class ShaderCompiler;

  ShaderType type;
  // TODO: store entry point
  VkShaderModule shaderModule;

public:
  void Destroy() const;
  VkPipelineShaderStageCreateInfo GetStageInfo() const;
};

class ShaderCompiler {
  _SINGLETON(ShaderCompiler)
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  // TODO: Write includer

  Shader _CompileShaderCode(const char *shaderName, std::vector<char> const &shaderCode, ShaderType type);

public:
  static inline Shader CompileShaderCode(const char *shaderName, std::vector<char> const &shaderCode, ShaderType type) {
    return instance->_CompileShaderCode(shaderName, shaderCode, type);
  };
};

inline shaderc_shader_kind ConvertToShaderKind(ShaderType const &type) {
  switch (type) {
  case ShaderType::COMPUTE:
    return shaderc_compute_shader;
  case ShaderType::VERTEX:
    return shaderc_vertex_shader;
  case ShaderType::GEOMETRY:
    return shaderc_geometry_shader;
  case ShaderType::FRAGMENT:
    return shaderc_fragment_shader;
  default:
    return static_cast<shaderc_shader_kind>(-1);
  }
}

inline VkShaderStageFlagBits ConvertToShaderStage(ShaderType const &type) {
  switch (type) {
  case ShaderType::COMPUTE:
    return VK_SHADER_STAGE_COMPUTE_BIT;
  case ShaderType::VERTEX:
    return VK_SHADER_STAGE_VERTEX_BIT;
  case ShaderType::GEOMETRY:
    return VK_SHADER_STAGE_GEOMETRY_BIT;
  case ShaderType::FRAGMENT:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  default:
    return static_cast<VkShaderStageFlagBits>(-1);
  }
}

} // namespace Engine::Graphics
