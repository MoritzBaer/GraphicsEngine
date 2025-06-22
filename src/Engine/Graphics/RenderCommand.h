#pragma once

#include "CommandQueue.h"
#include "GPUObjectManager.h"

namespace Engine::Graphics {
template <typename T_Object> class RenderCommand : public Command {
protected:
  Image<2> const drawImage;
  Image<2> const depthImage;
  Maths::Dimension2 const renderAreaSize;
  Maths::Dimension2 const renderAreaOffset;
  DescriptorAllocator &descriptorAllocator;
  DescriptorWriter &descriptorWriter;
  UniformBinding const uniformBinding;
  T_Object const bufferedObject;
  VkAttachmentLoadOp depthBufferLoadOp;

  virtual void DoRender(VkCommandBuffer const &) const;

public:
  RenderCommand(Image<2> const &drawImage, Image<2> const &depthImage, DescriptorAllocator &descriptorAllocator,
                DescriptorWriter &descriptorWriter, Maths::Dimension2 const &renderAreaSize,
                UniformBinding const &uniformBinding, T_Object const &object,
                VkAttachmentLoadOp const &depthBufferLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR)
      : drawImage(drawImage), depthImage(depthImage), bufferedObject(object), uniformBinding(uniformBinding),
        renderAreaSize(renderAreaSize), descriptorAllocator(descriptorAllocator), descriptorWriter(descriptorWriter),
        depthBufferLoadOp(depthBufferLoadOp) {}
  void QueueExecution(VkCommandBuffer const &) const override;
};

template <typename T_Object> class MultiRenderCommand : public RenderCommand<std::vector<T_Object>> {

  void DoSingleRender(VkCommandBuffer const &, T_Object const &, UniformBinding const &) const;
  virtual void DoRender(VkCommandBuffer const &queue) const override {
    for (auto const &object : RenderCommand<std::vector<T_Object>>::bufferedObject) {
      DoSingleRender(queue, object, RenderCommand<std::vector<T_Object>>::uniformBinding);
    }
  }

public:
  MultiRenderCommand(Image<2> const &drawImage, Image<2> const &depthImage, DescriptorAllocator &descriptorAllocator,
                     DescriptorWriter &descriptorWriter, Maths::Dimension2 const &renderAreaSize,
                     UniformBinding const &uniformBinding, std::vector<T_Object> const &objects)
      : RenderCommand<std::vector<T_Object>>(drawImage, depthImage, descriptorAllocator, descriptorWriter,
                                             renderAreaSize, uniformBinding, objects) {}
};

// +-----------------+
// | IMPLEMENTATIONS |
// +-----------------+

template <typename T_Object> void RenderCommand<T_Object>::QueueExecution(VkCommandBuffer const &queue) const {
  VkRenderingAttachmentInfo colourAttachmentInfo = drawImage.BindAsColourAttachment();
  VkRenderingAttachmentInfo depthAttachmentInfo = depthImage.BindAsDepthAttachment(depthBufferLoadOp);

  VkExtent2D drawExtent{renderAreaSize.x(), renderAreaSize.y()};
  VkOffset2D drawOffset{static_cast<int32_t>(renderAreaOffset.x()), static_cast<int32_t>(renderAreaOffset.y())};
  VkRenderingInfo renderingInfo = vkinit::RenderingInfo(colourAttachmentInfo, depthAttachmentInfo, drawExtent);

  VkViewport viewport{.x = static_cast<float>(drawOffset.x),
                      .y = static_cast<float>(drawOffset.y),
                      .width = static_cast<float>(drawExtent.width),
                      .height = static_cast<float>(drawExtent.height),
                      .minDepth = 0.0f,
                      .maxDepth = 1.0f};

  VkRect2D scissor{.offset = {2 * drawOffset.x, 2 * drawOffset.y},
                   .extent = {drawExtent.width * 20, drawExtent.height * 20}};

  vkCmdSetViewport(queue, 0, 1, &viewport);
  vkCmdSetScissor(queue, 0, 1, &scissor);

  vkCmdBeginRendering(queue, &renderingInfo);

  DoRender(queue);

  vkCmdEndRendering(queue);
}

template <typename T_Object> struct PartiallySpecializedRender {
  void operator()(VkCommandBuffer const &queue, Image<2> const &drawImage, Image<2> const &depthImage,
                  Maths::Dimension2 renderAreaSize, Maths::Dimension2 renderAreaOffset,
                  DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                  UniformBinding const &uniformBinding, T_Object const &bufferedObject) const {};
};

template <typename T_Object> void RenderCommand<T_Object>::DoRender(VkCommandBuffer const &queue) const {
  PartiallySpecializedRender<T_Object>{}(queue, drawImage, depthImage, renderAreaSize, renderAreaOffset,
                                         descriptorAllocator, descriptorWriter, uniformBinding, bufferedObject);
}

} // namespace Engine::Graphics
