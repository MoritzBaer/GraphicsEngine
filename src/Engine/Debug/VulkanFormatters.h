#pragma once

#include "vulkan/vulkan.h"
#include <format>
#include <string>
#include <type_traits>

// ---------------------------------------------------------------------------
// Helpers for logging arrays of Vulkan structs / handles.
//
// FormatArray expects `T` to already have a std::formatter (i.e. it's used
// for arrays of Vulkan *structs*, e.g. VkViewport, VkImageMemoryBarrier2...).
//
// FormatHandleArray is for arrays of Vulkan *handles* (VkBuffer, VkImage,
// VkDescriptorSet, ...), which are opaque pointers and are logged as void*.
// ---------------------------------------------------------------------------
namespace Engine::Graphics {

template <typename T> inline std::string FormatArray(T const *arr, size_t count) {
  std::string result = "[";
  for (size_t i = 0; i < count; ++i) {
    if (i)
      result += ", ";
    result += std::format("{}", arr[i]);
  }
  result += "]";
  return result;
}

template <typename Handle> inline std::string FormatHandleArray(Handle const *arr, size_t count) {
  std::string result = "[";
  for (size_t i = 0; i < count; ++i) {
    if (i)
      result += ", ";
    result += std::format("{}", (void const *)arr[i]);
  }
  result += "]";
  return result;
}

} // namespace Engine::Graphics

// ---------------------------------------------------------------------------
// Enum formatters.
//
// These print the underlying integer value tagged with the enum's type name,
// e.g. VkImageLayout(2). If human-readable names are wanted later, this macro
// is the single place to swap in a name lookup instead.
// ---------------------------------------------------------------------------
#define ENGINE_VK_ENUM_FORMATTER(EnumType)                                                                           \
  template <> struct std::formatter<EnumType> {                                                                      \
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }                                     \
    auto format(EnumType const &value, std::format_context &ctx) const {                                             \
      return std::format_to(ctx.out(), #EnumType "({})", static_cast<std::underlying_type_t<EnumType>>(value));      \
    }                                                                                                                 \
  };

ENGINE_VK_ENUM_FORMATTER(VkImageLayout)
ENGINE_VK_ENUM_FORMATTER(VkFilter)
ENGINE_VK_ENUM_FORMATTER(VkIndexType)
ENGINE_VK_ENUM_FORMATTER(VkPipelineBindPoint)
ENGINE_VK_ENUM_FORMATTER(VkAttachmentLoadOp)
ENGINE_VK_ENUM_FORMATTER(VkAttachmentStoreOp)
ENGINE_VK_ENUM_FORMATTER(VkResolveModeFlagBits)
ENGINE_VK_ENUM_FORMATTER(VkStructureType)

#undef ENGINE_VK_ENUM_FORMATTER

// ---------------------------------------------------------------------------
// Struct formatters, in dependency order (small structs first).
// ---------------------------------------------------------------------------

template <> struct std::formatter<VkOffset2D> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkOffset2D const &o, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{{x={}, y={}}}", o.x, o.y);
  }
};

template <> struct std::formatter<VkExtent2D> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkExtent2D const &e, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{{width={}, height={}}}", e.width, e.height);
  }
};

template <> struct std::formatter<VkOffset3D> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkOffset3D const &o, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{{x={}, y={}, z={}}}", o.x, o.y, o.z);
  }
};

template <> struct std::formatter<VkExtent3D> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkExtent3D const &e, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{{width={}, height={}, depth={}}}", e.width, e.height, e.depth);
  }
};

template <> struct std::formatter<VkRect2D> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkRect2D const &r, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{{offset={}, extent={}}}", r.offset, r.extent);
  }
};

template <> struct std::formatter<VkViewport> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkViewport const &v, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{{x={}, y={}, width={}, height={}, minDepth={}, maxDepth={}}}", v.x, v.y,
                           v.width, v.height, v.minDepth, v.maxDepth);
  }
};

template <> struct std::formatter<VkBufferCopy> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkBufferCopy const &r, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{{srcOffset={}, dstOffset={}, size={}}}", r.srcOffset, r.dstOffset, r.size);
  }
};

template <> struct std::formatter<VkImageSubresourceLayers> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkImageSubresourceLayers const &s, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{{aspectMask={}, mipLevel={}, baseArrayLayer={}, layerCount={}}}", s.aspectMask,
                           s.mipLevel, s.baseArrayLayer, s.layerCount);
  }
};

template <> struct std::formatter<VkImageSubresourceRange> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkImageSubresourceRange const &s, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                           "{{aspectMask={}, baseMipLevel={}, levelCount={}, baseArrayLayer={}, layerCount={}}}",
                           s.aspectMask, s.baseMipLevel, s.levelCount, s.baseArrayLayer, s.layerCount);
  }
};

