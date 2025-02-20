#ifndef HZ_VULKAN_H
#define HZ_VULKAN_H

#include "hz_utils.h"
#include "hz_render.h"
#include "hz_io.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#ifdef DEBUG
#define VK_CHECK(expression) ASSERT((expression) == VK_SUCCESS)
#else
#define VK_CHECK(expression) expression 
#endif

struct HZVKMemoryArena
{
    VkDeviceMemory base;
    u32 used;
    u32 size;
};

struct HZVKMemoryRef
{
    VkDeviceMemory base;
    u32 offset;
};

HZVKMemoryRef HZVKPushSize(HZVKMemoryArena* arena, u32 bytes)
{
    ASSERT((arena->size - arena->used) >= bytes);
    HZVKMemoryRef result = {};
    result.offset = arena->used;
    result.base = arena->base;
    arena->used += bytes;
    return result;
}

struct HZVKImage
{
    VkImage img;
    VkImageView view;
    VkExtent3D extent;
};

struct Renderer
{
    VkExtent2D screenSize;

    u32 gpuCount;
    VkPhysicalDevice* gpus;
    s32 gpuIndex;
    s32 graphicAndComputeIndex;
    VkQueue graphicAndComputeQueue;
    VkSurfaceFormatKHR surfaceFormat;
    VkDevice device;
    VkSwapchainKHR sc;
    u32 scImgCount;
    VkImage* scImgs;
    VkImageView* scImgViews;
    // VkFramebuffer* framebuffers;

    HZVKMemoryArena vRamArena;

    HZVKImage drawImg;

    VkPipeline compPipeline;
    VkDescriptorSet compDescriptor;
    VkPipelineLayout compPipelineLayout;
    VkShaderModule compShader;

    // VkRenderPass renderPass;

    VkCommandPool cmdPool;

    VkSemaphore acquireSemaphore;
    VkSemaphore submitSemaphore;
};


HZVKMemoryArena HZVKInitMemoryArena(VkDevice device, VkPhysicalDevice gpu, u32 size, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties memProp;
    vkGetPhysicalDeviceMemoryProperties(gpu, &memProp);
    u32 vRamType = INVALID_VALUE;
    for (u32 i = 0; i < memProp.memoryTypeCount; ++i)
    {
        if (OverlapFlag(memRequirements.memoryTypeBits, 1 << i) && OverlapFlag(memProp.memoryTypes[i].propertyFlags, flags))
        {
            vRamType = i;
            break;
        }
    }

    HZVKMemoryArena result;
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = vRamType;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, 0, &result.base));
    result.used = 0;
    result.size = size;
    return result;
}

char* HZVKGetDebugSeverityString(VkDebugUtilsMessageSeverityFlagBitsEXT severity)
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

const char* HZVKGetDebugTypeString(VkDebugUtilsMessageTypeFlagsEXT type)
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


VkBool32 HZVKDebugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    printf("Debug callback: %s\n", pCallbackData->pMessage);
    printf("Severity: %s\n", HZVKGetDebugSeverityString(messageSeverity));
    printf("Type: %s\n", HZVKGetDebugTypeString(messageTypes));

    printf("Objects ");
    for (u32 i = 0; i < pCallbackData->objectCount; ++i)
    {
        printf("%llx ", pCallbackData->pObjects[i].objectHandle);
    }

    printf("\n-----------\n");

    return VK_FALSE;
}

VkImageViewCreateInfo HZVKCreateImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
    // build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspectFlags;

    return info;
}

VkImageCreateInfo HZVKCreateImageInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;

    //for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
    info.samples = VK_SAMPLE_COUNT_1_BIT;

    //optimal tiling, which means the image is stored on the best gpu format
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    return info;
}

void HZVKTransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier imageBarrier = {};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.pNext = nullptr;

    // imageBarrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    // imageBarrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;

    VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.aspectMask = aspectMask;
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.image = image;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);
}

VkDescriptorSetLayoutBinding HZVKDescriptorSetLayoutBinding(u32 binding, VkDescriptorType type, VkShaderStageFlags stageFlags)
{
    VkDescriptorSetLayoutBinding result = {};

    result.binding = binding;
    result.descriptorCount = 1;
    result.descriptorType = type;
    result.stageFlags = stageFlags;

    return result;
}

