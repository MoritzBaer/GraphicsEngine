#pragma once

#include "AssetManager.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/DescriptorHandling.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/Image.h"
#include "Graphics/MemoryAllocator.h"
#include "Maths/Dimension.h"
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Engine::Graphics {
struct RenderBufferIdentifier;
}

template <> struct std::hash<Engine::Graphics::RenderBufferIdentifier> {
  inline size_t operator()(Engine::Graphics::RenderBufferIdentifier const &rbi) const;
};

namespace Engine::Graphics {

struct RenderBufferIdentifier {
  Maths::Dimension2 extent;

  inline bool operator==(RenderBufferIdentifier const &other) const { return extent == other.extent; }
};

class RenderBufferPool;

// TODO:
// - Accessors for colourImage, depthImage
// - SetResolution (only change if necessary)
// - Submit (blit buffer content to next layer)
// - GetSecondaryBuffer (prob. forward from GPUObjectManager)
class RenderBuffer {
  RenderBufferPool &allocator;
  CommandRecorder const &recorder;
  size_t const parentStackElem;
  size_t workingStackElem;
  bool resized, submitted;

  struct ColImRef {
  private:
    friend class RenderBuffer;
    RenderBufferPool &allocator;
    size_t const &elem;
    ColImRef(RenderBufferPool &allocator, size_t const &elem) : allocator(allocator), elem(elem) {}

  public:
    inline operator Image2 &() const;
  };
  struct DepthImRef {
  private:
    friend class RenderBuffer;
    RenderBufferPool &allocator;
    size_t const &elem;
    DepthImRef(RenderBufferPool &allocator, size_t const &elem) : allocator(allocator), elem(elem) {}

  public:
    inline operator Image2 &() const;
  };

public:
  RenderBuffer(RenderBufferPool &allocator, size_t stackElem, CommandRecorder const &recorder)
      : allocator(allocator), parentStackElem(stackElem), workingStackElem(stackElem), resized(false), submitted(false),
        colourImage(allocator, workingStackElem), depthImage(allocator, workingStackElem), recorder(recorder) {}
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
  inline RenderBuffer(RenderBuffer const &other) : RenderBuffer(other.allocator, other.workingStackElem, other.recorder) {}
  inline RenderBuffer(RenderBuffer &&) = delete;

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
   * Copy the content of this render buffer to the
   * parent buffer (if needed because they do not
   * coincide).
   *
   * Can only be called once on any instance of
   * `RenderBuffer`.
   */
  inline void Submit(CommandRecorder const &recorder) const;

  ColImRef colourImage;
  DepthImRef depthImage;
};

class RenderBufferPool {
  struct StoredRenderBuffer {
    Image2 colourImage;
    std::optional<Image2> depthImage;
    bool used;
  };

  GPUObjectManager RELEASE_CONST *objectManager;
  std::vector<StoredRenderBuffer> buffers;

  std::unordered_map<RenderBufferIdentifier, size_t> bufferMap;
  std::vector<size_t> bufferStack;

  inline Image2 AllocateColourImage(Maths::Dimension2 const &extent) RELEASE_CONST;
  inline Image2 AllocateDepthImage(Maths::Dimension2 const &extent) const;
  inline void DeallocateImage(Image2 const &im) const;

public:
  RenderBufferPool(GPUObjectManager RELEASE_CONST *objectManager)
      : objectManager(objectManager), bufferMap(), buffers(), bufferStack() {}

  inline RenderBuffer GetRenderBuffer(Maths::Dimension2 const &initialExtent, CommandRecorder const &submitRecorder) {
    return RenderBuffer(*this, 0, submitRecorder);
  }

  inline void PushBufferToStack(Maths::Dimension2 const &extent) {
    auto const key = RenderBufferIdentifier{extent};
    if (bufferMap.contains(key)) {
      auto bufIdx = bufferMap[key];
      buffers[bufIdx].used = true;
      bufferStack.push_back(bufIdx);
      return;
    }

    bufferStack.push_back(buffers.size());
    buffers.push_back(StoredRenderBuffer{.colourImage = AllocateColourImage(extent), .used = true});
  }

