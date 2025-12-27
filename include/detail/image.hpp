#pragma once
#include "gfx2.h"
#include "vk_mem_alloc.h"

#include "vulkan/vulkan_core.h"

#include <memory>
#include <optional>

namespace gfx2::internal
{
  struct Image
  {
    VkImageCreateInfo createInfo;
    VkImage image;
    VmaAllocation allocation;
  };

  VkImageAspectFlags FormatToAspectFlags(gfx_format format);
  VkFormat ToVkFormat(gfx_format format);
  gfx_format VkToFormat(VkFormat format);
}

struct gfx_image_t
{
  VkImageView imageView;
  std::shared_ptr<gfx2::internal::Image> internalImage;
  std::optional<gfx_sampled_image_descriptor> sampledDescriptor;
  std::optional<gfx_storage_image_descriptor> storageDescriptor;
};