#pragma once

#include "AssetManager.h"
#include "Core/ECS.h"
#include "Core/Scene.h"
#include "Core/Time.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/MemoryAllocator.h"
#include "Graphics/Renderer.h"
#include "Graphics/RenderingStrategies/ComputeBackground.h"
#include "Graphics/RenderingStrategy.h"
#include "Graphics/VulkanSuite.h"
#include "Util/DeletionQueue.h"
#include "WindowManager.h"

struct Game {
  Engine::DeletionQueue mainDeletionQueue;
  Engine::Graphics::VulkanSuite
#ifdef NDEBUG
      const
#endif
          *vulkan;
  Engine::Graphics::ShaderCompiler shaderCompiler;
  Engine::Core::ECS prefabs;
  Engine::Core::Scene *activeScene;
  Engine::AssetManager assetManager;
  Engine::Graphics::Renderer renderer;
  Engine::Graphics::RenderingStrategy *renderingStrategy;
  Engine::Core::Clock clock;

  bool rendering;
  bool running;

  const char *name;

  // TODO: Make CreateWindow use dimension, pass dimension to Game()
  Game(const char *name, Engine::Graphics::VulkanSuite
#ifdef NDEBUG
       const
#endif
           *vulkan);

  virtual void Init();
  virtual void CalculateFrame();
  inline bool IsRunning() { return running; }
  virtual void Start();

  virtual ~Game();
};
