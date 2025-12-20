#pragma once
#include "gfx2_vulkan.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan_core.h"

#include <cassert>
#include <vector>

struct gfx_compute_pipeline_t
{
  VkShaderModule shaderModule;
  VkPipeline pipeline;
};

struct gfx_command_buffer_t
{
  gfx_queue queue;
  VkCommandBuffer cmd;
};

struct gfx_semaphore_t
{
  VkSemaphore semaphore;
};

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
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkQueue queues[GFX_NUM_QUEUES];
    uint32_t graphicsQueueFamilyIndex;
    uint32_t computeQueueFamilyIndex;
    uint32_t transferQueueFamilyIndex;

    MemoryMappings memoryMappings;

    VmaAllocator allocator;
    VkCommandPool commandPools[GFX_NUM_QUEUES];
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout commonDescriptorSetLayout;
    VkPipelineLayout commonPipelineLayout;
  };

  void CreateContextInstance(const gfx_vulkan_init_info& info);
  void DestroyContextInstance();
  Context& GetContextInstance();
}