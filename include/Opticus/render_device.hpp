/*
* File: IRenderDevice.hpp
* Project: opticus-rhi
* Author: Collin Longoria
* Created on: 3/24/2026
*
* Copyright (c) 2025 Collin Longoria
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/

#ifndef OPTICUS_RENDERDEVICE_HPP
#define OPTICUS_RENDERDEVICE_HPP
#include <memory>
#include <vector>

#include "command_list.hpp"
#include "core.hpp"
#include "swapchain.hpp"

namespace opticus {

class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    virtual bool Initialize(const DeviceInitDescriptor& desc) = 0;
    virtual void Shutdown() = 0;

    virtual std::vector<AdapterInfo> EnumerateAdapters() = 0;

    virtual ISwapchain* CreateSwapchain() = 0;

    virtual void BeginFrame() = 0;
    virtual ICommandList* GetGraphicsCommandList() = 0;
    virtual void SubmitAndPresent() = 0;

    virtual void WaitIdle() = 0;
};

std::unique_ptr<IRenderDevice> CreateRenderDevice(GraphicsAPI api);

}

#endif //OPTICUS_RENDERDEVICE_HPP