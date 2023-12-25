#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#include <stdbool.h>
#include <vulkan/vulkan_core.h>

const char* vulkan_result_to_str(VkResult result, bool extended);
bool vulkan_result_is_ok(VkResult result);

#endif
