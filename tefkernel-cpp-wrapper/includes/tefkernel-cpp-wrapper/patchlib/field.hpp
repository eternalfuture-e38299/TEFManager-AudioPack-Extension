/*******************************************************************************
 * File: field.hpp
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

#include "tefkernel/patchlib/field.h"
#include "type.hpp"
#include <string>

namespace TEFKernel::PatchLib {
    /**
     * @brief 字段包装类
     */
    class Field {
    public:
        Field() : handle_(nullptr), ownsHandle_(false) {
        }

        explicit Field(patch_handle_t handle, const bool takeOwnership = false)
            : handle_(handle), ownsHandle_(takeOwnership) {
        }

        ~Field();

        // 拷贝构造
        Field(const Field &other);

        // 拷贝赋值
        Field &operator=(const Field &other);

        // 移动构造
        Field(Field &&other) noexcept
            : handle_(other.handle_), ownsHandle_(other.ownsHandle_) {
            other.handle_ = nullptr;
            other.ownsHandle_ = false;
        }

        // 移动赋值
        Field &operator=(Field &&other) noexcept;

        [[nodiscard]] patch_handle_t GetHandle() const { return handle_; }

        [[nodiscard]] bool IsValid() const;

        void Reset(patch_handle_t newHandle = nullptr, bool takeOwnership = false);

        patch_handle_t Release();

        // ==================== 基本信息 ====================

        [[nodiscard]] std::string GetName() const;

        [[nodiscard]] bool IsStatic() const;

        [[nodiscard]] bool IsInstance() const;

        [[nodiscard]] bool IsConst() const;

        [[nodiscard]] bool IsThreadStatic() const;

        // ==================== 值操作 ====================

        /**
         * @brief 获取字段值（模板版本）
         */
        template<typename T>
        T GetValue(patch_handle_t instance = nullptr) const {
#if !defined(__ANDROID__)
            T value;
            patchlib_field_get_value(handle_, instance, &value);
            return value;
#else
            return *static_cast<T*>(GetPointer(instance));
#endif
        }

        /**
         * @brief 获取字段值（void*版本）
         */
        void GetValue(patch_handle_t instance, void *value) const;

        /**
         * @brief 设置字段值（模板版本）
         */
        template<typename T>
        void SetValue(patch_handle_t instance, const T &value) const {
#if !defined(__ANDROID__)
            patchlib_field_set_value(handle_, instance, const_cast<void *>(static_cast<const void *>(&value)));
#else
            *static_cast<T*>(GetPointer(instance)) = value;
#endif
        }

        /**
         * @brief 设置字段值（void*版本）
         */
        void SetValue(patch_handle_t instance, void *value) const;

        // ==================== 类型信息 ====================

        [[nodiscard]] size_t GetSize() const;

        [[nodiscard]] patch_type_t GetType() const;

#if __ANDROID__
        [[nodiscard]] void *GetPointer(void *instance) const;
#endif

        // ==================== 运算符 ====================

        explicit operator bool() const;

        bool operator==(const Field &other) const;

        bool operator!=(const Field &other) const;

    private:
        patch_handle_t handle_;
        bool ownsHandle_;
    };
}