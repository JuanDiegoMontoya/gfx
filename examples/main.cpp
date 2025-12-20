#include "gfx2.h"
#include "gfx2_vulkan.h"
#include "detail/common.hpp"

#include "shaderc/shaderc.hpp"
#include "vulkan/vulkan_core.h"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

constexpr char sShader[] = // compute shadera
R"(
#version 460 core
layout(local_size_x = 2) in;
void main() {}
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

int main()
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

  auto queueFamilyCount = uint32_t{};
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
  auto queueFamilies = std::make_unique<VkQueueFamilyProperties[]>(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.get());

  auto graphicsQueueIndex = uint32_t{~0u};
  auto computeQueueIndex  = uint32_t{~0u};
  auto transferQueueIndex = uint32_t{~0u};

  for (uint32_t i = 0; i < queueFamilyCount; i++)
  {
    const auto& props = queueFamilies[i];
    if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT && props.queueFlags & VK_QUEUE_COMPUTE_BIT && props.queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
      graphicsQueueIndex = i;
    }
    else if (props.queueFlags & VK_QUEUE_COMPUTE_BIT && props.queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
      computeQueueIndex = i;
    }
    else if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
      transferQueueIndex = i;
    }
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

  if (computeQueueIndex != ~0u)
  {
    queueCreateInfos.push_back(queueCreateInfos.front());
    queueCreateInfos.back().queueFamilyIndex = computeQueueIndex;
  }

  if (transferQueueIndex != ~0u)
  {
    queueCreateInfos.push_back(queueCreateInfos.front());
    queueCreateInfos.back().queueFamilyIndex = transferQueueIndex;
  }

  auto features13 = VkPhysicalDeviceVulkan13Features{
    .sType                          = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    .shaderDemoteToHelperInvocation = true,
    .shaderTerminateInvocation      = true,
    .subgroupSizeControl            = true,
    .synchronization2               = true,
    .dynamicRendering               = true,
    .shaderIntegerDotProduct        = true,
    .maintenance4                   = true,
  };

  auto features12 = VkPhysicalDeviceVulkan12Features{
    .sType                                         = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
    .pNext                                         = &features13,
    .drawIndirectCount                             = true,
    .storageBuffer8BitAccess                       = true,
    .uniformAndStorageBuffer8BitAccess             = true,
    .shaderFloat16                                 = true,
    .shaderInt8                                    = true,
    .descriptorIndexing                            = true,
    .shaderSampledImageArrayNonUniformIndexing     = true,
    .shaderStorageBufferArrayNonUniformIndexing    = true,
    .shaderStorageImageArrayNonUniformIndexing     = true,
    .descriptorBindingSampledImageUpdateAfterBind  = true,
    .descriptorBindingStorageImageUpdateAfterBind  = true,
    .descriptorBindingStorageBufferUpdateAfterBind = true,
    .descriptorBindingUpdateUnusedWhilePending     = true,
    .descriptorBindingPartiallyBound               = true,
    .descriptorBindingVariableDescriptorCount      = true,
    .runtimeDescriptorArray                        = true,
    .samplerFilterMinmax                           = true,
    .scalarBlockLayout                             = true,
    .imagelessFramebuffer                          = true,
    .uniformBufferStandardLayout                   = true,
    .shaderSubgroupExtendedTypes                   = true,
    .hostQueryReset                                = true,
    .timelineSemaphore                             = true,
    .bufferDeviceAddress                           = true,
    .vulkanMemoryModel                             = true,
    .vulkanMemoryModelDeviceScope                  = true,
    .subgroupBroadcastDynamicId                    = true,
  };

  auto features11 = VkPhysicalDeviceVulkan11Features{
    .sType                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
    .pNext = &features12,
    .storageBuffer16BitAccess           = true,
    .uniformAndStorageBuffer16BitAccess = true,
    .variablePointersStorageBuffer      = true,
    .variablePointers                   = true,
    .shaderDrawParameters               = true,
  };

  auto features = VkPhysicalDeviceFeatures2{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    .pNext = &features11,
    .features =
      {
        .multiDrawIndirect                 = true,
        .fillModeNonSolid                  = true,
        .samplerAnisotropy                 = true,
        .textureCompressionBC              = true,
        .vertexPipelineStoresAndAtomics    = true,
        .fragmentStoresAndAtomics          = true,
        .shaderStorageImageExtendedFormats = true,
        // Apparently the next two features are not needed with 1.3 since you can query support in a granular fashion
        .shaderStorageImageReadWithoutFormat     = true,
        .shaderStorageImageWriteWithoutFormat    = true,
        .shaderUniformBufferArrayDynamicIndexing = true,
        .shaderSampledImageArrayDynamicIndexing  = true,
        .shaderStorageBufferArrayDynamicIndexing = true,
        .shaderStorageImageArrayDynamicIndexing  = true,
        .shaderClipDistance                      = true,
        .shaderCullDistance                      = true,
        .shaderFloat64                           = true,
        .shaderInt64                             = true,
        .shaderInt16                             = true,
      },
  };

  auto deviceCreateInfo = VkDeviceCreateInfo{
    .sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext                = &features,
    .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
    .pQueueCreateInfos    = queueCreateInfos.data(),
    //.enabledExtensionCount   =,
    //.ppEnabledExtensionNames =,
  };

  auto device = VkDevice{};
  CheckVkResult(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));

  auto graphicsQueue = VkQueue{};
  vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);

  auto computeQueue = graphicsQueue;
  if (computeQueueIndex != ~0u)
  {
    vkGetDeviceQueue(device, computeQueueIndex, 0, &computeQueue);
  }

  auto transferQueue = computeQueueIndex != ~0u ? computeQueue : graphicsQueue;
  if (transferQueueIndex != ~0u)
  {
    vkGetDeviceQueue(device, transferQueueIndex, 0, &transferQueue);
  }

  auto initInfo = gfx_vulkan_init_info{
    .instance       = instance,
    .device         = device,
    .physicalDevice = physicalDevice,
    .graphicsQueue  = graphicsQueue,
    .computeQueue   = computeQueue,
    .transferQueue = transferQueue,
    .graphicsQueueFamilyIndex = static_cast<int32_t>(graphicsQueueIndex),
    .computeQueueFamilyIndex  = static_cast<int32_t>(computeQueueIndex),
    .transferQueueFamilyIndex = static_cast<int32_t>(transferQueueIndex),
  };
  gfx_vulkan_initialize(&initInfo);


  const auto result = CompileShaderToSpirv(shaderc_compute_shader, sShader);
  const auto pipeline = gfx_create_compute_pipeline({.ptr = result.binarySpv.data(), .size = result.binarySpv.size() * sizeof(uint32_t)});

  gfx_free_compute_pipeline(pipeline);
  gfx_vulkan_shutdown();

  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);

  return 0;
}