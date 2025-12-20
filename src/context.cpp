#include "detail/context.hpp"
#include "detail/common.hpp"

#include <array>
#include <cassert>

namespace
{
  gfx2::internal::Context* sContext;
}

namespace
{
  void CreateVmaAllocator(gfx2::internal::Context& ctx)
  {
    CheckVkResult(vmaCreateAllocator(ToPtr(VmaAllocatorCreateInfo{
      .flags          = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = ctx.physicalDevice,
      .device         = ctx.device,
      //.pDeviceMemoryCallbacks = ToPtr(VmaDeviceMemoryCallbacks{
      //  .pfnAllocate = DeviceAllocCallback,
      //  .pfnFree     = DeviceFreeCallback,
      //}),
      .pVulkanFunctions = ToPtr(VmaVulkanFunctions{
        .vkGetInstanceProcAddr                   = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr                     = vkGetDeviceProcAddr,
        .vkGetPhysicalDeviceProperties           = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties     = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory                        = vkAllocateMemory,
        .vkFreeMemory                            = vkFreeMemory,
        .vkMapMemory                             = vkMapMemory,
        .vkUnmapMemory                           = vkUnmapMemory,
        .vkFlushMappedMemoryRanges               = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges          = vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory                      = vkBindBufferMemory,
        .vkBindImageMemory                       = vkBindImageMemory,
        .vkGetBufferMemoryRequirements           = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements            = vkGetImageMemoryRequirements,
        .vkCreateBuffer                          = vkCreateBuffer,
        .vkDestroyBuffer                         = vkDestroyBuffer,
        .vkCreateImage                           = vkCreateImage,
        .vkDestroyImage                          = vkDestroyImage,
        .vkCmdCopyBuffer                         = vkCmdCopyBuffer,
        .vkGetBufferMemoryRequirements2KHR       = vkGetBufferMemoryRequirements2,
        .vkGetImageMemoryRequirements2KHR        = vkGetImageMemoryRequirements2,
        .vkBindBufferMemory2KHR                  = vkBindBufferMemory2,
        .vkBindImageMemory2KHR                   = vkBindImageMemory2,
        .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2,
      }),
      .instance         = ctx.instance,
      .vulkanApiVersion = VK_API_VERSION_1_3,
    }), &ctx.allocator));
  }

  void CreateDescriptorSet(gfx2::internal::Context& ctx)
  {
    auto poolSizes = std::array<VkDescriptorPoolSize, 2>();
    poolSizes[0]   = {.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1'000'000};
    poolSizes[1]   = {.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1000};

    CheckVkResult(vkCreateDescriptorPool(ctx.device,
      ToPtr(VkDescriptorPoolCreateInfo{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets       = 1,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes    = poolSizes.data(),
      }),
      nullptr,
      &ctx.descriptorPool));

    auto bindings = std::array<VkDescriptorSetLayoutBinding, 2>();
    bindings[0]   = {.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1'000'000, .stageFlags = VK_SHADER_STAGE_ALL};
    bindings[1]   = {.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1000, .stageFlags = VK_SHADER_STAGE_ALL};

    auto bindingsFlags = std::array<VkDescriptorBindingFlags, 2>({
      VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
      VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
    });

    CheckVkResult(vkCreateDescriptorSetLayout(ctx.device,
      ToPtr(VkDescriptorSetLayoutCreateInfo{
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = ToPtr(VkDescriptorSetLayoutBindingFlagsCreateInfo{
                 .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                 .bindingCount  = static_cast<uint32_t>(bindingsFlags.size()),
                 .pBindingFlags = bindingsFlags.data(),
        }),
        .flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings    = bindings.data(),
      }),
      nullptr,
      &ctx.commonDescriptorSetLayout));

    CheckVkResult(vkCreatePipelineLayout(ctx.device,
      ToPtr(VkPipelineLayoutCreateInfo{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &ctx.commonDescriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = ToPtr(VkPushConstantRange{
             .stageFlags = VK_SHADER_STAGE_ALL,
             .offset     = 0,
             .size       = 8,
        }),
      }),
      nullptr,
      &ctx.commonPipelineLayout));
  }

