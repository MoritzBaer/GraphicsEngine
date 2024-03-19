#pragma once

#include "Material.h"
#include "Maths/Matrix.h"

namespace Engine::Graphics
{
    class TestMaterial : public Material
    {
    public:
        Maths::Vector3 colour;
        TestMaterial(VkPipelineLayout layout, VkPipeline pipeline, Maths::Vector3 const & colour) : Material(layout, pipeline), colour(colour) { }
        TestMaterial(Material const * other) : Material(other), colour({ 0, 0, 0 }) { }
        inline void AppendData(UniformAggregate & aggregate) const { aggregate.PushData(&colour); }
    };
    
} // namespace Engine::Graphics
