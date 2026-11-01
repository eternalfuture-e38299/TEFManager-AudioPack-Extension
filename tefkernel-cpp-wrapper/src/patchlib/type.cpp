/*******************************************************************************
 * File: type
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

#include "tefkernel-cpp-wrapper/patchlib/type.hpp"

#include "tefkernel-cpp-wrapper/patchlib/field.hpp"
#include "tefkernel-cpp-wrapper/patchlib/property.hpp"

TEFKernel::PatchLib::Type::Type(const std::string &ns, const std::string &name)
            : handle_(patchlib_type_get_type(ns.c_str(), name.c_str())), ownsHandle_(true) {
    if (handle_ == nullptr) {
        throw std::runtime_error("Failed to get type: " + ns + "." + name);
    }
}

TEFKernel::PatchLib::Type::~Type() {
    if (ownsHandle_) ManualDestroy();
}

void TEFKernel::PatchLib::Type::ManualDestroy() {
    if (ownsHandle_ && handle_ != nullptr) {
        patchlib_free(handle_);
    }
    Release();
}

TEFKernel::PatchLib::Type::Type(const Type &other)  : handle_(other.handle_), ownsHandle_(true) {
    if (other.ownsHandle_ && other.handle_ != nullptr) {
#if !defined(__ANDROID__)
        handle_ = patchlib_handle_copy(other.handle_);
        ownsHandle_ = true;
#endif
    }
}

TEFKernel::PatchLib::Type &TEFKernel::PatchLib::Type::operator=(const Type &other) {
    if (this != &other) {
        Reset();
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

TEFKernel::PatchLib::Type::Type(Type &&other) noexcept
            : handle_(other.handle_), ownsHandle_(other.ownsHandle_) {
    other.handle_ = nullptr;
    other.ownsHandle_ = false;
}

TEFKernel::PatchLib::Type &TEFKernel::PatchLib::Type::operator=(Type &&other) noexcept {
    if (this != &other) {
        Reset();
        handle_ = other.handle_;
        ownsHandle_ = other.ownsHandle_;
        other.handle_ = nullptr;
        other.ownsHandle_ = false;
    }
    return *this;
}

bool TEFKernel::PatchLib::Type::IsValid() const {
    return patchlib_is_valid(handle_);
}

void TEFKernel::PatchLib::Type::Reset(patch_handle_t newHandle, const bool takeOwnership) {
    if (ownsHandle_ && handle_ != nullptr) {
        patchlib_free(handle_);
    }
    handle_ = newHandle;
    ownsHandle_ = takeOwnership;
}

patch_handle_t TEFKernel::PatchLib::Type::Release() {
    patch_handle_t h = handle_;
    handle_ = nullptr;
    ownsHandle_ = false;
    return h;
}

patch_handle_t TEFKernel::PatchLib::Type::GetMonoType() const {
    return patchlib_type_get_mono_type(handle_);
}

std::string TEFKernel::PatchLib::Type::GetName() const {
    const char *name = patchlib_type_get_name(handle_);
    return name ? std::string(name) : std::string();
}

std::string TEFKernel::PatchLib::Type::GetNamespace() const {
    const char *ns = patchlib_type_get_namespace(handle_);
    return ns ? std::string(ns) : std::string();
}

std::string TEFKernel::PatchLib::Type::GetFullName() const {
    char *name = patchlib_type_get_full_name(handle_);
    if (!name) return {};
    std::string result(name);
    free(name);
    return result;
}

TEFKernel::PatchLib::Type TEFKernel::PatchLib::Type::GetParent() const {
    return Type(patchlib_type_get_parent(handle_), false);
}

TEFKernel::PatchLib::Type TEFKernel::PatchLib::Type::GetBasicType(const patch_type_t basicType) {
    return Type(patchlib_get_basic_type(basicType), false);
}

patch_handle_t TEFKernel::PatchLib::Type::NewInstance() const {
    return patchlib_type_new_instance(handle_);
}

TEFKernel::PatchLib::Type TEFKernel::PatchLib::Type::
MakeGenericType(const Std::Vector<patch_handle_t> &typeArgs) const {
    return Type(
        patchlib_type_make_generic_type(
            handle_,
            typeArgs.getHandle()
        ),
        true
    );
}

TEFKernel::PatchLib::Type TEFKernel::PatchLib::Type::GetInnerType(const std::string &name) const {
    return Type(patchlib_type_get_inner_type(handle_, name.c_str()), false);
}

TEFKernel::PatchLib::Field TEFKernel::PatchLib::Type::GetField(const std::string &name) const {
    return Field(patchlib_type_get_field(handle_, name.c_str()), false);
}

TEFKernel::Std::Vector<TEFKernel::PatchLib::Field> TEFKernel::PatchLib::Type::GetFields(
    const bool includingParent) const {
    tefstd_vector_t vec;
    if (!tefstd_vector_init(&vec, sizeof(patch_handle_t))) {
        return {};
    }

    if (patchlib_type_get_fields(handle_, includingParent, &vec)) {
        Std::Vector<Field> result;
        const auto *data = static_cast<patch_handle_t *>(vec.data);
        for (size_t i = 0; i < vec.size; ++i) {
            result.push_back(Field(data[i], false));
        }
        tefstd_vector_destroy(&vec);
        return result;
    }

    tefstd_vector_destroy(&vec);
    return {};
}

TEFKernel::Std::Vector<TEFKernel::PatchLib::Type> TEFKernel::PatchLib::Type::GetInnerTypes(
    const bool includingParent) const {
    tefstd_vector_t vec;
    if (!tefstd_vector_init(&vec, sizeof(patch_handle_t))) {
        return {};
    }

    if (patchlib_type_get_inner_types(handle_, includingParent, &vec)) {
        Std::Vector<Type> result;
        const auto *data = static_cast<patch_handle_t *>(vec.data);
        for (size_t i = 0; i < vec.size; ++i) {
            result.push_back(Type(data[i], false));
        }
        tefstd_vector_destroy(&vec);
        return result;
    }

    tefstd_vector_destroy(&vec);
    return {};
}


// Type类中需要的方法实现
TEFKernel::PatchLib::Property TEFKernel::PatchLib::Type::GetProperty(const std::string &name) const {
    return Property(patchlib_type_get_property(handle_, name.c_str()), false);
}

TEFKernel::Std::Vector<TEFKernel::PatchLib::Property> TEFKernel::PatchLib::Type::GetProperties(
    const bool includingParent) const {
    tefstd_vector_t vec;
    if (!tefstd_vector_init(&vec, sizeof(patch_handle_t))) {
        return {};
    }

    if (patchlib_type_get_properties(handle_, includingParent, &vec)) {
        Std::Vector<Property> result;
        const auto *data = static_cast<patch_handle_t *>(vec.data);
        for (size_t i = 0; i < vec.size; ++i) {
            result.push_back(Property(data[i], false));
        }
        tefstd_vector_destroy(&vec);
        return result;
    }

    tefstd_vector_destroy(&vec);
    return {};
}

bool TEFKernel::PatchLib::Type::operator==(const Type &other) const {
    return patchlib_type_is_same(handle_, other.handle_);
}

bool TEFKernel::PatchLib::Type::operator!=(const Type &other) const {
    return !(*this == other);
}

TEFKernel::PatchLib::Type::operator bool() const {
    return IsValid();
}

bool TEFKernel::PatchLib::Type::operator==(patch_handle_t other) const {
    return patchlib_type_is_same(handle_, other);
}

bool TEFKernel::PatchLib::Type::operator!=(patch_handle_t other) const {
    return !(*this == other);
}


// Type类中需要的方法实现
TEFKernel::PatchLib::Method TEFKernel::PatchLib::Type::GetMethod(const std::string &name) const {
    return Method(patchlib_type_get_method(handle_, name.c_str()), false);
}

TEFKernel::PatchLib::Method TEFKernel::PatchLib::Type::GetMethod(const std::string &name, const int argsCount) const {
    return Method(patchlib_type_get_method_by_param_count(handle_, name.c_str(), argsCount), false);
}

TEFKernel::PatchLib::Method TEFKernel::PatchLib::Type::GetMethod(
    const std::string &name,
    const Std::Vector<std::string> &argsNames
) const {
    // 构建C字符串数组
    std::vector<const char *> cStrings;
    cStrings.reserve(argsNames.size());
    for (const auto &s: argsNames) {
        cStrings.push_back(s.c_str());
    }
    return Method(
        patchlib_type_get_method_by_param_names(
            handle_,
            name.c_str(),
            static_cast<int>(argsNames.size()),
            cStrings.data()
        ),
        false
    );
}

TEFKernel::PatchLib::Method TEFKernel::PatchLib::Type::GetMethod(
    const std::string &name,
    const Std::Vector<Type> &argsTypes
) const {
    // 构建句柄数组
    Std::Vector<patch_handle_t> handles;
    handles.reserve(argsTypes.size());
    for (const auto &type: argsTypes) {
        handles.push_back(type.GetHandle());
    }
    return Method(
        patchlib_type_get_method_by_param_types(
            handle_,
            name.c_str(),
            static_cast<int>(argsTypes.size()),
            handles.data()
        ),
        false
    );
}

TEFKernel::PatchLib::Method TEFKernel::PatchLib::Type::GetMethod(
    const std::string &name,
    const Std::Vector<Type> &argsTypes,
    const Std::Vector<std::string> &argsNames
) const {
    // 构建句柄数组
    Std::Vector<patch_handle_t> handles;
    handles.reserve(argsTypes.size());
    for (const auto &type: argsTypes) {
        handles.push_back(type.GetHandle());
    }

    // 构建C字符串数组
    std::vector<const char *> cStrings;
    cStrings.reserve(argsNames.size());
    for (const auto &s: argsNames) {
        cStrings.push_back(s.c_str());
    }

    return Method(
        patchlib_type_get_method_by_signature(
            handle_,
            name.c_str(),
            static_cast<int>(argsTypes.size()),
            handles.data(),
            cStrings.data()
        ),
        false
    );
}

TEFKernel::Std::Vector<TEFKernel::PatchLib::Method> TEFKernel::PatchLib::Type::GetMethods(
    const bool includingParent) const {
    tefstd_vector_t vec;
    if (!tefstd_vector_init(&vec, sizeof(patch_handle_t))) {
        return {};
    }

    if (patchlib_type_get_methods(handle_, includingParent, &vec)) {
        Std::Vector<Method> result;
        const auto *data = static_cast<patch_handle_t *>(vec.data);
        for (size_t i = 0; i < vec.size; ++i) {
            result.push_back(Method(data[i], false));
        }
        tefstd_vector_destroy(&vec);
        return result;
    }

    tefstd_vector_destroy(&vec);
    return {};
}