template <> struct std::formatter<VkBufferImageCopy> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkBufferImageCopy const &r, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                           "{{bufferOffset={}, bufferRowLength={}, bufferImageHeight={}, imageSubresource={}, "
                           "imageOffset={}, imageExtent={}}}",
                           r.bufferOffset, r.bufferRowLength, r.bufferImageHeight, r.imageSubresource, r.imageOffset,
                           r.imageExtent);
  }
};

template <> struct std::formatter<VkClearColorValue> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkClearColorValue const &c, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{{float32=[{}, {}, {}, {}]}}", c.float32[0], c.float32[1], c.float32[2],
                           c.float32[3]);
  }
};

template <> struct std::formatter<VkClearDepthStencilValue> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkClearDepthStencilValue const &c, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{{depth={}, stencil={}}}", c.depth, c.stencil);
  }
};

template <> struct std::formatter<VkClearValue> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkClearValue const &c, std::format_context &ctx) const {
    // VkClearValue is a union; without the attachment's format we can't know whether
    // `color` or `depthStencil` is the active member, so `color` is always printed.
    return std::format_to(ctx.out(), "{}", c.color);
  }
};

template <> struct std::formatter<VkImageMemoryBarrier2> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkImageMemoryBarrier2 const &b, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                           "{{sType={}, srcStageMask={}, srcAccessMask={}, dstStageMask={}, dstAccessMask={}, "
                           "oldLayout={}, newLayout={}, srcQueueFamilyIndex={}, dstQueueFamilyIndex={}, image={}, "
                           "subresourceRange={}}}",
                           b.sType, b.srcStageMask, b.srcAccessMask, b.dstStageMask, b.dstAccessMask, b.oldLayout,
                           b.newLayout, b.srcQueueFamilyIndex, b.dstQueueFamilyIndex, (void const *)b.image,
                           b.subresourceRange);
  }
};

template <> struct std::formatter<VkDependencyInfo> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkDependencyInfo const &d, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                           "{{sType={}, dependencyFlags={}, memoryBarrierCount={}, bufferMemoryBarrierCount={}, "
                           "imageMemoryBarrierCount={}, pImageMemoryBarriers={}}}",
                           d.sType, d.dependencyFlags, d.memoryBarrierCount, d.bufferMemoryBarrierCount,
                           d.imageMemoryBarrierCount,
                           Engine::Graphics::FormatArray(d.pImageMemoryBarriers, d.imageMemoryBarrierCount));
  }
};

template <> struct std::formatter<VkImageBlit2> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkImageBlit2 const &b, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                           "{{sType={}, srcSubresource={}, srcOffsets=[{}, {}], dstSubresource={}, "
                           "dstOffsets=[{}, {}]}}",
                           b.sType, b.srcSubresource, b.srcOffsets[0], b.srcOffsets[1], b.dstSubresource,
                           b.dstOffsets[0], b.dstOffsets[1]);
  }
};

template <> struct std::formatter<VkBlitImageInfo2> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkBlitImageInfo2 const &b, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                           "{{sType={}, srcImage={}, srcImageLayout={}, dstImage={}, dstImageLayout={}, "
                           "regionCount={}, pRegions={}, filter={}}}",
                           b.sType, (void const *)b.srcImage, b.srcImageLayout, (void const *)b.dstImage,
                           b.dstImageLayout, b.regionCount, Engine::Graphics::FormatArray(b.pRegions, b.regionCount),
                           b.filter);
  }
};

template <> struct std::formatter<VkRenderingAttachmentInfo> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkRenderingAttachmentInfo const &a, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                           "{{sType={}, imageView={}, imageLayout={}, resolveMode={}, resolveImageView={}, "
                           "resolveImageLayout={}, loadOp={}, storeOp={}, clearValue={}}}",
                           a.sType, (void const *)a.imageView, a.imageLayout, a.resolveMode,
                           (void const *)a.resolveImageView, a.resolveImageLayout, a.loadOp, a.storeOp,
                           a.clearValue);
  }
};

template <> struct std::formatter<VkRenderingInfo> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  auto format(VkRenderingInfo const &r, std::format_context &ctx) const {
    return std::format_to(
        ctx.out(),
        "{{sType={}, flags={}, renderArea={}, layerCount={}, viewMask={}, colorAttachmentCount={}, "
        "pColorAttachments={}, pDepthAttachment={}, pStencilAttachment={}}}",
        r.sType, r.flags, r.renderArea, r.layerCount, r.viewMask, r.colorAttachmentCount,
        Engine::Graphics::FormatArray(r.pColorAttachments, r.colorAttachmentCount),
        r.pDepthAttachment ? std::format("{}", *r.pDepthAttachment) : std::string("null"),
        r.pStencilAttachment ? std::format("{}", *r.pStencilAttachment) : std::string("null"));
  }
};