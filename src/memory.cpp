#include "detail/common.hpp"
#include "detail/context.hpp"
#include "gfx2.h"

#include <algorithm>
#include <cassert>

void* gfx_malloc(size_t bytes)
{
  assert(bytes > 0);

  auto& ctx = gfx2::internal::GetContextInstance();

  VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                             VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

  auto queueFamilyIndices = std::vector<uint32_t>();
  queueFamilyIndices.push_back(ctx.graphicsQueueFamilyIndex);
  if (ctx.computeQueueFamilyIndex != -1)
    queueFamilyIndices.push_back(ctx.computeQueueFamilyIndex);
  if (ctx.transferQueueFamilyIndex != -1)
    queueFamilyIndices.push_back(ctx.transferQueueFamilyIndex);

  auto mapping = gfx2::internal::MemoryMapping{};
  auto allocationInfo = VmaAllocationInfo{};
  vmaCreateBuffer(ctx.allocator,
    ToPtr(VkBufferCreateInfo{
      .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size                  = bytes,
      .usage                 = usage,
      .sharingMode           = VK_SHARING_MODE_CONCURRENT,
      .queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size()),
      .pQueueFamilyIndices   = queueFamilyIndices.data(),
    }),
    ToPtr(VmaAllocationCreateInfo{
      .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    }),
    &mapping.buffer,
    &mapping.allocation,
    &allocationInfo);

  mapping.deviceAddress = vkGetBufferDeviceAddress(ctx.device,
    ToPtr(VkBufferDeviceAddressInfo{
      .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .buffer = mapping.buffer,
    }));

  mapping.begin = reinterpret_cast<uintptr_t>(allocationInfo.pMappedData);
  mapping.end   = mapping.begin + bytes;

  ctx.memoryMappings.mappings.emplace_back(mapping);

  return allocationInfo.pMappedData;
}

void gfx_free(void* ptr)
{
  auto& ctx = gfx2::internal::GetContextInstance();
  auto it = std::ranges::find_if(ctx.memoryMappings.mappings, [ptr](const auto& mapping) -> bool
  { 
    return mapping.begin == reinterpret_cast<uintptr_t>(ptr);
  });

  assert(it != ctx.memoryMappings.mappings.end());

  vmaDestroyBuffer(ctx.allocator, it->buffer, it->allocation);

  ctx.memoryMappings.mappings.erase(it);
}

void* gfx_host_to_device_ptr(void* ptr)
{
  static_assert(sizeof(void*) == sizeof(VkDeviceAddress));

  auto& ctx = gfx2::internal::GetContextInstance();
  return reinterpret_cast<void*>(ctx.memoryMappings.HostToDeviceAddress(ptr));
}