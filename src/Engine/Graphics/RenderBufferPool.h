#pragma once

#include "AssetManager.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/DescriptorHandling.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/Image.h"
#include "Graphics/MemoryAllocator.h"
#include "Graphics/Texture.h"
#include "Maths/Dimension.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <span>
#include <tuple>
#include <variant>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Engine::Graphics {
struct RenderBufferIdentifier;
struct AuxiliaryBufferIdentifier;
} // namespace Engine::Graphics

template <> struct std::hash<Engine::Graphics::RenderBufferIdentifier> {
  inline size_t operator()(Engine::Graphics::RenderBufferIdentifier const &rbi) const;
};
template <> struct std::hash<Engine::Graphics::AuxiliaryBufferIdentifier> {
  inline size_t operator()(Engine::Graphics::AuxiliaryBufferIdentifier const &rbi) const;
};

namespace Engine::Graphics {

struct RenderBufferIdentifier {
  Maths::Dimension2 extent;
  VkFormat format;

  inline bool operator==(RenderBufferIdentifier const &other) const {
    return extent == other.extent && format == other.format;
  }
};
struct AuxiliaryBufferIdentifier {
  Maths::Dimension2 extent;
  VkFormat format;
  uint32_t layers;
  VkImageAspectFlags aspect;

  inline bool operator==(AuxiliaryBufferIdentifier const &other) const {
    return extent == other.extent && format == other.format && layers == other.layers && other.aspect == aspect;
  }
};

class RenderBufferPool;

class RenderBuffer {
  RenderBufferPool &pool;
  CommandRecorder const &recorder;
  size_t const parentStackElem;
  size_t workingStackElem;
  bool resized, reformatted, submitted;
  Maths::Dimension2 resolution;
  VkFormat format;

  struct ColImRef {
  private:
    friend class RenderBuffer;
    RenderBufferPool &pool;
    size_t &elem;
    Maths::Dimension2 const &resolution;
    VkFormat const &format;
    ColImRef(RenderBufferPool &pool, size_t &elem, Maths::Dimension2 const &resolution, VkFormat const &format)
        : pool(pool), elem(elem), resolution(resolution), format(format) {}

  public:
    inline operator Image2 &() const;
    inline Image2 *operator->() const { return &(Image2&)*this; }
    inline Maths::Dimension2 GetExtent() const { return ((Image2) * this).GetExtent(); }
  };
  struct DepthImRef {
  private:
    friend class RenderBuffer;
    RenderBufferPool &pool;
    VkFormat const &colourFormat;
    size_t &elem;
    Maths::Dimension2 const &resolution;
    DepthImRef(RenderBufferPool &pool, size_t &elem, Maths::Dimension2 const &resolution, VkFormat const &format)
        : pool(pool), elem(elem), resolution(resolution), colourFormat(format) {}

  public:
    inline operator Image2 &() const;
  };

public:
  // TODO: Allow choosing format
  RenderBuffer(RenderBufferPool &pool, size_t stackElem, Maths::Dimension2 const &initialResolution,
               VkFormat initialFormat, CommandRecorder const &recorder)
      : pool(pool), parentStackElem(stackElem), workingStackElem(stackElem), resized(false), reformatted(false), submitted(false), resolution(initialResolution),
        format(initialFormat),  colourImage(pool, workingStackElem, resolution, format),
        depthImage(pool, workingStackElem, resolution, format), recorder(recorder) {}
  ~RenderBuffer() {
    if (!submitted) {
      Submit(recorder);
    }
    ENGINE_ASSERT(submitted, "Destroyed render buffer without submitting; this could have unexpected effects!");
  }

  /**
   * Return a forwarded version of this buffer.
   * The forwarded version can call `SetResolution`
   * and `Submit` again.
   */
  inline RenderBuffer(RenderBuffer const &other)
      : RenderBuffer(other.pool, other.workingStackElem, other.resolution, other.format, other.recorder) {}

