#pragma once

#include "AssetManager.h"
#include "Graphics/Material.h"
#include "MultiUseImplementations.h"

namespace Engine {
struct DescriptorBinding {
  uint32_t set;
  uint32_t binding;
  VkDescriptorType descriptorType;
};

struct PushConstant {
  size_t size, offset;
};

struct ShaderStage {
  std::string shaderName;
  std::vector<PushConstant> pushConstants;
  std::vector<uint8_t> boundDescriptorSets;
};

struct VertexInputAttribute {
  uint32_t location;
  uint32_t binding;
  VkFormat format;
  uint32_t offset;
};

struct VertexInputBinding {
  uint32_t binding;
  uint32_t stride;
  VkVertexInputRate inputRate;
  std::vector<VertexInputAttribute> attributes;
};

struct PipelineDSO {
  ShaderStage vertexStage;
  ShaderStage fragmentStage;
  ShaderStage geometryStage;

  VkPrimitiveTopology topology;
  VkPolygonMode polygonMode;

  VkCullModeFlags cullMode;
  VkFrontFace frontFace;
  VkFormat colourAttachmentFormat;
  VkFormat depthFormat;

  bool depthTestEnabled;
  bool depthWriteEnabled;

  VkCompareOp depthCompareOperation;

  bool blendingEnabled; // TODO: Use blend mode

  std::vector<DescriptorBinding> descriptorBindings;
  std::vector<VertexInputBinding> vertexInputBindings;
};

class PipelineConverter {
  AssetManager *assetManager;
  Graphics::InstanceManager const *instanceManager;

public:
  PipelineConverter(AssetManager *assetManager, Graphics::InstanceManager const *instanceManager)
      : assetManager(assetManager), instanceManager(instanceManager) {}
  Graphics::Pipeline *ConvertDSO(PipelineDSO const &dso) const;
};

class PipelineDestroyer {
  Graphics::InstanceManager const *instanceManager;

public:
  PipelineDestroyer(Graphics::InstanceManager const *instanceManager) : instanceManager(instanceManager) {}
  void DestroyAsset(Graphics::Pipeline *&asset) const;
};

using PipelineLoader = AssetLoaderImpl<Graphics::Pipeline *, PipelineDSO, JsonParser<PipelineDSO>, PipelineConverter>;
using PipelineCache = AssetCacheImpl<Graphics::Pipeline *, PipelineDestroyer>;
using PipelineManager = TypeManagerImpl<Graphics::Pipeline *, PipelineLoader, PipelineCache>;

struct MaterialInstanceData {
  virtual ~MaterialInstanceData() = default;
};

struct MaterialDSO {
  std::string pipelineName;
  MaterialInstanceData *instanceData;
};

class MaterialConverter {
  AssetManager *assetManager;

public:
  MaterialConverter(AssetManager *assetManager) : assetManager(assetManager) {}
  Graphics::Material *ConvertDSO(MaterialDSO const &dso) const;
};

using MaterialLoader = AssetLoaderImpl<Graphics::Material *, MaterialDSO, JsonParser<MaterialDSO>, MaterialConverter>;
using MaterialCache = AssetCacheImpl<Graphics::Material *, OwnedDestroyer<Graphics::Material>>;
using MaterialManager = TypeManagerImpl<Graphics::Material *, MaterialLoader, MaterialCache>;

} // namespace Engine