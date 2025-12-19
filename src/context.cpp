#include "detail/context.hpp"

#include <cassert>

namespace
{
  gfx2::internal::Context* sContext;
}

void gfx2::internal::CreateContextInstance(const gfx_vulkan_init_info& info)
{
  assert(!sContext);

  sContext = new Context();

  sContext->device = info.device;
  sContext->graphicsQueue = info.graphicsQueue;
  sContext->computeQueue = info.computeQueue;
  sContext->transferQueue = info.transferQueue;
}

void gfx2::internal::DestroyContextInstance()
{
  assert(sContext);
  delete sContext;
}

gfx2::internal::Context& gfx2::internal::GetContextInstance()
{
  assert(sContext);
  return *sContext;
}