/*******************************************************************************
 * File: field
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

#include "tefkernel-cpp-wrapper/patchlib/field.hpp"


TEFKernel::PatchLib::Field::~Field() {
    if (ownsHandle_ && handle_ != nullptr) {
#if !defined(__ANDROID__)
        patchlib_free(handle_);
#endif
    }
}

TEFKernel::PatchLib::Field::Field(const Field &other)  : handle_(other.handle_), ownsHandle_(false) {
    if (other.ownsHandle_ && other.handle_ != nullptr) {
#if !defined(__ANDROID__)
        handle_ = patchlib_handle_copy(other.handle_);
        ownsHandle_ = true;
#endif
    }
}

TEFKernel::PatchLib::Field & TEFKernel::PatchLib::Field::operator=(const Field &other)  {
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

TEFKernel::PatchLib::Field & TEFKernel::PatchLib::Field::operator=(Field &&other) noexcept  {
    if (this != &other) {
        Reset();
        handle_ = other.handle_;
        ownsHandle_ = other.ownsHandle_;
        other.handle_ = nullptr;
        other.ownsHandle_ = false;
    }
    return *this;
}

bool TEFKernel::PatchLib::Field::IsValid() const {
    return patchlib_is_valid(handle_);
}

void TEFKernel::PatchLib::Field::Reset(patch_handle_t newHandle, const bool takeOwnership)  {
    if (ownsHandle_ && handle_ != nullptr) {
#if !defined(__ANDROID__)
        patchlib_free(handle_);
#endif
    }
    handle_ = newHandle;
    ownsHandle_ = takeOwnership;
}

patch_handle_t TEFKernel::PatchLib::Field::Release() {
    patch_handle_t h = handle_;
    handle_ = nullptr;
    ownsHandle_ = false;
    return h;
}

std::string TEFKernel::PatchLib::Field::GetName() const {
    const char *name = patchlib_field_get_name(handle_);
    return name ? std::string(name) : std::string();
}

bool TEFKernel::PatchLib::Field::IsStatic() const {
    return patchlib_field_is_static(handle_);
}

bool TEFKernel::PatchLib::Field::IsInstance() const {
    return patchlib_field_is_instance(handle_);
}

bool TEFKernel::PatchLib::Field::IsConst() const {
    return patchlib_field_is_const(handle_);
}

bool TEFKernel::PatchLib::Field::IsThreadStatic() const {
    return patchlib_field_is_thread_static(handle_);
}

void TEFKernel::PatchLib::Field::GetValue(patch_handle_t instance, void *value) const {
    patchlib_field_get_value(handle_, instance, value);
}

void TEFKernel::PatchLib::Field::SetValue(patch_handle_t instance, void *value) const {
    patchlib_field_set_value(handle_, instance, value);
}

size_t TEFKernel::PatchLib::Field::GetSize() const {
    return patchlib_field_get_size(handle_);
}

patch_type_t TEFKernel::PatchLib::Field::GetType() const {
    return patchlib_field_get_type(handle_);
}

#if __ANDROID__
void * TEFKernel::PatchLib::Field::GetPointer(void *instance) const  {
    return patchlib_field_get_pointer(handle_, instance);
}
#endif


TEFKernel::PatchLib::Field::operator bool() const {
    return IsValid();
}

bool TEFKernel::PatchLib::Field::operator==(const Field &other) const {
    return handle_ == other.handle_;
}

bool TEFKernel::PatchLib::Field::operator!=(const Field &other) const {
    return !(*this == other);
}