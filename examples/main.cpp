#include "gfx2.h"
#include "gfx2_vulkan.h"
#include "detail/common.hpp"

#include "shaderc/shaderc.hpp"
#include "vulkan/vulkan_core.h"

#include <cassert>
#include <memory>
#include <print>
#include <string_view>
#include <vector>
#include <tuple>

namespace
{
  constexpr char sShader[] = // compute shader.
    R"(
#version 460 core
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout(binding = 1) uniform texture2D textures[];

layout(buffer_reference, buffer_reference_align = 4) buffer Data
{
  float value;
};

layout(buffer_reference) buffer Pointer
{
  Data data;
  uint descriptor;
};

layout(push_constant) uniform PC
{
  Pointer pointer;
};

layout(local_size_x = 2, local_size_y = 2) in;
void main()
{
  uvec2 gid = gl_GlobalInvocationID.xy;
  pointer.data[gid.x + gid.y * gl_WorkGroupSize.x].value = texelFetch(textures[pointer.descriptor], ivec2(gid), 0).r;
}
    )";

  struct ShaderCompileInfo
  {
    std::vector<uint32_t> binarySpv;
    uint32_t workgroupSize_[3];
  };

  ShaderCompileInfo CompileShaderToSpirv(shaderc_shader_kind glslangStage, std::string_view source)
  {
    auto compiler = shaderc::Compiler();
    auto options  = shaderc::CompileOptions();
    options.SetSourceLanguage(shaderc_source_language_glsl);
    options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
    options.SetTargetSpirv(shaderc_spirv_version_1_6);
    options.SetGenerateDebugInfo();

    auto result = compiler.CompileGlslToSpv(source.data(), source.size(), glslangStage, "shader.glsl", options);
    assert(result.GetNumErrors() == 0);

    return ShaderCompileInfo{.binarySpv = {result.begin(), result.end()}};
  }

  auto InitializeVulkan()
  {
    auto appInfo = VkApplicationInfo{
      .sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .apiVersion = VK_API_VERSION_1_3,
    };
    auto instanceInfo = VkInstanceCreateInfo{
      .sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
    };

    auto instance = VkInstance{};
    CheckVkResult(vkCreateInstance(&instanceInfo, nullptr, &instance));

    auto physicalDeviceCount = uint32_t{};
    CheckVkResult(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
    auto physicalDevices = std::make_unique<VkPhysicalDevice[]>(physicalDeviceCount);
    CheckVkResult(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.get()));

    auto physicalDevice = VkPhysicalDevice{};
    auto u32one         = uint32_t{1};
    CheckVkResult(vkEnumeratePhysicalDevices(instance, &u32one, &physicalDevice));
    
    auto graphicsQueueIndex = uint32_t{};
    auto computeQueueIndex  = uint32_t{};
    auto transferQueueIndex = uint32_t{};
    if (gfx_vulkan_get_queue_family_indices(physicalDevice, &graphicsQueueIndex, &computeQueueIndex, &transferQueueIndex))
    {
      assert(0);
      exit(1);
    }

    assert(graphicsQueueIndex != ~0u);

    auto f32one = float{1};

    auto queueCreateInfos = std::vector<VkDeviceQueueCreateInfo>();
    queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = graphicsQueueIndex,
      .queueCount       = 1,
      .pQueuePriorities = &f32one,
    });

    queueCreateInfos.push_back(queueCreateInfos.front());
    queueCreateInfos.back().queueFamilyIndex = computeQueueIndex;

    queueCreateInfos.push_back(queueCreateInfos.front());
    queueCreateInfos.back().queueFamilyIndex = transferQueueIndex;

    auto features13 = VkPhysicalDeviceVulkan13Features{};
    auto features12 = VkPhysicalDeviceVulkan12Features{};
    auto features11 = VkPhysicalDeviceVulkan11Features{};
    auto features10 = VkPhysicalDeviceFeatures2{};
    gfx_vulkan_get_required_features(&features10, &features11, &features12, &features13);

    auto deviceCreateInfo = VkDeviceCreateInfo{
      .sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext                = &features10,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos    = queueCreateInfos.data(),
    };

    auto device = VkDevice{};
    CheckVkResult(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));

    return std::make_tuple(instance, physicalDevice, device, graphicsQueueIndex, computeQueueIndex, transferQueueIndex);
  }
} // namespace

