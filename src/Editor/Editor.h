#pragma once

#include "Core/ECS.h"
#include "Editor/DebugGUIRenderingStrategy.h"
#include "Editor/Display.h"
#include "Editor/ImGUIManager.h"
#include "EntityDetails.h"
#include "Game.h"
#include "Graphics/RenderingStrategies/ForwardRendering.h"
#include "SceneView.h"

namespace Editor {
struct GameControl : public Engine::Graphics::ImGUIView {
  bool *runGame;
  GameControl(Engine::Graphics::ImGUIManager &imGuiManager, bool *runGame)
      : ImGUIView(imGuiManager), runGame(runGame) {}

  void Draw() const override {
    if (ImGui::Button("Start Game")) {
      *runGame = true;
    }
  }
};

struct Editor : public Game {
  Game *game;
  bool runGame;
  bool gameInitialized;
  Engine::Graphics::ImGUIManager imGuiManager;
  GameControl gameControl;
  SceneView sceneView;

  Engine::Core::Entity selectedEntity;
  EntityDetails entityDetails;

  Editor(Game *game)
      : Game("Editor"), game(game), gameControl(imGuiManager, &runGame), runGame(false), gameInitialized(false),
        imGuiManager(mainWindow, renderer.GetSwapchainFormat(), &instanceManager),
        sceneView(imGuiManager, &game->sceneHierarchy, &selectedEntity), entityDetails(imGuiManager, &selectedEntity) {}
  ~Editor() {}

  inline void SpecializedInit() override {
    mainCam = ecs.CreateEntity();
    mainCam.AddComponent<Engine::Graphics::Camera>();
    mainCam.AddComponent<Display>()->AssignLabel("Main camera");
    auto camTransform = mainCam.AddComponent<Engine::Graphics::Transform>();
    camTransform->position = {0, 0, 5};
    camTransform->LookAt({0, 0, 0});
    renderer.SetRenderingStrategy(
        new DebugGUIRenderingStrategy(new Engine::Graphics::RenderingStrategies::ForwardRendering()));
  }

  inline void CalculateFrame() override {
    if (runGame) {
      if (!gameInitialized) {
        game->Init();
        gameInitialized = true;
      }
      game->CalculateFrame();
    }
    imGuiManager.BeginFrame();
    Game::CalculateFrame();
  }
};

} // namespace Editor
