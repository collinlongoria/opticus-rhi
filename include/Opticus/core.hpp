/*
* File: Core.hpp
* Project: opticus-rhi
* Author: Collin Longoria
* Created on: 3/24/2026
*
* Copyright (c) 2025 Collin Longoria
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/

#ifndef OPTICUS_CORE_HPP
#define OPTICUS_CORE_HPP
#include <cstdint>
#include <functional>
#include <string>

namespace opticus {

enum class GraphicsAPI {
    Vulkan,
    DirectX12,
};

enum class DevicePreference {
    HighPerformance, // Discrete GPU
    LowPower         // Integrated GPU
};

struct Extent2D {
    uint32_t width;
    uint32_t height;
};

struct Color {
    float r, g, b, a;
};

// GPU Information
struct AdapterInfo {
    uint32_t id;
    std::string name;
    size_t vramBytes;
    bool isDiscrete;
};

// Callback that takes an Instance and returns a Surface
using SurfaceCreateCallback = std::function<void*(void* instance)>;

struct DeviceInitDescriptor {
    GraphicsAPI api;
    DevicePreference preference;
    bool enableValidationLayers;
    SurfaceCreateCallback surfaceCreateCallback;
    uint32_t frameBufferWidth;
    uint32_t frameBufferHeight;
};

}

#endif //OPTICUS_CORE_HPP