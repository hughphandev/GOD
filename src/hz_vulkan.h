#ifndef HZ_VULKAN_H
#define HZ_VULKAN_H

#include "hz_utils.h"
#include "hz_render.h"
#include "vulkan/vulkan.h"

void VKInit(const char* name)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(layerCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
    for (u32 i = 0; i < layerCount; ++i)
    {
        LOG_INFO(availableLayers[i].layerName);
    }
    free(availableLayers);

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = name;
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName = "Vulkan";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    char* layers[] =
    {
        "VK_LAYER_KHRONOS_validation"
    };
    char* extentions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        #if defined(_WIN32)
        "VK_KHR_win32_surface",
        #endif
    };

    VkInstanceCreateInfo instanceInfo = {};

    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = NULL;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = ARRAY_COUNT(layers);
    instanceInfo.ppEnabledLayerNames = layers;
    instanceInfo.enabledExtensionCount = ARRAY_COUNT(extentions);
    instanceInfo.ppEnabledExtensionNames = extentions;
    VkInstance instance = {};
    if (vkCreateInstance(&instanceInfo, NULL, &instance) == VK_SUCCESS)
    {
        LOG_INFO("Init Vulkan success!");
    }
}

#endif