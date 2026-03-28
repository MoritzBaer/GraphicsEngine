#pragma once

#include "GPUObjectManager.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/RenderBufferPool.h"
#include "Graphics/VulkanUtil.h"
#include "MemoryAllocator.h"
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
  GPUObjectManager RELEASE_CONST *gpuObjectManager;

  RenderObjectBuffer<T_Object> renderObjectBuffer;

  class BufferedStrategy : public RenderingStrategy {
    RenderObjectBuffer<T_Object> &buffer;

    std::vector<Buffer<T_Object>> bufferDump;

    GPUObjectManager RELEASE_CONST *gpuObjectManager;
    GPUDispatcher gpuDispatcher;
    RenderingStrategy *wrappedStrategy;

    T_Uniform ExtractUniformData(RenderingRequest const &request) const;

  public:
    BufferedStrategy(InstanceManager const *instanceManager, GPUObjectManager RELEASE_CONST *gpuObjectManager,
                     RenderingStrategy *subStrategy, RenderObjectBuffer<T_Object> &buffer)
        : buffer(buffer), gpuObjectManager(gpuObjectManager), wrappedStrategy(subStrategy),
          gpuDispatcher(instanceManager, gpuObjectManager->CreateCommandQueue()) {}

    void RecordRenderingCommands(RenderingRequest const &request, UniformBinder &uniformBufferProvider,
                                 DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                                 RenderBuffer renderTarget, CommandRecorder const &recorder) override;
  };

public:
  BufferedRenderer(InstanceManager const *instanceManager, GPUObjectManager RELEASE_CONST *gpuObjectManager,
                   Material *material = nullptr)
      : instanceManager(instanceManager), gpuObjectManager(gpuObjectManager), renderObjectBuffer(material) {
    renderObjectBuffer.gpuBuffer = gpuObjectManager->CreateBuffer<T_Object>(
        INITIAL_BUFFER_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);
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

template <typename T_Object, typename T_Uniform>
void BufferedRenderer<T_Object, T_Uniform>::BufferedStrategy::RecordRenderingCommands(
    RenderingRequest const &request, UniformBinder &uniformBufferProvider, DescriptorAllocator &descriptorAllocator,
    DescriptorWriter &descriptorWriter, RenderBuffer renderTarget, CommandRecorder const &recorder) {

  // Execute wrapped strategy
  wrappedStrategy->RecordRenderingCommands(request, uniformBufferProvider, descriptorAllocator, descriptorWriter,
                                           renderTarget, recorder);

  // Cleanup buffers used in previous frames
  if (!bufferDump.empty()) {
    for (auto &buffer : bufferDump) {
      gpuObjectManager->DestroyBuffer(buffer);
    }
    bufferDump.clear();
  }

  if (!buffer.objects.empty()) {
    if (buffer.objects.size() > buffer.gpuBuffer.Size()) {
      // Allocate new buffer of fitting size
      bufferDump.push_back(buffer.gpuBuffer);
      buffer.gpuBuffer = gpuObjectManager->CreateBuffer<T_Object>(
          std::max(buffer.objects.size(), buffer.gpuBuffer.Size() * 2),
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    }

    // Upload data to GPU
    auto stagingBuffer = gpuObjectManager->CreateBuffer<T_Object>(
        buffer.objects.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    stagingBuffer.SetData(buffer.objects);
    gpuDispatcher.Dispatch([&](auto const &recorder) { recorder.RecordCopy(stagingBuffer, buffer.gpuBuffer); });
    gpuObjectManager->DestroyBuffer(stagingBuffer);

    auto uniformBinding = uniformBufferProvider.GetBinding(ExtractUniformData(request));

    auto const usedLoadOp = renderTarget.DepthBufferInUse() ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;

    recorder.RecordViewports(vkutil::MakeViewport(renderTarget.colourImage.GetExtent()));
    recorder.RecordScissors(vkutil::MakeRect(renderTarget.colourImage.GetExtent()));
    recorder.RecordRenderPass()
        .WithDrawImage(renderTarget.colourImage)
        .WithDepthImage(renderTarget.depthImage)
        .WithDepthBufferLoadOp(usedLoadOp)
        .As([&](RenderPassRecorder const &recorder) {
          recorder.RecordWithBoundPipeline(
              buffer.material->GetPipeline(), VK_PIPELINE_BIND_POINT_GRAPHICS, [&](DrawCallRecorder const &recorder) {
                recorder.RecordDescriptorBind(
                    {buffer.material->WriteDescriptors(descriptorAllocator, descriptorWriter, uniformBinding)});
                recorder.RecordDraw(buffer.gpuBuffer);
              });
        });
  }
}

} // namespace Engine::Graphics
