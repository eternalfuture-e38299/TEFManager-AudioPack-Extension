/*******************************************************************************
 * File: method
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

#include "tefkernel-cpp-wrapper/patchlib/method.hpp"

TEFKernel::PatchLib::MethodSignature::MethodSignature()  : signature_{} {
    signature_.method = nullptr;
    signature_.is_instance = false;
    signature_.return_type = PATCH_VOID;
    signature_.arg_types = {};
    signature_.arg_names = {};
    signature_.token = 0;
}

TEFKernel::PatchLib::MethodSignature::~MethodSignature()  {
    patchlib_method_signature_free(&signature_);
}

TEFKernel::PatchLib::MethodSignature::MethodSignature(MethodSignature &&other) noexcept
            : signature_(other.signature_) {
    std::memset(&other.signature_, 0, sizeof(other.signature_));
}

TEFKernel::PatchLib::MethodSignature & TEFKernel::PatchLib::MethodSignature::operator
=(MethodSignature &&other) noexcept {
    if (this != &other) {
        patchlib_method_signature_free(&signature_);
        signature_ = other.signature_;
        std::memset(&other.signature_, 0, sizeof(other.signature_));
    }
    return *this;
}

TEFKernel::Std::Vector<patch_type_t> TEFKernel::PatchLib::MethodSignature::GetArgTypes() const {
    Std::Vector<patch_type_t> result;
    const auto *types = static_cast<const patch_type_t *>(signature_.arg_types.data);
    for (size_t i = 0; i < tefstd_vector_size(&signature_.arg_types); ++i) {
        result.push_back(types[i]);
    }
    return result;
}

TEFKernel::Std::Vector<std::string> TEFKernel::PatchLib::MethodSignature::GetArgNames() const {
    Std::Vector<std::string> result;
    const auto *names = static_cast<const char * const*>(signature_.arg_names.data);
    for (size_t i = 0; i < tefstd_vector_size(&signature_.arg_names); ++i) {
        if (names[i]) {
            result.push_back(std::string(names[i]));
        }
    }
    return result;
}

TEFKernel::PatchLib::Method::~Method() {
    if (ownsHandle_ && handle_ != nullptr) {
#if !defined(__ANDROID__)
        patchlib_free(handle_);
#endif
    }
}

TEFKernel::PatchLib::Method::Method(const Method &other)  : handle_(other.handle_), ownsHandle_(false) {
    if (other.ownsHandle_ && other.handle_ != nullptr) {
#if !defined(__ANDROID__)
        handle_ = patchlib_handle_copy(other.handle_);
        ownsHandle_ = true;
#endif
    }
}

TEFKernel::PatchLib::Method & TEFKernel::PatchLib::Method::operator=(const Method &other) {
    if (this != &other) {
        reset();
        handle_ = other.handle_;
        ownsHandle_ = false;
        if (other.ownsHandle_ && other.handle_ != nullptr) {
#if !defined(__ANDROID__)
            handle_ = patchlib_handle_copy(other.handle_);
            ownsHandle_ = true;
#endif
        }
    }
    return *this;
}

TEFKernel::PatchLib::Method::Method(Method &&other) noexcept
            : handle_(other.handle_), ownsHandle_(other.ownsHandle_) {
    other.handle_ = nullptr;
    other.ownsHandle_ = false;
}

TEFKernel::PatchLib::Method & TEFKernel::PatchLib::Method::operator=(Method &&other) noexcept {
    if (this != &other) {
        reset();
        handle_ = other.handle_;
        ownsHandle_ = other.ownsHandle_;
        other.handle_ = nullptr;
        other.ownsHandle_ = false;
    }
    return *this;
}

bool TEFKernel::PatchLib::Method::IsValid() const {
    return patchlib_is_valid(handle_);
}

void TEFKernel::PatchLib::Method::reset(patch_handle_t newHandle, const bool takeOwnership)  {
    if (ownsHandle_ && handle_ != nullptr) {
#if !defined(__ANDROID__)
        patchlib_free(handle_);
#endif
    }
    handle_ = newHandle;
    ownsHandle_ = takeOwnership;
}

patch_handle_t TEFKernel::PatchLib::Method::Release() {
    patch_handle_t h = handle_;
    handle_ = nullptr;
    ownsHandle_ = false;
    return h;
}

std::string TEFKernel::PatchLib::Method::GetName() const {
    const char *name = patchlib_method_get_name(handle_);
    return name ? std::string(name) : std::string();
}

int TEFKernel::PatchLib::Method::GetParamCount() const {
    return patchlib_method_get_param_count(handle_);
}

bool TEFKernel::PatchLib::Method::IsInstance() const {
    return patchlib_method_is_instance(handle_);
}

bool TEFKernel::PatchLib::Method::IsStatic() const {
    return patchlib_method_is_static(handle_);
}

int TEFKernel::PatchLib::Method::GetToken() const {
    return patchlib_method_get_token(handle_);
}

patch_type_t TEFKernel::PatchLib::Method::GetReturnType() const {
    return patchlib_method_get_return_type(handle_);
}

TEFKernel::PatchLib::Method TEFKernel::PatchLib::Method::MakeGenericInstance(
const Std::Vector<patch_handle_t> &templateTypes) const {
    return Method(
        patchlib_method_make_generic_instance(
            handle_,
            templateTypes.getHandle()
        ),
        true
    );
}

#if __ANDROID__
[[nodiscard]] void *TEFKernel::PatchLib::Method::GetPointer() const {
    return patchlib_method_get_pointer(handle_);
}
#endif


bool TEFKernel::PatchLib::Method::InvokeArgs(patch_handle_t instance, void *returnValue, void **args) const {
    return patchlib_method_invoke_args(handle_, instance, returnValue, args);
}

bool TEFKernel::PatchLib::Method::InvokeConstructorRaw(patch_handle_t*return_instance, void **args) const {
    if (return_instance == nullptr) {
        return false;
    }
    return patchlib_constructor_invoke(handle_, return_instance, args);
}

std::unique_ptr<TEFKernel::PatchLib::MethodSignature> TEFKernel::PatchLib::Method::GetSignature() const {
    if (auto sig = std::make_unique<MethodSignature>(); patchlib_method_get_signature(handle_, const_cast<patch_method_signature_t *>(sig->GetRaw()))) {
        return sig;
    }
    return nullptr;
}

TEFKernel::PatchLib::HookId TEFKernel::PatchLib::Method::InstallPrePostHook(HookCallbacks::PrefixCallback prefix,
HookCallbacks::PostfixCallback postfix) const {
    const auto callbacks = std::make_shared<HookCallbacks>(prefix, postfix);

    const patch_hook_id_t id = patchlib_install_prepost_hook(
        handle_,
        callbacks.get()->getPrefix(), // 实际应传入prefixWrapper
        callbacks.get()->getPostfix() // 实际应传入postfixWrapper
    );

    return HookId(id);
}

bool TEFKernel::PatchLib::Method::UninstallHook(const HookId &hookId) {
    return patchlib_uninstall_hook(hookId.Get());
}

TEFKernel::PatchLib::Method::operator bool() const {
    return IsValid();
}

bool TEFKernel::PatchLib::Method::operator==(const Method &other) const {
    return handle_ == other.handle_;
}

bool TEFKernel::PatchLib::Method::operator!=(const Method &other) const {
    return !(*this == other);
}
