#include "gfx2.h"
#include "detail/common.hpp"
#include "detail/context.hpp"

gfx_compute_pipeline gfx_create_compute_pipeline(gfx_byte_span code)
{
  auto& ctx = gfx2::internal::GetContextInstance();

  auto* pipeline = new gfx_compute_pipeline_t{};
  auto shaderModule = VkShaderModule{};
  vkCreateShaderModule(ctx.device,
    ToPtr(VkShaderModuleCreateInfo{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = code.size,
      .pCode    = static_cast<const uint32_t*>(code.ptr),
    }),
    nullptr,
    &shaderModule);

  CheckVkResult(vkCreateComputePipelines(ctx.device,
    VK_NULL_HANDLE,
    1,
    ToPtr(VkComputePipelineCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage =
        VkPipelineShaderStageCreateInfo{
          .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
          .module              = shaderModule,
          .pName               = "main",
          .pSpecializationInfo = nullptr,
        },
      .layout = ctx.commonPipelineLayout,
    }),
    nullptr,
    &pipeline->pipeline));

  vkDestroyShaderModule(ctx.device, shaderModule, nullptr);

  return pipeline;
}

void gfx_destroy_compute_pipeline(gfx_compute_pipeline pipeline)
{
  assert(pipeline);
  auto& ctx = gfx2::internal::GetContextInstance();

  vkDestroyPipeline(ctx.device, pipeline->pipeline, nullptr);
  delete pipeline;
}