#include "vulkan_backend.h"
#include "collections/vector.h"
#include "core/log.h"
#include "core/str.h"
#include "defines.h"
#include "vulkan_device.h"
#include "vulkan_types.h"

VkBool32 vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
                               const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* data);

static VulkanBackend backend;

bool vulkan_backend_create(const char* app_name, Window* window) {
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = app_name;
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    Vector(const char*) extensions = vector_new(const char*);
    Vector(const char*) validation_layers = 0; // Optional
    window_required_vulkan_extensions(&extensions);
    vector_push(extensions, &VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

#ifdef _DEBUG
    vector_push(extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    DEBUG("Required Extensions: ");
    for (u32 i = 0; i < vector_length(extensions); i++) {
        DEBUG(" - %s ", extensions[i]);
    }
    validation_layers = vector_new(const char*);
    vector_push(validation_layers, &"VK_LAYER_KHRONOS_validation");
    u32 layer_count;
    VK_FN_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, 0));
    VkLayerProperties available_layers[layer_count];
    VK_FN_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, available_layers));

    DEBUG("Required Validation Layers: ");
    for (u32 i = 0; i < 1; i++) {
        bool found = false;
        const char* required_layer = validation_layers[i];
        DEBUG(" - %s", required_layer);
        for (u32 j = 0; j < layer_count; j++) {
            if (str_equals(required_layer, available_layers[j].layerName)) {
                found = true;
                break;
            }
        }
        if (!found) {
            ERROR("Missing Vulkan Validation Layer: %s", required_layer);
        }
    }
    DEBUG("Validation Layers available");
#endif
    create_info.enabledExtensionCount = vector_length(extensions);
    create_info.ppEnabledExtensionNames = extensions;
    create_info.enabledLayerCount = vector_length(validation_layers);
    create_info.ppEnabledLayerNames = validation_layers;
    create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    VK_FN_CHECK(vkCreateInstance(&create_info, backend.allocator, &backend.instance));

    vector_free(extensions);
    vector_free(validation_layers);

#ifdef _DEBUG
    u32 severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    // NOTE: It might be useful to add info & verbose severity for debugging
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.messageSeverity = severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = vulkan_debug_callback;
    debug_create_info.pUserData = 0;
    PFN_vkCreateDebugUtilsMessengerEXT fn =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(backend.instance, "vkCreateDebugUtilsMessengerEXT");
    VK_FN_CHECK(fn(backend.instance, &debug_create_info, backend.allocator, &backend.debugger));
    DEBUG("Vulkan Debugger created");
#endif
    window_create_vulkan_surface(window, &backend);

    vulkan_device_create(&backend);
    return true;
}
void vulkan_backend_resize(u16 width, u16 height) {}
bool vulkan_backend_begin_frame(f32 dt) { return true; }
bool vulkan_backend_end_frame(f32 dt) { return true; }
void vulkan_backend_destroy(void) {}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT type,
                                                     const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                     void* data) {
    UNUSED(severity);
    UNUSED(data);
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        TRACE(callback_data->pMessage);
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        INFO(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        WARN(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ERROR(callback_data->pMessage);
        break;
    }
    return VK_FALSE;
}