void HZVKInit(Renderer* renderer, HINSTANCE hinstance, HWND hwnd, const char* name, MemoryArena* arena)
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

    debugInfo.pfnUserCallback = &HZVKDebugCallBack;
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
    renderer->graphicAndComputeIndex = -1;
    for (u32 i = 0; i < renderer->gpuCount; ++i)
    {
        VkPhysicalDevice* gpu = &renderer->gpus[i];

        u32 queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, NULL);
        VkQueueFamilyProperties* queueProperties = PUSH_ARRAY(arena, VkQueueFamilyProperties, queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, queueProperties);

        for (u32 qIndex = 0; qIndex < queueFamilyCount; ++qIndex)
        {
            if (OverlapFlag(queueProperties[qIndex].queueFlags, VK_QUEUE_GRAPHICS_BIT) && OverlapFlag(queueProperties[qIndex].queueFlags, VK_QUEUE_COMPUTE_BIT))
            {
                VkBool32 supported = false;
                VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(*gpu, qIndex, surface, &supported));
                if (supported)
                {
                    renderer->gpuIndex = i;
                    renderer->graphicAndComputeIndex = qIndex;
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
    queueInfo.queueFamilyIndex = renderer->graphicAndComputeIndex;
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

    vkGetDeviceQueue(renderer->device, renderer->graphicAndComputeIndex, 0, &renderer->graphicAndComputeQueue);

    VkSurfaceCapabilitiesKHR surfaceCap;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*gpu, surface, &surfaceCap));
    renderer->screenSize = surfaceCap.currentExtent;

    u32 surfaceFormatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(*gpu, surface, &surfaceFormatCount, NULL));
    VkSurfaceFormatKHR* surfaceFormats = PUSH_ARRAY(arena, VkSurfaceFormatKHR, surfaceFormatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(*gpu, surface, &surfaceFormatCount, surfaceFormats));
    for (u32 i = 0; i < surfaceFormatCount; ++i)
    {
        if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            renderer->surfaceFormat = surfaceFormats[i];
            // break;
        }
    }


    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = Min(surfaceCap.minImageCount + 1, surfaceCap.maxImageCount);
    swapchainInfo.imageFormat = renderer->surfaceFormat.format;
    // swapchainInfo.imageColorSpace;
    swapchainInfo.imageExtent = renderer->screenSize;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    // swapchainInfo.imageSharingMode;
    // swapchainInfo.queueFamilyIndexCount;
    // swapchainInfo.pQueueFamilyIndices;
    swapchainInfo.preTransform = surfaceCap.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // swapchainInfo.presentMode;
    // swapchainInfo.clipped;
    // swapchainInfo.oldSwapchain;
    VK_CHECK(vkCreateSwapchainKHR(renderer->device, &swapchainInfo, NULL, &renderer->sc));

    VK_CHECK(vkGetSwapchainImagesKHR(renderer->device, renderer->sc, &renderer->scImgCount, NULL));
    renderer->scImgs = PUSH_ARRAY(arena, VkImage, renderer->scImgCount);
    VK_CHECK(vkGetSwapchainImagesKHR(renderer->device, renderer->sc, &renderer->scImgCount, renderer->scImgs));

    renderer->scImgViews = PUSH_ARRAY(arena, VkImageView, renderer->scImgCount);
    for (u32 i = 0; i < renderer->scImgCount; ++i)
    {
        auto imgViewInfo = HZVKCreateImageViewInfo(renderer->surfaceFormat.format, renderer->scImgs[i], VK_IMAGE_ASPECT_COLOR_BIT);
        VK_CHECK(vkCreateImageView(renderer->device, &imgViewInfo, NULL, &renderer->scImgViews[i]));
    }

    VkImageUsageFlags drawImageUsages = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    auto drawImgInfo = HZVKCreateImageInfo(VK_FORMAT_R8G8B8A8_UNORM, drawImageUsages, VkExtent3D{ surfaceCap.currentExtent.width, surfaceCap.currentExtent.height, 1 });
    VK_CHECK(vkCreateImage(renderer->device, &drawImgInfo, NULL, &renderer->drawImg.img));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(renderer->device, renderer->drawImg.img, &memRequirements);
    renderer->vRamArena = HZVKInitMemoryArena(renderer->device, *gpu, MEGABYTES(256), memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    auto drawImgMem = HZVKPushSize(&renderer->vRamArena, drawImgInfo.extent.width * drawImgInfo.extent.height * drawImgInfo.extent.depth * 4);
    VK_CHECK(vkBindImageMemory(renderer->device, renderer->drawImg.img, drawImgMem.base, drawImgMem.offset));
    renderer->drawImg.extent = drawImgInfo.extent;

    auto drawImgViewInfo = HZVKCreateImageViewInfo(drawImgInfo.format, renderer->drawImg.img, VK_IMAGE_ASPECT_COLOR_BIT);
    VK_CHECK(vkCreateImageView(renderer->device, &drawImgViewInfo, NULL, &renderer->drawImg.view));

    VkDescriptorSetLayoutBinding compShaderBindings[] =
    {
        HZVKDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT),
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
    descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutInfo.pNext = nullptr;

    descriptorLayoutInfo.pBindings = compShaderBindings;
    descriptorLayoutInfo.bindingCount = ARRAY_COUNT(compShaderBindings);
    // info.flags = flags;

    VkDescriptorSetLayout descriptorLayout;
    VK_CHECK(vkCreateDescriptorSetLayout(renderer->device, &descriptorLayoutInfo, nullptr, &descriptorLayout));

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo descPoolInfo = {};
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descPoolInfo.maxSets = 1;
    descPoolInfo.poolSizeCount = 1;
    descPoolInfo.pPoolSizes = &poolSize;
    VkDescriptorPool descPool;
    vkCreateDescriptorPool(renderer->device, &descPoolInfo, NULL, &descPool);

    VkDescriptorSetAllocateInfo setLayoutInfo = {};
    setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setLayoutInfo.descriptorPool = descPool;
    setLayoutInfo.descriptorSetCount = 1;
    setLayoutInfo.pSetLayouts = &descriptorLayout;
    VK_CHECK(vkAllocateDescriptorSets(renderer->device, &setLayoutInfo, &renderer->compDescriptor));

    VkDescriptorImageInfo imgInfo = {};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView = renderer->drawImg.view;

    VkWriteDescriptorSet drawImageWrite = {};
    drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    drawImageWrite.pNext = nullptr;

    drawImageWrite.dstBinding = 0;
    drawImageWrite.dstSet = renderer->compDescriptor;
    drawImageWrite.descriptorCount = 1;
    drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    drawImageWrite.pImageInfo = &imgInfo;

    vkUpdateDescriptorSets(renderer->device, 1, &drawImageWrite, 0, nullptr);

    auto computeShaderCode = ReadFile("shader/compute.spv", arena);

    VkShaderModuleCreateInfo shaderModInfo;
    shaderModInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModInfo.codeSize = computeShaderCode.contentSize;
    shaderModInfo.pCode = (u32*)computeShaderCode.content;
    VK_CHECK(vkCreateShaderModule(renderer->device, &shaderModInfo, NULL, &renderer->compShader));

    VkPipelineShaderStageCreateInfo compShaderInfo = {};
    compShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    compShaderInfo.pNext;
    compShaderInfo.flags;
    compShaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    compShaderInfo.module = renderer->compShader;
    compShaderInfo.pName = "main";
    compShaderInfo.pSpecializationInfo = NULL;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // pipelineLayoutInfo.pNext;
    // pipelineLayoutInfo.flags;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;
    VK_CHECK(vkCreatePipelineLayout(renderer->device, &pipelineLayoutInfo, NULL, &renderer->compPipelineLayout));

    VkComputePipelineCreateInfo compInfo = {};

    compInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    // compInfo.pNext;
    // compInfo.flags = ;
    compInfo.stage = compShaderInfo;
    compInfo.layout = renderer->compPipelineLayout;

    VK_CHECK(vkCreateComputePipelines(renderer->device, NULL, 1, &compInfo, NULL, &renderer->compPipeline));

    // VkAttachmentDescription attachment = {};
    // // attachment.flags;
    // attachment.format = renderer->surfaceFormat.format;
    // attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // // attachment.stencilLoadOp;
    // // attachment.stencilStoreOp;
    // attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // VkAttachmentReference attachmentRef = {};
    // attachmentRef.attachment = 0;
    // attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // VkAttachmentDescription attachments[] =
    // {
    //     attachment,
    // };

    // VkSubpassDescription subpassDesc = {};
    // // subpassDesc.flags;
    // // subpassDesc.pipelineBindPoint;
    // // subpassDesc.inputAttachmentCount;
    // // subpassDesc.pInputAttachments;
    // subpassDesc.colorAttachmentCount = 1;
    // subpassDesc.pColorAttachments = &attachmentRef;
    // // subpassDesc.pResolveAttachments;
    // // subpassDesc.pDepthStencilAttachment;
    // // subpassDesc.preserveAttachmentCount;
    // // subpassDesc.pPreserveAttachments;

    // VkRenderPassCreateInfo renderPassInfo = {};
    // renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    // // renderPassInfo.flags;
    // renderPassInfo.attachmentCount = 1;
    // renderPassInfo.pAttachments = attachments;
    // renderPassInfo.subpassCount = ARRAY_COUNT(attachments);
    // renderPassInfo.pSubpasses = &subpassDesc;
    // renderPassInfo.dependencyCount;
    // renderPassInfo.pDependencies;

    // VK_CHECK(vkCreateRenderPass(renderer->device, &renderPassInfo, NULL, &renderer->renderPass));


    // renderer->framebuffers = PUSH_ARRAY(arena, VkFramebuffer, renderer->scImgCount);
    // VkFramebufferCreateInfo fbInfo = {};
    // fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    // fbInfo.flags;
    // fbInfo.renderPass = 0; //renderer->renderPass;
    // fbInfo.width = renderer->screenSize.width;
    // fbInfo.height = renderer->screenSize.height;
    // fbInfo.layers = 1;
    // fbInfo.attachmentCount = 1;

    // for (u32 i = 0; i < renderer->scImgCount; ++i)
    // {
    //     fbInfo.pAttachments = &renderer->scImgViews[i];
    //     VK_CHECK(vkCreateFramebuffer(renderer->device, &fbInfo, NULL, &renderer->framebuffers[i]));
    // }

    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = renderer->graphicAndComputeIndex;

    VK_CHECK(vkCreateCommandPool(renderer->device, &cmdPoolInfo, NULL, &renderer->cmdPool));


    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(renderer->device, &semaphoreInfo, NULL, &renderer->acquireSemaphore);
    vkCreateSemaphore(renderer->device, &semaphoreInfo, NULL, &renderer->submitSemaphore);
}

