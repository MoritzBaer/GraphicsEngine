#pragma once

#include "Material.h"
#include "Maths/Matrix.h"

namespace Engine::Graphics {

class TestMaterial : public Material {
public:
  Maths::Vector3 colour;
  // TestMaterial(VkPipelineLayout l, VkPipeline p) { pipeline = new Pipeline(l, p)}
  TestMaterial(Material const *other) : Material(other) {
    if (TestMaterial const *testMat = dynamic_cast<TestMaterial const *>(other)) {
      colour = testMat->colour;
    } else {
      colour = {1, 1, 1};
    }
  }
  TestMaterial(Pipeline const *pipeline, Maths::Vector3 const &colour) : Material(pipeline), colour(colour) {}
  inline void AppendData(UniformAggregate &aggregate) const { aggregate.PushData(&colour); }
};

} // namespace Engine::Graphics
