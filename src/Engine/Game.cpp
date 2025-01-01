#include "Game.h"

#include "Core/Time.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "Editor/Display.h"
#include "Graphics/Camera.h"
#include "Graphics/ComputeEffect.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/RenderingStrategies/ForwardRendering.h"
#include "Graphics/Transform.h"

using Engine::Graphics::ComputeEffect;
using Engine::Graphics::ComputePushConstants;
using Engine::Graphics::Shader;
using Engine::Graphics::ShaderType;

std::vector<ComputeEffect<ComputePushConstants>> InitializedComputeEffects(Engine::AssetManager &assetManager) {
  PROFILE_FUNCTION()
  std::vector<ComputeEffect<ComputePushConstants>> backgroundEffects = {
      ComputeEffect<ComputePushConstants>{.name = "gradient_colour",
                                          .constants{.data1 = {1, 0, 0, 1}, .data2 = {0, 0, 1, 1}}},
      ComputeEffect<ComputePushConstants>{.name = "sky", .constants{.data1 = {0.02f, 0, 0.05f, 0.99f}}},
  };

  for (auto &effect : backgroundEffects) {
    effect.effectShader = assetManager.LoadAsset<Shader<ShaderType::COMPUTE>>(effect.name.c_str());
  }

  return backgroundEffects;
}

using namespace Engine;

Game::Game(const char *name)
    : mainWindow(Engine::WindowManager::CreateWindow({1600, 900}, name)), mainDeletionQueue(),
      instanceManager(name, mainWindow), assetManager(&gpuObjectManager, &ecs, &shaderCompiler, &instanceManager),
      shaderCompiler(&instanceManager), ecs(), memoryAllocator(instanceManager),
      gpuObjectManager(&instanceManager, &memoryAllocator),
      renderer({1600, 900}, &instanceManager, &gpuObjectManager, InitializedComputeEffects(assetManager)),
      sceneHierarchy(&ecs), rendering(true), running(true) {}

void Game::Init() {
  BEGIN_PROFILE_SESSION()
  PROFILE_FUNCTION()

  ecs.RegisterComponent<Editor::Display>(); // Must be at the top so name is displayed above other
                                            // components
  ecs.RegisterComponent<Engine::Graphics::Transform>();
  ecs.RegisterComponent<Engine::Graphics::MeshRenderer>();
  ecs.RegisterComponent<Engine::Graphics::Camera>();

  mainWindow->SetCloseCallback([this]() { running = false; });
  mainWindow->SetMinimizeCallback([this]() { rendering = false; });
  mainWindow->SetRestoreCallback([this]() { rendering = true; });
  mainWindow->SetResizeCallback(
      [this](Engine::Maths::Dimension2 newWindowSize) { renderer.SetWindowSize(newWindowSize); });

  renderer.SetRenderingStrategy(new Engine::Graphics::RenderingStrategies::ForwardRendering());

  SpecializedInit();

  sceneHierarchy.BuildHierarchy();

  Engine::Time::Update();
  WRITE_PROFILE_SESSION("Init")
}

void Game::CalculateFrame() {
  {
    PROFILE_FUNCTION()

    Engine::Time::Update();

    Engine::WindowManager::HandleEventsOnAllWindows();

    if (rendering) {
      auto renderersWithTransforms = ecs.FilterEntities<Engine::Graphics::MeshRenderer, Engine::Graphics::Transform>();
      for (auto &[_, transform] : renderersWithTransforms) {
        if (transform->parent)
          transform = transform->parent->entity.GetComponent<Engine::Graphics::Transform>();
        transform->rotation = (Engine::Maths::Transformations::RotateAroundAxis(Engine::Maths::Vector3(0, 1, 0),
                                                                                Engine::Time::deltaTime * 0.2f) *
                               transform->rotation)
                                  .Normalized();
        transform->position.y() = std::sin(Engine::Time::time) * 0.1f;
      }
      std::vector<Engine::Graphics::MeshRenderer const *> meshRenderers(renderersWithTransforms.size());
      std::transform(renderersWithTransforms.begin(), renderersWithTransforms.end(), meshRenderers.begin(),
                     [](auto const &t) { return std::get<0>(t); });
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

Game::~Game() {
  BEGIN_PROFILE_SESSION()
  PROFILE_FUNCTION()

  instanceManager.WaitUntilDeviceIdle();
  mainDeletionQueue.Flush();
  WRITE_PROFILE_SESSION("Cleanup")
}
