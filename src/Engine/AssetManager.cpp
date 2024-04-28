#include "AssetManager.h"

#include "Core/SceneHierarchy.h"
#include "Debug/Profiling.h"
#include "Editor/Display.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/TestMaterial.h"
#include "Util/FileIO.h"
#include "Util/Parsing.h"

#include <functional>
#include <stdlib.h>
#include <unordered_map>

#define RESOURCE_PATH "res/"
#define SHADER_PATH "shaders/"
#define MESH_PATH "meshes/"
#define MATERIAL_PATH "materials/"
#define PREFAB_PATH "prefabs/"

#define CONSTRUCT_PATHS(a, b) a b

#define MAKE_FILE_PATH(fileName, directoryName)                                                                        \
  char dirPath[] = CONSTRUCT_PATHS(RESOURCE_PATH, directoryName);                                                      \
  char *filePath = static_cast<char *>(malloc(strlen(fileName) * sizeof(char) + sizeof(dirPath)));                     \
  strcpy(filePath, dirPath);                                                                                           \
  strcat(filePath, fileName);

#define _INSERT_ASSET_IF_NEW(key, table, constructor)                                                                  \
  bool isNew = instance->table.find(key) == instance->table.end();                                                     \
  if (isNew) {                                                                                                         \
    instance->table.insert({key, constructor});                                                                        \
  }

#define _RETURN_ASSET(key, table, constructor)                                                                         \
  _INSERT_ASSET_IF_NEW(key, table, constructor)                                                                        \
  return instance->table[key];

namespace Engine {
class PipelineBuilder {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  VkPipelineRasterizationStateCreateInfo rasterizer;
  VkPipelineColorBlendAttachmentState colourBlendAttachment;
  VkPipelineMultisampleStateCreateInfo multisampling;
  VkPipelineLayout pipelineLayout;
  VkPipelineDepthStencilStateCreateInfo depthStencil;
  VkPipelineRenderingCreateInfo renderInfo;
  VkFormat colourAttachmentformat;

  inline void SetBlendFactors(VkBlendFactor const &srcFactor, VkBlendFactor const &dstFactor);

public:
  enum class BlendMode { ALPHA, ADDITIVE };
  PipelineBuilder &Reset();

  PipelineBuilder() { Reset(); }

  inline PipelineBuilder &SetPipelineLayout(VkPipelineLayout const &layout);
  inline PipelineBuilder &SetShaderStages(Graphics::Shader const &vertexShader, Graphics::Shader const &fragmentShader);
  inline PipelineBuilder &SetInputTopology(VkPrimitiveTopology const &topology);
  inline PipelineBuilder &SetPolygonMode(VkPolygonMode const &polygonMode);
  inline PipelineBuilder &SetCullMode(VkCullModeFlags const &cullMode, VkFrontFace const &frontFace);
  inline PipelineBuilder &SetColourAttachmentFormat(VkFormat const &format);
  inline PipelineBuilder &SetDepthFormat(VkFormat const &format);
  inline PipelineBuilder &DisableDepthTest();
  inline PipelineBuilder &DisableDepthWriting();
  inline PipelineBuilder &SetDepthCompareOperation(VkCompareOp const &compareOp);
  inline PipelineBuilder &EnableBlending(BlendMode const &mode);

  VkPipeline BuildPipeline() const;
  void BuildPipeline(VkPipeline *pipeline) const;
};

inline void PipelineBuilder::SetBlendFactors(VkBlendFactor const &srcFactor, VkBlendFactor const &dstFactor) {
  colourBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colourBlendAttachment.blendEnable = VK_TRUE;
  colourBlendAttachment.srcColorBlendFactor = srcFactor;
  colourBlendAttachment.dstColorBlendFactor = dstFactor;
  colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

PipelineBuilder &PipelineBuilder::Reset() {
  inputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                   .primitiveRestartEnable = VK_FALSE};

  rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .lineWidth = 1.0f};

  colourBlendAttachment = {};

  multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                   .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                   .sampleShadingEnable = VK_FALSE,
                   .minSampleShading = 1.0f,
                   .alphaToCoverageEnable = VK_FALSE,
                   .alphaToOneEnable = VK_FALSE};

  colourBlendAttachment = {.blendEnable = VK_FALSE,
                           .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                             VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

  pipelineLayout = {};
  depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                  .depthTestEnable = VK_TRUE,
                  .depthWriteEnable = VK_TRUE,
                  .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
                  .depthBoundsTestEnable = VK_FALSE,
                  .stencilTestEnable = VK_FALSE,
                  .front = {},
                  .back = {},
                  .minDepthBounds = 0.f,
                  .maxDepthBounds = 1.f};

  renderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &colourAttachmentformat};

  shaderStages.clear();

  return *this;
}

