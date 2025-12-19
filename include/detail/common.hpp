#pragma once
#include "vulkan/vulkan_core.h"
#include <stdexcept>
#include <string>

template<typename T>
[[nodiscard]] T* ToPtr(T&& x)
{
  return &x;
}

inline void CheckVkResult(VkResult result)
{
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("vkResult was not VK_SUCCESS. Code: " + std::to_string(result));
  }
}