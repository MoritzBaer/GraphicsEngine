#pragma once

#include "AssetManager.h"
#include "Core/EntityIdentifier.h"
#include "Core/Scene.h"
#include "Core/Time.h"
#include "Debug/DebugRenderer.h"
#include "Graphics/MemoryAllocator.h"
#include "Graphics/Renderer.h"
#include "Graphics/RenderingStrategy.h"
#include "Graphics/VulkanSuite.h"
#include "Util/DeletionQueue.h"

struct Game {
  Engine::DeletionQueue mainDeletionQueue;
  Engine::Graphics::VulkanSuite RELEASE_CONST         *vulkan;
  Engine::Graphics::ShaderCompiler shaderCompiler;
  Engine::Core::Scene *activeScene;
  Engine::AssetManager assetManager;
  Engine::Graphics::Renderer renderer;
  Engine::Graphics::RenderingStrategy *renderingStrategy;
  Engine::Core::Clock clock;
  Engine::Core::IdentifierStorage identifierStorage;
#ifndef NDEBUG
  Engine::Debug::DebugRenderer debugRenderer;
#endif

  bool rendering;
  bool running;

  const char *name;

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