inline PipelineBuilder &PipelineBuilder::SetPipelineLayout(VkPipelineLayout const &layout) {
  pipelineLayout = layout;
  return *this;
}

inline PipelineBuilder &PipelineBuilder::SetShaderStages(Graphics::Shader const &vertexShader,
                                                         Graphics::Shader const &fragmentShader) {
  shaderStages.clear();

  shaderStages.push_back(vertexShader.GetStageInfo());
  shaderStages.push_back(fragmentShader.GetStageInfo());
  return *this;
}

inline PipelineBuilder &PipelineBuilder::SetInputTopology(VkPrimitiveTopology const &topology) {
  inputAssembly.topology = topology;
  return *this;
}

inline PipelineBuilder &PipelineBuilder::SetPolygonMode(VkPolygonMode const &polygonMode) {
  rasterizer.polygonMode = polygonMode;
  return *this;
}

inline PipelineBuilder &PipelineBuilder::SetCullMode(VkCullModeFlags const &cullMode, VkFrontFace const &frontFace) {
  rasterizer.cullMode = cullMode;
  rasterizer.frontFace = frontFace;
  return *this;
}

inline PipelineBuilder &PipelineBuilder::SetColourAttachmentFormat(VkFormat const &format) {
  colourAttachmentformat = format;
  return *this;
}

inline PipelineBuilder &PipelineBuilder::SetDepthFormat(VkFormat const &format) {
  renderInfo.depthAttachmentFormat = format;
  return *this;
}

inline PipelineBuilder &PipelineBuilder::DisableDepthTest() {
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.depthWriteEnable = VK_FALSE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.stencilTestEnable = VK_FALSE;
  return *this;
}

inline PipelineBuilder &PipelineBuilder::DisableDepthWriting() {
  depthStencil.depthWriteEnable = VK_FALSE;
  return *this;
}

inline PipelineBuilder &PipelineBuilder::SetDepthCompareOperation(VkCompareOp const &compareOp) {
  depthStencil.depthCompareOp = compareOp;
  return *this;
}

inline PipelineBuilder &PipelineBuilder::EnableBlending(BlendMode const &mode) {
  switch (mode) {
  case BlendMode::ALPHA:
    SetBlendFactors(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
    break;
  case BlendMode::ADDITIVE:
    SetBlendFactors(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_DST_ALPHA);
    break;
  }
  return *this;
}

VkPipeline PipelineBuilder::BuildPipeline() const {
  VkPipeline newPipeline;
  BuildPipeline(&newPipeline);
  return newPipeline;
}

void PipelineBuilder::BuildPipeline(VkPipeline *pipeline) const {
  VkPipelineViewportStateCreateInfo viewportInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};

  VkPipelineColorBlendStateCreateInfo colourBlendInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                                      .logicOpEnable = VK_FALSE,
                                                      .attachmentCount = 1,
                                                      .pAttachments = &colourBlendAttachment};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{.sType =
                                                           VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  VkDynamicState states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicStateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .dynamicStateCount = 2, .pDynamicStates = states};

  VkGraphicsPipelineCreateInfo pipelineInfo{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                            .pNext = &renderInfo,
                                            .stageCount = static_cast<uint32_t>(shaderStages.size()),
                                            .pStages = shaderStages.data(),
                                            .pVertexInputState = &vertexInputInfo,
                                            .pInputAssemblyState = &inputAssembly,
                                            .pViewportState = &viewportInfo,
                                            .pRasterizationState = &rasterizer,
                                            .pMultisampleState = &multisampling,
                                            .pDepthStencilState = &depthStencil,
                                            .pColorBlendState = &colourBlendInfo,
                                            .pDynamicState = &dynamicStateInfo,
                                            .layout = pipelineLayout};

  Graphics::InstanceManager::CreateGraphicsPipeline(pipelineInfo, pipeline);
}

AssetManager::AssetManager() : loadedShaders(), loadedMaterials(), loadedMeshes(), loadedPipelines() {}
AssetManager::~AssetManager() {}
void AssetManager::Init() { instance = new AssetManager(); }
void AssetManager::Cleanup() { delete instance; }

Graphics::Shader AssetManager::LoadShader(char const *shaderName, Graphics::ShaderType shaderType) {
  PROFILE_FUNCTION()
  MAKE_FILE_PATH(shaderName, SHADER_PATH)
  _INSERT_ASSET_IF_NEW(shaderName, loadedShaders,
                       Graphics::ShaderCompiler::CompileShaderCode(Util::FileIO::ReadFile(filePath), shaderType))
  Graphics::Shader &loadedShader = instance->loadedShaders[shaderName];
  if (isNew) {
    mainDeletionQueue.Push(&loadedShader);
  }
  return loadedShader;
}

