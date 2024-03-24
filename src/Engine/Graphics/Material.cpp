#include "Material.h"

#include "InstanceManager.h"

void Engine::Graphics::Pipeline::Destroy() const {
  InstanceManager::DestroyPipeline(pipeline);
  InstanceManager::DestroyPipelineLayout(layout);
}