void HZVKRenderOutput(RenderGroup* renderGroup)
{
    Renderer* renderer = renderGroup->renderer;
    MemoryArena pushBuffer = renderGroup->pushBuffer;

    u32 imgIndex;
    VK_CHECK(vkAcquireNextImageKHR(renderer->device, renderer->sc, 0, renderer->acquireSemaphore, 0, &imgIndex));

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

    // VkClearValue clearValue = {};
    // clearValue.color = { 0, 1, 0, 1 };

    // VkRenderPassBeginInfo rpBeginInfo = {};
    // rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    // rpBeginInfo.renderPass = renderer->renderPass;
    // rpBeginInfo.framebuffer = renderer->framebuffers[imgIndex];
    // rpBeginInfo.renderArea.extent = renderer->screenSize;
    // rpBeginInfo.clearValueCount = 1;
    // rpBeginInfo.pClearValues = &clearValue;
    // vkCmdBeginRenderPass(cmd, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // render
    {
        HZVKTransitionImage(cmd, renderer->drawImg.img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        VkClearColorValue clearValue = { { 1.0f, 0.0f, 0.0f, 1.0f } };

        VkImageSubresourceRange clearRange = {};
        clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // clearRange.baseMipLevel;
        clearRange.levelCount = 1;
        // clearRange.baseArrayLayer;
        clearRange.layerCount = 1;
        vkCmdClearColorImage(cmd, renderer->drawImg.img, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

        //clear image
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, renderer->compPipeline);

        // bind the descriptor set containing the draw image for the compute pipeline
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, renderer->compPipelineLayout, 0, 1, &renderer->compDescriptor, 0, nullptr);

        // execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
        vkCmdDispatch(cmd, (u32)Ceil((f32)renderer->drawImg.extent.width / 16.0f), (u32)Ceil((f32)renderer->drawImg.extent.height / 16.0f), 1);

        HZVKTransitionImage(cmd, renderer->drawImg.img, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        HZVKTransitionImage(cmd, renderer->scImgs[imgIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkImageBlit blitInfo = {};

        VkImageSubresourceLayers subRes = {};
        subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subRes.mipLevel = 0;
        subRes.baseArrayLayer = 0;
        subRes.layerCount = 1;

        blitInfo.srcSubresource = blitInfo.dstSubresource = subRes;

        blitInfo.srcOffsets[1] =
        {
            (int)renderer->drawImg.extent.width,
            (int)renderer->drawImg.extent.height,
            (int)renderer->drawImg.extent.depth,
        };

        blitInfo.dstOffsets[1] =
        {
            (int)renderer->drawImg.extent.width,
            (int)renderer->drawImg.extent.height,
            (int)renderer->drawImg.extent.depth,
        };

        vkCmdBlitImage(cmd, renderer->drawImg.img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, renderer->scImgs[imgIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitInfo, VK_FILTER_NEAREST);

        HZVKTransitionImage(cmd, renderer->scImgs[imgIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    }

    // vkCmdEndRenderPass(cmd);
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
    VK_CHECK(vkQueueSubmit(renderer->graphicAndComputeQueue, 1, &submitInfo, 0));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &renderer->sc;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderer->submitSemaphore;
    presentInfo.pImageIndices = &imgIndex;

    VK_CHECK(vkQueuePresentKHR(renderer->graphicAndComputeQueue, &presentInfo));

    VK_CHECK(vkDeviceWaitIdle(renderer->device));

    vkFreeCommandBuffers(renderer->device, renderer->cmdPool, 1, &cmd);
}

#endif