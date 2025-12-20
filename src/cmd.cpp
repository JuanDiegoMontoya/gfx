#include "gfx2.h"
#include "detail/common.hpp"
#include "detail/context.hpp"

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

void gfx_submit(gfx_command_buffer command_buffer, gfx_semaphore semaphore, uint64_t signal)
{
  auto& ctx = gfx2::internal::GetContextInstance();
  CheckVkResult(vkEndCommandBuffer(command_buffer->cmd));
  CheckVkResult(vkQueueSubmit2(ctx.queues[command_buffer->queue],
    1,
    ToPtr(VkSubmitInfo2{
      .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .commandBufferInfoCount   = 1,
      .pCommandBufferInfos      = ToPtr(VkCommandBufferSubmitInfo{
             .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
             .commandBuffer = command_buffer->cmd,
      }),
      .signalSemaphoreInfoCount = semaphore != nullptr ? 1u : 0u,
      .pSignalSemaphoreInfos    = ToPtr(VkSemaphoreSubmitInfo{
           .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
           .semaphore = semaphore->semaphore,
           .value     = signal,
           .stageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
      }),
    }),
    VK_NULL_HANDLE));
}

void gfx_cmd_dispatch(gfx_command_buffer command_buffer, gfx_compute_pipeline pipeline, uint32_t x, uint32_t y, uint32_t z, const void* args)
{
  auto& ctx = gfx2::internal::GetContextInstance();
  vkCmdBindPipeline(command_buffer->cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline);
  vkCmdPushConstants(command_buffer->cmd, ctx.commonPipelineLayout, VK_SHADER_STAGE_ALL, 0, 8, static_cast<const void*>(&args));
  vkCmdDispatch(command_buffer->cmd, x, y, z);
}