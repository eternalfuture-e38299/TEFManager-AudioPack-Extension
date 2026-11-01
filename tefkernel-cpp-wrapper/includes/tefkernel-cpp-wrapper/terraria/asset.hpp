/*******************************************************************************
 * File: asset
 * Project: tefkernel_cpp_wrapper
 * Created: 2026/10/29
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

#include "tefkernel/terraria/asset.h"
#include "../patchlib/type.hpp"

namespace TEFKernel::Terraria {

    /**
     * @brief 资源（Asset`1）包装类
     *
     * 对应 Terraria 的 `Asset<T>` 泛型类。一个 Asset 实例由
     * (type, value) 两个句柄构成，其中 value 通常是一个 patch_handle_t。
     *
     * @note 该类不拥有底层句柄的所有权（Asset 由 IL2CPP 运行时管理），
     *       仅作为轻量包装，不要在其析构函数中 free。
     */
    class Asset {
    public:
        /**
         * @brief 默认构造（无效资源）
         */
        Asset() : Handle_(nullptr) {
        }

        /**
         * @brief 从已有句柄构造
         * @param handle 资源句柄
         */
        explicit Asset(patch_handle_t handle) : Handle_(handle) {
        }

        /**
         * @brief 创建资源
         * @param type 资源类型句柄
         * @param value 资源值句柄
         * @throws std::runtime_error 创建失败时抛出
         */
        Asset(patch_handle_t type, patch_handle_t value);

        // 拷贝/移动（句柄由运行时管理，不涉及所有权转移，全部为浅拷贝语义）

        Asset(const Asset &other);
        Asset &operator=(const Asset &other);
        Asset(Asset &&other) noexcept;
        Asset &operator=(Asset &&other) noexcept;

        ~Asset();

        // ==================== 句柄访问 ====================

        /**
         * @brief 获取原始资源句柄
         */
        [[nodiscard]] patch_handle_t GetHandle() const { return Handle_; }

        /**
         * @brief 隐式转换为原始句柄（方便传给其它 C API）
         */
        explicit operator patch_handle_t() const { return Handle_; }

        /**
         * @brief 检查句柄是否有效（非空）
         */
        [[nodiscard]] bool IsValid() const { return Handle_ != nullptr; }

        explicit operator bool() const { return IsValid(); }

        /**
         * @brief 重置句柄
         */
        void Reset(patch_handle_t newHandle = nullptr);

        /**
         * @brief 释放所有权（返回句柄并置空自身）
         */
        patch_handle_t Release();

        // ==================== 资源操作 ====================

        /**
         * @brief 获取资源值句柄
         * @return 资源值句柄；失败返回 nullptr
         */
        [[nodiscard]] patch_handle_t GetValue() const;
        /**
         * @brief 获取资源值句柄（同 GetValue，语义化命名）
         */
        [[nodiscard]] patch_handle_t Value() const;

        // ==================== 泛型类获取 ====================

        /**
         * @brief 获取指定元素类型的 `Asset<T>` 泛型类句柄
         * @param elementType 元素类型句柄（如 Texture2D、LocalizedText 等）
         * @return 泛型 `Asset<elementType>` 的类句柄；失败返回 nullptr
         */
        static PatchLib::Type GetGenericClass(patch_handle_t elementType);

        // ==================== 运算符 ====================

        bool operator==(const Asset &other) const;

        bool operator!=(const Asset &other) const;

    private:
        patch_handle_t Handle_;
    };
}