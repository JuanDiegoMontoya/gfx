#include "gfx2.h"
#include "detail/common.hpp"
#include "detail/context.hpp"

gfx_semaphore gfx_create_semaphore(uint64_t initial_value)
{
  auto* semaphore = new gfx_semaphore_t();

  auto& ctx = gfx2::internal::GetContextInstance();
  CheckVkResult(vkCreateSemaphore(ctx.device,
    ToPtr(VkSemaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = ToPtr(VkSemaphoreTypeCreateInfo{
        .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue  = initial_value,
      }),
    }),
    nullptr,
    &semaphore->semaphore));

  return semaphore;
}

void gfx_wait_semaphore(gfx_semaphore semaphore, uint64_t value)
{
  auto& ctx = gfx2::internal::GetContextInstance();
  vkWaitSemaphores(ctx.device,
    ToPtr(VkSemaphoreWaitInfo{
      .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
      .semaphoreCount = 1,
      .pSemaphores    = &semaphore->semaphore,
      .pValues        = &value,
    }),
    UINT64_MAX);
}

void gfx_destroy_semaphore(gfx_semaphore semaphore)
{
  auto& ctx = gfx2::internal::GetContextInstance();
  vkDestroySemaphore(ctx.device, semaphore->semaphore, nullptr);
  delete semaphore;
}