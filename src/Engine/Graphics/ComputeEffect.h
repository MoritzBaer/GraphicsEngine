#pragma once

#include "Shader.h"
#include "vulkan/vulkan.h"
#include <string>

namespace Engine::Graphics {
template <typename T> struct ComputeEffect {
  std::string name;
  Shader<ShaderType::COMPUTE> effectShader;
  T constants;
};
struct ComputePushConstants {
  Maths::Vector4 data1, data2, data3, data4;
};
} // namespace Engine::Graphics
