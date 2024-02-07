#include "Renderer.h"
#include "glfw3.h"
#include "InstanceManager.h"
#include "../WindowManager.h"

namespace Engine::Graphics
{
    void Renderer::Init() { instance = new Renderer(); }

    void Renderer::Cleanup() { delete instance; }

    Renderer::Renderer() {}
    Renderer::~Renderer() {}

} // namespace Engine::Graphics
