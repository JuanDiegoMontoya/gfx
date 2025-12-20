#include "gfx2_vulkan.h"
#include "detail/context.hpp"

#include <memory>

gfx_error_t gfx_vulkan_initialize(const gfx_vulkan_init_info* initInfo)
{
  assert(initInfo);
  gfx2::internal::CreateContextInstance(*initInfo);
  return 0;
}

void gfx_vulkan_shutdown()
{
  gfx2::internal::DestroyContextInstance();
}

gfx_error_t gfx_vulkan_get_queue_family_indices(VkPhysicalDevice physicalDevice, uint32_t* graphicsIndex, uint32_t* computeIndex, uint32_t* transferIndex)
{
  assert(graphicsIndex);
  assert(computeIndex);
  assert(transferIndex);

  auto queueFamilyCount = uint32_t{};
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
  auto queueFamilies = std::make_unique<VkQueueFamilyProperties[]>(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.get());

  *graphicsIndex = ~0u;
  *computeIndex  = ~0u;
  *transferIndex = ~0u;

  for (uint32_t i = 0; i < queueFamilyCount; i++)
  {
    const auto& props = queueFamilies[i];
    if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT && props.queueFlags & VK_QUEUE_COMPUTE_BIT && props.queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
      *graphicsIndex = i;
    }
    else if (props.queueFlags & VK_QUEUE_COMPUTE_BIT && props.queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
      *computeIndex = i;
    }
    else if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
      *transferIndex = i;
    }
  }
  
  if (*graphicsIndex == ~0u)
    return 1;

  if (*computeIndex == ~0u)
    return 1;

  if (*transferIndex == ~0u)
    return 1;

  return 0;
}

void gfx_vulkan_get_required_features(VkPhysicalDeviceFeatures2* features10,
  VkPhysicalDeviceVulkan11Features* features11,
  VkPhysicalDeviceVulkan12Features* features12,
  VkPhysicalDeviceVulkan13Features* features13)
{
  assert(features10);
  assert(features11);
  assert(features12);
  assert(features13);

  *features13 = VkPhysicalDeviceVulkan13Features{
    .sType                          = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    .shaderDemoteToHelperInvocation = true,
    .shaderTerminateInvocation      = true,
    .subgroupSizeControl            = true,
    .synchronization2               = true,
    .dynamicRendering               = true,
    .shaderIntegerDotProduct        = true,
    .maintenance4                   = true,
  };

  *features12 = VkPhysicalDeviceVulkan12Features{
    .sType                                         = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
    .pNext                                         = features13,
    .drawIndirectCount                             = true,
    .storageBuffer8BitAccess                       = true,
    .uniformAndStorageBuffer8BitAccess             = true,
    .shaderFloat16                                 = true,
    .shaderInt8                                    = true,
    .descriptorIndexing                            = true,
    .shaderSampledImageArrayNonUniformIndexing     = true,
    .shaderStorageBufferArrayNonUniformIndexing    = true,
    .shaderStorageImageArrayNonUniformIndexing     = true,
    .descriptorBindingSampledImageUpdateAfterBind  = true,
    .descriptorBindingStorageImageUpdateAfterBind  = true,
    .descriptorBindingStorageBufferUpdateAfterBind = true,
    .descriptorBindingUpdateUnusedWhilePending     = true,
    .descriptorBindingPartiallyBound               = true,
    .descriptorBindingVariableDescriptorCount      = true,
    .runtimeDescriptorArray                        = true,
    .samplerFilterMinmax                           = true,
    .scalarBlockLayout                             = true,
    .imagelessFramebuffer                          = true,
    .uniformBufferStandardLayout                   = true,
    .shaderSubgroupExtendedTypes                   = true,
    .hostQueryReset                                = true,
    .timelineSemaphore                             = true,
    .bufferDeviceAddress                           = true,
    .vulkanMemoryModel                             = true,
    .vulkanMemoryModelDeviceScope                  = true,
    .subgroupBroadcastDynamicId                    = true,
  };

  *features11 = VkPhysicalDeviceVulkan11Features{
    .sType                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
    .pNext                              = features12,
    .storageBuffer16BitAccess           = true,
    .uniformAndStorageBuffer16BitAccess = true,
    .variablePointersStorageBuffer      = true,
    .variablePointers                   = true,
    .shaderDrawParameters               = true,
  };

  *features10 = VkPhysicalDeviceFeatures2{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    .pNext = features11,
    .features =
      {
        .multiDrawIndirect                       = true,
        .fillModeNonSolid                        = true,
        .samplerAnisotropy                       = true,
        .textureCompressionBC                    = true,
        .vertexPipelineStoresAndAtomics          = true,
        .fragmentStoresAndAtomics                = true,
        .shaderStorageImageExtendedFormats       = true,
        .shaderStorageImageReadWithoutFormat     = true,
        .shaderStorageImageWriteWithoutFormat    = true,
        .shaderUniformBufferArrayDynamicIndexing = true,
        .shaderSampledImageArrayDynamicIndexing  = true,
        .shaderStorageBufferArrayDynamicIndexing = true,
        .shaderStorageImageArrayDynamicIndexing  = true,
        .shaderClipDistance                      = true,
        .shaderCullDistance                      = true,
        .shaderFloat64                           = true,
        .shaderInt64                             = true,
        .shaderInt16                             = true,
      },
  };
}