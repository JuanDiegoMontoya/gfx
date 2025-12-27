#pragma once
#include "gfx2_vulkan.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan_core.h"

#include <cassert>
#include <stack>
#include <vector>

struct gfx_compute_pipeline_t
{
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

    VkDeviceAddress HostToDeviceAddress(const void* ptr) const
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

    MemoryMapping DeviceAddressToMapping(const void* ptr) const
    {
      const auto uPtr = reinterpret_cast<uintptr_t>(ptr);
      for (const auto& mapping : mappings)
      {
        if (uPtr >= mapping.deviceAddress && uPtr < mapping.deviceAddress + (mapping.end - mapping.begin))
        {
          return mapping;
        }
      }

      assert(false);
      return {};
    }
  };

  class IndexAllocator
  {
  public:
    IndexAllocator() = default;
    IndexAllocator(uint32_t numIndices);

    [[nodiscard]] uint32_t Allocate();
    void Free(uint32_t index);

  private:
    std::stack<uint32_t> freeSlots_;
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
    IndexAllocator sampledImageDescriptorAllocator;
    IndexAllocator storageImageDescriptorAllocator;
    IndexAllocator samplerDescriptorAllocator;

    gfx_semaphore_t semaphores[GFX_NUM_QUEUES];
    uint64_t semaphoreValues[GFX_NUM_QUEUES];
    VmaAllocator allocator;
    VkCommandPool commandPools[GFX_NUM_QUEUES];
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout commonDescriptorSetLayout;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout commonPipelineLayout;

    static constexpr uint32_t STORAGE_IMAGE_BINDING = 0;
    static constexpr uint32_t SAMPLED_IMAGE_BINDING = 1;
    static constexpr uint32_t SAMPLER = 2;
  };

  void CreateContextInstance(const gfx_vulkan_init_info& info);
  void DestroyContextInstance();
  Context& GetContextInstance();
}