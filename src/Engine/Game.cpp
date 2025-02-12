#include "Game.h"

#include "Core/Script.h"
#include "Core/Time.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "Editor/Display.h"
#include "Graphics/Camera.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/RenderingStrategies/ForwardRendering.h"
#include "Graphics/Transform.h"
#include "Util/AssetParsing/Members.h"

using Engine::Graphics::Shader;
using Engine::Graphics::ShaderType;

using namespace Engine;

#define REGISTER_SHADER_TYPE(Type)                                                                                     \
  assetManager.RegisterAssetType<Shader<ShaderType::Type>>(                                                            \
      new AssetCacheImpl<Shader<ShaderType::Type>>(&shaderCompiler), &shaderCompiler);

struct TextureCache : public AssetManager::AssetCache<Graphics::Texture2D> {
  AssetCacheImpl<Graphics::Texture2D> baseCache;
  Graphics::GPUObjectManager *gpuObjectManager;

  TextureCache(Graphics::GPUObjectManager *gpuObjectManager)
      : baseCache(gpuObjectManager), gpuObjectManager(gpuObjectManager) {
    uint32_t white = 0xFFFFFFFF;
    uint32_t normalUp = 0xFFFF8080;
    baseCache.InsertAsset("white", gpuObjectManager->CreateTexture(Maths::Dimension2(1, 1), &white));
    baseCache.InsertAsset("normalUp", gpuObjectManager->CreateTexture(Maths::Dimension2(1, 1), &normalUp));
  }
  bool HasAsset(char const *assetName) override { return baseCache.HasAsset(assetName); }
  void InsertAsset(char const *assetName, Graphics::Texture2D const &asset) override {
    baseCache.InsertAsset(assetName, asset);
  }
  Graphics::Texture2D LoadAsset(char const *assetName) override { return baseCache.LoadAsset(assetName); }
};

struct SceneCache : public AssetManager::AssetCache<Core::Scene *> {
  AssetCacheImpl<Core::Scene *> baseCache;

  SceneCache() : baseCache() {}
  bool HasAsset(char const *assetName) override { return baseCache.HasAsset(assetName); }
  void InsertAsset(char const *assetName, Core::Scene *const &asset) override {
    baseCache.InsertAsset(assetName, asset);
  }
  Core::Scene *LoadAsset(char const *assetName) override {
    auto pattern = baseCache.LoadAsset(assetName);
    auto copy = new Core::Scene();
    copy->ecs.Copy(&pattern->ecs);
    copy->sceneHierarchy.Rebuild();
    copy->mainCamera = pattern->mainCamera.InOtherECS(&copy->ecs);
    for (auto &[transform] : copy->ecs.FilterEntities<Graphics::Transform>()) {
      if (transform->hierarchy->parent) {
        transform->parent = transform->hierarchy->parent->entity.GetComponent<Graphics::Transform>();
      }
    }
    return copy;
  }
};

Game::Game(const char *name, Engine::Graphics::VulkanSuite
#ifdef NDEBUG
           const
#endif
               *vulkan)
    : mainDeletionQueue(), assetManager(), vulkan(vulkan), shaderCompiler(&vulkan->instanceManager), prefabs(),
      renderingStrategy(nullptr), renderer(&vulkan->instanceManager, &vulkan->gpuObjectManager), activeScene(nullptr),
      rendering(true), running(true) {
}

void Game::Init() {
  BEGIN_PROFILE_SESSION()
  PROFILE_FUNCTION()

  Core::ECS::RegisterComponent<Editor::Display>();
  Core::ECS::RegisterComponent<Engine::Core::HierarchyComponent>(); // Must be registered before all
                                                                    // HierarchicalComponents
  Core::ECS::RegisterComponent<Engine::Graphics::Transform>();
  Core::ECS::RegisterComponent<Engine::Graphics::MeshRenderer>();
  Core::ECS::RegisterComponent<Engine::Graphics::Camera>();
  Core::ECS::RegisterComponent<Engine::Core::ScriptComponent>();

  auto textureCache = new TextureCache(&vulkan->gpuObjectManager);
  assetManager.RegisterAssetType<Graphics::Texture2D>(textureCache, &vulkan->gpuObjectManager, textureCache);
  REGISTER_SHADER_TYPE(VERTEX);
  REGISTER_SHADER_TYPE(GEOMETRY);
  REGISTER_SHADER_TYPE(FRAGMENT);
  REGISTER_SHADER_TYPE(COMPUTE);
  assetManager.RegisterAssetType<Graphics::Material *>(new AssetCacheImpl<Graphics::Material *>(), &assetManager);
  assetManager.RegisterAssetType<Core::Entity>(new AssetCacheImpl<Core::Entity>(&prefabs), &prefabs, &assetManager);
  assetManager.RegisterAssetType<Graphics::RenderingStrategies::CompiledEffect>(
      new AssetCacheImpl<Graphics::RenderingStrategies::CompiledEffect>(&vulkan->instanceManager),
      &vulkan->instanceManager, &assetManager);
  assetManager.RegisterAssetType<Graphics::RenderingStrategies::ComputeBackground *>(
      new AssetCacheImpl<Graphics::RenderingStrategies::ComputeBackground *>(), &vulkan->instanceManager,
      &assetManager);
  assetManager.RegisterAssetType<Graphics::Pipeline *>(
      new AssetCacheImpl<Graphics::Pipeline *>(&vulkan->instanceManager), &vulkan->instanceManager, &assetManager);
  assetManager.RegisterAssetType<Graphics::AllocatedMesh *>(
      new AssetCacheImpl<Graphics::AllocatedMesh *>(&vulkan->gpuObjectManager), &vulkan->gpuObjectManager);
  assetManager.RegisterAssetType<Core::Scene *>(new SceneCache(), &assetManager);

  renderingStrategy = new Engine::Graphics::RenderingStrategies::ForwardRendering(
      &vulkan->instanceManager, &vulkan->gpuObjectManager,
      assetManager.LoadAsset<Engine::Graphics::RenderingStrategies::ComputeBackground *>("nightsky"));
  renderer.SetRenderingStrategy(renderingStrategy);

  WRITE_PROFILE_SESSION("Init")
}

void Game::CalculateFrame() {
  {
    PROFILE_FUNCTION()

    Engine::Time::Update();

    Engine::WindowManager::HandleEventsOnAllWindows();

    auto scripts = activeScene->ecs.FilterEntities<Engine::Core::ScriptComponent>();
    for (auto &[scriptComponent] : scripts) {
      scriptComponent->UpdateScripts(Engine::Time::deltaTime);
    }

    if (rendering) {
      auto renderersWithTransforms =
          activeScene->ecs
              .FilterEntities<Engine::Graphics::MeshRenderer, Engine::Graphics::Transform, Editor::Display>();
      std::vector<Engine::Graphics::MeshRenderer const *> meshRenderers{};
      meshRenderers.reserve(renderersWithTransforms.size());
      for (auto &[meshRenderer, transform, __] : renderersWithTransforms) {
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

void Game::Start() { Engine::Time::Update(); }

Game::~Game() {
  BEGIN_PROFILE_SESSION()
  PROFILE_FUNCTION()

  vulkan->instanceManager.WaitUntilDeviceIdle();
  delete renderingStrategy;
  mainDeletionQueue.Flush();
  WRITE_PROFILE_SESSION("Cleanup")
}
