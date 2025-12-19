#pragma once
#include "gfx2.h"

typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;

typedef struct gfx_vulkan_init_info
{
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue computeQueue;
  VkQueue transferQueue;
} gfx_vulkan_init_info;

gfx_error_t gfx_vulkan_initialize(const gfx_vulkan_init_info* initInfo);
void gfx_vulkan_shutdown();