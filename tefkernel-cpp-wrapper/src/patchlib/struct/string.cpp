/*******************************************************************************
 * File: string
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

#include "tefkernel-cpp-wrapper/patchlib/struct/string.hpp"

TEFKernel::PatchLib::Struct::String::String(const char *str)
            : handle_(nullptr), ownsHandle_(true) {
    handle_ = patchlib_string_create(str);
    if (handle_ == nullptr) {
        throw std::runtime_error("Failed to create string");
    }
}

TEFKernel::PatchLib::Struct::String::~String()  {
    if (ownsHandle_ && handle_ != nullptr) {
#if !defined(__ANDROID__)
        patchlib_free(handle_);
#endif
    }
}

TEFKernel::PatchLib::Struct::String::String(const String &other)  : handle_(other.handle_), ownsHandle_(false) {
    if (other.ownsHandle_ && other.handle_ != nullptr) {
#if !defined(__ANDROID__)
        handle_ = patchlib_handle_copy(other.handle_);
        ownsHandle_ = true;
#endif
    }
}

TEFKernel::PatchLib::Struct::String & TEFKernel::PatchLib::Struct::String::operator=(const String &other) {
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

TEFKernel::PatchLib::Struct::String::String(String &&other) noexcept
            : handle_(other.handle_), ownsHandle_(other.ownsHandle_) {
    other.handle_ = nullptr;
    other.ownsHandle_ = false;
}

TEFKernel::PatchLib::Struct::String & TEFKernel::PatchLib::Struct::String::operator=(String &&other) noexcept {
    if (this != &other) {
        Reset();
        handle_ = other.handle_;
        ownsHandle_ = other.ownsHandle_;
        other.handle_ = nullptr;
        other.ownsHandle_ = false;
    }
    return *this;
}

TEFKernel::PatchLib::Struct::String & TEFKernel::PatchLib::Struct::String::operator=(const char *str) {
    Reset();
    handle_ = patchlib_string_create(str);
    ownsHandle_ = true;
    if (handle_ == nullptr) {
        throw std::runtime_error("Failed to create string");
    }
    return *this;
}

TEFKernel::PatchLib::Struct::String & TEFKernel::PatchLib::Struct::String::operator=(const std::string &str) {
    return *this = str.c_str();
}

bool TEFKernel::PatchLib::Struct::String::IsValid() const {
    return patchlib_is_valid(handle_);
}

void TEFKernel::PatchLib::Struct::String::Reset(patch_handle_t newHandle, const bool takeOwnership) {
    if (ownsHandle_ && handle_ != nullptr) {
#if !defined(__ANDROID__)
        patchlib_free(handle_);
#endif
    }
    handle_ = newHandle;
    ownsHandle_ = takeOwnership;
}

patch_handle_t TEFKernel::PatchLib::Struct::String::Release() {
    patch_handle_t h = handle_;
    handle_ = nullptr;
    ownsHandle_ = false;
    return h;
}

std::unique_ptr<char, decltype(&free)> TEFKernel::PatchLib::Struct::String::CStr() const {
    char *str = patchlib_string_cstr(handle_);
    // 使用大括号初始化列表，避免重复返回类型
    return {str, free};
}

std::string TEFKernel::PatchLib::Struct::String::ToString() const  {
    const auto cstrPtr = CStr();
    return cstrPtr ? std::string(cstrPtr.get()) : std::string();
}

size_t TEFKernel::PatchLib::Struct::String::Length() const {
    return patchlib_string_length(handle_);
}

bool TEFKernel::PatchLib::Struct::String::Empty() const {
    return patchlib_string_empty(handle_);
}

TEFKernel::PatchLib::Struct::String::operator bool() const {
    return IsValid() && !Empty();
}

bool TEFKernel::PatchLib::Struct::String::operator==(const String &other) const {
    return ToString() == other.ToString();
}

bool TEFKernel::PatchLib::Struct::String::operator==(const char *str) const {
    return ToString() == str;
}

bool TEFKernel::PatchLib::Struct::String::operator==(const std::string &str) const {
    return ToString() == str;
}

bool TEFKernel::PatchLib::Struct::String::operator!=(const String &other) const {
    return !(*this == other);
}

bool TEFKernel::PatchLib::Struct::String::operator!=(const char *str) const {
    return !(*this == str);
}

bool TEFKernel::PatchLib::Struct::String::operator!=(const std::string &str) const {
    return !(*this == str);
}

bool TEFKernel::PatchLib::Struct::String::operator<(const String &other) const {
    return ToString() < other.ToString();
}

bool TEFKernel::PatchLib::Struct::String::operator>(const String &other) const {
    return ToString() > other.ToString();
}

bool TEFKernel::PatchLib::Struct::String::operator<=(const String &other) const {
    return ToString() <= other.ToString();
}

bool TEFKernel::PatchLib::Struct::String::operator>=(const String &other) const {
    return ToString() >= other.ToString();
}

bool TEFKernel::PatchLib::Struct::operator==(const char *lhs, const String &rhs) {
    return rhs == lhs;
}

bool TEFKernel::PatchLib::Struct::operator==(const std::string &lhs, const String &rhs) {
    return rhs == lhs;
}

bool TEFKernel::PatchLib::Struct::operator!=(const char *lhs, const String &rhs) {
    return rhs != lhs;
}

bool TEFKernel::PatchLib::Struct::operator!=(const std::string &lhs, const String &rhs) {
    return rhs != lhs;
}
