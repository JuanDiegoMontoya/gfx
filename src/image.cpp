#include "detail/image.hpp"

#include "detail/common.hpp"
#include "detail/context.hpp"

#include <array>

using namespace gfx2::internal;

namespace
{
  VkImageViewType ToVkImageViewType(gfx_image_type type)
  {
    switch (type)
    {
    case GFX_IMAGE_TYPE_1D: return VK_IMAGE_VIEW_TYPE_1D;
    case GFX_IMAGE_TYPE_2D: return VK_IMAGE_VIEW_TYPE_2D;
    case GFX_IMAGE_TYPE_3D: return VK_IMAGE_VIEW_TYPE_3D;
    case GFX_IMAGE_TYPE_CUBE: return VK_IMAGE_VIEW_TYPE_CUBE;
    case GFX_IMAGE_TYPE_1D_ARRAY: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case GFX_IMAGE_TYPE_2D_ARRAY: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case GFX_IMAGE_TYPE_CUBE_ARRAY: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    default: assert(0); return {};
    }
  }

  VkImageType ViewTypeToImageType(VkImageViewType viewType)
  {
    switch (viewType)
    {
    case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
    case VK_IMAGE_VIEW_TYPE_1D: return VK_IMAGE_TYPE_1D;
    case VK_IMAGE_VIEW_TYPE_CUBE:
    case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
    case VK_IMAGE_VIEW_TYPE_2D: return VK_IMAGE_TYPE_2D;
    case VK_IMAGE_VIEW_TYPE_3D: return VK_IMAGE_TYPE_3D;
    default: assert(0); return {};
    }
  }

  bool FormatIsDepth(gfx_format format)
  {
    switch (format)
    {
    case GFX_FORMAT_D16_UNORM:
    case GFX_FORMAT_D24_UNORM_S8_UINT:
    case GFX_FORMAT_X8_D24_UNORM:
    case GFX_FORMAT_D32_SFLOAT: [[fallthrough]];
    case GFX_FORMAT_D32_SFLOAT_S8_UINT: return true;
    default: return false;
    }
  }

  bool FormatIsStencil(gfx_format format)
  {
    switch (format)
    {
    case GFX_FORMAT_D24_UNORM_S8_UINT: [[fallthrough]];
    case GFX_FORMAT_D32_SFLOAT_S8_UINT: return true;
    default: return false;
    }
  }

  bool FormatIsColor(gfx_format format)
  {
    return !(FormatIsDepth(format) || FormatIsStencil(format));
  }

  bool FormatIsSrgb(gfx_format format)
  {
    switch (format)
    {
    case GFX_FORMAT_R8G8B8A8_SRGB:
    case GFX_FORMAT_B8G8R8A8_SRGB:
    case GFX_FORMAT_BC1_RGBA_SRGB:
    case GFX_FORMAT_BC1_RGB_SRGB:
    case GFX_FORMAT_BC2_RGBA_SRGB:
    case GFX_FORMAT_BC3_RGBA_SRGB:
    case GFX_FORMAT_BC7_RGBA_SRGB: return true;
    default: return false;
    }
  }

  uint32_t AllocateStorageImageDescriptor(VkImageView imageView)
  {
    auto& ctx = GetContextInstance();
    const auto index = ctx.storageImageDescriptorAllocator.Allocate();

    vkUpdateDescriptorSets(ctx.device,
      1,
      ToPtr(VkWriteDescriptorSet{
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet          = ctx.descriptorSet,
        .dstBinding      = Context::STORAGE_IMAGE_BINDING,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo      = ToPtr(VkDescriptorImageInfo{.imageView = imageView, .imageLayout = VK_IMAGE_LAYOUT_GENERAL}),
      }),
      0,
      nullptr);

    return index;
  }

  uint32_t AllocateSampledImageDescriptor(VkImageView imageView)
  {
    auto& ctx        = GetContextInstance();
    const auto index = ctx.sampledImageDescriptorAllocator.Allocate();

    vkUpdateDescriptorSets(ctx.device,
      1,
      ToPtr(VkWriteDescriptorSet{
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet          = ctx.descriptorSet,
        .dstBinding      = Context::SAMPLED_IMAGE_BINDING,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo      = ToPtr(VkDescriptorImageInfo{.imageView = imageView, .imageLayout = VK_IMAGE_LAYOUT_GENERAL}),
      }),
      0,
      nullptr);

    return index;
  }
} // namespace

