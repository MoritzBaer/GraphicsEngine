#include "Material.h"

#include "InstanceManager.h"

Engine::Graphics::Material::~Material()
{
    InstanceManager::DestroyPipeline(pipeline);
    InstanceManager::DestroyPipelineLayout(layout);
}