#pragma once

#include "Core/ECS.h"
#include "Editor/Display.h"
#include "Editor/EditorGUIRenderingStrategy.h"
#include "Editor/ImGUIManager.h"
#include "EntityDetails.h"
#include "Game.h"
#include "Graphics/RenderingStrategies/ComputeBackground.h"
#include "Graphics/RenderingStrategies/ForwardRendering.h"
#include "RenderView.h"
#include "SceneView.h"
#include "WindowedApplication.h"

namespace Editor {
struct GameControl : public Engine::Graphics::ImGUIView {
  bool *runGame;
  GameControl(bool *runGame) : ImGUIView("Game Control"), runGame(runGame) {}

  void DrawContent() override {
    if (ImGui::Button("Start Game")) {
      *runGame = true;
    }
  }
};

struct DebugInfo : public Engine::Graphics::ImGUIView {
  bool showImGuiDemo = false;
  DebugInfo() : ImGUIView("Debug Info") {}

  void DrawContent() override {
    ImGui::Text("FPS: %.1f", 1.0f / Time::deltaTime);
    ImGui::SameLine();
    if (showImGuiDemo) {
      if (ImGui::Button("Hide demo window")) {
        showImGuiDemo = false;
      }
    } else if (ImGui::Button("Show demo window")) {
      showImGuiDemo = true;
    }
    if (showImGuiDemo) {
      ImGui::ShowDemoWindow();
    }
  }
};

template <class GameInstance> struct Editor : public Game {
  GameInstance game;
  bool runGame;
  bool gameInitialized;
  Engine::Graphics::ImGUIManager *imGuiManager;
  GameControl gameControl;
  DebugInfo debugInfo;
  SceneView sceneView;
  RenderView gameView;
  RenderView viewport;

  Engine::Core::Entity selectedEntity;
  EntityDetails entityDetails;

  template <typename... GameArgs>
  Editor(Engine::Graphics::VulkanSuite
#ifdef NDEBUG
         const
#endif
             *vulkan,
         Engine::Graphics::ImGUIManager *imGuiManager, GameArgs &&...gameArgs)
      : Game("Editor", vulkan), game(vulkan, std::forward<GameArgs>(gameArgs)...), gameControl(&runGame),
        runGame(false), gameInitialized(false), imGuiManager(imGuiManager), sceneView(&selectedEntity),
        entityDetails(&selectedEntity), gameView(&vulkan->gpuObjectManager, &vulkan->instanceManager),
        viewport(&vulkan->gpuObjectManager, &vulkan->instanceManager) {
  }

  ~Editor() { vulkan->instanceManager.WaitUntilDeviceIdle(); }

  inline void Init() override {
    imGuiManager->RegisterView(&debugInfo);
    imGuiManager->RegisterView(&gameControl);
    imGuiManager->RegisterView(&sceneView);
    imGuiManager->RegisterView(&entityDetails);
    imGuiManager->RegisterView(&gameView);
    imGuiManager->RegisterView(&viewport);

    Game::Init();
    activeScene = assetManager.LoadAsset<Engine::Core::Scene *>("editor");
    game.Init();
    sceneView.SetSceneHierarchy(&game.activeScene->sceneHierarchy);
    game.renderer.SetRenderResourceProvider(&gameView);
    delete renderingStrategy;
    renderingStrategy = new EditorGUIRenderingStrategy(
        &vulkan->gpuObjectManager,
        assetManager.LoadAsset<Engine::Graphics::RenderingStrategies::ComputeBackground *>("editor"));
    renderer.SetRenderingStrategy(renderingStrategy);
  }

  inline void CalculateFrame() override {
    if (runGame) {
      if (!gameInitialized) {
        game.Start();
        gameInitialized = true;
      }
      game.CalculateFrame();
    }
    imGuiManager->BeginFrame();
    Game::CalculateFrame();
  }
};

template <class GameInstance> class EditorApp : public GameApp<Editor<GameInstance>> {
private:
  Engine::Graphics::ImGUIManager imGuiManager;

public:
  template <typename... GameArgs>
  EditorApp(const char *name, Engine::Maths::Dimension2 const &windowSize, GameArgs &&...gameArgs)
      : GameApp<Editor<GameInstance>>(name, windowSize, &imGuiManager, std::forward<GameArgs>(gameArgs)...),
        imGuiManager(&this->windowedApplication.GetVulkan()->instanceManager) {
    imGuiManager.InitImGUIOnWindow(this->windowedApplication.GetWindow(),
                                   this->windowedApplication.GetSwapChainProvider()->GetSwapchainFormat());
  }
};

} // namespace Editor
