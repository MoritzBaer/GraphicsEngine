#include "Game.h"

#include "Core/Script.h"
#include "Core/Time.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "Graphics/Camera.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/RenderingStrategies/ForwardRendering.h"
#include "Graphics/Transform.h"
#include "Util/AssetParsing/MaterialParsing.h"
#include "Util/AssetParsing/MeshParsing.h"
#include "Util/AssetParsing/MultiUseImplementations.h"
#include "Util/AssetParsing/PrefabParsing.h"
#include "Util/AssetParsing/ShaderParsing.h"
#include "Util/AssetParsing/TextureParsing.h"
#include <thread>

using Engine::Graphics::Shader;
using Engine::Graphics::ShaderType;

using namespace Engine;

#define REGISTER_SHADER_TYPE(Type)                                                                                     \
  assetManager                                                                                                         \
      .RegisterAssetType<Shader<ShaderType::Type> *, ShaderLoader<ShaderType::Type>, ShaderCache<ShaderType::Type>>(   \
          ShaderLoader<Graphics::ShaderType::Type>(&shaderCompiler),                                                   \
          ShaderCache<Graphics::ShaderType::Type>(destroyer));

Game::Game(const char *name, Engine::Graphics::VulkanSuite
#ifdef NDEBUG
           const
#endif
               *vulkan)
    : mainDeletionQueue(), assetManager(), vulkan(vulkan), shaderCompiler(&vulkan->instanceManager),
      renderingStrategy(nullptr), renderer(&vulkan->instanceManager), activeScene(nullptr), rendering(true),
      running(true), clock() {
}

void Game::Init() {
  BEGIN_PROFILE_SESSION()
  PROFILE_FUNCTION()

  Core::ECS::RegisterComponent<Engine::Core::HierarchyComponent>(); // Must be registered before all
                                                                    // HierarchicalComponents
  Core::ECS::RegisterComponent<Engine::Graphics::Transform>();
  Core::ECS::RegisterComponent<Engine::Graphics::MeshRenderer>();
  Core::ECS::RegisterComponent<Engine::Graphics::Camera>();
  Core::ECS::RegisterComponent<Engine::Core::ScriptComponent>();

  assetManager.RegisterAssetType<Graphics::Texture2D, TextureLoader, TextureCache>(
      TextureLoader(&vulkan->gpuObjectManager, &assetManager), TextureCache(&vulkan->gpuObjectManager));
  ShaderDestroyer destroyer = ShaderDestroyer(&shaderCompiler);
  REGISTER_SHADER_TYPE(VERTEX);
  REGISTER_SHADER_TYPE(GEOMETRY);
  REGISTER_SHADER_TYPE(FRAGMENT);
  REGISTER_SHADER_TYPE(COMPUTE);
  assetManager.RegisterAssetType<Graphics::Material *, MaterialLoader, MaterialCache>(MaterialLoader(&assetManager),
                                                                                      MaterialCache());
  assetManager.RegisterAssetType<Core::Entity, EntityManager>(&assetManager);
  assetManager
      .RegisterAssetType<Graphics::RenderingStrategies::CompiledEffect, CompiledEffectLoader, CompiledEffectCache>(
          CompiledEffectLoader(&vulkan->instanceManager, &assetManager), CompiledEffectCache(&vulkan->instanceManager));
  assetManager.RegisterAssetType<Graphics::RenderingStrategies::ComputeBackground *, ComputeBackgroundLoader,
                                 ComputeBackgroundCache>(
      ComputeBackgroundLoader(&vulkan->instanceManager, &assetManager), ComputeBackgroundCache());
  assetManager.RegisterAssetType<Graphics::Pipeline *, PipelineLoader, PipelineCache>(
      PipelineLoader(&assetManager, &vulkan->instanceManager), PipelineCache(&vulkan->instanceManager));
  assetManager.RegisterAssetType<Graphics::AllocatedMesh *, MeshLoader, MeshCache>(
      MeshLoader(&vulkan->gpuObjectManager), MeshCache(&vulkan->gpuObjectManager));
  assetManager.RegisterAssetType<Core::Scene *, SceneManager>(SceneLoader(&assetManager), SceneCache());

  renderingStrategy = new Engine::Graphics::RenderingStrategies::ForwardRendering(
      &vulkan->instanceManager, &vulkan->gpuObjectManager,
      assetManager.LoadAsset<Engine::Graphics::RenderingStrategies::ComputeBackground *>("nightsky"));
  renderer.SetRenderingStrategy(renderingStrategy);

  WRITE_PROFILE_SESSION("Init")
}

void Game::CalculateFrame() {
  {
    PROFILE_FUNCTION()

    clock.Update();

    Engine::WindowManager::HandleEventsOnAllWindows();

    auto scripts = activeScene->ecs.FilterEntities<Engine::Core::ScriptComponent>();
    for (auto &[scriptComponent] : scripts) {
      scriptComponent->UpdateScripts(clock);
    }

    if (rendering) {
      auto renderersWithTransforms =
          activeScene->ecs.FilterEntities<Engine::Graphics::MeshRenderer, Engine::Graphics::Transform>();
      std::vector<Engine::Graphics::MeshRenderer const *> meshRenderers{};
      meshRenderers.reserve(renderersWithTransforms.size());
      for (auto &[meshRenderer, transform] : renderersWithTransforms) {
        if (!transform->HasInactiveParent())
          meshRenderers.push_back(meshRenderer);
      }
      Engine::Graphics::RenderingRequest request{
          .objectsToDraw = meshRenderers,
          .camera = activeScene->mainCamera.GetComponent<Engine::Graphics::Camera>(),
          .sceneData = {.cameraPosition = activeScene->mainCamera.GetComponent<Engine::Graphics::Transform>()->position,
                        .lightDirection = {0, -5, -1.5},
                        .lightColour = {1, 1, 1}}};
      renderer.DrawFrame(request);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

void Game::Start() { clock.Start(); }

Game::~Game() {
  BEGIN_PROFILE_SESSION()
  PROFILE_FUNCTION()

  vulkan->instanceManager.WaitUntilDeviceIdle();
  delete renderingStrategy;
  mainDeletionQueue.Flush();
  WRITE_PROFILE_SESSION("Cleanup")
}
