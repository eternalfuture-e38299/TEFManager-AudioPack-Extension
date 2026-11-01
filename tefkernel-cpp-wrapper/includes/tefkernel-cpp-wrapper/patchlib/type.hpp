/*******************************************************************************
 * File: type.hpp
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

#include "tefkernel/patchlib/type.h"
#include "../tefstd/vector.hpp"
#include <string>
#include <memory>

namespace TEFKernel::PatchLib {
    /**
     * @brief 类型包装类
     *
     * 提供对C API中类型操作的C++封装
     */
    class Type {
    public:
        /**
         * @brief 默认构造函数
         */
        Type() : handle_(nullptr), ownsHandle_(false) {
        }

        /**
         * @brief 从已有句柄构造
         * @param handle 类型句柄
         * @param takeOwnership 是否取得所有权
         */
        explicit Type(patch_handle_t handle, const bool takeOwnership = false)
            : handle_(handle), ownsHandle_(takeOwnership) {
        }

        /**
         * @brief 根据命名空间和名称获取类型
         * @param ns 命名空间
         * @param name 类型名称
         */
        Type(const std::string &ns, const std::string &name);

        /**
         * @brief 析构函数
         */
        ~Type();

        void ManualDestroy();

        // 拷贝构造
        Type(const Type &other);

        // 拷贝赋值
        Type &operator=(const Type &other);

        // 移动构造
        Type(Type &&other) noexcept;

        // 移动赋值
        Type &operator=(Type &&other) noexcept;

        /**
         * @brief 获取原始句柄
         */
        [[nodiscard]] patch_handle_t GetHandle() const { return handle_; }

        /**
         * @brief 检查是否有效
         */
        [[nodiscard]] bool IsValid() const;

        /**
         * @brief 重置句柄
         */
        void Reset(patch_handle_t newHandle = nullptr, bool takeOwnership = false);

        /**
         * @brief 释放所有权（不释放资源）
         */
        patch_handle_t Release();

        /**
         * @brief 获取MonoType
         */
        [[nodiscard]] patch_handle_t GetMonoType() const;
        // ==================== 类型信息查询 ====================

        /**
         * @brief 获取类型名称
         */
        [[nodiscard]] std::string GetName() const;

        /**
         * @brief 获取命名空间
         */
        [[nodiscard]] std::string GetNamespace() const;

        /**
         * @brief 获取完整名称
         */
        [[nodiscard]] std::string GetFullName() const;

        /**
         * @brief 获取父类型
         */
        [[nodiscard]] Type GetParent() const;

        // ==================== 静态工厂方法 ====================
        /**
         * @brief 获取基础类型
         */
        static Type GetBasicType(patch_type_t basicType);

        /**
         * @brief 创建类型的新实例
         */
        [[nodiscard]] patch_handle_t NewInstance() const;

        /**
         * @brief 实例化泛型类型
         */
        [[nodiscard]] Type MakeGenericType(const Std::Vector<patch_handle_t> &typeArgs) const;

        // ==================== 成员获取 ====================

        /**
         * @brief 获取内嵌类型
         */
        [[nodiscard]] Type GetInnerType(const std::string &name) const;

        /**
         * @brief 获取字段
         */
        [[nodiscard]] class Field GetField(const std::string &name) const;

        /**
         * @brief 获取属性
         */
        [[nodiscard]] class Property GetProperty(const std::string &name) const;

        /**
         * @brief 获取方法
         */
        [[nodiscard]] class Method GetMethod(const std::string &name) const;

        /**
         * @brief 根据参数数量获取方法
         */
        [[nodiscard]] Method GetMethod(const std::string &name, int argsCount) const;

        /**
         * @brief 根据参数名称获取方法
         */
        [[nodiscard]] Method GetMethod(
            const std::string &name,
            const Std::Vector<std::string> &argsNames
        ) const;

        /**
         * @brief 根据参数类型获取方法
         */
        [[nodiscard]] Method GetMethod(
            const std::string &name,
            const Std::Vector<Type> &argsTypes
        ) const;

        /**
         * @brief 根据签名获取方法
         */
        [[nodiscard]] Method GetMethod(
            const std::string &name,
            const Std::Vector<Type> &argsTypes,
            const Std::Vector<std::string> &argsNames
        ) const;

        // ==================== 批量获取 ====================

        /**
         * @brief 获取所有嵌套类型
         */
        [[nodiscard]] Std::Vector<Type> GetInnerTypes(bool includingParent = false) const;

        /**
         * @brief 获取所有方法
         */
        [[nodiscard]] Std::Vector<Method> GetMethods(bool includingParent = false) const;

        /**
         * @brief 获取所有字段
         */
        [[nodiscard]] Std::Vector<Field> GetFields(bool includingParent = false) const;

        /**
         * @brief 获取所有属性
         */
        [[nodiscard]] Std::Vector<Property> GetProperties(bool includingParent = false) const;

        // ==================== 运算符 ====================

        bool operator==(const Type &other) const;

        bool operator!=(const Type &other) const;

        explicit operator bool() const;

        bool operator==(patch_handle_t other) const;

        bool operator!=(patch_handle_t other) const;

    private:
        patch_handle_t handle_;
        bool ownsHandle_;
    };
}