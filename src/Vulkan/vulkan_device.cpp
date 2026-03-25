/*
* File: vulkan_device.cpp
* Project: opticus-rhi
* Author: Collin Longoria
* Created on: 3/24/2026
*
* Copyright (c) 2025 Collin Longoria
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/

#include "Vulkan/vulkan_device.hpp"

#include <algorithm>
#include <iostream>

#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif

#ifdef __linux__
#include <vulkan/vulkan_xcb.h>
#endif

namespace opticus {

static const char* VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* userData) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "[Vulkan Validation]" << pCallbackData->pMessage << std::endl;
    }
    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func) return func(instance, pCreateInfo, pAllocator, pMessenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* allocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func) func(instance, messenger, allocator);
}

class VulkanSwapchain : public ISwapchain {
public:
    VulkanSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t graphicsFamily, uint32_t width, uint32_t height)
        : m_device(device), m_physicalDevice(physicalDevice), m_surface(surface), m_graphicsFamily(graphicsFamily) {
        CreateSwapchain(width, height);
    }

    ~VulkanSwapchain() {
        DestroySwapchainResources();
    }

    void Resize(uint32_t width, uint32_t height) override {
        vkDeviceWaitIdle(m_device);
        DestroySwapchainResources();
        CreateSwapchain(width, height);
    }

    [[nodiscard]] Extent2D GetExtent() const override {
        return m_extent;
    }

    VkSwapchainKHR GetHandle() const { return m_swapchain; }
    VkFormat GetFormat() const { return m_format; }
    const std::vector<VkImageView>& GetImageViews() const { return m_imageViews; }
    uint32_t GetImageCount() const { return static_cast<uint32_t>(m_imageViews.size()); }

private:
    VkDevice m_device{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    uint32_t m_graphicsFamily{0};

    VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
    VkFormat m_format{VK_FORMAT_B8G8R8A8_SRGB};
    Extent2D m_extent{0, 0};
    std::vector<VkImage> m_images{};
    std::vector<VkImageView> m_imageViews{};

    void CreateSwapchain(uint32_t width, uint32_t height) {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data());

        // Pick surface format
        VkSurfaceFormatKHR chosenFormat = formats[0];
        for (const auto& f : formats) {
            if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                chosenFormat = f;
                break;
            }
        }
        m_format = chosenFormat.format;

        // Pick present mode
        VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& mode : presentModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                chosenPresentMode = mode;
                break;
            }
        }
        // Pick extent
        VkExtent2D extent;
        if (capabilities.currentExtent.width != UINT32_MAX) {
            extent = capabilities.currentExtent;
        } else {
            extent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }
        m_extent = {extent.width, extent.height};

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = chosenFormat.format;
        createInfo.imageColorSpace = chosenFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = chosenPresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain");
        }

        // Get images
        uint32_t swapImageCount = 0;
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapImageCount, nullptr);
        m_images.resize(swapImageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapImageCount, m_images.data());

        // Create image views
        m_imageViews.resize(m_images.size());
        for (size_t i = 0; i < m_images.size(); i++) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_format;
            viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create swapchain image view");
            }
        }
    }

    void DestroySwapchainResources() {
        for (auto view : m_imageViews) {
            vkDestroyImageView(m_device, view, nullptr);
        }
        m_imageViews.clear();
        m_images.clear();
        if (m_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
    }
};

class VulkanCommandList : public ICommandList {
public:
    VulkanCommandList(VkDevice device, uint32_t queueFamily)
        : m_device(device) {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamily;

        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool");
        }

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffer");
        }
    }

    ~VulkanCommandList() override {
        if (m_commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        }
    }

    void Begin() override {
        vkResetCommandBuffer(m_commandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin command buffer");
        }
    }

    void End() override {
        if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to end command buffer");
        }
    }
    void BeginRendering(const Extent2D& renderArea, const Color& clearColor) override {
        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = m_currentImageView;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = {{clearColor.r, clearColor.g, clearColor.b, clearColor.a}};

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = {{0, 0}, {renderArea.width, renderArea.height}};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void EndRendering() override {
        vkCmdEndRendering(m_commandBuffer);
    }

    VkCommandBuffer GetHandle() const { return m_commandBuffer; }

    void SetTargetImageView(VkImageView view) { m_currentImageView = view; }

    void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess, VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess) {
        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.srcStageMask = srcStage;
        barrier.srcAccessMask = srcAccess;
        barrier.dstStageMask = dstStage;
        barrier.dstAccessMask = dstAccess;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkDependencyInfo depInfo{};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(m_commandBuffer, &depInfo);
    }


private:
    VkDevice m_device{VK_NULL_HANDLE};
    VkCommandPool m_commandPool{VK_NULL_HANDLE};
    VkCommandBuffer m_commandBuffer{VK_NULL_HANDLE};
    VkImageView m_currentImageView{VK_NULL_HANDLE};
};

bool VulkanDevice::Initialize(const DeviceInitDescriptor &desc) {
    try {
        CreateInstance(desc.enableValidationLayers);
        if (desc.enableValidationLayers) {
            SetupDebugMessenger();
        }
        CreateSurface(desc.nativeWindowHandle);
        SelectPhysicalDevice(desc.preference);
        CreateLogicalDevice();
        CreateSyncObjects();

        m_swapchain = std::make_unique<VulkanSwapchain>(
            m_logicalDevice, m_physicalDevice, m_surface,
            m_graphicsQueueFamily, desc.frameBufferWidth, desc.frameBufferHeight);

        m_commandList = std::make_unique<VulkanCommandList>(
            m_logicalDevice, m_graphicsQueueFamily);
    } catch (const std::exception& e) {
        std::cerr << "[VulkanDevice] Initialization failed: " << e.what() << std::endl;
        Shutdown();
        return false;
    }
    return true;
}

void VulkanDevice::Shutdown() {
    if (m_logicalDevice != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_logicalDevice);
    }

    m_commandList.reset();
    m_swapchain.reset();

    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyFence(m_logicalDevice, m_inFlightFence, nullptr);
        m_inFlightFence = VK_NULL_HANDLE;
    }
    if (m_renderFinishedSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphore, nullptr);
        m_renderFinishedSemaphore = VK_NULL_HANDLE;
    }
    if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphore, nullptr);
        m_imageAvailableSemaphore = VK_NULL_HANDLE;
    }
    if (m_logicalDevice != VK_NULL_HANDLE) {
        vkDestroyDevice(m_logicalDevice, nullptr);
        m_logicalDevice = VK_NULL_HANDLE;
    }
    if (m_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    if (m_debugMessenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        m_debugMessenger = VK_NULL_HANDLE;
    }
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

std::vector<AdapterInfo> VulkanDevice::EnumerateAdapters() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(m_instance, &count, devices.data());

    std::vector<AdapterInfo> adapters;
    adapters.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(devices[i], &memProps);

        size_t vram = 0;
        for (uint32_t j = 0; j < memProps.memoryHeapCount; j++) {
            if (memProps.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                vram = memProps.memoryHeaps[j].size;
                break;
            }
        }

        AdapterInfo info{};
        info.id = i;
        info.name = props.deviceName;
        info.vramBytes = vram;
        info.isDiscrete = (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        adapters.push_back(info);
    }

    return adapters;
}

ISwapchain *VulkanDevice::CreateSwapchain() {
    return m_swapchain.get();
}

void VulkanDevice::BeginFrame() {
    vkWaitForFences(m_logicalDevice, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_logicalDevice, 1, &m_inFlightFence);

    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(m_logicalDevice, m_swapchain->GetHandle(), UINT64_MAX,
                          m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    m_commandList->SetTargetImageView(m_swapchain->GetImageViews()[imageIndex]);
    m_commandList->Begin();

    // Get the swapchain images to transition layout
    uint32_t imgCount = 0;
    vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain->GetHandle(), &imgCount, nullptr);
    std::vector<VkImage> images(imgCount);
    vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain->GetHandle(), &imgCount, images.data());

    m_commandList->TransitionImageLayout(
        images[imageIndex],
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    m_currentImageIndex = imageIndex;
}


}