  /**
   * Must only be called once on any instance of
   * `RenderBuffer`. If multiple resolutions are
   * needed, make multiple copies.
   *
   * @param keepAspectRatio Determine whether to retain the current aspect ratio
   * @param desiredResolution The target resolution. If `keepAspectRatio` is set, t
   *                          the actual resolution will be the largest possible
   *                          fit of the current aspect ratio into `desiredResolution`.
   */
  inline void SetResolution(Maths::Dimension2 const &desiredResolution, bool keepAspectRatio = true);

  /**
   *
   * Must only be called once on any instance of
   * `RenderBuffer`. If multiple formats are
   * needed, make multiple copies.
   *
   * @param keepAspectRatio Determine whether to retain the current aspect ratio
   * @param desiredFormat The target format.
   */
  inline void SetFormat(VkFormat desiredFormat);

  /**
   * Copy the content of this render buffer to the
   * parent buffer (if needed because they do not
   * coincide).
   *
   * Can only be called once on any instance of
   * `RenderBuffer`.
   */
  inline void Submit(CommandRecorder const &recorder);

  inline Image2 &GetAuxiliaryBuffer(Maths::Dimension2 const &extent, VkFormat format,
                                    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t layerCount = 1);

  inline bool DepthBufferInUse() const;

  ColImRef colourImage;
  DepthImRef depthImage;
};

template <typename RangeT, typename BufferT, typename BufferIdentifierT>
concept BufferRange = std::ranges::range<RangeT> &&
                      std::same_as<std::ranges::range_value_t<RangeT>, std::tuple<BufferT, BufferIdentifierT>>;

class RenderBufferPool {
  struct RenderBufferImage {
    std::variant<Image2, Texture2D> i;

    inline Image2 &Image() {
      if (std::holds_alternative<Image2>(i)) {
        return std::get<Image2>(i);
      }
      return std::get<Texture2D>(i);
    }
  };
  struct StoredRenderBuffer {
    RenderBufferImage colourImage;
    std::optional<RenderBufferImage> depthImage;
    bool used;
  };

  struct StoredAuxiliaryBuffer {
    RenderBufferImage bufferImage;
    bool used;
  };

  GPUObjectManager RELEASE_CONST *objectManager;
  std::vector<std::tuple<StoredRenderBuffer, RenderBufferIdentifier>> buffers;
  std::vector<std::tuple<StoredAuxiliaryBuffer, AuxiliaryBufferIdentifier>> auxiliaryBuffers;

  std::unordered_map<RenderBufferIdentifier, size_t> bufferMap;
  std::unordered_map<AuxiliaryBufferIdentifier, size_t> auxiliaryMap;
  std::vector<size_t> bufferStack;

  inline RenderBufferImage AllocateColourImage(Maths::Dimension2 const &extent, VkFormat format) RELEASE_CONST;
  inline RenderBufferImage AllocateDepthImage(Maths::Dimension2 const &extent) const;
  inline void DeallocateImage(RenderBufferImage const &im) const;

  // Return the number of elements to be erased
  template <bool onlyUnused, typename BufferT, typename BufferIdentifierT>
  inline size_t PurgeBuffers(BufferRange<BufferT, BufferIdentifierT> auto &buffers,
                             std::unordered_map<BufferIdentifierT, size_t> &indexMap) const;
  template <typename BufferT> inline void FreeBuffer(BufferT const &buffer) const;
  template <bool onlyUnused> inline void PurgeAllBuffers();

public:
  RenderBufferPool(GPUObjectManager RELEASE_CONST *objectManager, Image2 const &initialTarget);
  RenderBufferPool() : objectManager(), buffers(), auxiliaryBuffers(), bufferMap(), auxiliaryMap(), bufferStack() {}
  ~RenderBufferPool() {
    PurgeAllBuffers<false>();
    if (std::get<0>(buffers[0]).depthImage) {
      DeallocateImage(*std::get<0>(buffers[0]).depthImage);
    }
  }

