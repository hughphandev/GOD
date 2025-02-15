#ifndef HZ_VULKAN_H
#define HZ_VULKAN_H

#include "hz_utils.h"
#include "hz_render.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#ifdef DEBUG
#define VK_CHECK(expression) ASSERT((expression) == VK_SUCCESS)
#else
#define VK_CHECK(expression) expression 
#endif

struct Renderer
{
    u32 gpuCount;
    VkPhysicalDevice* gpus;
    s32 gpuIndex;
    s32 graphicIndex;
    VkQueue graphicQueue;
    VkSurfaceFormatKHR surfaceFormat;
    VkDevice device;
    VkSwapchainKHR swapchain;
    u32 swapchainImgCount;
    VkImage* swapchainImgs;
    VkCommandPool cmdPool;

    VkSemaphore acquireSemaphore;
    VkSemaphore submitSemaphore;
};

char* VKGetDebugSeverityString(VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
    switch (severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT";
        default:
            return "Invalid case!";
    }
}

const char* VKGetDebugTypeString(VkDebugUtilsMessageTypeFlagsEXT type)
{
    switch (type)
    {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT ";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT ";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT ";
        default:
            return "Invalid case!";
    }
}


VkBool32 VKDebugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    printf("Debug callback: %s\n", pCallbackData->pMessage);
    printf("Severity: %s\n", VKGetDebugSeverityString(messageSeverity));
    printf("Type: %s\n", VKGetDebugTypeString(messageTypes));

    printf("Objects ");
    for (u32 i = 0; i < pCallbackData->objectCount; ++i)
    {
        printf("%llx ", pCallbackData->pObjects[i].objectHandle);
    }

    printf("\n-----------\n");

    return VK_FALSE;
}

