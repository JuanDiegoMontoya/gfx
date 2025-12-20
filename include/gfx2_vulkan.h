#pragma once
#include "gfx2.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct VkInstance_T* VkInstance;
typedef struct VkDevice_T* VkDevice;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkQueue_T* VkQueue;

typedef struct gfx_vulkan_init_info
{
  VkInstance instance;
  VkDevice device;
  VkPhysicalDevice physicalDevice;
  VkQueue graphicsQueue;
  VkQueue computeQueue;
  VkQueue transferQueue;
  int32_t graphicsQueueFamilyIndex;
  int32_t computeQueueFamilyIndex;
  int32_t transferQueueFamilyIndex;
} gfx_vulkan_init_info;

gfx_error_t gfx_vulkan_initialize(const gfx_vulkan_init_info* initInfo);
void gfx_vulkan_shutdown();


#ifdef __cplusplus
}
#endif