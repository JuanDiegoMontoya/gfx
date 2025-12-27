#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef int gfx_error_t;
typedef uint32_t gfx_flags_t;

typedef struct gfx_byte_span
{
  const void* ptr;
  size_t size;
} gfx_byte_span;

typedef enum gfx_queue
{
  GFX_QUEUE_GRAPHICS,
  GFX_QUEUE_COMPUTE,
  GFX_QUEUE_TRANSFER,

  GFX_NUM_QUEUES,
} gfx_queue;

typedef enum gfx_stage_flag_bits
{
  GFX_STAGE_ALL                    = 0x7FFFFFFF,
  GFX_STAGE_VERTEX_SHADER          = 1 << 0,
  GFX_STAGE_EARLY_FRAGMENT_TESTS   = 1 << 1,
  GFX_STAGE_FRAGMENT_SHADER        = 1 << 2,
  GFX_STAGE_LATE_FRAGMENT_TESTS    = 1 << 3,
  GFX_STAGE_FRAGMENT_SHADER_OUTPUT = 1 << 4,

  GFX_STAGE_COMPUTE = 1 << 5,

  GFX_STAGE_TRANSFER = 1 << 6,
} gfx_stage_flag_bits;
typedef gfx_flags_t gfx_stage_flags;

typedef enum gfx_access_flag_bits
{
  GFX_ACCESS_ALL   = 0x7FFFFFFF,
  GFX_ACCESS_READ  = 1 << 0,
  GFX_ACCESS_WRITE = 1 << 1,
} gfx_access_flag_bits;
typedef gfx_flags_t gfx_access_flags;

typedef enum gfx_image_type
{
  GFX_IMAGE_TYPE_1D,
  GFX_IMAGE_TYPE_2D,
  GFX_IMAGE_TYPE_3D,
  GFX_IMAGE_TYPE_CUBE,
  GFX_IMAGE_TYPE_1D_ARRAY,
  GFX_IMAGE_TYPE_2D_ARRAY,
  GFX_IMAGE_TYPE_CUBE_ARRAY,
} gfx_image_type;

typedef enum gfx_format
{
  GFX_FORMAT_UNDEFINED,

  // Color formats
  GFX_FORMAT_R8_UNORM,
  GFX_FORMAT_R8_SNORM,
  GFX_FORMAT_R16_UNORM,
  GFX_FORMAT_R16_SNORM,
  GFX_FORMAT_R8G8_UNORM,
  GFX_FORMAT_R8G8_SNORM,
  GFX_FORMAT_R16G16_UNORM,
  GFX_FORMAT_R16G16_SNORM,
  GFX_FORMAT_R4G4B4A4_UNORM,
  GFX_FORMAT_R5G5B5A1_UNORM,
  GFX_FORMAT_R8G8B8A8_UNORM,
  GFX_FORMAT_B8G8R8A8_UNORM,
  GFX_FORMAT_R8G8B8A8_SNORM,
  GFX_FORMAT_A2R10G10B10_UNORM,
  GFX_FORMAT_A2B10G10R10_UNORM,
  GFX_FORMAT_A2R10G10B10_UINT,
  GFX_FORMAT_R16G16B16A16_UNORM,
  GFX_FORMAT_R16G16B16A16_SNORM,
  GFX_FORMAT_R8G8B8A8_SRGB,
  GFX_FORMAT_B8G8R8A8_SRGB,
  GFX_FORMAT_R16_SFLOAT,
  GFX_FORMAT_R16G16_SFLOAT,
  GFX_FORMAT_R16G16B16A16_SFLOAT,
  GFX_FORMAT_R32_SFLOAT,
  GFX_FORMAT_R32G32_SFLOAT,
  GFX_FORMAT_R32G32B32A32_SFLOAT,
  GFX_FORMAT_B10G11R11_UFLOAT,
  GFX_FORMAT_E5B9G9R9_UFLOAT,
  GFX_FORMAT_R8_SINT,
  GFX_FORMAT_R8_UINT,
  GFX_FORMAT_R16_SINT,
  GFX_FORMAT_R16_UINT,
  GFX_FORMAT_R32_SINT,
  GFX_FORMAT_R32_UINT,
  GFX_FORMAT_R8G8_SINT,
  GFX_FORMAT_R8G8_UINT,
  GFX_FORMAT_R16G16_SINT,
  GFX_FORMAT_R16G16_UINT,
  GFX_FORMAT_R32G32_SINT,
  GFX_FORMAT_R32G32_UINT,
  GFX_FORMAT_R8G8B8A8_SINT,
  GFX_FORMAT_R8G8B8A8_UINT,
  GFX_FORMAT_R16G16B16A16_SINT,
  GFX_FORMAT_R16G16B16A16_UINT,
  GFX_FORMAT_R32G32B32A32_SINT,
  GFX_FORMAT_R32G32B32A32_UINT,

  // Depth & stencil formats
  GFX_FORMAT_D32_SFLOAT,
  GFX_FORMAT_X8_D24_UNORM,
  GFX_FORMAT_D16_UNORM,
  GFX_FORMAT_D32_SFLOAT_S8_UINT,
  GFX_FORMAT_D24_UNORM_S8_UINT,

  // Compressed formats
  GFX_FORMAT_BC1_RGB_UNORM,
  GFX_FORMAT_BC1_RGB_SRGB,
  GFX_FORMAT_BC1_RGBA_UNORM,
  GFX_FORMAT_BC1_RGBA_SRGB,
  GFX_FORMAT_BC2_RGBA_UNORM,
  GFX_FORMAT_BC2_RGBA_SRGB,
  GFX_FORMAT_BC3_RGBA_UNORM,
  GFX_FORMAT_BC3_RGBA_SRGB,
  GFX_FORMAT_BC4_R_UNORM,
  GFX_FORMAT_BC4_R_SNORM,
  GFX_FORMAT_BC5_RG_UNORM,
  GFX_FORMAT_BC5_RG_SNORM,
  GFX_FORMAT_BC6H_RGB_UFLOAT,
  GFX_FORMAT_BC6H_RGB_SFLOAT,
  GFX_FORMAT_BC7_RGBA_UNORM,
  GFX_FORMAT_BC7_RGBA_SRGB,
} gfx_format;