  void CreateCommandPools(gfx2::internal::Context& ctx)
  {
    CheckVkResult(vkCreateCommandPool(ctx.device,
      ToPtr(VkCommandPoolCreateInfo{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = ctx.graphicsQueueFamilyIndex,
      }),
      nullptr,
      &ctx.commandPools[GFX_QUEUE_GRAPHICS]));

    CheckVkResult(vkCreateCommandPool(ctx.device,
      ToPtr(VkCommandPoolCreateInfo{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = ctx.computeQueueFamilyIndex,
      }),
      nullptr,
      &ctx.commandPools[GFX_QUEUE_COMPUTE]));

    CheckVkResult(vkCreateCommandPool(ctx.device,
      ToPtr(VkCommandPoolCreateInfo{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = ctx.transferQueueFamilyIndex,
      }),
      nullptr,
      &ctx.commandPools[GFX_QUEUE_TRANSFER]));
  }
}

void gfx2::internal::CreateContextInstance(const gfx_vulkan_init_info& info)
{
  assert(!sContext);

  sContext = new Context();

  sContext->instance       = info.instance;
  sContext->device         = info.device;
  sContext->physicalDevice = info.physicalDevice;

  sContext->graphicsQueueFamilyIndex = info.graphicsQueueFamilyIndex;
  sContext->computeQueueFamilyIndex  = info.computeQueueFamilyIndex;
  sContext->transferQueueFamilyIndex = info.transferQueueFamilyIndex;
  
  vkGetDeviceQueue(sContext->device, info.graphicsQueueFamilyIndex, 0, &sContext->queues[GFX_QUEUE_GRAPHICS]);
  vkGetDeviceQueue(sContext->device, info.computeQueueFamilyIndex, 0, &sContext->queues[GFX_QUEUE_COMPUTE]);
  vkGetDeviceQueue(sContext->device, info.transferQueueFamilyIndex, 0, &sContext->queues[GFX_QUEUE_TRANSFER]);

  for (auto& [semaphore] : sContext->semaphores)
  {
    CheckVkResult(vkCreateSemaphore(sContext->device,
      ToPtr(VkSemaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = ToPtr(VkSemaphoreTypeCreateInfo{
          .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
          .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
          .initialValue  = 0,
        }),
      }),
      nullptr,
      &semaphore));
  }

  for (auto& value : sContext->semaphoreValues)
  {
    value = 0;
  }

  CreateVmaAllocator(*sContext);
  CreateDescriptorSet(*sContext);
  CreateCommandPools(*sContext);
}

void gfx2::internal::DestroyContextInstance()
{
  assert(sContext);

  vkDestroyPipelineLayout(sContext->device, sContext->commonPipelineLayout, nullptr);
  vkDestroyDescriptorSetLayout(sContext->device, sContext->commonDescriptorSetLayout, nullptr);
  vkDestroyDescriptorPool(sContext->device, sContext->descriptorPool, nullptr);

  vkDestroyCommandPool(sContext->device, sContext->commandPools[GFX_QUEUE_TRANSFER], nullptr);
  vkDestroyCommandPool(sContext->device, sContext->commandPools[GFX_QUEUE_COMPUTE], nullptr);
  vkDestroyCommandPool(sContext->device, sContext->commandPools[GFX_QUEUE_GRAPHICS], nullptr);

  vmaDestroyAllocator(sContext->allocator);

  for (auto [semaphore] : sContext->semaphores)
  {
    vkDestroySemaphore(sContext->device, semaphore, nullptr);
  }

  delete sContext;
  sContext = nullptr;
}

gfx2::internal::Context& gfx2::internal::GetContextInstance()
{
  assert(sContext);
  return *sContext;
}