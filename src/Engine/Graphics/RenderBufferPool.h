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
  size_t operator()(Engine::Graphics::RenderBufferIdentifier const &rbi) const;
};
template <> struct std::hash<Engine::Graphics::AuxiliaryBufferIdentifier> {
  size_t operator()(Engine::Graphics::AuxiliaryBufferIdentifier const &rbi) const;
};

namespace Engine::Graphics {

struct RenderBufferIdentifier {
  Maths::Dimension2 extent;
  VkFormat format;

  bool operator==(RenderBufferIdentifier const &other) const {
    return extent == other.extent && format == other.format;
  }
};
struct AuxiliaryBufferIdentifier {
  Maths::Dimension2 extent;
  VkFormat format;
  uint32_t layers;
  VkImageAspectFlags aspect;

  bool operator==(AuxiliaryBufferIdentifier const &other) const {
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
    operator Image2 &() const;
    Image2 *operator->() const { return &(Image2 &)*this; }
    Maths::Dimension2 GetExtent() const { return ((Image2) * this).GetExtent(); }
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
    operator Image2 &() const;
  };

public:
  RenderBuffer(RenderBufferPool &pool, size_t stackElem, Maths::Dimension2 const &initialResolution,
               VkFormat initialFormat, CommandRecorder const &recorder)
      : pool(pool), parentStackElem(stackElem), workingStackElem(stackElem), resized(false), reformatted(false),
        submitted(false), resolution(initialResolution), format(initialFormat),
        colourImage(pool, workingStackElem, resolution, format), depthImage(pool, workingStackElem, resolution, format),
        recorder(recorder) {}
  ~RenderBuffer();

  /**
   * Return a forwarded version of this buffer.
   * The forwarded version can call `SetResolution`
   * and `Submit` again.
   */
  RenderBuffer(RenderBuffer const &other)
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
  void SetResolution(Maths::Dimension2 const &desiredResolution, bool keepAspectRatio = true);

  /**
   *
   * Must only be called once on any instance of
   * `RenderBuffer`. If multiple formats are
   * needed, make multiple copies.
   *
   * @param keepAspectRatio Determine whether to retain the current aspect ratio
   * @param desiredFormat The target format.
   */
  void SetFormat(VkFormat desiredFormat);

  /**
   * Copy the content of this render buffer to the
   * parent buffer (if needed because they do not
   * coincide).
   *
   * Can only be called once on any instance of
   * `RenderBuffer`.
   */
  void Submit(CommandRecorder const &recorder);

  Image2 &GetAuxiliaryBuffer(Maths::Dimension2 const &extent, VkFormat format,
                             VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                             VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t layerCount = 1);

  bool DepthBufferInUse() const;

  ColImRef colourImage;
  DepthImRef depthImage;
};

template <typename RangeT, typename BufferT, typename BufferIdentifierT>
concept BufferRange = std::ranges::range<RangeT> &&
                      std::same_as<std::ranges::range_value_t<RangeT>, std::tuple<BufferT, BufferIdentifierT>>;

class RenderBufferPool {
  struct RenderBufferImage {
    Image2 image;
    Texture2D texture;
    bool isTexture;

    RenderBufferImage(Image2 const &im) : image(im), texture(), isTexture(false) {}
    RenderBufferImage(Texture2D const &tex) : image(), texture(tex), isTexture(true) {}
    Image2 &operator=(Image2 const &im) {
      image = im;
      isTexture = false;
      return image;
    }
    Texture2D &operator=(Texture2D const &tex) {
      texture = tex;
      isTexture = true;
      return texture;
    }

    Image2 &Image() { return isTexture ? texture : image; }
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

  RenderBufferImage AllocateColourImage(Maths::Dimension2 const &extent, VkFormat format) RELEASE_CONST;
  RenderBufferImage AllocateDepthImage(Maths::Dimension2 const &extent) const;
  void DeallocateImage(RenderBufferImage const &im) const;

  // Return the number of elements to be erased
  template <bool onlyUnused, typename BufferT, typename BufferIdentifierT>
  size_t PurgeBuffers(BufferRange<BufferT, BufferIdentifierT> auto &buffers,
                      std::unordered_map<BufferIdentifierT, size_t> &indexMap) const;
  template <typename BufferT> void FreeBuffer(BufferT const &buffer) const;
  template <bool onlyUnused> void PurgeAllBuffers();

public:
  RenderBufferPool(GPUObjectManager RELEASE_CONST *objectManager, Image2 const &initialTarget);
  RenderBufferPool() : objectManager(), buffers(), auxiliaryBuffers(), bufferMap(), auxiliaryMap(), bufferStack() {}
  RenderBufferPool(RenderBufferPool const &other) = delete;
  RenderBufferPool(RenderBufferPool &&other);
  RenderBufferPool &operator=(RenderBufferPool const &other) = delete;
  RenderBufferPool &operator=(RenderBufferPool &&other);
  ~RenderBufferPool();

  RenderBuffer GetRenderBuffer(CommandRecorder const &submitRecorder);
  void PushBufferToStack(Maths::Dimension2 const &extent, VkFormat colourFormat);
  Image2 &GetAuxiliaryBuffer(Maths::Dimension2 const &extent, VkFormat format, VkImageUsageFlags usage,
                             VkImageAspectFlags aspect, uint32_t layerCount);
  void CollapseStackBuffer(size_t stackElem, CommandRecorder const &rec);
  void PrepareFrame();
  Image2 &ColourImage(size_t stackElem);
  Image2 &DepthImage(size_t stackElem);
  bool DepthBufferInUse(size_t stackElem) const;
};

} // namespace Engine::Graphics
