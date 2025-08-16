#pragma once

#include "GPUObjectManager.h"
#include "MemoryAllocator.h"
#include "RenderCommand.h"
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
  GetRenderTarget(Image2 &givenRenderTarget, std::optional<Image2> &givenDepthTarget,
                  std::vector<Command const *> &previousCommands) {
    if (givenDepthTarget) {
      return {givenRenderTarget, *givenDepthTarget, VK_ATTACHMENT_LOAD_OP_LOAD};
    }

    for (auto const &discardedBuffer : discardedDepthBuffers) {
      gpuObjectManager->DestroyImage(discardedBuffer);
    }
    discardedDepthBuffers.clear();

    if (depthBuffer.GetExtent() != givenRenderTarget.GetExtent()) {
      discardedDepthBuffers.push_back(depthBuffer);
      depthBuffer = gpuObjectManager->AllocateImage(
          VK_FORMAT_D32_SFLOAT, Maths::Dimension2(1600, 900), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
          VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, VK_SAMPLE_COUNT_1_BIT DEBUG_LABEL_VALUE("BufferedStrategy depth buffer"));
    }

    return {givenRenderTarget, depthBuffer, VK_ATTACHMENT_LOAD_OP_CLEAR};
  }

  virtual std::vector<Command *> GetTargetSwapCommands(Image2 &givenRenderTarget,
                                                       std::optional<Image2> &givenDepthTarget) {
    if (!givenDepthTarget)
      givenDepthTarget.emplace(depthBuffer);
    return {};
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

    std::vector<Command const *> GetRenderingCommands(RenderingRequest const &request,
                                                      UniformBinder &uniformBufferProvider,
                                                      DescriptorAllocator &descriptorAllocator,
                                                      DescriptorWriter &descriptorWriter, Image<2> &renderTarget,
                                                      std::optional<Image<2>> &depthTarget) override;
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
std::vector<Command const *> BufferedRenderer<T_Object, T_Uniform>::BufferedStrategy::GetRenderingCommands(
    RenderingRequest const &request, UniformBinder &uniformBufferProvider, DescriptorAllocator &descriptorAllocator,
    DescriptorWriter &descriptorWriter, Image<2> &renderTarget, std::optional<Image<2>> &depthTarget) {

  // Execute wrapped strategy
  auto subRendering = wrappedStrategy->GetRenderingCommands(request, uniformBufferProvider, descriptorAllocator,
                                                            descriptorWriter, renderTarget, depthTarget);

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
    gpuDispatcher.Dispatch(
        GPUMemoryManager::CopyBufferToBuffer(stagingBuffer, buffer.gpuBuffer, stagingBuffer.PhysicalSize()));
    gpuObjectManager->DestroyBuffer(stagingBuffer);

    auto uniformBinding = uniformBufferProvider.GetBinding(ExtractUniformData(request));

    // Get render targets to use for buffered rendering call
    auto [usedRenderTarget, usedDepthTarget, usedLoadOp] =
        renderTargetProvider->GetRenderTarget(renderTarget, depthTarget, subRendering);

    subRendering.push_back(new RenderCommand<RenderObjectBuffer<T_Object>>(
        usedRenderTarget, usedDepthTarget, descriptorAllocator, descriptorWriter, usedRenderTarget.GetExtent(),
        uniformBinding, buffer, usedLoadOp));

    // Copy contents to given buffers
    auto targetSwap = renderTargetProvider->GetTargetSwapCommands(renderTarget, depthTarget);
    std::copy(std::begin(targetSwap), std::end(targetSwap), std::back_inserter(subRendering));
  }
  return subRendering;
}

} // namespace Engine::Graphics
