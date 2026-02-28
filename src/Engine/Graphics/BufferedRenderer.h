#pragma once

#include "GPUObjectManager.h"
#include "Graphics/CommandQueue.h"
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

class RenderTargetProvider {
  Image2 depthBuffer;
  std::vector<Image2> discardedDepthBuffers;

protected:
  GPUObjectManager RELEASE_CONST *gpuObjectManager;

public:
  RenderTargetProvider(GPUObjectManager RELEASE_CONST *gpuObjectManager)
      : depthBuffer(), discardedDepthBuffers(), gpuObjectManager(gpuObjectManager) {}
  virtual std::tuple<Image2, Image2, VkAttachmentLoadOp>
  GetRenderTarget(Image2 &givenRenderTarget, std::optional<Image2> &givenDepthTarget, CommandRecorder const &commands) {
    if (givenDepthTarget) {
      return {givenRenderTarget, *givenDepthTarget, VK_ATTACHMENT_LOAD_OP_LOAD};
    }

    for (auto const &discardedBuffer : discardedDepthBuffers) {
      gpuObjectManager->DestroyImage(discardedBuffer);
    }
    discardedDepthBuffers.clear();

    if (depthBuffer.GetExtent() != givenRenderTarget.GetExtent()) {
      discardedDepthBuffers.push_back(depthBuffer);
      depthBuffer = gpuObjectManager->CreateDepthBuffer(Maths::Dimension2(1600, 900));
    }

    return {givenRenderTarget, depthBuffer, VK_ATTACHMENT_LOAD_OP_CLEAR};
  }

  virtual void GetTargetSwapCommands(Image2 &givenRenderTarget, std::optional<Image2> &givenDepthTarget,
                                     CommandRecorder const &recorder) {
    if (!givenDepthTarget)
      givenDepthTarget.emplace(depthBuffer);
  }
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

    RenderTargetProvider *renderTargetProvider;

    T_Uniform ExtractUniformData(RenderingRequest const &request) const;

  public:
    BufferedStrategy(InstanceManager const *instanceManager, GPUObjectManager RELEASE_CONST *gpuObjectManager,
                     RenderingStrategy *subStrategy, RenderObjectBuffer<T_Object> &buffer,
                     RenderTargetProvider *renderTargetProvider)
        : buffer(buffer), gpuObjectManager(gpuObjectManager), wrappedStrategy(subStrategy),
          gpuDispatcher(instanceManager, gpuObjectManager->CreateCommandQueue()),
          renderTargetProvider(renderTargetProvider) {}

    void RecordRenderingCommands(RenderingRequest const &request, UniformBinder &uniformBufferProvider,
                                 DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                                 Image<2> &renderTarget, std::optional<Image<2>> &depthTarget,
                                 CommandRecorder const &recorder) override;
  };

  RenderTargetProvider *renderTargetProvider;

public:
  BufferedRenderer(InstanceManager const *instanceManager, GPUObjectManager RELEASE_CONST *gpuObjectManager,
                   RenderTargetProvider *renderTargetProvider = nullptr, Material *material = nullptr)
      : instanceManager(instanceManager), gpuObjectManager(gpuObjectManager), renderObjectBuffer(material),
        renderTargetProvider(renderTargetProvider) {
    renderObjectBuffer.gpuBuffer = gpuObjectManager->CreateBuffer<T_Object>(
        INITIAL_BUFFER_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);
    if (!renderTargetProvider)
      this->renderTargetProvider = new RenderTargetProvider(gpuObjectManager);
  }

  void SetMaterial(Material *material) { renderObjectBuffer.material = material; }

  RenderingStrategy *WrapWithBufferedStrategy(RenderingStrategy *subStrategy) {
    return new BufferedStrategy(instanceManager, gpuObjectManager, subStrategy, renderObjectBuffer,
                                renderTargetProvider);
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
    DescriptorWriter &descriptorWriter, Image<2> &renderTarget, std::optional<Image<2>> &depthTarget,
    CommandRecorder const &recorder) {

  // Execute wrapped strategy
  wrappedStrategy->RecordRenderingCommands(request, uniformBufferProvider, descriptorAllocator, descriptorWriter,
                                           renderTarget, depthTarget, recorder);

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

    // Get render targets to use for buffered rendering call
    auto [usedRenderTarget, usedDepthTarget, usedLoadOp] =
        renderTargetProvider->GetRenderTarget(renderTarget, depthTarget, recorder);

    recorder.RecordViewports(vkutil::MakeViewport(usedRenderTarget.GetExtent()));
    recorder.RecordScissors(vkutil::MakeRect(usedRenderTarget.GetExtent()));
    recorder.RecordRenderPass()
        .WithDrawImage(usedRenderTarget)
        .WithDepthImage(usedDepthTarget)
        .WithDepthBufferLoadOp(usedLoadOp)
        .As([&](RenderPassRecorder const &recorder) {
          recorder.RecordWithBoundPipeline(
              buffer.material->GetPipeline(), VK_PIPELINE_BIND_POINT_GRAPHICS, [&](DrawCallRecorder const &recorder) {
                recorder.RecordDescriptorBind(
                    {buffer.material->WriteDescriptors(descriptorAllocator, descriptorWriter, uniformBinding)});
                recorder.RecordDraw(buffer.gpuBuffer);
              });
        });

    // Copy contents to given buffers
    renderTargetProvider->GetTargetSwapCommands(renderTarget, depthTarget, recorder);
  }
}

} // namespace Engine::Graphics
