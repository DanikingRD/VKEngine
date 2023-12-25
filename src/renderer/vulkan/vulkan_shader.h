#ifndef VULKAN_SHADER_MODULE_H
#define VULKAN_SHADER_MODULE_H

#include "vulkan_types.h"

bool vulkan_shader_create(VulkanBackend* backend, Shader* shader);
void vulkan_shader_bind(VulkanBackend* backend, Shader* shader);
void vulkan_shader_destroy(VulkanBackend* backend, Shader* shader);

#endif