  inline RenderBuffer GetRenderBuffer(CommandRecorder const &submitRecorder) {
    auto const &i = std::get<0>(buffers[0]).colourImage.Image();
    return RenderBuffer(*this, 0, i.GetExtent(), i.GetFormat(), submitRecorder);
  }

  inline void PushBufferToStack(Maths::Dimension2 const &extent, VkFormat colourFormat) {
    auto const key = RenderBufferIdentifier{extent, colourFormat};
    if (bufferMap.contains(key)) {
      auto bufIdx = bufferMap[key];
      std::get<0>(buffers[bufIdx]).used = true;
      bufferStack.push_back(bufIdx);
      return;
    }

    bufferStack.push_back(buffers.size());
    bufferMap[key] = buffers.size();
    buffers.push_back(std::make_tuple(
        StoredRenderBuffer{.colourImage = AllocateColourImage(extent, colourFormat), .used = true}, key));
  }

  inline Image2 &GetAuxiliaryBuffer(Maths::Dimension2 const &extent, VkFormat format, VkImageUsageFlags usage,
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

    return std::get<0>(auxiliaryBuffers.back()).bufferImage.Image();
  }

  inline void CollapseStackBuffer(size_t stackElem, CommandRecorder const &rec) {
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

  inline void PrepareFrame() { PurgeAllBuffers<true>(); }

  inline Image2 &ColourImage(size_t stackElem) {
    return std::get<0>(buffers[bufferStack[stackElem]]).colourImage.Image();
  }
  inline Image2 &DepthImage(size_t stackElem) {
    auto &buf = std::get<0>(buffers[bufferStack[stackElem]]);
    auto &di = buf.depthImage;
    if (di) {
      return di->Image();
    }
    return di.emplace(AllocateDepthImage(buf.colourImage.Image().GetExtent())).Image();
  }

  inline bool DepthBufferInUse(size_t stackElem) const {
    return std::get<0>(buffers[bufferStack[stackElem]]).depthImage.has_value();
  }
};

inline RenderBufferPool::RenderBufferPool(GPUObjectManager RELEASE_CONST *objectManager, Image2 const &initialTarget)
    : objectManager(objectManager), bufferMap(), buffers(), bufferStack(), auxiliaryBuffers() {
  auto const initialKey =
      RenderBufferIdentifier{.extent = initialTarget.GetExtent(), .format = initialTarget.GetFormat()};
  buffers.push_back(std::make_tuple(StoredRenderBuffer{.colourImage = {.i = initialTarget}, .used = true}, initialKey));

  bufferStack.push_back(0);
  bufferMap[initialKey] = 0;
}

inline RenderBuffer::ColImRef::operator Engine::Graphics::Image2 &() const {
  auto &currentIm = pool.ColourImage(elem);
  if (currentIm.GetExtent() != resolution || currentIm.GetFormat() != format) {

    pool.PushBufferToStack(resolution, format);

    elem++;
  }

  return pool.ColourImage(elem);
}

inline RenderBuffer::DepthImRef::operator Engine::Graphics::Image2 &() const {
  auto &currentIm = pool.ColourImage(elem);
  if (currentIm.GetExtent() != resolution || currentIm.GetFormat() != colourFormat) {

    pool.PushBufferToStack(resolution, colourFormat);

    elem++;
  }

  return pool.DepthImage(elem);
}

inline RenderBufferPool::RenderBufferImage RenderBufferPool::AllocateColourImage(Maths::Dimension2 const &extent,
                                                                                 VkFormat format) RELEASE_CONST {
  return {.i = objectManager->AllocateImage<2>(
              format, extent,
              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
              VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VK_SAMPLE_COUNT_1_BIT DEBUG_LABEL_VALUE("RENDER_BUFFER"))};
}

inline RenderBufferPool::RenderBufferImage RenderBufferPool::AllocateDepthImage(Maths::Dimension2 const &extent) const {
  return {.i = objectManager->CreateDepthBuffer(extent)};
}

inline void RenderBufferPool::DeallocateImage(RenderBufferImage const &im) const {
  if (std::holds_alternative<Image2>(im.i)) {
    objectManager->DestroyImage(std::get<Image2>(im.i));
  } else {
    objectManager->DestroyTexture(std::get<Texture2D>(im.i));
  }
}

inline bool RenderBuffer::DepthBufferInUse() const { return pool.DepthBufferInUse(workingStackElem); }

inline void RenderBuffer::SetResolution(Maths::Dimension2 const &desiredResolution, bool keepAspectRatio) {
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

inline void RenderBuffer::SetFormat(VkFormat desiredFormat) {
  if (reformatted) {
    ENGINE_ERROR("Changed render buffer format twice without forwarding!");
    return;
  }

  reformatted = true;

  format = desiredFormat;
}

inline void RenderBuffer::Submit(CommandRecorder const &recorder) {
  if (workingStackElem != parentStackElem) {
    pool.CollapseStackBuffer(workingStackElem, recorder);
  }
  submitted = true;
}

template <bool onlyUnused, typename BufferT, typename BufferIdentifierT>
inline size_t RenderBufferPool::PurgeBuffers(BufferRange<BufferT, BufferIdentifierT> auto &buffers,
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
    indexMap[key] = cursor - buffers.begin();
  }

  return std::end(buffers) - end;
}

template <> inline void RenderBufferPool::FreeBuffer(RenderBufferPool::StoredRenderBuffer const &buffer) const {
  DeallocateImage(buffer.colourImage);
  if (buffer.depthImage) {
    DeallocateImage(*buffer.depthImage);
  }
}

template <> inline void RenderBufferPool::FreeBuffer(RenderBufferPool::StoredAuxiliaryBuffer const &buffer) const {
  DeallocateImage(buffer.bufferImage);
}

template <bool onlyUnused> inline void RenderBufferPool::PurgeAllBuffers() {
  bufferStack.erase(bufferStack.begin() + 1, bufferStack.end()); // The initial buffer is not removed
  auto nonInitialBuffers = std::span{buffers.begin() + 1, buffers.end()};
  auto numRemoved = PurgeBuffers<onlyUnused, StoredRenderBuffer, RenderBufferIdentifier>(nonInitialBuffers, bufferMap);
  buffers.erase(buffers.end() - numRemoved, buffers.end());
  numRemoved =
      PurgeBuffers<onlyUnused, StoredAuxiliaryBuffer, AuxiliaryBufferIdentifier>(auxiliaryBuffers, auxiliaryMap);
  auxiliaryBuffers.erase(auxiliaryBuffers.end() - numRemoved, auxiliaryBuffers.end());
}

inline Image2 &RenderBuffer::GetAuxiliaryBuffer(Maths::Dimension2 const &extent, VkFormat format,
                                                VkImageUsageFlags usage, VkImageAspectFlags aspect,
                                                uint32_t layerCount) {
  return pool.GetAuxiliaryBuffer(extent, format, usage, aspect, layerCount);
}

} // namespace Engine::Graphics

inline std::size_t std::hash<Engine::Graphics::RenderBufferIdentifier>::operator()(
    Engine::Graphics::RenderBufferIdentifier const &rbi) const {
  return std::hash<Engine::Maths::Dimension2>{}(rbi.extent) | std::hash<VkFormat>{}(rbi.format);
}
inline std::size_t std::hash<Engine::Graphics::AuxiliaryBufferIdentifier>::operator()(
    Engine::Graphics::AuxiliaryBufferIdentifier const &rbi) const {
  return std::hash<Engine::Maths::Dimension2>{}(rbi.extent) | std::hash<VkFormat>{}(rbi.format) |
         std::hash<uint32_t>{}(rbi.layers) | hash<VkImageAspectFlags>{}(rbi.aspect);
}