  inline void CollapseStackBuffer(size_t stackElem, CommandRecorder const &rec) {
    if (stackElem == 0) {
      ENGINE_ERROR("Trying to collapse stack with single buffer!");
      return;
    }

    auto const srcIdx = bufferStack[stackElem];
    auto const dstIdx = bufferStack[stackElem - 1];

    rec.RecordBlit(buffers[srcIdx].colourImage, buffers[dstIdx].colourImage, VK_FILTER_LINEAR);
    if (buffers[srcIdx].depthImage) {
      if (!buffers[dstIdx].depthImage) {
        buffers[dstIdx].depthImage.emplace(AllocateDepthImage(buffers[dstIdx].colourImage.GetExtent()));
      }
      rec.RecordBlit(*buffers[srcIdx].depthImage, *buffers[dstIdx].depthImage, VK_FILTER_NEAREST);
    }

    bufferStack.erase(bufferStack.begin() + stackElem, bufferStack.end());
  }

  inline void PrepareFrame() {
    bufferStack.clear();
    auto cursor = buffers.begin();
    auto end = buffers.end();

    while (cursor < end) {
      if (cursor->used) {
        (cursor++)->used = false;
        continue;
      }

      // Remove buffer, close gap
      DeallocateImage(cursor->colourImage);
      if (cursor->depthImage) {
        DeallocateImage(cursor->depthImage.value());
      }

      *cursor = *(--end);

      // Update index in map
      bufferMap[RenderBufferIdentifier{cursor->colourImage.GetExtent()}] = cursor - buffers.begin();
    }

    buffers.erase(end, buffers.end());
  }

  inline Image2 &ColourImage(size_t stackElem) { return buffers[bufferStack[stackElem]].colourImage; }
  inline Image2 &DepthImage(size_t stackElem) {
    auto &di = buffers[bufferStack[stackElem]].depthImage;
    if (di) {
      return di.value();
    }
    return di.emplace(AllocateDepthImage(buffers[bufferStack[stackElem]].colourImage.GetExtent()));
  }
};

inline RenderBuffer::ColImRef::operator Engine::Graphics::Image2 &() const { return allocator.ColourImage(elem); }

inline RenderBuffer::DepthImRef::operator Engine::Graphics::Image2 &() const { return allocator.DepthImage(elem); }

inline Image2 RenderBufferPool::AllocateColourImage(Maths::Dimension2 const &extent) RELEASE_CONST {
  return objectManager->AllocateImage<2>(VK_FORMAT_R16G16B16A16_SNORM, extent, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                         VK_IMAGE_ASPECT_COLOR_BIT, 1, 1,
                                         VK_SAMPLE_COUNT_1_BIT DEBUG_LABEL_VALUE("RENDER_BUFFER"));
}

inline Image2 RenderBufferPool::AllocateDepthImage(Maths::Dimension2 const &extent) const {
  return objectManager->CreateDepthBuffer(extent);
}

inline void RenderBufferPool::DeallocateImage(Image2 const &im) const { objectManager->DestroyImage(im); }

inline void RenderBuffer::SetResolution(Maths::Dimension2 const &desiredResolution, bool keepAspectRatio) {
  if (resized) {
    ENGINE_ERROR("Resized render buffer twice without forwarding!");
    return;
  }

  resized = true;

  auto const oldRes = ((Image2)colourImage).GetExtent();
  auto const rOld = float(oldRes.x()) / oldRes.y();
  auto const rNew = float(desiredResolution.x()) / desiredResolution.y();
  auto const actualResolution =
      keepAspectRatio ? (rNew > rOld ? Maths::Dimension2(rOld * desiredResolution.y(), desiredResolution.y())
                                     : Maths::Dimension2(desiredResolution.x(), desiredResolution.x() / rOld))
                      : desiredResolution;

  if (oldRes != actualResolution) {
    allocator.PushBufferToStack(actualResolution);
  }

  workingStackElem++;
}

inline void RenderBuffer::Submit(CommandRecorder const &recorder) const {}

} // namespace Engine::Graphics

inline std::size_t std::hash<Engine::Graphics::RenderBufferIdentifier>::operator()(
    Engine::Graphics::RenderBufferIdentifier const &rbi) const {
  return std::hash<Engine::Maths::Dimension2>{}(rbi.extent);
}
