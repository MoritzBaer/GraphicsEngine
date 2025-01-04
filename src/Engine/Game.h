#pragma once

#include "AssetManager.h"
#include "Core/ECS.h"
#include "Core/SceneHierarchy.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/MemoryAllocator.h"
#include "Graphics/Renderer.h"
#include "Graphics/RenderingStrategies/ComputeBackground.h"
#include "Graphics/RenderingStrategy.h"
#include "Util/DeletionQueue.h"
#include "WindowManager.h"

struct Game {
  Engine::DeletionQueue mainDeletionQueue;
  Engine::Window *mainWindow;
  Engine::Graphics::InstanceManager instanceManager;
  Engine::Graphics::MemoryAllocator memoryAllocator;
  Engine::Graphics::GPUObjectManager gpuObjectManager;
  Engine::Graphics::ShaderCompiler shaderCompiler;
  Engine::Core::SceneHierarchy sceneHierarchy;
  Engine::Core::ECS ecs;
  Engine::AssetManager assetManager;
  Engine::Graphics::Renderer renderer;

  Engine::Graphics::RenderingStrategies::ComputeBackground background;
  Engine::Core::Entity mainCam;
  bool rendering;
  bool running;

  // TODO: Make CreateWindow use dimension, pass dimension to Game()
  Game(const char *name);

  virtual void Init();
  virtual void CalculateFrame();
  inline bool IsRunning() { return running; }
  virtual void Start();

  ~Game();
};
