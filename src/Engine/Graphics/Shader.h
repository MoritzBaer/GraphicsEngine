#pragma once

#include "Image.h"
#include "Util/DeletionQueue.h"
#include "Util/Macros.h"
#include "shaderc/shaderc.hpp"
#include <deque>
#include <span>

namespace Engine::Graphics {
class ShaderCompiler;

enum class ShaderType {
  VERTEX = shaderc_vertex_shader,
  GEOMETRY = shaderc_geometry_shader,
  FRAGMENT = shaderc_fragment_shader,
  COMPUTE = shaderc_compute_shader
};

class Shader : public Destroyable {
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

  Shader _CompileShaderCode(std::vector<char> const &shaderCode, ShaderType type);

public:
  static inline Shader CompileShaderCode(std::vector<char> const &shaderCode, ShaderType type) {
    return instance->_CompileShaderCode(shaderCode, type);
  };
};

enum class DescriptorType {

};

class DescriptorLayoutBuilder {
  std::vector<VkDescriptorSetLayoutBinding> bindings{};

public:
  void AddBinding(uint32_t binding, VkDescriptorType type);
  void Clear();
  VkDescriptorSetLayout Build(VkShaderStageFlags shaderStages);
};

class DescriptorAllocator {

public:
  struct PoolSizeRatio {
    VkDescriptorType type;
    float ratio;
  };

  void InitPools(uint32_t initialSets, std::span<PoolSizeRatio> poolRatios);
  void ClearDescriptors();
  void DestroyPools();

  VkDescriptorSet Allocate(VkDescriptorSetLayout layout);

private:
  VkDescriptorPool GetPool();
  VkDescriptorPool CreatePool(uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);

  std::vector<PoolSizeRatio> poolRatios;
  std::vector<VkDescriptorPool> fullPools;
  std::vector<VkDescriptorPool> readyPools;
  uint32_t setsPerPool;
};

class DescriptorWriter {
  std::deque<VkDescriptorImageInfo> imageInfos;
  std::deque<VkDescriptorBufferInfo> bufferInfos;
  std::vector<VkWriteDescriptorSet> writes;

public:
  template <uint8_t N>
  // Image must be passed as a pointer to allow subclasses substituting
  void WriteImage(uint32_t binding, Image<N> const &image, VkImageLayout layout, VkDescriptorType type);
  // TODO: Refactor to use Buffer
  void WriteBuffer(uint32_t binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);
  void Clear();
  void UpdateSet(VkDescriptorSet set);
};

template <uint8_t N>
void DescriptorWriter::WriteImage(uint32_t binding, Image<N> const &image, VkImageLayout layout,
                                  VkDescriptorType type) {

  VkDescriptorImageInfo const &imageInfo = imageInfos.emplace_back(image.BindInDescriptor(layout));

  VkWriteDescriptorSet write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = VK_NULL_HANDLE,
      .dstBinding = binding,
      .descriptorCount = 1,
      .descriptorType = type,
      .pImageInfo = &imageInfo,
  };

  writes.push_back(write);
}

} // namespace Engine::Graphics
