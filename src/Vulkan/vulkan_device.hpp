/*
* File: VulkanDevice.hpp
* Project: opticus-rhi
* Author: Collin Longoria
* Created on: 3/24/2026
*
* Copyright (c) 2025 Collin Longoria
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/

#ifndef OPTICUS_VULKANDEVICE_HPP
#define OPTICUS_VULKANDEVICE_HPP
#include "Opticus/render_device.hpp"
#include <vulkan/vulkan.h>

namespace opticus {

class VulkanSwapchain;
class VulkanCommandList;

class VulkanDevice : public IRenderDevice {
public:
    bool Initialize(const DeviceInitDescriptor &desc) override;
    void Shutdown() override;

    std::vector<AdapterInfo> EnumerateAdapters() override;

    ISwapchain* CreateSwapchain() override;

    void BeginFrame() override;
    ICommandList *GetGraphicsCommandList() override;
    void SubmitAndPresent() override;

    void WaitIdle() override;

private:
    // Core handles
    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_logicalDevice{VK_NULL_HANDLE};

    // Queues
    VkQueue m_graphicsQueue{VK_NULL_HANDLE};
    uint32_t m_graphicsQueueFamily{0};

    // Internal impls
    std::unique_ptr<VulkanSwapchain> m_swapchain;
    std::unique_ptr<VulkanCommandList> m_commandList;

    // Sync Objects
    VkSemaphore m_imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore m_renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence m_inFlightFence{VK_NULL_HANDLE};

    uint32_t m_currentImageIndex{0};

    // Helpers
    void CreateInstance(bool enableValidation);
    void SetupDebugMessenger();
    void CreateSurface(void* nativeWindowHandle);
    void SelectPhysicalDevice(DevicePreference pref);
    void CreateLogicalDevice();
    void CreateSyncObjects();
};

}

#endif //OPTICUS_VULKANDEVICE_HPP
