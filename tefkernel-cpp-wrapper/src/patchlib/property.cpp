/*******************************************************************************
 * File: property
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

#include "tefkernel-cpp-wrapper/patchlib/property.hpp"

TEFKernel::PatchLib::Property::~Property() {
    if (ownsHandle_ && handle_ != nullptr) {
#if !defined(__ANDROID__)
        patchlib_free(handle_);
#endif
    }
}

TEFKernel::PatchLib::Property::Property(const Property &other)  : handle_(other.handle_), ownsHandle_(false) {
    if (other.ownsHandle_ && other.handle_ != nullptr) {
#if !defined(__ANDROID__)
        handle_ = patchlib_handle_copy(other.handle_);
        ownsHandle_ = true;
#endif
    }
}

TEFKernel::PatchLib::Property & TEFKernel::PatchLib::Property::operator=(const Property &other) {
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

TEFKernel::PatchLib::Property::Property(Property &&other) noexcept
            : handle_(other.handle_), ownsHandle_(other.ownsHandle_) {
    other.handle_ = nullptr;
    other.ownsHandle_ = false;
}

TEFKernel::PatchLib::Property & TEFKernel::PatchLib::Property::operator=(Property &&other) noexcept  {
    if (this != &other) {
        Reset();
        handle_ = other.handle_;
        ownsHandle_ = other.ownsHandle_;
        other.handle_ = nullptr;
        other.ownsHandle_ = false;
    }
    return *this;
}

bool TEFKernel::PatchLib::Property::IsValid() const {
    return patchlib_is_valid(handle_);
}

void TEFKernel::PatchLib::Property::Reset(patch_handle_t newHandle, const bool takeOwnership)  {
    if (ownsHandle_ && handle_ != nullptr) {
#if !defined(__ANDROID__)
        patchlib_free(handle_);
#endif
    }
    handle_ = newHandle;
    ownsHandle_ = takeOwnership;
}

patch_handle_t TEFKernel::PatchLib::Property::Release() {
    patch_handle_t h = handle_;
    handle_ = nullptr;
    ownsHandle_ = false;
    return h;
}

std::string TEFKernel::PatchLib::Property::GetName() const {
    const char *name = patchlib_property_get_name(handle_);
    return name ? std::string(name) : std::string();
}

TEFKernel::PatchLib::Method TEFKernel::PatchLib::Property::GetGetMethod() const {
    return Method(patchlib_property_get_get_method(handle_), true);
}

TEFKernel::PatchLib::Method TEFKernel::PatchLib::Property::GetSetMethod() const {
    return Method(patchlib_property_get_set_method(handle_), true);
}

TEFKernel::PatchLib::Property::operator bool() const {
    return IsValid();
}

bool TEFKernel::PatchLib::Property::operator==(const Property &other) const {
    return handle_ == other.handle_;
}

bool TEFKernel::PatchLib::Property::operator!=(const Property &other) const {
    return !(*this == other);
}