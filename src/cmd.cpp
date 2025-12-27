#include "gfx2.h"
#include "detail/common.hpp"
#include "detail/context.hpp"
#include "detail/image.hpp"

#include <vector>

namespace
{
  VkPipelineStageFlags2 ToVkStageFlags(gfx_stage_flags inFlags)
  {
    auto flags = VkPipelineStageFlags2{};

    flags |= inFlags & GFX_STAGE_VERTEX_SHADER ? VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT : 0;
    flags |= inFlags & GFX_STAGE_EARLY_FRAGMENT_TESTS ? VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT : 0;
    flags |= inFlags & GFX_STAGE_FRAGMENT_SHADER ? VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT : 0;
    flags |= inFlags & GFX_STAGE_LATE_FRAGMENT_TESTS ? VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT : 0;
    flags |= inFlags & GFX_STAGE_FRAGMENT_SHADER_OUTPUT ? VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT : 0;
    flags |= inFlags & GFX_STAGE_COMPUTE ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT : 0;
    flags |= inFlags & GFX_STAGE_TRANSFER ? VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT : 0;

    return flags;
  }

  VkAccessFlags2 ToVkAccessFlags(gfx_access_flags inFlags)
  {
    auto flags = VkAccessFlags2{};

    flags |= inFlags & GFX_ACCESS_READ ? VK_ACCESS_2_MEMORY_READ_BIT : 0;
    flags |= inFlags & GFX_ACCESS_WRITE ? VK_ACCESS_2_MEMORY_WRITE_BIT : 0;

    return flags;
  }

  VkImageAspectFlagBits ToVkAspectFlagBits(gfx_aspect_flag_bits inBit)
  {
    switch (inBit)
    {
    case GFX_ASPECT_COLOR: return VK_IMAGE_ASPECT_COLOR_BIT;
    case GFX_ASPECT_DEPTH: return VK_IMAGE_ASPECT_DEPTH_BIT;
    case GFX_ASPECT_STENCIL: return VK_IMAGE_ASPECT_STENCIL_BIT;
    default: assert(0); return {};
    }
  }
}

gfx_command_buffer gfx_create_command_buffer(gfx_queue queue)
{
  auto& ctx = gfx2::internal::GetContextInstance();

  auto* commandBuffer = new gfx_command_buffer_t();
  commandBuffer->queue = queue;
  CheckVkResult(vkAllocateCommandBuffers(ctx.device,
    ToPtr(VkCommandBufferAllocateInfo{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = ctx.commandPools[queue],
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
    }),
    &commandBuffer->cmd));

  CheckVkResult(vkBeginCommandBuffer(commandBuffer->cmd,
    ToPtr(VkCommandBufferBeginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    })));

  return commandBuffer;
}

void gfx_destroy_command_buffer(gfx_command_buffer command_buffer)
{
  auto& ctx = gfx2::internal::GetContextInstance();
  vkFreeCommandBuffers(ctx.device, ctx.commandPools[command_buffer->queue], 1, &command_buffer->cmd);
  delete command_buffer;
}

gfx_submit_token gfx_submit(gfx_command_buffer command_buffer, const gfx_submit_token* wait_tokens, uint32_t num_wait_tokens)
{
  assert(num_wait_tokens == 0 || wait_tokens != nullptr);
  auto& ctx = gfx2::internal::GetContextInstance();

  auto waitSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>();
  waitSemaphoreInfos.reserve(num_wait_tokens);
  for (uint32_t i = 0; i < num_wait_tokens; i++)
  {
    waitSemaphoreInfos.push_back(VkSemaphoreSubmitInfo{
      .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .semaphore = wait_tokens[i].semaphore->semaphore,
      .value     = wait_tokens[i].value,
      .stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    });
  }

  CheckVkResult(vkEndCommandBuffer(command_buffer->cmd));
  CheckVkResult(vkQueueSubmit2(ctx.queues[command_buffer->queue],
    1,
    ToPtr(VkSubmitInfo2{
      .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .waitSemaphoreInfoCount   = num_wait_tokens,
      .pWaitSemaphoreInfos      = waitSemaphoreInfos.data(),
      .commandBufferInfoCount   = 1,
      .pCommandBufferInfos      = ToPtr(VkCommandBufferSubmitInfo{
             .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
             .commandBuffer = command_buffer->cmd,
      }),
      .signalSemaphoreInfoCount = 1,
      .pSignalSemaphoreInfos    = ToPtr(VkSemaphoreSubmitInfo{
           .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
           .semaphore = ctx.semaphores[command_buffer->queue].semaphore,
           .value     = ++ctx.semaphoreValues[command_buffer->queue],
           .stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      }),
    }),
    VK_NULL_HANDLE));

  return {&ctx.semaphores[command_buffer->queue], ctx.semaphoreValues[command_buffer->queue]};
}

void gfx_wait_submit(gfx_submit_token token)
{
  auto& ctx = gfx2::internal::GetContextInstance();
  vkWaitSemaphores(ctx.device,
    ToPtr(VkSemaphoreWaitInfo{
      .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
      .semaphoreCount = 1,
      .pSemaphores    = &token.semaphore->semaphore,
      .pValues        = &token.value,
    }),
    UINT64_MAX);
}

