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

#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif

#ifdef __linux__
#include <vulkan/vulkan_xcb.h>
#endif

namespace opticus {

}