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

namespace Editor {
struct GameControl : public Engine::Graphics::ImGUIView {
  bool *runGame;
  GameControl(Engine::Graphics::ImGUIManager &imGuiManager, bool *runGame)
      : ImGUIView(imGuiManager), runGame(runGame) {}

  void Draw() override {
    if (ImGui::Button("Start Game")) {
      *runGame = true;
    }
  }
};

struct Editor : public Game {
  Game *game;
  bool runGame;
  bool gameInitialized;
  Engine::Graphics::RenderingStrategies::ComputeBackground editorBackground;
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

  inline void Init() override {
    Game::Init();
    game->Init();
    editorBackground = assetManager.LoadAsset<Engine::Graphics::RenderingStrategies::ComputeBackground>("editor");
    renderer.SetRenderingStrategy(new EditorGUIRenderingStrategy(&gpuObjectManager, &editorBackground));
  }

  inline void CalculateFrame() override {
    if (runGame) {
      if (!gameInitialized) {
        game->Start();
        gameInitialized = true;
      }
      game->CalculateFrame();
    }
    imGuiManager.BeginFrame();
    Game::CalculateFrame();
  }
};

} // namespace Editor
