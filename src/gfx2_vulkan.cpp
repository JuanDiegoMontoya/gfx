#include "gfx2_vulkan.h"
#include "detail/context.hpp"

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