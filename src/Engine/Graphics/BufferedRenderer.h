#pragma once

#include "GPUObjectManager.h"
#include "RenderCommand.h"
#include "RenderingStrategy.h"

namespace Engine::Graphics {
template <typename T_Object> struct RenderObjectBuffer {
  Buffer<T_Object> gpuBuffer;
  std::vector<T_Object> objects;
  Material *material;

  RenderObjectBuffer(Material *material) : material(material) {}
};

template <typename T_Object, typename T_Uniform> class BufferedRenderer {
  inline static const size_t INITIAL_BUFFER_SIZE = 16;

  InstanceManager const *instanceManager;
  GPUObjectManager *gpuObjectManager;

  RenderObjectBuffer<T_Object> renderObjectBuffer;

  class BufferedStrategy : public RenderingStrategy {
    RenderObjectBuffer<T_Object> &buffer;

    std::vector<Buffer<T_Object>> bufferDump;

    GPUObjectManager *gpuObjectManager;
    RenderingStrategy *wrappedStrategy;

    Image2 depthBuffer;

    T_Uniform ExtractUniformData(RenderingRequest const &request, Image<2> const &renderTarget,
                                 Image<2> const *depthTarget) const;

  public:
    BufferedStrategy(InstanceManager const *instanceManager, GPUObjectManager *gpuObjectManager,
                     RenderingStrategy *subStrategy, RenderObjectBuffer<T_Object> &buffer)
        : buffer(buffer), gpuObjectManager(gpuObjectManager), wrappedStrategy(subStrategy),
          depthBuffer(gpuObjectManager->CreateAllocatedImage(
              VK_FORMAT_D32_SFLOAT, Maths::Dimension2(1600, 900), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, VK_SAMPLE_COUNT_1_BIT, "BufferedStrategy depth buffer")) {}

    std::vector<Command *> GetRenderingCommands(RenderingRequest const &request, UniformBinder &uniformBufferProvider,
                                                DescriptorAllocator &descriptorAllocator,
                                                DescriptorWriter &descriptorWriter, Image<2> &renderTarget,
                                                Image<2> *depthTarget) override;
  };

public:
  BufferedRenderer(InstanceManager const *instanceManager, GPUObjectManager *gpuObjectManager,
                   Material *material = nullptr)
      : instanceManager(instanceManager), gpuObjectManager(gpuObjectManager), renderObjectBuffer(material) {
    renderObjectBuffer.gpuBuffer = gpuObjectManager->CreateBuffer<T_Object>(
        INITIAL_BUFFER_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
  }

  void SetMaterial(Material *material) { renderObjectBuffer.material = material; }

  RenderingStrategy *WrapWithBufferedStrategy(RenderingStrategy *subStrategy) {
    return new BufferedStrategy(instanceManager, gpuObjectManager, subStrategy, renderObjectBuffer);
  }

  void AddToBuffer(T_Object const &object) { renderObjectBuffer.objects.push_back(object); }

  void ClearBuffer() { renderObjectBuffer.objects.clear(); }
};

// +-----------------+
// | IMPLEMENTATIONS |
// +-----------------+

template <typename T_Object> struct PartiallySpecializedRender<RenderObjectBuffer<T_Object>> {
  void operator()(VkCommandBuffer const &queue, Image<2> const &drawImage, Image<2> const &depthImage,
                  Maths::Dimension2 renderAreaSize, Maths::Dimension2 renderAreaOffset,
                  DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                  UniformBinding const &uniformBinding, RenderObjectBuffer<T_Object> bufferedObject) const {
    bufferedObject.material->Apply(queue, descriptorAllocator, descriptorWriter, uniformBinding);

    bufferedObject.gpuBuffer.BindAsVertexBuffer(queue);

    // Draw the object
    vkCmdDraw(queue, bufferedObject.objects.size(), 1, 0, 0);
  }
};

template <typename T_Object, typename T_Uniform>
std::vector<Command *> BufferedRenderer<T_Object, T_Uniform>::BufferedStrategy::GetRenderingCommands(
    RenderingRequest const &request, UniformBinder &uniformBufferProvider, DescriptorAllocator &descriptorAllocator,
    DescriptorWriter &descriptorWriter, Image<2> &renderTarget, Image<2> *depthTarget) {
  auto subRendering = wrappedStrategy->GetRenderingCommands(request, uniformBufferProvider, descriptorAllocator,
                                                            descriptorWriter, renderTarget);

  if (!bufferDump.empty()) {
    for (auto &buffer : bufferDump) {
      gpuObjectManager->DestroyBuffer(buffer);
    }
    bufferDump.clear();
  }

  if (!buffer.objects.empty()) {
    // TODO: Increase GPU buffer size if necessary
    if (buffer.objects.size() > buffer.gpuBuffer.Size()) {
      bufferDump.push_back(buffer.gpuBuffer);
      buffer.gpuBuffer =
          gpuObjectManager->CreateBuffer<T_Object>(std::max(buffer.objects.size(), buffer.gpuBuffer.Size() * 2),
                                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    }
    // TODO: Use staging buffer
    buffer.gpuBuffer.SetData(buffer.objects);

    auto uniformBinding = uniformBufferProvider.GetBinding(ExtractUniformData(request, renderTarget, depthTarget));

    subRendering.push_back(new RenderCommand<RenderObjectBuffer<T_Object>>(
        renderTarget, depthBuffer, descriptorAllocator, descriptorWriter, renderTarget.GetExtent(), uniformBinding,
        buffer));
    // TODO: Add depth buffer to depth target?
  }
  return subRendering;
}

} // namespace Engine::Graphics