int main()
{
  auto [instance, physicalDevice, device, graphicsQueueIndex, computeQueueIndex, transferQueueIndex] = InitializeVulkan();

  auto initInfo = gfx_vulkan_init_info{
    .instance                 = instance,
    .physicalDevice           = physicalDevice,
    .device                   = device,
    .graphicsQueueFamilyIndex = graphicsQueueIndex,
    .computeQueueFamilyIndex  = computeQueueIndex,
    .transferQueueFamilyIndex = transferQueueIndex,
  };
  gfx_vulkan_initialize(&initInfo);

  const auto result = CompileShaderToSpirv(shaderc_compute_shader, sShader);
  const auto pipeline = gfx_create_compute_pipeline({.ptr = result.binarySpv.data(), .size = result.binarySpv.size() * sizeof(uint32_t)});
  
  const auto ToColor = [](uint8_t r, uint8_t g, uint8_t b, uint8_t a) -> uint32_t { return r | (g << 8) | (b << 16) | (a << 24); };

  {
    auto cmd     = gfx_create_command_buffer(GFX_QUEUE_COMPUTE);
    auto* memory = static_cast<uint32_t*>(gfx_malloc(sizeof(uint32_t) * 4));
    const auto gray  = ToColor(127, 127, 127, 255);
    const auto dark = ToColor(68, 68, 68, 255);
    memory[0]         = gray;
    memory[1]         = dark;
    memory[2]         = dark;
    memory[3]         = gray;

    auto image = gfx_create_image(ToPtr(gfx_image_create_info{
      .type         = GFX_IMAGE_TYPE_2D,
      .format       = GFX_FORMAT_R8G8B8A8_UNORM,
      .extent       = {2, 2, 1},
      .mip_levels   = 1,
      .array_layers = 1,
    }));

    gfx_cmd_init_discard_image(cmd, image);
    gfx_cmd_barrier(cmd, GFX_STAGE_TRANSFER, GFX_ACCESS_ALL, GFX_STAGE_TRANSFER, GFX_ACCESS_ALL);
    auto copy = gfx_copy_buffer_image_info{
      .buffer           = gfx_host_to_device_ptr(memory),
      .image            = image,
      .layer_count      = 1,
      .extent           = {2, 2, 1},
    };
    gfx_cmd_copy_buffer_to_image(cmd, &copy);
    gfx_cmd_barrier(cmd, GFX_STAGE_TRANSFER, GFX_ACCESS_ALL, GFX_STAGE_COMPUTE, GFX_ACCESS_ALL);

    struct PC
    {
      const void* data;
      uint32_t descriptor;
    };
    auto* pc = static_cast<PC*>(gfx_malloc(sizeof(PC)));
    auto* output = static_cast<float*>(gfx_malloc(sizeof(float) * 4));

    pc->data   = gfx_host_to_device_ptr(output);
    pc->descriptor = gfx_get_sampled_image_descriptor(image).index;
    gfx_cmd_dispatch(cmd, pipeline, 1, 1, 1, gfx_host_to_device_ptr(pc));

    auto token = gfx_submit(cmd, nullptr, 0);
    gfx_wait_submit(token);
    gfx_destroy_command_buffer(cmd);

    std::println("output:\n{}, {}\n{}, {}", output[0], output[1], output[2], output[3]);

    gfx_destroy_image(image);

    gfx_free(output);
    gfx_free(pc);
    gfx_free(memory);
  }

  gfx_destroy_compute_pipeline(pipeline);
  gfx_vulkan_shutdown();

  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);

  return 0;
}