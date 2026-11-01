/*******************************************************************************
 * File: property.hpp
 * Project: tefkernel_cpp_wrapper
 * Created: 2026/6/28
 * Author: eternalfuture-e38299
 * Github: https://github.com/eternalfuture-e38299
 *
 * MIT License
 *
 * Copyright (c) 2026 eternalfuture-e38299
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/

#pragma once

#include "tefkernel/patchlib/property.h"
#include "type.hpp"
#include "method.hpp"
#include <string>

namespace TEFKernel::PatchLib {
    /**
     * @brief 属性包装类
     */
    class Property {
    public:
        Property() : handle_(nullptr), ownsHandle_(false) {
        }

        explicit Property(patch_handle_t handle, const bool takeOwnership = false)
            : handle_(handle), ownsHandle_(takeOwnership) {
        }

        ~Property();

        // 拷贝构造
        Property(const Property &other);

        // 拷贝赋值
        Property &operator=(const Property &other);

        // 移动构造
        Property(Property &&other) noexcept;

        // 移动赋值
        Property &operator=(Property &&other) noexcept;

        [[nodiscard]] patch_handle_t GetHandle() const { return handle_; }

        [[nodiscard]] bool IsValid() const;

        void Reset(patch_handle_t newHandle = nullptr, bool takeOwnership = false);

        patch_handle_t Release();

        // ==================== 基本信息 ====================

        [[nodiscard]] std::string GetName() const;

        // ==================== 方法获取 ====================

        [[nodiscard]] Method GetGetMethod() const;

        [[nodiscard]] Method GetSetMethod() const;

        // ==================== 运算符 ====================

        explicit operator bool() const;

        bool operator==(const Property &other) const;

        bool operator!=(const Property &other) const;

    private:
        patch_handle_t handle_;
        bool ownsHandle_;
    };
}