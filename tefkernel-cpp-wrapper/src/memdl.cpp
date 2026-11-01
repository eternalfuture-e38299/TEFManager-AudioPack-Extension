/*******************************************************************************
 * File: memdl
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

#include "tefkernel-cpp-wrapper/memdl.hpp"

TEFKernel::Memdl::Memdl(const std::string &filename, const int flags) {
    OpenFile(filename, flags);
}

TEFKernel::Memdl::Memdl(const void *data, const size_t size, const int flags) {
    OpenMemory(data, size, flags);
}

TEFKernel::Memdl::Memdl(const std::vector<uint8_t> &data, const int flags) {
    OpenMemory(data.data(), data.size(), flags);
}

TEFKernel::Memdl & TEFKernel::Memdl::operator=(Memdl &&other) noexcept {
    if (this != &other) {
        Close();
        handle_ = other.handle_;
        other.handle_ = nullptr;
    }
    return *this;
}

bool TEFKernel::Memdl::OpenFile(const std::string &filename, const int flags)  {
    Close();
    handle_ = memdl_open_file(filename.c_str(), flags);
    return handle_ != nullptr;
}

bool TEFKernel::Memdl::OpenMemory(const void *data, const size_t size, const int flags) {
    Close();
    handle_ = memdl_open(data, size, flags);
    return handle_ != nullptr;
}

bool TEFKernel::Memdl::OpenMemory(const std::vector<uint8_t> &data, const int flags)  {
    return OpenMemory(data.data(), data.size(), flags);
}

void * TEFKernel::Memdl::GetSymbol(const std::string &symbol) const {
    if (!IsOpen()) {
        return nullptr;
    }
    return memdl_sym(handle_, symbol.c_str());
}

void TEFKernel::Memdl::Close()  {
    if (handle_) {
        memdl_close(handle_);
        handle_ = nullptr;
    }
}

bool TEFKernel::Memdl::IsOpen() const {
    return handle_ != nullptr;
}

memdl_handle_t TEFKernel::Memdl::GetHandle() const {
    return handle_;
}

std::string TEFKernel::Memdl::GetLastError() {
    const char *err = memdl_error();
    return err ? std::string(err) : std::string();
}

bool TEFKernel::Memdl::Validate(const void *data, const size_t size)  {
    return memdl_validate(data, size) != 0;
}

bool TEFKernel::Memdl::Validate(const std::vector<uint8_t> &data)  {
    return Validate(data.data(), data.size());
}

int TEFKernel::Memdl::GetArch(const void *data, const size_t size)  {
    return memdl_get_arch(data, size);
}

int TEFKernel::Memdl::GetArch(const std::vector<uint8_t> &data) {
    return GetArch(data.data(), data.size());
}

int TEFKernel::Memdl::GetPlatform() {
    return memdl_get_platform();
}

bool TEFKernel::Memdl::HasSymbol(const std::string &symbol) const {
    return GetSymbol(symbol) != nullptr;
}

TEFKernel::MemdlPtr TEFKernel::MakeMemdl(const std::string &filename, const int flags)  {
    auto memdl = std::make_unique<Memdl>();
    if (!memdl->OpenFile(filename, flags)) {
        throw MemdlException("Failed to open library: " + filename +
                             ", error: " + Memdl::GetLastError());
    }
    return memdl;
}

TEFKernel::MemdlPtr TEFKernel::MakeMemdl(const void *data, const size_t size, const int flags)  {
    auto memdl = std::make_unique<Memdl>();
    if (!memdl->OpenMemory(data, size, flags)) {
        throw MemdlException("Failed to open library from memory, error: " + Memdl::GetLastError());
    }
    return memdl;
}

TEFKernel::MemdlPtr TEFKernel::MakeMemdl(const std::vector<uint8_t> &data, const int flags) {
    return MakeMemdl(data.data(), data.size(), flags);
}