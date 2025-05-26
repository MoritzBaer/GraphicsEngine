#include "DebugRenderer.h"

namespace Engine::Graphics {

    template <>
    Debug::DebugRenderer::Uniform
    BufferedRenderer<Debug::DebugPoint, Debug::DebugRenderer::Uniform>::BufferedStrategy::ExtractUniformData(
        Graphics::RenderingRequest const &request, Image<2> const &renderTarget, Image<2> const *depthTarget) const {
      Debug::DebugRenderer::Uniform uniform;
      uniform.view = request.camera->entity.GetComponent<Transform>()->WorldToModelMatrix();
      uniform.projection = request.camera->projection;
      return uniform;
    }

    template <>
    Debug::DebugRenderer::Uniform
    BufferedRenderer<Debug::DebugLine, Debug::DebugRenderer::Uniform>::BufferedStrategy::ExtractUniformData(
        Graphics::RenderingRequest const &request, Image<2> const &renderTarget, Image<2> const *depthTarget) const {
      Debug::DebugRenderer::Uniform uniform;
      uniform.view = request.camera->entity.GetComponent<Transform>()->WorldToModelMatrix();
      uniform.projection = request.camera->projection;
      return uniform;
    }
    } // namespace Engine::Graphics