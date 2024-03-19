#include "Material.h"

#include "InstanceManager.h"

void Engine::Graphics::Material::Destroy() const
{
    InstanceManager::DestroyPipeline(pipeline);
    InstanceManager::DestroyPipelineLayout(layout);
}
