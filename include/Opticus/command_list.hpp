/*
* File: command_list.hpp
* Project: Opticus
* Author: ${AUTHOR}
* Created on: 3/24/2026
*
* Copyright (c) 2025 Collin Longoria
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/

#ifndef OPTICUS_COMMAND_LIST_HPP
#define OPTICUS_COMMAND_LIST_HPP
#include "Opticus/core.hpp"

namespace opticus {

class ICommandList {
public:
    virtual ~ICommandList() = default;

    virtual void Begin() = 0;
    virtual void End() = 0;

    virtual void BeginRendering(const Extent2D& renderArea, const Color& clearColor) = 0;
    virtual void EndRendering() = 0;
};

}
#endif //OPTICUS_COMMAND_LIST_HPP