void VKInit(Renderer* renderer, HINSTANCE hinstance, HWND hwnd, const char* name, MemoryArena* arena)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
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
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        #endif
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = ARRAY_COUNT(layers);
    instanceInfo.ppEnabledLayerNames = layers;
    instanceInfo.enabledExtensionCount = ARRAY_COUNT(extentions);
    instanceInfo.ppEnabledExtensionNames = extentions;
    VkInstance instance = {};
    VK_CHECK(vkCreateInstance(&instanceInfo, NULL, &instance));

    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.pNext = NULL;
    debugInfo.flags = 0;
    debugInfo.messageSeverity =
        // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    debugInfo.pfnUserCallback = &VKDebugCallBack;
    debugInfo.pUserData = NULL;

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debugInfo, NULL, &debugMessenger));

    VkSurfaceKHR surface;
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hinstance = hinstance;
    surfaceInfo.hwnd = hwnd;
    VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surfaceInfo, NULL, &surface));

    VK_CHECK(vkEnumeratePhysicalDevices(instance, &renderer->gpuCount, nullptr));
    renderer->gpus = PUSH_ARRAY(arena, VkPhysicalDevice, renderer->gpuCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &renderer->gpuCount, renderer->gpus));

    renderer->gpuIndex = -1;
    renderer->graphicIndex = -1;
    for (u32 i = 0; i < renderer->gpuCount; ++i)
    {
        VkPhysicalDevice* gpu = &renderer->gpus[i];

        u32 queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, NULL);
        VkQueueFamilyProperties* queueProperties = PUSH_ARRAY(arena, VkQueueFamilyProperties, queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, queueProperties);

        for (u32 qIndex = 0; qIndex < queueFamilyCount; ++qIndex)
        {
            if (OverlapFlag(queueProperties[qIndex].queueFlags, VK_QUEUE_GRAPHICS_BIT))
            {
                VkBool32 supported = false;
                VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(*gpu, qIndex, surface, &supported));
                if (supported)
                {
                    renderer->gpuIndex = i;
                    renderer->graphicIndex = qIndex;
                    break;
                }
            }
        }
    }

    if (renderer->gpuIndex < 0)
    {
        LOG_ERROR("Can't find any device!");
        return;
    }

    VkPhysicalDevice* gpu = &renderer->gpus[renderer->gpuIndex];

    f32 queuePiority = 1;
    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueCount = 1;
    queueInfo.queueFamilyIndex = renderer->graphicIndex;
    queueInfo.pQueuePriorities = &queuePiority;

    char* swapchainExts[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = ARRAY_COUNT(swapchainExts);
    deviceInfo.ppEnabledExtensionNames = swapchainExts;
    deviceInfo.enabledLayerCount = ARRAY_COUNT(layers);
    deviceInfo.ppEnabledLayerNames = layers;
    VK_CHECK(vkCreateDevice(*gpu, &deviceInfo, NULL, &renderer->device));

    vkGetDeviceQueue(renderer->device, renderer->graphicIndex, 0, &renderer->graphicQueue);

    VkSurfaceCapabilitiesKHR surfaceCap;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*gpu, surface, &surfaceCap));

    u32 surfaceFormatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(*gpu, surface, &surfaceFormatCount, NULL));
    VkSurfaceFormatKHR* surfaceFormats = PUSH_ARRAY(arena, VkSurfaceFormatKHR, surfaceFormatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(*gpu, surface, &surfaceFormatCount, surfaceFormats));
    for (u32 i = 0; i < surfaceFormatCount; ++i)
    {
        if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            renderer->surfaceFormat = surfaceFormats[i];
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = Min(surfaceCap.minImageCount + 1, surfaceCap.maxImageCount);
    swapchainInfo.imageFormat = renderer->surfaceFormat.format;
    // swapchainInfo.imageColorSpace;
    swapchainInfo.imageExtent = surfaceCap.currentExtent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // swapchainInfo.imageSharingMode;
    // swapchainInfo.queueFamilyIndexCount;
    // swapchainInfo.pQueueFamilyIndices;
    swapchainInfo.preTransform = surfaceCap.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // swapchainInfo.presentMode;
    // swapchainInfo.clipped;
    // swapchainInfo.oldSwapchain;
    VK_CHECK(vkCreateSwapchainKHR(renderer->device, &swapchainInfo, NULL, &renderer->swapchain));

    VK_CHECK(vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->swapchainImgCount, NULL));
    renderer->swapchainImgs = PUSH_ARRAY(arena, VkImage, renderer->swapchainImgCount);
    VK_CHECK(vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->swapchainImgCount, renderer->swapchainImgs));

    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = renderer->graphicIndex;

    VK_CHECK(vkCreateCommandPool(renderer->device, &cmdPoolInfo, NULL, &renderer->cmdPool));


    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(renderer->device, &semaphoreInfo, NULL, &renderer->acquireSemaphore);
    vkCreateSemaphore(renderer->device, &semaphoreInfo, NULL, &renderer->submitSemaphore);
}

void VkRenderOutput(RenderGroup* renderGroup)
{
    Renderer* renderer = renderGroup->renderer;
    MemoryArena pushBuffer = renderGroup->pushBuffer;

    u32 imgIndex;
    VK_CHECK(vkAcquireNextImageKHR(renderer->device, renderer->swapchain, 0, renderer->acquireSemaphore, 0, &imgIndex));

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = renderer->cmdPool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    VK_CHECK(vkAllocateCommandBuffers(renderer->device, &allocateInfo, &cmd));

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // render
    {
        VkClearColorValue color = { 1, 0, 0, 1 };
        VkImageSubresourceRange range = {};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.levelCount = 1;
        range.layerCount = 1;

        vkCmdClearColorImage(cmd, renderer->swapchainImgs[imgIndex], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &color, 1, &range);
    }

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags pipelineStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &pipelineStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &renderer->acquireSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderer->submitSemaphore;
    VK_CHECK(vkQueueSubmit(renderer->graphicQueue, 1, &submitInfo, 0));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &renderer->swapchain;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderer->submitSemaphore;
    presentInfo.pImageIndices = &imgIndex;

    VK_CHECK(vkQueuePresentKHR(renderer->graphicQueue, &presentInfo));

    VK_CHECK(vkDeviceWaitIdle(renderer->device));

    vkFreeCommandBuffers(renderer->device, renderer->cmdPool, 1, &cmd);
}

#endif