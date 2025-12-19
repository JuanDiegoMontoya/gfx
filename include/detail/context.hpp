#pragma once
#include "gfx2_vulkan.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan_core.h"

#include <cassert>
#include <vector>

namespace gfx2::internal
{
  struct MemoryMapping
  {
    uintptr_t begin;
    uintptr_t end;
    VkDeviceAddress deviceAddress;
    VmaAllocation allocation;
    VkBuffer buffer;
  };

  struct MemoryMappings
  {
    std::vector<MemoryMapping> mappings;

    VkDeviceAddress HostToDeviceAddress(void* ptr) const
    {
      const auto uPtr = reinterpret_cast<uintptr_t>(ptr);
      for (const auto& mapping : mappings)
      {
        if (uPtr >= mapping.begin && uPtr < mapping.end)
        {
          return mapping.deviceAddress + (uPtr - mapping.begin);
        }
      }

      assert(false);
      return 0;
    }
  };

  struct Context
  {
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;
    int32_t graphicsQueueFamilyIndex = -1;
    int32_t computeQueueFamilyIndex = -1;
    int32_t transferQueueFamilyIndex = -1;

    MemoryMappings memoryMappings;

    VmaAllocator allocator;
  };

  void CreateContextInstance(const gfx_vulkan_init_info& info);
  void DestroyContextInstance();
  Context& GetContextInstance();
}