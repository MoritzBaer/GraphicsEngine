#include "RenderBufferPool.h"
#include "AssetManager.h"
#include <utility>

namespace Engine::Graphics {

RenderBufferPool::RenderBufferPool(GPUObjectManager RELEASE_CONST *objectManager, Image2 const &initialTarget)
    : objectManager(objectManager), bufferMap(), buffers(), bufferStack(), auxiliaryBuffers() {
  auto const initialKey =
      RenderBufferIdentifier{.extent = initialTarget.GetExtent(), .format = initialTarget.GetFormat()};
  buffers.push_back(std::make_tuple(StoredRenderBuffer{.colourImage = initialTarget, .used = true}, initialKey));

  bufferStack.push_back(0);
  bufferMap[initialKey] = 0;
}
RenderBufferPool::RenderBufferPool(RenderBufferPool &&other)
    : buffers(std::move(other.buffers)), auxiliaryBuffers(std::move(other.auxiliaryBuffers)),
      bufferMap(std::move(other.bufferMap)), auxiliaryMap(std::move(other.auxiliaryMap)),
      bufferStack(std::move(other.bufferStack)), objectManager(other.objectManager) {}
RenderBufferPool &RenderBufferPool::operator=(RenderBufferPool &&other) {
  objectManager = other.objectManager;
  buffers = std::move(other.buffers);
  auxiliaryBuffers = std::move(other.auxiliaryBuffers);
  bufferMap = std::move(other.bufferMap);
  auxiliaryMap = std::move(other.auxiliaryMap);
  bufferStack = std::move(other.bufferStack);
  return *this;
}
RenderBufferPool::~RenderBufferPool() {
  ENGINE_DEBUG("Clearing render pool with {} main buffers and {} auxiliary buffers", buffers.size(), auxiliaryBuffers.size());
  PurgeAllBuffers<false>();
  if (buffers.size() && std::get<0>(buffers[0]).depthImage) {
    DeallocateImage(*std::get<0>(buffers[0]).depthImage);
  }
}

RenderBuffer RenderBufferPool::GetRenderBuffer(CommandRecorder const &submitRecorder) {
  auto const &i = std::get<0>(buffers[0]).colourImage.Image();
  return RenderBuffer(*this, 0, i.GetExtent(), i.GetFormat(), submitRecorder);
}

void RenderBufferPool::PushBufferToStack(Maths::Dimension2 const &extent, VkFormat colourFormat) {
  auto const key = RenderBufferIdentifier{extent, colourFormat};
  if (bufferMap.contains(key)) {
    auto bufIdx = bufferMap[key];
    std::get<0>(buffers[bufIdx]).used = true;
    bufferStack.push_back(bufIdx);
    return;
  }

  bufferStack.push_back(buffers.size());
  bufferMap[key] = buffers.size();
  buffers.push_back(
      std::make_tuple(StoredRenderBuffer{.colourImage = AllocateColourImage(extent, colourFormat), .used = true}, key));
  ENGINE_DEBUG("Created new render buffer; now holding {}.", buffers.size());
}

Image2 &RenderBufferPool::GetAuxiliaryBuffer(Maths::Dimension2 const &extent, VkFormat format, VkImageUsageFlags usage,
                                             VkImageAspectFlags aspect, uint32_t layerCount) {
  auto const key = AuxiliaryBufferIdentifier{.extent = extent, .format = format, .layers = layerCount};

  if (auxiliaryMap.contains(key)) {
    auto const idx = auxiliaryMap[key];
    std::get<0>(auxiliaryBuffers[idx]).used = true;
    return std::get<0>(auxiliaryBuffers[idx]).bufferImage.Image();
  }

  auxiliaryMap[key] = auxiliaryBuffers.size();
  auxiliaryBuffers.push_back(std::make_tuple(
      StoredAuxiliaryBuffer{
          .bufferImage = objectManager->AllocateImage<2>(format, extent, usage, aspect, 1, layerCount), .used = true},
      key));

  ENGINE_DEBUG("Created new auxiliary buffer; now holding {}.", auxiliaryBuffers.size());
  return std::get<0>(auxiliaryBuffers.back()).bufferImage.Image();
}

void RenderBufferPool::CollapseStackBuffer(size_t stackElem, CommandRecorder const &rec) {
  if (stackElem == 0) {
    ENGINE_ERROR("Trying to collapse stack with single buffer!");
    return;
  }

  auto &src = std::get<0>(buffers[stackElem]);
  auto &dst = std::get<0>(buffers[stackElem - 1]);

  rec.RecordTransition(src.colourImage.Image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  rec.RecordTransition(dst.colourImage.Image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  rec.RecordBlit(src.colourImage.Image(), dst.colourImage.Image(), VK_FILTER_LINEAR);
  if (src.depthImage) {
    if (!dst.depthImage) {
      dst.depthImage.emplace(AllocateDepthImage(dst.colourImage.Image().GetExtent()));
    }
    rec.RecordTransition(src.depthImage->Image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    rec.RecordTransition(dst.depthImage->Image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    rec.RecordBlit(src.depthImage->Image(), dst.depthImage->Image(), VK_FILTER_NEAREST);
  }

  bufferStack.erase(bufferStack.begin() + stackElem, bufferStack.end());
}

void RenderBufferPool::PrepareFrame() { PurgeAllBuffers<true>(); }

Image2 &RenderBufferPool::ColourImage(size_t stackElem) {
  return std::get<0>(buffers[bufferStack[stackElem]]).colourImage.Image();
}
Image2 &RenderBufferPool::DepthImage(size_t stackElem) {
  auto &buf = std::get<0>(buffers[bufferStack[stackElem]]);
  auto &di = buf.depthImage;
  if (di) {
    return di->Image();
  }
  return di.emplace(AllocateDepthImage(buf.colourImage.Image().GetExtent())).Image();
}

bool RenderBufferPool::DepthBufferInUse(size_t stackElem) const {
  return std::get<0>(buffers[bufferStack[stackElem]]).depthImage.has_value();
}

RenderBuffer::ColImRef::operator Engine::Graphics::Image2 &() const {
  auto &currentIm = pool.ColourImage(elem);
  if (currentIm.GetExtent() != resolution || currentIm.GetFormat() != format) {

    pool.PushBufferToStack(resolution, format);

    elem++;
  }

  return pool.ColourImage(elem);
}

RenderBuffer::DepthImRef::operator Engine::Graphics::Image2 &() const {
  auto &currentIm = pool.ColourImage(elem);
  if (currentIm.GetExtent() != resolution || currentIm.GetFormat() != colourFormat) {

    pool.PushBufferToStack(resolution, colourFormat);

    elem++;
  }

  return pool.DepthImage(elem);
}

RenderBufferPool::RenderBufferImage RenderBufferPool::AllocateColourImage(Maths::Dimension2 const &extent,
                                                                          VkFormat format) RELEASE_CONST {
  return {objectManager->AllocateImage<2>(
      format, extent,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VK_SAMPLE_COUNT_1_BIT DEBUG_LABEL_VALUE("RENDER_BUFFER"))};
}

RenderBufferPool::RenderBufferImage RenderBufferPool::AllocateDepthImage(Maths::Dimension2 const &extent) const {
  return {objectManager->CreateDepthBuffer(extent)};
}

void RenderBufferPool::DeallocateImage(RenderBufferImage const &im) const {
  if (im.isTexture) {
    objectManager->DestroyTexture(im.texture);
  } else {
    objectManager->DestroyImage(im.image);
  }
}

bool RenderBuffer::DepthBufferInUse() const { return pool.DepthBufferInUse(workingStackElem); }

void RenderBuffer::SetResolution(Maths::Dimension2 const &desiredResolution, bool keepAspectRatio) {
  if (resized) {
    ENGINE_ERROR("Resized render buffer twice without forwarding!");
    return;
  }

  resized = true;

  auto const oldRes = colourImage.GetExtent();
  auto const rOld = float(oldRes.x()) / oldRes.y();
  auto const rNew = float(desiredResolution.x()) / desiredResolution.y();
  auto const actualResolution =
      keepAspectRatio ? (rNew > rOld ? Maths::Dimension2(rOld * desiredResolution.y(), desiredResolution.y())
                                     : Maths::Dimension2(desiredResolution.x(), desiredResolution.x() / rOld))
                      : desiredResolution;

  resolution = actualResolution;
}

void RenderBuffer::SetFormat(VkFormat desiredFormat) {
  if (reformatted) {
    ENGINE_ERROR("Changed render buffer format twice without forwarding!");
    return;
  }

  reformatted = true;

  format = desiredFormat;
}

void RenderBuffer::Submit(CommandRecorder const &recorder) {
  if (workingStackElem != parentStackElem) {
    pool.CollapseStackBuffer(workingStackElem, recorder);
  }
  submitted = true;
}

template <bool onlyUnused, typename BufferT, typename BufferIdentifierT>
size_t RenderBufferPool::PurgeBuffers(BufferRange<BufferT, BufferIdentifierT> auto &buffers,
                                      std::unordered_map<BufferIdentifierT, size_t> &indexMap) const {
  auto cursor = std::begin(buffers);
  auto end = std::end(buffers);

  while (cursor < end) {
    auto &buf = std::get<0>(*cursor);
    auto &key = std::get<1>(*cursor);
    ++cursor;

    if constexpr (onlyUnused) {
      if (buf.used) {
        buf.used = false;
        continue;
      }
    }

    // Remove buffer, close gap
    FreeBuffer(buf);
    indexMap.erase(key);
    *cursor = *(--end);

    // Update index in map
    indexMap[std::get<1>(*cursor)] = cursor - buffers.begin();
  }

  return std::end(buffers) - end;
}

template <> void RenderBufferPool::FreeBuffer(RenderBufferPool::StoredRenderBuffer const &buffer) const {
  DeallocateImage(buffer.colourImage);
  if (buffer.depthImage) {
    DeallocateImage(*buffer.depthImage);
  }
}

template <> void RenderBufferPool::FreeBuffer(RenderBufferPool::StoredAuxiliaryBuffer const &buffer) const {
  DeallocateImage(buffer.bufferImage);
}

template <bool onlyUnused> void RenderBufferPool::PurgeAllBuffers() {
  bufferStack.erase(bufferStack.begin() + 1, bufferStack.end()); // The initial buffer is not removed
  auto nonInitialBuffers = std::span{buffers.begin() + 1, buffers.end()};
  auto numRemoved = PurgeBuffers<onlyUnused, StoredRenderBuffer, RenderBufferIdentifier>(nonInitialBuffers, bufferMap);
  buffers.erase(buffers.end() - numRemoved, buffers.end());
  numRemoved =
      PurgeBuffers<onlyUnused, StoredAuxiliaryBuffer, AuxiliaryBufferIdentifier>(auxiliaryBuffers, auxiliaryMap);
  auxiliaryBuffers.erase(auxiliaryBuffers.end() - numRemoved, auxiliaryBuffers.end());
}

Image2 &RenderBuffer::GetAuxiliaryBuffer(Maths::Dimension2 const &extent, VkFormat format, VkImageUsageFlags usage,
                                         VkImageAspectFlags aspect, uint32_t layerCount) {
  return pool.GetAuxiliaryBuffer(extent, format, usage, aspect, layerCount);
}

RenderBuffer::~RenderBuffer() {
  if (!submitted) {
    Submit(recorder);
  }
  ENGINE_ASSERT(submitted, "Destroyed render buffer without submitting; this could have unexpected effects!");
}

} // namespace Engine::Graphics

std::size_t std::hash<Engine::Graphics::RenderBufferIdentifier>::operator()(
    Engine::Graphics::RenderBufferIdentifier const &rbi) const {
  return std::hash<Engine::Maths::Dimension2>{}(rbi.extent) | std::hash<VkFormat>{}(rbi.format);
}
std::size_t std::hash<Engine::Graphics::AuxiliaryBufferIdentifier>::operator()(
    Engine::Graphics::AuxiliaryBufferIdentifier const &rbi) const {
  return std::hash<Engine::Maths::Dimension2>{}(rbi.extent) | std::hash<VkFormat>{}(rbi.format) |
         std::hash<uint32_t>{}(rbi.layers) | hash<VkImageAspectFlags>{}(rbi.aspect);
}
