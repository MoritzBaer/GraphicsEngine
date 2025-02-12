#pragma once

#include "Core/ECS.h"
#include "Editor/Display.h"
#include "Editor/EditorGUIRenderingStrategy.h"
#include "Editor/ImGUIManager.h"
#include "EntityDetails.h"
#include "Game.h"
#include "Graphics/RenderingStrategies/ComputeBackground.h"
#include "Graphics/RenderingStrategies/ForwardRendering.h"
#include "SceneView.h"
#include "WindowedApplication.h"

namespace Editor {
struct GameControl : public Engine::Graphics::ImGUIView {
  bool *runGame;
  GameControl(bool *runGame) : ImGUIView(), runGame(runGame) {}

  void Draw() override {
    if (ImGui::Button("Start Game")) {
      *runGame = true;
    }
  }
};

template <class GameInstance> struct Editor : public Game {
  GameInstance game;
  bool runGame;
  bool gameInitialized;
  Engine::Graphics::ImGUIManager *imGuiManager;
  GameControl gameControl;
  SceneView sceneView;

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
        entityDetails(&selectedEntity) {
  }

  ~Editor() { vulkan->instanceManager.WaitUntilDeviceIdle(); }

  inline void Init() override {
    imGuiManager->RegisterView(&gameControl);
    imGuiManager->RegisterView(&sceneView);
    imGuiManager->RegisterView(&entityDetails);
    Game::Init();
    activeScene = assetManager.LoadAsset<Engine::Core::Scene *>("editor");
    game.Init();
    sceneView.SetSceneHierarchy(&game.activeScene->sceneHierarchy);
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
