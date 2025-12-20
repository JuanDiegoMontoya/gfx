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
typedef struct VkPhysicalDeviceFeatures2 VkPhysicalDeviceFeatures2;
typedef struct VkPhysicalDeviceVulkan11Features VkPhysicalDeviceVulkan11Features;
typedef struct VkPhysicalDeviceVulkan12Features VkPhysicalDeviceVulkan12Features;
typedef struct VkPhysicalDeviceVulkan13Features VkPhysicalDeviceVulkan13Features;

typedef struct gfx_vulkan_init_info
{
  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  int32_t graphicsQueueFamilyIndex;
  int32_t computeQueueFamilyIndex;
  int32_t transferQueueFamilyIndex;
} gfx_vulkan_init_info;

gfx_error_t gfx_vulkan_initialize(const gfx_vulkan_init_info* initInfo);
void gfx_vulkan_shutdown();

gfx_error_t gfx_vulkan_get_queue_family_indices(VkPhysicalDevice physicalDevice, uint32_t* graphicsIndex, uint32_t* computeIndex, uint32_t* transferIndex);

void gfx_vulkan_get_required_features(VkPhysicalDeviceFeatures2* features10,
  VkPhysicalDeviceVulkan11Features* features11,
  VkPhysicalDeviceVulkan12Features* features12,
  VkPhysicalDeviceVulkan13Features* features13);


#ifdef __cplusplus
}
#endif