namespace gfx2::internal
{
  VkImageAspectFlags FormatToAspectFlags(gfx_format format)
  {
    auto aspectFlags = VkImageAspectFlags{};
    aspectFlags |= FormatIsDepth(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
    aspectFlags |= FormatIsStencil(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
    aspectFlags |= FormatIsColor(format) ? VK_IMAGE_ASPECT_COLOR_BIT : 0;
    return aspectFlags;
  }

  VkFormat ToVkFormat(gfx_format format)
  {
    switch (format)
    {
    case GFX_FORMAT_UNDEFINED: return VK_FORMAT_UNDEFINED;
    case GFX_FORMAT_R8_UNORM: return VK_FORMAT_R8_UNORM;
    case GFX_FORMAT_R8_SNORM: return VK_FORMAT_R8_SNORM;
    case GFX_FORMAT_R16_UNORM: return VK_FORMAT_R16_UNORM;
    case GFX_FORMAT_R16_SNORM: return VK_FORMAT_R16_SNORM;
    case GFX_FORMAT_R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
    case GFX_FORMAT_R8G8_SNORM: return VK_FORMAT_R8G8_SNORM;
    case GFX_FORMAT_R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
    case GFX_FORMAT_R16G16_SNORM: return VK_FORMAT_R16G16_SNORM;
    case GFX_FORMAT_R4G4B4A4_UNORM: return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    case GFX_FORMAT_R5G5B5A1_UNORM: return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
    case GFX_FORMAT_R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
    case GFX_FORMAT_B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
    case GFX_FORMAT_R8G8B8A8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
    case GFX_FORMAT_A2R10G10B10_UNORM: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    case GFX_FORMAT_A2B10G10R10_UNORM: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case GFX_FORMAT_A2R10G10B10_UINT: return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    case GFX_FORMAT_R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
    case GFX_FORMAT_R16G16B16A16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
    case GFX_FORMAT_R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
    case GFX_FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
    case GFX_FORMAT_R16_SFLOAT: return VK_FORMAT_R16_SFLOAT;
    case GFX_FORMAT_R16G16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
    case GFX_FORMAT_R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
    case GFX_FORMAT_R32_SFLOAT: return VK_FORMAT_R32_SFLOAT;
    case GFX_FORMAT_R32G32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT;
    case GFX_FORMAT_R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case GFX_FORMAT_B10G11R11_UFLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case GFX_FORMAT_E5B9G9R9_UFLOAT: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    case GFX_FORMAT_R8_SINT: return VK_FORMAT_R8_SINT;
    case GFX_FORMAT_R8_UINT: return VK_FORMAT_R8_UINT;
    case GFX_FORMAT_R16_SINT: return VK_FORMAT_R16_SINT;
    case GFX_FORMAT_R16_UINT: return VK_FORMAT_R16_UINT;
    case GFX_FORMAT_R32_SINT: return VK_FORMAT_R32_SINT;
    case GFX_FORMAT_R32_UINT: return VK_FORMAT_R32_UINT;
    case GFX_FORMAT_R8G8_SINT: return VK_FORMAT_R8G8_SINT;
    case GFX_FORMAT_R8G8_UINT: return VK_FORMAT_R8G8_UINT;
    case GFX_FORMAT_R16G16_SINT: return VK_FORMAT_R16G16_SINT;
    case GFX_FORMAT_R16G16_UINT: return VK_FORMAT_R16G16_UINT;
    case GFX_FORMAT_R32G32_SINT: return VK_FORMAT_R32G32_SINT;
    case GFX_FORMAT_R32G32_UINT: return VK_FORMAT_R32G32_UINT;
    case GFX_FORMAT_R8G8B8A8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
    case GFX_FORMAT_R8G8B8A8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
    case GFX_FORMAT_R16G16B16A16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
    case GFX_FORMAT_R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
    case GFX_FORMAT_R32G32B32A32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
    case GFX_FORMAT_R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
    case GFX_FORMAT_D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
    case GFX_FORMAT_X8_D24_UNORM: return VK_FORMAT_X8_D24_UNORM_PACK32;
    case GFX_FORMAT_D16_UNORM: return VK_FORMAT_D16_UNORM;
    case GFX_FORMAT_D32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case GFX_FORMAT_D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
    case GFX_FORMAT_BC1_RGB_UNORM: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case GFX_FORMAT_BC1_RGB_SRGB: return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
    case GFX_FORMAT_BC1_RGBA_UNORM: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case GFX_FORMAT_BC1_RGBA_SRGB: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case GFX_FORMAT_BC2_RGBA_UNORM: return VK_FORMAT_BC2_UNORM_BLOCK;
    case GFX_FORMAT_BC2_RGBA_SRGB: return VK_FORMAT_BC2_SRGB_BLOCK;
    case GFX_FORMAT_BC3_RGBA_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
    case GFX_FORMAT_BC3_RGBA_SRGB: return VK_FORMAT_BC3_SRGB_BLOCK;
    case GFX_FORMAT_BC4_R_UNORM: return VK_FORMAT_BC4_UNORM_BLOCK;
    case GFX_FORMAT_BC4_R_SNORM: return VK_FORMAT_BC4_SNORM_BLOCK;
    case GFX_FORMAT_BC5_RG_UNORM: return VK_FORMAT_BC5_UNORM_BLOCK;
    case GFX_FORMAT_BC5_RG_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
    case GFX_FORMAT_BC6H_RGB_UFLOAT: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case GFX_FORMAT_BC6H_RGB_SFLOAT: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case GFX_FORMAT_BC7_RGBA_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
    case GFX_FORMAT_BC7_RGBA_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
    }

    assert(false);
    return VK_FORMAT_UNDEFINED;
  }

  gfx_format VkToFormat(VkFormat format)
  {
    switch (format)
    {
    case VK_FORMAT_UNDEFINED: return GFX_FORMAT_UNDEFINED;
    case VK_FORMAT_R8_UNORM: return GFX_FORMAT_R8_UNORM;
    case VK_FORMAT_R8_SNORM: return GFX_FORMAT_R8_SNORM;
    case VK_FORMAT_R16_UNORM: return GFX_FORMAT_R16_UNORM;
    case VK_FORMAT_R16_SNORM: return GFX_FORMAT_R16_SNORM;
    case VK_FORMAT_R8G8_UNORM: return GFX_FORMAT_R8G8_UNORM;
    case VK_FORMAT_R8G8_SNORM: return GFX_FORMAT_R8G8_SNORM;
    case VK_FORMAT_R16G16_UNORM: return GFX_FORMAT_R16G16_UNORM;
    case VK_FORMAT_R16G16_SNORM: return GFX_FORMAT_R16G16_SNORM;
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return GFX_FORMAT_R4G4B4A4_UNORM;
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return GFX_FORMAT_R5G5B5A1_UNORM;
    case VK_FORMAT_R8G8B8A8_UNORM: return GFX_FORMAT_R8G8B8A8_UNORM;
    case VK_FORMAT_B8G8R8A8_UNORM: return GFX_FORMAT_B8G8R8A8_UNORM;
    case VK_FORMAT_R8G8B8A8_SNORM: return GFX_FORMAT_R8G8B8A8_SNORM;
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return GFX_FORMAT_A2R10G10B10_UNORM;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return GFX_FORMAT_A2B10G10R10_UNORM;
    case VK_FORMAT_A2R10G10B10_UINT_PACK32: return GFX_FORMAT_A2R10G10B10_UINT;
    case VK_FORMAT_R16G16B16A16_UNORM: return GFX_FORMAT_R16G16B16A16_UNORM;
    case VK_FORMAT_R16G16B16A16_SNORM: return GFX_FORMAT_R16G16B16A16_SNORM;
    case VK_FORMAT_R8G8B8A8_SRGB: return GFX_FORMAT_R8G8B8A8_SRGB;
    case VK_FORMAT_B8G8R8A8_SRGB: return GFX_FORMAT_B8G8R8A8_SRGB;
    case VK_FORMAT_R16_SFLOAT: return GFX_FORMAT_R16_SFLOAT;
    case VK_FORMAT_R16G16_SFLOAT: return GFX_FORMAT_R16G16_SFLOAT;
    case VK_FORMAT_R16G16B16A16_SFLOAT: return GFX_FORMAT_R16G16B16A16_SFLOAT;
    case VK_FORMAT_R32_SFLOAT: return GFX_FORMAT_R32_SFLOAT;
    case VK_FORMAT_R32G32_SFLOAT: return GFX_FORMAT_R32G32_SFLOAT;
    case VK_FORMAT_R32G32B32A32_SFLOAT: return GFX_FORMAT_R32G32B32A32_SFLOAT;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return GFX_FORMAT_B10G11R11_UFLOAT;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return GFX_FORMAT_E5B9G9R9_UFLOAT;
    case VK_FORMAT_R8_SINT: return GFX_FORMAT_R8_SINT;
    case VK_FORMAT_R8_UINT: return GFX_FORMAT_R8_UINT;
    case VK_FORMAT_R16_SINT: return GFX_FORMAT_R16_SINT;
    case VK_FORMAT_R16_UINT: return GFX_FORMAT_R16_UINT;
    case VK_FORMAT_R32_SINT: return GFX_FORMAT_R32_SINT;
    case VK_FORMAT_R32_UINT: return GFX_FORMAT_R32_UINT;
    case VK_FORMAT_R8G8_SINT: return GFX_FORMAT_R8G8_SINT;
    case VK_FORMAT_R8G8_UINT: return GFX_FORMAT_R8G8_UINT;
    case VK_FORMAT_R16G16_SINT: return GFX_FORMAT_R16G16_SINT;
    case VK_FORMAT_R16G16_UINT: return GFX_FORMAT_R16G16_UINT;
    case VK_FORMAT_R32G32_SINT: return GFX_FORMAT_R32G32_SINT;
    case VK_FORMAT_R32G32_UINT: return GFX_FORMAT_R32G32_UINT;
    case VK_FORMAT_R8G8B8A8_SINT: return GFX_FORMAT_R8G8B8A8_SINT;
    case VK_FORMAT_R8G8B8A8_UINT: return GFX_FORMAT_R8G8B8A8_UINT;
    case VK_FORMAT_R16G16B16A16_SINT: return GFX_FORMAT_R16G16B16A16_SINT;
    case VK_FORMAT_R16G16B16A16_UINT: return GFX_FORMAT_R16G16B16A16_UINT;
    case VK_FORMAT_R32G32B32A32_SINT: return GFX_FORMAT_R32G32B32A32_SINT;
    case VK_FORMAT_R32G32B32A32_UINT: return GFX_FORMAT_R32G32B32A32_UINT;
    case VK_FORMAT_D32_SFLOAT: return GFX_FORMAT_D32_SFLOAT;
    case VK_FORMAT_X8_D24_UNORM_PACK32: return GFX_FORMAT_X8_D24_UNORM;
    case VK_FORMAT_D16_UNORM: return GFX_FORMAT_D16_UNORM;
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return GFX_FORMAT_D32_SFLOAT_S8_UINT;
    case VK_FORMAT_D24_UNORM_S8_UINT: return GFX_FORMAT_D24_UNORM_S8_UINT;
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return GFX_FORMAT_BC1_RGB_UNORM;
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return GFX_FORMAT_BC1_RGB_SRGB;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return GFX_FORMAT_BC1_RGBA_UNORM;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return GFX_FORMAT_BC1_RGBA_SRGB;
    case VK_FORMAT_BC2_UNORM_BLOCK: return GFX_FORMAT_BC2_RGBA_UNORM;
    case VK_FORMAT_BC2_SRGB_BLOCK: return GFX_FORMAT_BC2_RGBA_SRGB;
    case VK_FORMAT_BC3_UNORM_BLOCK: return GFX_FORMAT_BC3_RGBA_UNORM;
    case VK_FORMAT_BC3_SRGB_BLOCK: return GFX_FORMAT_BC3_RGBA_SRGB;
    case VK_FORMAT_BC4_UNORM_BLOCK: return GFX_FORMAT_BC4_R_UNORM;
    case VK_FORMAT_BC4_SNORM_BLOCK: return GFX_FORMAT_BC4_R_SNORM;
    case VK_FORMAT_BC5_UNORM_BLOCK: return GFX_FORMAT_BC5_RG_UNORM;
    case VK_FORMAT_BC5_SNORM_BLOCK: return GFX_FORMAT_BC5_RG_SNORM;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK: return GFX_FORMAT_BC6H_RGB_UFLOAT;
    case VK_FORMAT_BC6H_SFLOAT_BLOCK: return GFX_FORMAT_BC6H_RGB_SFLOAT;
    case VK_FORMAT_BC7_UNORM_BLOCK: return GFX_FORMAT_BC7_RGBA_UNORM;
    case VK_FORMAT_BC7_SRGB_BLOCK: return GFX_FORMAT_BC7_RGBA_SRGB;
    }

    assert(false);
    return GFX_FORMAT_UNDEFINED;
  }
}

gfx_image gfx_create_image(const gfx_image_create_info* create_info)
{
  auto& ctx = gfx2::internal::GetContextInstance();

  const uint32_t colorOrDepthStencilUsage = FormatIsColor(create_info->format) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  const uint32_t storageUsage = (FormatIsColor(create_info->format) && !FormatIsSrgb(create_info->format)) ? VK_IMAGE_USAGE_STORAGE_BIT : 0;
  uint32_t usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | storageUsage | colorOrDepthStencilUsage;

  const auto queueFamilyIndices = std::array{ctx.graphicsQueueFamilyIndex, ctx.computeQueueFamilyIndex, ctx.transferQueueFamilyIndex};

  auto internalImage = std::shared_ptr<gfx2::internal::Image>(new gfx2::internal::Image(),
    [allocator = ctx.allocator](gfx2::internal::Image* img)
    {
      vmaDestroyImage(allocator, img->image, img->allocation);
      delete img;
    });

  internalImage->createInfo = VkImageCreateInfo{
    .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .flags                 = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
    .imageType             = ViewTypeToImageType(ToVkImageViewType(create_info->type)),
    .format                = ToVkFormat(create_info->format),
    .extent                = {create_info->extent.width, create_info->extent.height, create_info->extent.depth},
    .mipLevels             = create_info->mip_levels,
    .arrayLayers           = create_info->array_layers,
    .samples               = VK_SAMPLE_COUNT_1_BIT,
    .tiling                = VK_IMAGE_TILING_OPTIMAL,
    .usage                 = usage,
    .sharingMode           = VK_SHARING_MODE_CONCURRENT,
    .queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size()),
    .pQueueFamilyIndices   = queueFamilyIndices.data(),
    .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  CheckVkResult(vmaCreateImage(ctx.allocator,
    &internalImage->createInfo,
    ToPtr(VmaAllocationCreateInfo{.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE}),
    &internalImage->image,
    &internalImage->allocation,
    nullptr));

  auto* image = new gfx_image_t();

  image->internalImage = std::move(internalImage);

  vkCreateImageView(ctx.device,
    ToPtr(VkImageViewCreateInfo{
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image    = image->internalImage->image,
      .viewType = ToVkImageViewType(create_info->type),
      .format   = ToVkFormat(create_info->format),
      .subresourceRange =
        VkImageSubresourceRange{
          .aspectMask     = gfx2::internal::FormatToAspectFlags(create_info->format),
          .baseMipLevel   = 0,
          .levelCount     = VK_REMAINING_MIP_LEVELS,
          .baseArrayLayer = 0,
          .layerCount     = VK_REMAINING_ARRAY_LAYERS,
        },
    }),
    nullptr,
    &image->imageView);

  if (FormatIsColor(create_info->format))
  {
    image->storageDescriptor = {.index = AllocateStorageImageDescriptor(image->imageView)};
  }

  image->sampledDescriptor = {.index = AllocateSampledImageDescriptor(image->imageView)};

  return image;
}

gfx_image gfx_create_image_view(const gfx_image_view_create_info* create_info)
{
  auto& ctx   = gfx2::internal::GetContextInstance();
  auto* image = new gfx_image_t();

  image->internalImage = create_info->base_image->internalImage;

  vkCreateImageView(ctx.device,
    ToPtr(VkImageViewCreateInfo{
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image    = image->internalImage->image,
      .viewType = ToVkImageViewType(create_info->type),
      .format   = ToVkFormat(create_info->format),
      .subresourceRange =
        VkImageSubresourceRange{
          .aspectMask     = gfx2::internal::FormatToAspectFlags(create_info->format),
          .baseMipLevel   = create_info->base_mip_level,
          .levelCount     = create_info->level_count,
          .baseArrayLayer = create_info->base_array_layer,
          .layerCount     = create_info->layer_count,
        },
    }),
    nullptr,
    &image->imageView);

  const bool isIdentitySwizzle = create_info->components.r == GFX_COMPONENT_SWIZZLE_IDENTITY && create_info->components.g == GFX_COMPONENT_SWIZZLE_IDENTITY &&
                                 create_info->components.b == GFX_COMPONENT_SWIZZLE_IDENTITY && create_info->components.a == GFX_COMPONENT_SWIZZLE_IDENTITY;
  if (FormatIsColor(create_info->format) && isIdentitySwizzle)
  {
    image->storageDescriptor = {.index = AllocateStorageImageDescriptor(image->imageView)};
  }

  image->sampledDescriptor = {.index = AllocateSampledImageDescriptor(image->imageView)};

  return image;
}

void gfx_destroy_image(gfx_image image)
{
  auto& ctx = gfx2::internal::GetContextInstance();
  vkDestroyImageView(ctx.device, image->imageView, nullptr);

  if (image->sampledDescriptor)
  {
    ctx.sampledImageDescriptorAllocator.Free(image->sampledDescriptor->index);
  }

  if (image->storageDescriptor)
  {
    ctx.storageImageDescriptorAllocator.Free(image->storageDescriptor->index);
  }

  delete image;
}

gfx_sampled_image_descriptor gfx_get_sampled_image_descriptor(gfx_image image)
{
  return image->sampledDescriptor.value();
}

gfx_storage_image_descriptor gfx_get_storage_image_descriptor(gfx_image image)
{
  return image->storageDescriptor.value();
}