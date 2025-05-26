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

struct AlbedoAndBumpData : public MaterialInstanceData {
  std::string albedoTexture;
  std::string bumpTexture;
  Maths::Vector3 hue{1.0f, 1.0f, 1.0f};
  float specularStrength; // TODO: Extract into Phong
  float phongExponent;
};

} // namespace Engine

JSON_ENUM(VkPrimitiveTopology, VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
          VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
          VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
JSON_ENUM(VkPolygonMode, VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT,
          VK_POLYGON_MODE_FILL_RECTANGLE_NV);
JSON_ENUM(VkFrontFace, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FRONT_FACE_CLOCKWISE);
JSON_ENUM(
    VkFormat, VK_FORMAT_UNDEFINED, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8_UNORM, VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16B16A16_SFLOAT,VK_FORMAT_R32_SFLOAT,VK_FORMAT_R32G32_SFLOAT,VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_R8_UINT) // Add more formats if/when needed. Can't do all because that would be too much block nesting.
JSON_ENUM(VkCompareOp, VK_COMPARE_OP_NEVER, VK_COMPARE_OP_LESS, VK_COMPARE_OP_EQUAL, VK_COMPARE_OP_LESS_OR_EQUAL,
          VK_COMPARE_OP_GREATER, VK_COMPARE_OP_NOT_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL, VK_COMPARE_OP_ALWAYS)
JSON_ENUM(VkDescriptorType, VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
          VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
          VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
          VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM,
          VK_DESCRIPTOR_TYPE_MUTABLE_EXT, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, VK_DESCRIPTOR_TYPE_MUTABLE_VALVE)
JSON_ENUM(VkVertexInputRate, VK_VERTEX_INPUT_RATE_VERTEX, VK_VERTEX_INPUT_RATE_INSTANCE)
JSON(Engine::PushConstant, FIELDS(size, offset))
JSON(Engine::ShaderStage, FIELDS(shaderName, pushConstants, boundDescriptorSets))
JSON(Engine::DescriptorBinding, FIELDS(set, binding, descriptorType));
JSON(Engine::VertexInputAttribute, FIELDS(location, binding, format, offset))
JSON(Engine::VertexInputBinding, FIELDS(binding, stride, inputRate, attributes))

JSON(Engine::PipelineDSO, FIELDS(vertexStage, fragmentStage, geometryStage, topology, polygonMode, cullMode, frontFace,
                                 colourAttachmentFormat, depthFormat, depthTestEnabled, depthWriteEnabled,
                                 depthCompareOperation, blendingEnabled, descriptorBindings, vertexInputBindings));

JSON(Engine::MaterialDSO, FIELDS(pipelineName, instanceData));

JSON(Engine::AlbedoAndBumpData, FIELDS(albedoTexture, bumpTexture, hue, specularStrength, phongExponent));

JSON(Engine::MaterialInstanceData *, SUBTYPES(Engine::AlbedoAndBumpData));