/*******************************************************************************
 * File: tefpkg
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

#include "tefkernel-cpp-wrapper/tefpkg.hpp"

TEFKernel::TEFPackage::EntryInfo::EntryInfo(const tefpkg_entry_t *c_entry) {
    if (c_entry) {
        index = c_entry->index;
        data_offset = c_entry->data_offset;
        compressed_size = c_entry->compressed_size;
        original_size = c_entry->original_size;
        checksum = c_entry->checksum;
        timestamp = c_entry->timestamp;
        compress_type = static_cast<CompressionType>(c_entry->compress_type);
        compress_level = c_entry->compress_level;
    }
}

double TEFKernel::TEFPackage::EntryInfo::CompressionRatio() const {
    if (compressed_size == 0) return 1.0;
    return static_cast<double>(original_size) / compressed_size;
}

bool TEFKernel::TEFPackage::EntryInfo::IsCompressed() const {
    return compress_type != CompressionType::None;
}

TEFKernel::TEFPackage::TefPkg::TefPkg(const std::string &filename, const uint16_t reserved_entries)  {
    tefpkg_t *pkg = nullptr;
    if (const auto result = tefpkg_create_reserved_from_file(filename.c_str(), reserved_entries, &pkg);
        result != TEF_OK || !pkg)
        throw TefPkgException("Failed to create package from file: " + filename, result);
    pkg_ = pkg;
    own_pkg_ = true;
}

TEFKernel::TEFPackage::TefPkg::TefPkg(const uint16_t reserved_entries)  {
    tefpkg_t *pkg = nullptr;
    if (const auto result = tefpkg_create_reserved_from_memory(reserved_entries, &pkg);
        result != TEF_OK || !pkg)
        throw TefPkgException("Failed to create package from memory", result);
    pkg_ = pkg;
    own_pkg_ = true;
}

TEFKernel::TEFPackage::TefPkg::TefPkg(const std::string &filename)  {
    tefpkg_t *pkg = nullptr;
    if (const auto result = tefpkg_open_readonly(filename.c_str(), &pkg); result != TEF_OK || !pkg)
        throw TefPkgException("Failed to open package: " + filename, result);
    pkg_ = pkg;
    own_pkg_ = true;
}

TEFKernel::TEFPackage::TefPkg::TefPkg(const std::vector<uint8_t> &data)  {
    tefpkg_t *pkg = nullptr;
    const auto result = tefpkg_open_from_memory(data.data(),
                                                static_cast<uint32_t>(data.size()),
                                                &pkg);
    if (result != TEF_OK || !pkg)
        throw TefPkgException("Failed to open package from memory", result);
    pkg_ = pkg;
    own_pkg_ = true;
}

TEFKernel::TEFPackage::TefPkg::~TefPkg()  {
    Close();
}

TEFKernel::TEFPackage::TefPkg & TEFKernel::TEFPackage::TefPkg::operator=(TefPkg &&other) noexcept  {
    if (this != &other) {
        Close();
        pkg_ = other.pkg_;
        own_pkg_ = other.own_pkg_;
        other.pkg_ = nullptr;
        other.own_pkg_ = false;
    }
    return *this;
}

void TEFKernel::TEFPackage::TefPkg::Save(const std::string &filename, const uint64_t fingerprint) const  {
    if (!pkg_)
        throw TefPkgException("Package not initialized");
    if (const auto result = tefpkg_save_memory_file(filename.c_str(), pkg_, fingerprint); result != TEF_OK)
        throw TefPkgException("Failed to save package to file: " + filename, result);
}

void TEFKernel::TEFPackage::TefPkg::Save(const uint64_t fingerprint) const  {
    if (!pkg_) {
        throw TefPkgException("Package not initialized");
    }
    if (const auto result = tefpkg_save_file(pkg_, fingerprint); result != TEF_OK)
        throw TefPkgException("Failed to save package", result);
}

void TEFKernel::TEFPackage::TefPkg::Close()  {
    if (pkg_ && own_pkg_) {
        tefpkg_close(pkg_);
        pkg_ = nullptr;
    }
}

void TEFKernel::TEFPackage::TefPkg::AddEntry(const std::vector<uint8_t> &data, CompressionType compress_type,
const uint8_t compress_level) const {
    if (!pkg_) {
        throw TefPkgException("Package not initialized");
    }
    const auto result = tefpkg_add_entry_from_memory(
        pkg_,
        static_cast<tefpkg_compress_t>(compress_type),
        compress_level,
        const_cast<uint8_t *>(data.data()),
        static_cast<uint32_t>(data.size())
    );
    if (result != TEF_OK) {
        throw TefPkgException("Failed to add entry from memory", result);
    }
}

void TEFKernel::TEFPackage::TefPkg::AddEntryFromFile(const std::string &filepath, CompressionType compress_type,
const uint8_t compress_level) const {
    if (!pkg_) {
        throw TefPkgException("Package not initialized");
    }
    const auto result = tefpkg_add_entry_from_file(
        pkg_,
        filepath.c_str(),
        static_cast<tefpkg_compress_t>(compress_type),
        compress_level
    );
    if (result != TEF_OK) {
        throw TefPkgException("Failed to add entry from file: " + filepath, result);
    }
}

std::vector<uint8_t> TEFKernel::TEFPackage::TefPkg::ExtractEntry(const uint32_t index) const  {
    if (!pkg_) {
        throw TefPkgException("Package not initialized");
    }
    uint8_t *data = nullptr;
    uint32_t data_size = 0;
    if (const auto result = tefpkg_extract_entry_to_memory(pkg_, index, &data, &data_size);
        result != TEF_OK || !data) {
        throw TefPkgException("Failed to extract entry at index: " + std::to_string(index), result);
        }

    // 复制数据到vector并释放C内存
    std::vector result_data(data, data + data_size);
    free(data); // 假设使用malloc分配，需要free
    return result_data;
}

void TEFKernel::TEFPackage::TefPkg::ExtractEntryToFile(const uint32_t index, const std::string &output_path) const  {
    if (!pkg_)
        throw TefPkgException("Package not initialized");
    if (const auto result = tefpkg_extract_entry_to_file(pkg_, index, output_path.c_str()); result != TEF_OK)
        throw TefPkgException("Failed to extract entry to file: " + output_path, result);
}

std::optional<TEFKernel::TEFPackage::EntryInfo> TEFKernel::TEFPackage::TefPkg::
GetEntryInfo(const uint32_t index) const  {
    if (!pkg_) {
        throw TefPkgException("Package not initialized");
    }
    tefpkg_entry_t *info = nullptr;
    if (const auto result = tefpkg_get_entry_info(pkg_, index, &info); result != TEF_OK || !info)
        return std::nullopt;
    return EntryInfo(info);
}

uint16_t TEFKernel::TEFPackage::TefPkg::GetEntryCount() const  {
    if (!pkg_) {
        return 0;
    }
    return tefpkg_get_entries_count(pkg_);
}

uint16_t TEFKernel::TEFPackage::TefPkg::GetReservedEntries() const  {
    if (!pkg_) {
        return 0;
    }
    return tefpkg_get_reserved_entries(pkg_);
}

bool TEFKernel::TEFPackage::TefPkg::VerifyEntry(const uint32_t index) const  {
    if (!pkg_) {
        throw TefPkgException("Package not initialized");
    }
    const auto result = tefpkg_verify_entry(pkg_, index);
    if (result == TEF_OK) {
        return true;
    }
    if (result == TEF_ERROR_INTEGRITY) {
        return false;
    }
    throw TefPkgException("Failed to verify entry", result);
}

bool TEFKernel::TEFPackage::TefPkg::VerifyPackage() const  {
    if (!pkg_) {
        throw TefPkgException("Package not initialized");
    }
    const auto result = tefpkg_verify_pkg(pkg_);
    if (result == TEF_OK) {
        return true;
    }
    if (result == TEF_ERROR_INTEGRITY) {
        return false;
    }
    throw TefPkgException("Failed to verify package", result);
}

bool TEFKernel::TEFPackage::TefPkg::VerifySignature(const uint64_t fingerprint) const {
    if (!pkg_) {
        throw TefPkgException("Package not initialized");
    }
    const auto result = tefpkg_verify_signature(pkg_, fingerprint);
    if (result == TEF_OK) {
        return true;
    }
    if (result == TEF_ERROR_NOT_SIGNATURE || result == TEF_ERROR_INTEGRITY) {
        return false;
    }
    throw TefPkgException("Failed to verify signature", result);
}

void TEFKernel::TEFPackage::TefPkg::SignPackage(const uint64_t fingerprint) const {
    if (!pkg_) {
        throw TefPkgException("Package not initialized");
    }
    if (const auto result = tefpkg_sign_package(pkg_, fingerprint); result != TEF_OK) {
        throw TefPkgException("Failed to sign package", result);
    }
}

TEFKernel::TEFPackage::AccessMode TEFKernel::TEFPackage::TefPkg::GetAccessMode() const  {
    if (!pkg_) {
        return AccessMode::Closed;
    }
    return static_cast<AccessMode>(pkg_->access_mode);
}

bool TEFKernel::TEFPackage::TefPkg::IsOpen() const {
    return pkg_ != nullptr;
}

tefpkg_t * TEFKernel::TEFPackage::TefPkg::GetHandle() const {
    return pkg_;
}

tefpkg_t * TEFKernel::TEFPackage::TefPkg::Release() {
    const auto pkg = pkg_;
    pkg_ = nullptr;
    own_pkg_ = false;
    return pkg;
}

TEFKernel::TEFPackage::TefPkgPtr TEFKernel::TEFPackage::MakeTefPkg(const std::string &filename) {
    return std::make_unique<TefPkg>(filename);
}

TEFKernel::TEFPackage::TefPkgPtr TEFKernel::TEFPackage::MakeTefPkg(const std::string &filename,
uint16_t reserved_entries)  {
    return std::make_unique<TefPkg>(filename, reserved_entries);
}

TEFKernel::TEFPackage::TefPkgPtr TEFKernel::TEFPackage::MakeTefPkg(const std::vector<uint8_t> &data) {
    return std::make_unique<TefPkg>(data);
}

std::string TEFKernel::TEFPackage::ResultToString(const tefpkg_result_t result)  {
    switch (result) {
        case TEF_OK: return "Success";
        case TEF_ERROR: return "General error";
        case TEF_ERROR_SIGNATURE: return "Signature validation failed";
        case TEF_ERROR_CORRUPT: return "Data corrupted";
        case TEF_ERROR_MEMORY: return "Memory allocation failed";
        case TEF_ERROR_IO: return "I/O error";
        case TEF_ERROR_KEYFILE: return "Key file error";
        case TEF_ERROR_NOT_FOUND: return "Not found";
        case TEF_ERROR_INVALID: return "Invalid parameter or state";
        case TEF_ERROR_NOT_SIGNATURE: return "Not signed";
        case TEF_ERROR_INTEGRITY: return "Integrity check failed";
        case TEF_ERROR_NO_SPACE: return "No space left";
        default: return "Unknown error";
    }
}