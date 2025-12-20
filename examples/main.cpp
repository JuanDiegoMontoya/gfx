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
  constexpr char sShader[] =
    R"(
      #version 460 core
      #extension GL_EXT_buffer_reference : require

      layout(buffer_reference) buffer Pointer
      {
        int value;
      };

      layout(push_constant) uniform PC
      {
        Pointer pointer;
      };

      layout(local_size_x = 1) in;
      void main()
      {
        pointer.value *= 2;
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
  
  {
    auto cmd       = gfx_create_command_buffer(GFX_QUEUE_COMPUTE);
    auto* memory   = (int*)gfx_malloc(sizeof(int));
    *memory        = 124;

    std::println("*memory was: {}", *memory);
    gfx_cmd_dispatch(cmd, pipeline, 1, 1, 1, gfx_host_to_device_ptr(memory));

    auto token = gfx_submit(cmd, nullptr, 0);
    gfx_wait_token(token);
    gfx_destroy_command_buffer(cmd);

    std::println("*memory is: {}", *memory);
    gfx_free(memory);
  }

  gfx_destroy_compute_pipeline(pipeline);
  gfx_vulkan_shutdown();

  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);

  return 0;
}