#pragma once

#include "AssetManager.h"
#include "Graphics/InstanceManager.h"
#include "vulkan/vulkan.h"

#include "Core/ECS.h"
#include "Graphics/AllocatedMesh.h"
#include "Graphics/Materials/AlbedoAndBump.h"
#include "Graphics/RenderingStrategies/ComputeBackground.h"

namespace Engine {

// Meshes

template <> struct AssetManager::LoaderMembers<Graphics::AllocatedMesh *> {
  Graphics::GPUObjectManager *gpuObjectManager;

  LoaderMembers(Graphics::GPUObjectManager *gpuObjectManager) : gpuObjectManager(gpuObjectManager) {}
};

template <> struct AssetManager::DestroyerMembers<Graphics::AllocatedMesh *> {
  Graphics::GPUObjectManager *gpuObjectManager;

  DestroyerMembers(Graphics::GPUObjectManager *gpuObjectManager) : gpuObjectManager(gpuObjectManager) {}
};

// Materials

template <> struct AssetManager::LoaderMembers<Graphics::Pipeline *> {
  Graphics::InstanceManager const *instanceManager;
  AssetManager *assetManager;

  LoaderMembers(Graphics::InstanceManager const *instanceManager, AssetManager *assetManager)
      : instanceManager(instanceManager), assetManager(assetManager) {}
};

template <> struct AssetManager::DestroyerMembers<Graphics::Pipeline *> {
  Graphics::InstanceManager const *instanceManager;

  DestroyerMembers(Graphics::InstanceManager const *instanceManager) : instanceManager(instanceManager) {}
};

template <Graphics::ShaderType Type> struct AssetManager::LoaderMembers<Graphics::Shader<Type>> {
  Graphics::ShaderCompiler const *shaderCompiler;

  LoaderMembers(Graphics::ShaderCompiler const *compiler) : shaderCompiler(compiler) {}
};

template <Graphics::ShaderType Type> struct AssetManager::DestroyerMembers<Graphics::Shader<Type>> {
  Graphics::ShaderCompiler const *shaderCompiler;

  DestroyerMembers(Graphics::ShaderCompiler const *compiler) : shaderCompiler(compiler) {}
};

template <> struct AssetManager::LoaderMembers<Graphics::Material *> {
  AssetManager *assetManager;

  LoaderMembers(AssetManager *assetManager) : assetManager(assetManager) {}
};
template <> struct AssetManager::DestroyerMembers<Graphics::Material *> {
  DestroyerMembers() {}
};

// Prefabs

template <> struct AssetManager::LoaderMembers<Core::Entity> {
  Core::ECS *ecs;
  AssetManager *assetManager;
  LoaderMembers(Core::ECS *ecs, AssetManager *assetManager) : ecs(ecs), assetManager(assetManager) {}
};

template <> struct AssetManager::DestroyerMembers<Core::Entity> {
  Core::ECS *ecs;
  DestroyerMembers(Core::ECS *ecs) : ecs(ecs) {}
};

// Textures

template <> struct AssetManager::LoaderMembers<Graphics::Texture2D> {
  Graphics::GPUObjectManager *gpuObjectManager;
  AssetManager::AssetCache<Graphics::Texture2D> *textureCache;

  LoaderMembers(Graphics::GPUObjectManager *gpuObjectManager,
                AssetManager::AssetCache<Graphics::Texture2D> *textureCache)
      : gpuObjectManager(gpuObjectManager), textureCache(textureCache) {

    std::vector<uint32_t> errorTextureData(16 * 16, 0xFFFF00FF);
    for (int x = 0; x < 16; x++) {
      for (int y = x % 2; y < 16; y += 2) {
        errorTextureData[x * 16 + y] = 0xFF000000;
      }
    }
    textureCache->InsertAsset("missing",
                              gpuObjectManager->CreateTexture(Maths::Dimension2(16, 16), errorTextureData.data(),
                                                              VK_FILTER_NEAREST, VK_FILTER_NEAREST));
  }
};

template <> struct AssetManager::DestroyerMembers<Graphics::Texture2D> {
  Graphics::GPUObjectManager *gpuObjectManager;

  DestroyerMembers(Graphics::GPUObjectManager *gpuObjectManager) : gpuObjectManager(gpuObjectManager) {}
};

// Backgrounds

template <> struct AssetManager::LoaderMembers<Graphics::RenderingStrategies::CompiledEffect> {

  inline static constexpr VkPushConstantRange pushConstants{
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = sizeof(Engine::Graphics::RenderingStrategies::ComputePushConstants)};

  Graphics::DescriptorLayoutBuilder descriptorLayoutBuilder{nullptr};
  VkDescriptorSetLayout renderBufferDescriptorLayout;
  VkComputePipelineCreateInfo pipelineInfo;
  VkPipelineLayout computePipelineLayout;
  Graphics::InstanceManager const *instanceManager;
  AssetManager *assetManager;

  LoaderMembers(Graphics::InstanceManager const *instanceManager, AssetManager *assetManager)
      : instanceManager(instanceManager), assetManager(assetManager), descriptorLayoutBuilder(instanceManager),
        renderBufferDescriptorLayout(VK_NULL_HANDLE), pipelineInfo{}, computePipelineLayout(VK_NULL_HANDLE) {
    descriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    renderBufferDescriptorLayout = descriptorLayoutBuilder.Build(VK_SHADER_STAGE_COMPUTE_BIT);
    descriptorLayoutBuilder.Clear();

    VkPipelineLayoutCreateInfo computeLayoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                 .setLayoutCount = 1,
                                                 .pSetLayouts = &renderBufferDescriptorLayout,
                                                 .pushConstantRangeCount = 1,
                                                 .pPushConstantRanges = &pushConstants};
    instanceManager->CreatePipelineLayout(&computeLayoutInfo, &computePipelineLayout);

    pipelineInfo = VkComputePipelineCreateInfo{.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                               .layout = computePipelineLayout};
  }
  ~LoaderMembers() {
    instanceManager->DestroyDescriptorSetLayout(renderBufferDescriptorLayout);
    instanceManager->DestroyPipelineLayout(computePipelineLayout);
  }
};

template <> struct AssetManager::DestroyerMembers<Graphics::RenderingStrategies::CompiledEffect> {
  Graphics::InstanceManager const *instanceManager;

  DestroyerMembers(Graphics::InstanceManager const *instanceManager) : instanceManager(instanceManager) {}
};

template <> struct AssetManager::LoaderMembers<Graphics::RenderingStrategies::ComputeBackground> {
  Graphics::InstanceManager const *instanceManager;
  AssetManager *assetManager;

  LoaderMembers(Graphics::InstanceManager const *instanceManager, AssetManager *assetManager)
      : instanceManager(instanceManager), assetManager(assetManager) {}
};

template <> struct AssetManager::DestroyerMembers<Graphics::RenderingStrategies::ComputeBackground> {};

} // namespace Engine