typedef enum gfx_component_swizzle
{
  GFX_COMPONENT_SWIZZLE_IDENTITY,
  GFX_COMPONENT_SWIZZLE_ZERO,
  GFX_COMPONENT_SWIZZLE_ONE,
  GFX_COMPONENT_SWIZZLE_R,
  GFX_COMPONENT_SWIZZLE_G,
  GFX_COMPONENT_SWIZZLE_B,
  GFX_COMPONENT_SWIZZLE_A,
} gfx_component_swizzle;

typedef enum gfx_aspect_flag_bits
{
  GFX_ASPECT_COLOR,
  GFX_ASPECT_DEPTH,
  GFX_ASPECT_STENCIL,
} gfx_aspect_flag_bits;

#define GFX_REMAINING_MIP_LEVELS (~0u)
#define GFX_REMAINING_ARRAY_LAYERS (~0u)

// Opaque handles.
typedef struct gfx_command_buffer_t* gfx_command_buffer;
typedef struct gfx_semaphore_t* gfx_semaphore;
typedef struct gfx_compute_pipeline_t* gfx_compute_pipeline;
typedef struct gfx_image_t* gfx_image;

typedef struct gfx_offset_2D
{
  int32_t x, y;
} gfx_offset_2D;

typedef struct gfx_offset_3D
{
  int32_t x, y, z;
} gfx_offset_3D;

typedef struct gfx_extent_2D
{
  uint32_t width, height;
} gfx_extent_2D;

typedef struct gfx_extent_3D
{
  uint32_t width, height, depth;
} gfx_extent_3D;

typedef struct gfx_submit_token
{
  gfx_semaphore semaphore;
  uint64_t value;
} gfx_submit_token;

typedef struct gfx_image_create_info
{
  gfx_image_type type;
  gfx_format format;
  gfx_extent_3D extent;
  uint32_t mip_levels;
  uint32_t array_layers;
} gfx_image_create_info;

typedef struct gfx_component_mapping
{
  gfx_component_swizzle r, g, b, a;
} gfx_component_mapping;

typedef struct gfx_image_view_create_info
{
  gfx_image base_image;
  gfx_image_type type;
  gfx_format format;
  gfx_component_mapping components;
  uint32_t base_mip_level;
  uint32_t level_count;
  uint32_t base_array_layer;
  uint32_t layer_count;
} gfx_image_view_create_info;

typedef struct gfx_sampled_image_descriptor
{
  uint32_t index;
} gfx_sampled_image_descriptor;

typedef struct gfx_storage_image_descriptor
{
  uint32_t index;
} gfx_storage_image_descriptor;

typedef struct gfx_copy_buffer_image_info
{
  const void* buffer;
  gfx_image image;
  gfx_aspect_flag_bits aspect;
  uint32_t mip_level;
  uint32_t base_array_layer;
  uint32_t layer_count;
  gfx_offset_3D offset;
  gfx_extent_3D extent;
  uint32_t row_length;
  uint32_t image_height;
} gfx_copy_buffer_image_info;

gfx_image gfx_create_image(const gfx_image_create_info* create_info);
gfx_image gfx_create_image_view(const gfx_image_view_create_info* create_info);
void gfx_destroy_image(gfx_image image);
gfx_sampled_image_descriptor gfx_get_sampled_image_descriptor(gfx_image image);
gfx_storage_image_descriptor gfx_get_storage_image_descriptor(gfx_image image);

gfx_command_buffer gfx_create_command_buffer(gfx_queue queue);
void gfx_destroy_command_buffer(gfx_command_buffer command_buffer);

gfx_submit_token gfx_submit(gfx_command_buffer command_buffer, const gfx_submit_token* wait_tokens, uint32_t num_wait_tokens);

void gfx_wait_submit(gfx_submit_token token);

gfx_compute_pipeline gfx_create_compute_pipeline(gfx_byte_span code);
void gfx_destroy_compute_pipeline(gfx_compute_pipeline pipeline);

void gfx_cmd_barrier(gfx_command_buffer command_buffer, gfx_stage_flags srcStage, gfx_access_flags srcAccess, gfx_stage_flags dstStage, gfx_access_flags dstAccess);
void gfx_cmd_dispatch(gfx_command_buffer command_buffer, gfx_compute_pipeline pipeline, uint32_t x, uint32_t y, uint32_t z, const void* args);
void gfx_cmd_init_discard_image(gfx_command_buffer command_buffer, gfx_image image);
void gfx_cmd_copy_buffer_to_image(gfx_command_buffer command_buffer, const gfx_copy_buffer_image_info* info);
void gfx_cmd_copy_image_to_buffer(gfx_command_buffer command_buffer, const gfx_copy_buffer_image_info* info);

void* gfx_malloc(size_t bytes);
void gfx_free(void* ptr);
void* gfx_host_to_device_ptr(void* ptr);


#ifdef __cplusplus
}
#endif