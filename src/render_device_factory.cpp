/*
* File: RenderDeviceFactory.cpp
* Project: opticus-rhi
* Author: Collin Longoria
* Created on: 3/24/2026
*
* Copyright (c) 2025 Collin Longoria
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/

#include <memory>

#include "Opticus/render_device.hpp"
#include "Vulkan/vulkan_device.hpp"


namespace opticus {

std::unique_ptr<IRenderDevice> CreateRenderDevice(GraphicsAPI api) {
    switch (api) {
        case GraphicsAPI::Vulkan:
            return std::make_unique<VulkanDevice>();
        case GraphicsAPI::DirectX12:
            return nullptr;
        default:
            return nullptr;
    }
}

}
