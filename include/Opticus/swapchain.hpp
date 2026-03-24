/*
* File: swapchain.hpp
* Project: Opticus
* Author: Collin Longoria
* Created on: 3/24/2026
*
* Copyright (c) 2025 Collin Longoria
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/

#ifndef OPTICUS_SWAPCHAIN_HPP
#define OPTICUS_SWAPCHAIN_HPP
#include "core.hpp"

namespace opticus {

class ISwapchain {
public:
    virtual ~ISwapchain() = default;

    virtual void Resize(uint32_t width, uint32_t height) = 0;
    [[nodiscard]] virtual Extent2D GetExtent() const = 0;
};

}

#endif //OPTICUS_SWAPCHAIN_HPP