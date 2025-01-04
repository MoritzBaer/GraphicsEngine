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
  void InsertAsset(char const *assetName, Graphics::Texture2D asset) override {
    baseCache.InsertAsset(assetName, asset);
  }
  Graphics::Texture2D LoadAsset(char const *assetName) override { return baseCache.LoadAsset(assetName); }
};

struct EntityCache : public AssetManager::AssetCache<Core::Entity> {
  AssetCacheImpl<Core::Entity> baseCache;

  EntityCache(Core::ECS *ecs) : baseCache(ecs) {}
  bool HasAsset(char const *assetName) override { return baseCache.HasAsset(assetName); }
  void InsertAsset(char const *assetName, Core::Entity asset) override { baseCache.InsertAsset(assetName, asset); }
  Core::Entity LoadAsset(char const *assetName) override { return baseCache.LoadAsset(assetName).Duplicate(); }
};

Game::Game(const char *name)
    : mainWindow(Engine::WindowManager::CreateWindow({1600, 900}, name)), mainDeletionQueue(),
      instanceManager(name, mainWindow), assetManager(), shaderCompiler(&instanceManager), ecs(),
      memoryAllocator(instanceManager), gpuObjectManager(&instanceManager, &memoryAllocator),
      renderingStrategy(nullptr), renderer({1600, 900}, &instanceManager, &gpuObjectManager), sceneHierarchy(&ecs),
      rendering(true), running(true) {}

void Game::Init() {
  BEGIN_PROFILE_SESSION()
  PROFILE_FUNCTION()

  ecs.RegisterComponent<Editor::Display>(); // TODO: Move to Editor (Will require figuring out a way to have components
                                            // on entities in the scene that are only relevant to the editor)
  ecs.RegisterComponent<Engine::Graphics::Transform>();
  ecs.RegisterComponent<Engine::Graphics::MeshRenderer>();
  ecs.RegisterComponent<Engine::Graphics::Camera>();
  ecs.RegisterComponent<Engine::Core::ScriptComponent>();

  auto textureCache = new TextureCache(&gpuObjectManager);
  assetManager.RegisterAssetType<Graphics::Texture2D>(textureCache, &gpuObjectManager, textureCache);
  REGISTER_SHADER_TYPE(VERTEX);
  REGISTER_SHADER_TYPE(GEOMETRY);
  REGISTER_SHADER_TYPE(FRAGMENT);
  REGISTER_SHADER_TYPE(COMPUTE);
  assetManager.RegisterAssetType<Graphics::Material *>(new AssetCacheImpl<Graphics::Material *>(), &assetManager);
  assetManager.RegisterAssetType<Core::Entity>(new EntityCache(&ecs), &ecs, &assetManager);
  assetManager.RegisterAssetType<Graphics::RenderingStrategies::CompiledEffect>(
      new AssetCacheImpl<Graphics::RenderingStrategies::CompiledEffect>(&instanceManager), &instanceManager,
      &assetManager);
  assetManager.RegisterAssetType<Graphics::RenderingStrategies::ComputeBackground *>(
      new AssetCacheImpl<Graphics::RenderingStrategies::ComputeBackground *>(), &instanceManager, &assetManager);
  assetManager.RegisterAssetType<Graphics::Pipeline *>(new AssetCacheImpl<Graphics::Pipeline *>(&instanceManager),
                                                       &instanceManager, &assetManager);
  assetManager.RegisterAssetType<Graphics::AllocatedMesh *>(
      new AssetCacheImpl<Graphics::AllocatedMesh *>(&gpuObjectManager), &gpuObjectManager);

  mainWindow->SetCloseCallback([this]() { running = false; });
  mainWindow->SetMinimizeCallback([this]() { rendering = false; });
  mainWindow->SetRestoreCallback([this]() { rendering = true; });
  mainWindow->SetResizeCallback(
      [this](Engine::Maths::Dimension2 newWindowSize) { renderer.SetWindowSize(newWindowSize); });

  renderingStrategy = new Engine::Graphics::RenderingStrategies::ForwardRendering(
      &instanceManager, &gpuObjectManager,
      assetManager.LoadAsset<Engine::Graphics::RenderingStrategies::ComputeBackground *>("nightsky"));
  renderer.SetRenderingStrategy(renderingStrategy);

  mainCam = ecs.CreateEntity();
  mainCam.AddComponent<Engine::Graphics::Camera>();
  mainCam.AddComponent<Editor::Display>()->AssignLabel("Main camera");
  auto camTransform = mainCam.AddComponent<Engine::Graphics::Transform>();
  camTransform->position = {0, 0, 5};
  camTransform->LookAt({0, 0, 0});

  WRITE_PROFILE_SESSION("Init")
}

void Game::CalculateFrame() {
  {
    PROFILE_FUNCTION()

    Engine::Time::Update();

    Engine::WindowManager::HandleEventsOnAllWindows();

    auto scripts = ecs.FilterEntities<Engine::Core::ScriptComponent>();
    for (auto &[scriptComponent] : scripts) {
      scriptComponent->UpdateScripts(Engine::Time::deltaTime);
    }

    if (rendering) {
      auto renderersWithTransforms =
          ecs.FilterEntities<Engine::Graphics::MeshRenderer, Engine::Graphics::Transform, Editor::Display>();
      std::vector<Engine::Graphics::MeshRenderer const *> meshRenderers{};
      meshRenderers.reserve(renderersWithTransforms.size());
      for (auto &[meshRenderer, transform, __] : renderersWithTransforms) {
        if (!transform->HasInactiveParent())
          meshRenderers.push_back(meshRenderer);
      }
      Engine::Graphics::RenderingRequest request{
          .objectsToDraw = meshRenderers,
          .camera = mainCam.GetComponent<Engine::Graphics::Camera>(),
          .sceneData = {.cameraPosition = mainCam.GetComponent<Engine::Graphics::Transform>()->position,
                        .lightDirection = {0, -5, -1.5},
                        .lightColour = {1, 1, 1}}};
      renderer.DrawFrame(request);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

void Game::Start() {
  sceneHierarchy.BuildHierarchy();

  Engine::Time::Update();
}

Game::~Game() {
  BEGIN_PROFILE_SESSION()
  PROFILE_FUNCTION()

  instanceManager.WaitUntilDeviceIdle();
  delete renderingStrategy;
  mainDeletionQueue.Flush();
  WRITE_PROFILE_SESSION("Cleanup")
}
