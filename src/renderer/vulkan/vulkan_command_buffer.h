#ifndef VULKAN_COMMAND_BUFFER_H
#define VULKAN_COMMAND_BUFFER_H
#include "vulkan_types.h"

void vulkan_command_buffer_allocate(VulkanBackend* backend, VkCommandPool pool, bool is_primary,
                                    CommandBuffer* out_command_buffer);

void vulkan_command_buffer_begin(CommandBuffer* command_buffer, bool single_use,
                                 bool render_pass_continue, bool simultaneous_use);

void vulkan_command_buffer_end(CommandBuffer* command_buffer);

void vulkan_command_buffer_set_submitted(CommandBuffer* command_buffer);

void vulkan_command_buffer_allocate_and_begin_single_use(VulkanBackend* backend, VkCommandPool pool,
                                                         CommandBuffer* out_command_buffer);

void vulkan_command_buffer_end_single_use(VulkanBackend* backend, VkCommandPool pool,
                                          CommandBuffer* out_command_buffer, VkQueue queue);

void vulkan_command_buffer_free(VulkanBackend* backend, VkCommandPool pool,
                                CommandBuffer* command_buffer);

#endif
