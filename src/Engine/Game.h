#pragma once

#include "AssetManager.h"
#include "Core/ECS.h"
#include "Core/SceneHierarchy.h"
#include "Graphics/ImGUIManager.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/MemoryAllocator.h"
#include "Graphics/Renderer.h"
#include "Util/DeletionQueue.h"
#include "WindowManager.h"

struct Game {
  Engine::Window *mainWindow;
  Engine::Graphics::InstanceManager instanceManager;
  Engine::DeletionQueue mainDeletionQueue;
  Engine::Graphics::MemoryAllocator memoryAllocator;
  Engine::Graphics::GPUObjectManager gpuObjectManager;
  Engine::Graphics::ShaderCompiler shaderCompiler;
  Engine::Core::SceneHierarchy sceneHierarchy;
  Engine::AssetManager assetManager;
  Engine::Graphics::Renderer renderer;
  Engine::Graphics::ImGUIManager imGuiManager;
  Engine::Core::ECS ecs;

  Engine::Core::Entity mainCam;
  bool rendering;
  bool running;

  // TODO: Make CreateWindow use dimension, pass dimension to Game()
  Game(const char *name);

  void CalculateFrame();
  inline bool IsRunning() { return running; }

  ~Game();
};