void gfx_cmd_barrier(gfx_command_buffer command_buffer, gfx_stage_flags srcStage, gfx_access_flags srcAccess, gfx_stage_flags dstStage, gfx_access_flags dstAccess)
{
  vkCmdPipelineBarrier2(command_buffer->cmd,
    ToPtr(VkDependencyInfo{
      .sType              = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .memoryBarrierCount = 1,
      .pMemoryBarriers    = ToPtr(VkMemoryBarrier2{
           .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
           .srcStageMask  = ToVkStageFlags(srcStage),
           .srcAccessMask = ToVkAccessFlags(srcAccess),
           .dstStageMask  = ToVkStageFlags(dstStage),
           .dstAccessMask = ToVkAccessFlags(dstAccess),
      }),
    }));
}

void gfx_cmd_dispatch(gfx_command_buffer command_buffer, gfx_compute_pipeline pipeline, uint32_t x, uint32_t y, uint32_t z, const void* args)
{
  auto& ctx = gfx2::internal::GetContextInstance();
  vkCmdBindDescriptorSets(command_buffer->cmd, VK_PIPELINE_BIND_POINT_COMPUTE, ctx.commonPipelineLayout, 0, 1, &ctx.descriptorSet, 0, nullptr);
  vkCmdBindPipeline(command_buffer->cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline);
  vkCmdPushConstants(command_buffer->cmd, ctx.commonPipelineLayout, VK_SHADER_STAGE_ALL, 0, 8, static_cast<const void*>(&args));
  vkCmdDispatch(command_buffer->cmd, x, y, z);
}

void gfx_cmd_init_discard_image(gfx_command_buffer command_buffer, gfx_image image)
{
  vkCmdPipelineBarrier2(command_buffer->cmd,
    ToPtr(VkDependencyInfo{
      .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers    = ToPtr(VkImageMemoryBarrier2{
           .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
           // This is unsatisfying, but some arbitrary stage must be chosen to make barriers that sync the transition reasonable.
           .srcStageMask  = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT,
           .srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
           .dstStageMask  = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT,
           .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
           .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED,
           .newLayout     = VK_IMAGE_LAYOUT_GENERAL,
           .image         = image->internalImage->image,
           .subresourceRange =
          VkImageSubresourceRange{
               .aspectMask     = gfx2::internal::FormatToAspectFlags(gfx2::internal::VkToFormat(image->internalImage->createInfo.format)),
               .baseMipLevel   = 0,
               .levelCount     = VK_REMAINING_MIP_LEVELS,
               .baseArrayLayer = 0,
               .layerCount     = VK_REMAINING_ARRAY_LAYERS,
          },
      }),
    }));
}

void gfx_cmd_copy_buffer_to_image(gfx_command_buffer command_buffer, const gfx_copy_buffer_image_info* info)
{
  auto& ctx               = gfx2::internal::GetContextInstance();
  const auto mapping      = ctx.memoryMappings.DeviceAddressToMapping(info->buffer);
  const auto bufferOffset = reinterpret_cast<VkDeviceAddress>(info->buffer) - mapping.deviceAddress;
  // clang-format off
  vkCmdCopyBufferToImage2(command_buffer->cmd,
    ToPtr(VkCopyBufferToImageInfo2{
      .sType          = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
      .srcBuffer      = mapping.buffer,
      .dstImage       = info->image->internalImage->image,
      .dstImageLayout = VK_IMAGE_LAYOUT_GENERAL,
      .regionCount    = 1,
      .pRegions       = ToPtr(VkBufferImageCopy2{
        .sType             = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
        .bufferOffset      = bufferOffset,
        .bufferRowLength   = info->row_length,
        .bufferImageHeight = info->image_height,
        .imageSubresource  = VkImageSubresourceLayers{
          .aspectMask     = static_cast<VkImageAspectFlags>(ToVkAspectFlagBits(info->aspect)),
          .mipLevel       = info->mip_level,
          .baseArrayLayer = info->base_array_layer,
          .layerCount     = info->layer_count,
        },
        .imageOffset = {info->offset.x, info->offset.y, info->offset.z},
        .imageExtent = {info->extent.width, info->extent.height, info->extent.depth},
      }),
    }));
  // clang-format on
}

void gfx_cmd_copy_image_to_buffer(gfx_command_buffer command_buffer, const gfx_copy_buffer_image_info* info)
{
  auto& ctx               = gfx2::internal::GetContextInstance();
  const auto mapping      = ctx.memoryMappings.DeviceAddressToMapping(info->buffer);
  const auto bufferOffset = reinterpret_cast<VkDeviceAddress>(info->buffer) - mapping.deviceAddress;
  // clang-format off
  vkCmdCopyImageToBuffer2(command_buffer->cmd,
    ToPtr(VkCopyImageToBufferInfo2{
      .sType          = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2,
      .srcImage       = info->image->internalImage->image,
      .srcImageLayout = VK_IMAGE_LAYOUT_GENERAL,
      .dstBuffer      = mapping.buffer,
      .regionCount    = 1,
      .pRegions       = ToPtr(VkBufferImageCopy2{
        .sType             = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
        .bufferOffset      = bufferOffset,
        .bufferRowLength   = info->row_length,
        .bufferImageHeight = info->image_height,
        .imageSubresource  = VkImageSubresourceLayers{
          .aspectMask     = static_cast<VkImageAspectFlags>(ToVkAspectFlagBits(info->aspect)),
          .mipLevel       = info->mip_level,
          .baseArrayLayer = info->base_array_layer,
          .layerCount     = info->layer_count,
        },
        .imageOffset = {info->offset.x, info->offset.y, info->offset.z},
        .imageExtent = {info->extent.width, info->extent.height, info->extent.depth},
      }),
    }));
  // clang-format on
}