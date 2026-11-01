/*******************************************************************************
 * File: method.hpp
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

#include "tefkernel/patchlib/method.h"
#include "type.hpp"
#include "../tefstd/vector.hpp"
#include <string>
#include <memory>

namespace TEFKernel::PatchLib {
    /**
     * @brief 方法签名信息包装类
     */
    class MethodSignature {
    public:
        MethodSignature();

        explicit MethodSignature(const patch_method_signature_t &sig) : signature_(sig) {
        }

        ~MethodSignature();

        // 移动构造
        MethodSignature(MethodSignature &&other) noexcept;

        // 移动赋值
        MethodSignature &operator=(MethodSignature &&other) noexcept;

        // 拷贝被禁用
        MethodSignature(const MethodSignature &) = delete;

        MethodSignature &operator=(const MethodSignature &) = delete;

        [[nodiscard]] patch_handle_t getMethod() const { return signature_.method; }
        [[nodiscard]] bool isInstance() const { return signature_.is_instance; }
        [[nodiscard]] patch_type_t getReturnType() const { return signature_.return_type; }
        [[nodiscard]] int getToken() const { return signature_.token; }

        /**
         * @brief 获取参数类型列表（转换为C++ Vector）
         */
        [[nodiscard]] Std::Vector<patch_type_t> GetArgTypes() const;

        /**
         * @brief 获取参数名称列表
         */
        [[nodiscard]] Std::Vector<std::string> GetArgNames() const;

        [[nodiscard]] const patch_method_signature_t *GetRaw() const { return &signature_; }

    private:
        patch_method_signature_t signature_;
    };

    /**
     * @brief Hook ID包装类
     */
    class HookId {
    public:
        HookId() : id_(PATCH_HOOK_INVALID_ID) {
        }

        explicit HookId(const patch_hook_id_t id) : id_(id) {
        }

        [[nodiscard]] patch_hook_id_t Get() const { return id_; }
        [[nodiscard]] bool IsValid() const { return id_ != PATCH_HOOK_INVALID_ID; }

        void Reset() { id_ = PATCH_HOOK_INVALID_ID; }

        bool operator==(const HookId &other) const { return id_ == other.id_; }
        bool operator!=(const HookId &other) const { return id_ != other.id_; }
        explicit operator bool() const { return IsValid(); }

    private:
        patch_hook_id_t id_;
    };

    /**
     * @brief Hook回调函数包装类
     */
    class HookCallbacks {
    public:
        using PrefixCallback = prefix_callback_t;
        using PostfixCallback = postfix_callback_t;

        HookCallbacks() = default;

        HookCallbacks(const PrefixCallback prefix, const PostfixCallback postfix)
            : prefix_(prefix), postfix_(postfix) {
        }

        void SetPrefix(const PrefixCallback callback) { prefix_ = callback; }
        void SetPostfix(const PostfixCallback callback) { postfix_ = callback; }

        prefix_callback_t getPrefix() { return prefix_; }
        postfix_callback_t getPostfix() { return postfix_; }
        [[nodiscard]] const PrefixCallback &getPrefix() const { return prefix_; }
        [[nodiscard]] const PostfixCallback &getPostfix() const { return postfix_; }

    private:
        PrefixCallback prefix_;
        PostfixCallback postfix_;
    };

    /**
     * @brief 方法包装类
     */
    class Method {
    public:
        Method() : handle_(nullptr), ownsHandle_(false) {
        }

        explicit Method(patch_handle_t handle, const bool takeOwnership = false)
            : handle_(handle), ownsHandle_(takeOwnership) {
        }

        ~Method();

        // 拷贝构造
        Method(const Method &other);

        // 拷贝赋值
        Method &operator=(const Method &other);

        // 移动构造
        Method(Method &&other) noexcept;

        // 移动赋值
        Method &operator=(Method &&other) noexcept;

        [[nodiscard]] patch_handle_t GetHandle() const { return handle_; }

        [[nodiscard]] bool IsValid() const;

        void reset(patch_handle_t newHandle = nullptr, bool takeOwnership = false);

        patch_handle_t Release();

        // ==================== 基本信息 ====================

        [[nodiscard]] std::string GetName() const;

        [[nodiscard]] int GetParamCount() const;

        [[nodiscard]] bool IsInstance() const;

        [[nodiscard]] bool IsStatic() const;

        [[nodiscard]] int GetToken() const;

        [[nodiscard]] patch_type_t GetReturnType() const;

        // ==================== 泛型 ====================

        [[nodiscard]] Method MakeGenericInstance(const Std::Vector<patch_handle_t> &templateTypes) const;

        // ==================== 调用 ====================

#if __ANDROID__
        [[nodiscard]] void *GetPointer() const;
#endif

        /**
         * @brief 调用方法（模板版本）
         */
        template<typename ReturnType, typename... Args>
        // ReSharper disable once CppNotAllPathsReturnValue
        ReturnType Invoke(patch_handle_t instance, Args... args) const {
#if !defined(__ANDROID__)
            void *argArray[] = {&args...};
            ReturnType result;
            patchlib_method_invoke_args(handle_, instance, &result, argArray);

            if constexpr (!std::is_same_v<ReturnType, void>) return result;
#else
            if (IsInstance())
                return static_cast<ReturnType(*)(void *, Args..., void *)>(GetPointer())(instance, args..., handle_);

            return static_cast<ReturnType(*)(Args..., void *)>(GetPointer())(args..., handle_);
#endif
        }

        /**
         * @brief 调用方法（原始指针版本）
         */
        bool InvokeArgs(patch_handle_t instance, void *returnValue, void **args) const;

        /**
         * @brief 调用构造函数并创建新实例
         * @param args 构造函数的参数
         * @return 新创建的实例句柄，失败返回PATCH_NULL
         */
        template<typename... Args>
        patch_handle_t InvokeConstructor(Args... args) const {
            void* argArray[] = {&args...};
            patch_handle_t instance = nullptr;

            if (patchlib_constructor_invoke(handle_, &instance, argArray)) {
                return instance;
            }
            return nullptr;
        }

        /**
         * @brief 调用构造函数（原始指针版本）
         * @param return_instance 输出新实例句柄
         * @param args 参数指针数组
         * @return 是否成功
         */
        bool InvokeConstructorRaw(patch_handle_t* return_instance, void** args) const;

        // ==================== 签名 ====================

        [[nodiscard]] std::unique_ptr<MethodSignature> GetSignature() const;

        // ==================== Hook ====================

        /**
         * @brief 安装Prefix/Postfix Hook
         */
        [[nodiscard]] HookId InstallPrePostHook(
            HookCallbacks::PrefixCallback prefix,
            HookCallbacks::PostfixCallback postfix
        ) const;

        /**
         * @brief 卸载Hook
         */
        static bool UninstallHook(const HookId &hookId);

        // ==================== 运算符 ====================

        explicit operator bool() const;

        bool operator==(const Method &other) const;

        bool operator!=(const Method &other) const;

    private:
        patch_handle_t handle_;
        bool ownsHandle_;
    };
}