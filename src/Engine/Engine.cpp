#include "Engine.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace Engine {
bool quit = false;
bool render = true;
} // namespace Engine

void Engine::Init(const char *applicationName) {}

void Engine::RunMainLoop() {}

void Engine::Cleanup() {}

void Engine::Quit() { quit = true; }