Graphics::Shader AssetManager::LoadShaderWithInferredType(char const *shaderName) {
  char const *extension = strrchr(shaderName, '.');

  if (strcmp(extension, ".vert") == 0) {
    return LoadShader(shaderName, Graphics::ShaderType::VERTEX);
  } else if (strcmp(extension, ".frag") == 0) {
    return LoadShader(shaderName, Graphics::ShaderType::FRAGMENT);
  } else if (strcmp(extension, ".comp") == 0) {
    return LoadShader(shaderName, Graphics::ShaderType::COMPUTE);
  } else if (strcmp(extension, ".geom") == 0) {
    return LoadShader(shaderName, Graphics::ShaderType::GEOMETRY);
  }

  ENGINE_ERROR("Could not infer shader type from file extension: {}!", extension);
  return Graphics::Shader();
}

Graphics::Mesh AssetManager::LoadMeshFromOBJ(char const *meshName) {
  MAKE_FILE_PATH(meshName, MESH_PATH)
  auto meshData = Util::FileIO::ReadFile(filePath);
  _RETURN_ASSET(meshName, loadedMeshes, Util::ParseOBJ(meshData.data()))
}

Graphics::AllocatedMesh *AssetManager::LoadMesh(char const *meshName) {
  Graphics::Mesh mesh = LoadMeshFromOBJ(meshName);
  return new Graphics::AllocatedMeshT(mesh); // TODO: Decide if it would be better to store this in a map as well
}

Graphics::Pipeline *ParsePipeline(char const *pipelineData) {
  // Dummy implementation // TODO: implement properly
  Graphics::Shader vertexShader = AssetManager::LoadShader("coloured_triangle_mesh.vert", Graphics::ShaderType::VERTEX);
  Graphics::Shader fragmentShader = AssetManager::LoadShader("coloured_triangle.frag", Graphics::ShaderType::FRAGMENT);
  PipelineBuilder builder = PipelineBuilder();

  VkPipelineLayout layout;

  size_t uniformSize = sizeof(VkDeviceAddress) + sizeof(Maths::Matrix4) + sizeof(Maths::Vector3);

  VkPushConstantRange uniformInfo{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = static_cast<uint32_t>(uniformSize)};

  VkPipelineLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                        .pushConstantRangeCount = 1,
                                        .pPushConstantRanges = &uniformInfo};

  Graphics::InstanceManager::CreatePipelineLayout(&layoutInfo, &layout);

  VkPipeline pipeline = builder.SetPipelineLayout(layout)
                            .SetShaderStages(vertexShader, fragmentShader)
                            .SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                            .SetPolygonMode(VK_POLYGON_MODE_FILL)
                            .SetColourAttachmentFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
                            .SetDepthFormat(VK_FORMAT_D32_SFLOAT)
                            .SetDepthCompareOperation(VK_COMPARE_OP_LESS_OR_EQUAL)
                            .EnableBlending(PipelineBuilder::BlendMode::ALPHA)
                            .BuildPipeline();

  return new Graphics::Pipeline(layout, pipeline);
}

Graphics::Material *ParseMAT(char const *materialData) {
  auto pl = AssetManager::LoadPipeline("dummy");
  return new Graphics::TestMaterial(pl, Maths::Vector3{0.3, 0.9, 0.5});
}

Graphics::Material *AssetManager::LoadMaterial(char const *materialName) {
  MAKE_FILE_PATH(materialName, MATERIAL_PATH)
  // auto materialData = Util::FileIO::ReadFile(filePath);
  char const *materialData = "dummy";
  //_INSERT_ASSET_IF_NEW(materialName, loadedMaterials, ParseMAT(materialData.data()))
  bool isNew = instance->loadedMaterials.find(materialName) == instance->loadedMaterials.end();
  if (isNew) {
    instance->loadedMaterials.insert({materialName, ParseMAT(materialData)});
  }
  Graphics::Material const *loadedMaterial = instance->loadedMaterials[materialName];
  return new Graphics::TestMaterial(loadedMaterial);
}

Graphics::Pipeline const *AssetManager::LoadPipeline(char const *pipelineName) {
  _INSERT_ASSET_IF_NEW(pipelineName, loadedPipelines, ParsePipeline(pipelineName));
  auto p = instance->loadedPipelines[pipelineName];
  if (isNew) {
    mainDeletionQueue.Push(p);
  }
  return p;
}

Core::Entity AssetManager::LoadPrefab(char const *prefabName) {
  MAKE_FILE_PATH(prefabName, PREFAB_PATH);

  auto prefabData = Util::FileIO::ReadFile(filePath);
  const char *dataString = prefabData.data();
  auto e = Util::ParseEntity(dataString);
  Core::SceneHierarchy::BuildHierarchy();
  return e;
}

} // namespace Engine