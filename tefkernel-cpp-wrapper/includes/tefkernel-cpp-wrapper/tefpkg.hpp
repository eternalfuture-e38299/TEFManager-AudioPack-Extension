/*******************************************************************************
 * File: tefpkg
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

#include "tefkernel/tefpackage/tefpkg.h"
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <filesystem>
#include <optional>

namespace TEFKernel::TEFPackage {
    /**
     * @brief TEF包异常类
     */
    class TefPkgException : public std::runtime_error {
    public:
        explicit TefPkgException(const std::string &message, const tefpkg_result_t code = TEF_ERROR)
            : std::runtime_error(message), error_code_(code) {
        }

        [[nodiscard]] tefpkg_result_t errorCode() const noexcept { return error_code_; }

    private:
        tefpkg_result_t error_code_;
    };

    /**
     * @brief 压缩类型枚举（C++风格）
     */
    enum class CompressionType : uint8_t {
        None = COMPRESS_NONE,
        LZ4 = COMPRESS_LZ4,
        LZ4HC = COMPRESS_LZ4HC
    };

    /**
     * @brief 访问模式枚举（C++风格）
     */
    enum class AccessMode : uint8_t {
        Closed = TEF_ACCESS_CLOSED,
        Memory = TEF_ACCESS_MEMORY,
        File = TEF_ACCESS_FILE, // 已弃用
        ReadOnly = TEF_ACCESS_READONLY,
        MemData = TEF_ACCESS_MEMDATA,
        ReadWrite = TEF_ACCESS_READWRITE
    };

    /**
     * @brief 条目信息结构体（C++风格）
     */
    struct EntryInfo {
        uint32_t index;
        uint32_t data_offset;
        uint32_t compressed_size;
        uint32_t original_size;
        uint64_t checksum;
        uint64_t timestamp;
        CompressionType compress_type;
        uint8_t compress_level;

        // 从C结构体转换
        explicit EntryInfo(const tefpkg_entry_t *c_entry);

        // 获取压缩比率
        [[nodiscard]] double CompressionRatio() const;

        // 是否已压缩
        [[nodiscard]] bool IsCompressed() const;
    };

    /**
     * @brief TEF包管理类（RAII风格）
     *
     * 提供完整的TEF包生命周期管理，支持从文件/内存创建、打开，
     * 添加/提取条目，验证和签名等功能。
     */
    class TefPkg {
    public:
        // 常量
        static constexpr uint32_t Magic = TEFPKG_MAGIC;
        static constexpr uint16_t Version = TEFPKG_VERSION;
        static constexpr uint16_t MaxFiles = TEFPKG_MAX_FILES;

        /**
         * @brief 默认构造函数
         */
        TefPkg() : pkg_(nullptr), own_pkg_(true) {
        }

        /**
         * @brief 从文件创建新的TEF包（预留空间版本）
         * @param filename 文件名
         * @param reserved_entries 预留的条目数量
         * @throws TefPkgException 创建失败时抛出
         */
        TefPkg(const std::string &filename, uint16_t reserved_entries);

        /**
         * @brief 从内存创建新的TEF包（预留空间版本）
         * @param reserved_entries 预留的条目数量
         * @throws TefPkgException 创建失败时抛出
         */
        explicit TefPkg(uint16_t reserved_entries);

        /**
         * @brief 以只读方式打开现有的TEF包
         * @param filename 包文件名
         * @throws TefPkgException 打开失败时抛出
         */
        explicit TefPkg(const std::string &filename);

        /**
         * @brief 从内存数据打开TEF包
         * @param data 内存数据
         * @throws TefPkgException 打开失败时抛出
         */
        explicit TefPkg(const std::vector<uint8_t> &data);

        /**
         * @brief 使用现有C指针构造（不接管所有权）
         * @param pkg C指针
         */
        explicit TefPkg(tefpkg_t *pkg) : pkg_(pkg), own_pkg_(false) {
        }

        /**
         * @brief 移动构造函数
         */
        TefPkg(TefPkg &&other) noexcept
            : pkg_(other.pkg_), own_pkg_(other.own_pkg_) {
            other.pkg_ = nullptr;
            other.own_pkg_ = false;
        }

        /**
         * @brief 析构函数
         */
        ~TefPkg();

        // 禁止拷贝
        TefPkg(const TefPkg &) = delete;

        TefPkg &operator=(const TefPkg &) = delete;

        /**
         * @brief 移动赋值操作符
         */
        TefPkg &operator=(TefPkg &&other) noexcept;

        /**
         * @brief 保存包到文件
         * @param filename 保存到的文件
         * @param fingerprint 指纹
         * @throws TefPkgException 保存失败时抛出
         */
        void Save(const std::string &filename, uint64_t fingerprint = 0) const;

        /**
         * @brief 保存包到文件（当前包的文件名）
         * @param fingerprint 指纹
         * @throws TefPkgException 保存失败时抛出
         */
        void Save(uint64_t fingerprint = 0) const;

        /**
         * @brief 关闭包并释放资源
         */
        void Close();

        /**
         * @brief 添加条目（从内存）
         * @param data 数据
         * @param compress_type 压缩类型
         * @param compress_level 压缩等级
         * @throws TefPkgException 添加失败时抛出
         */
        void AddEntry(const std::vector<uint8_t> &data,
                      CompressionType compress_type = CompressionType::None,
                      uint8_t compress_level = 0) const;

        /**
         * @brief 添加条目（从文件）
         * @param filepath 文件路径
         * @param compress_type 压缩类型
         * @param compress_level 压缩等级
         * @throws TefPkgException 添加失败时抛出
         */
        void AddEntryFromFile(const std::string &filepath,
                              CompressionType compress_type = CompressionType::None,
                              uint8_t compress_level = 0) const;

        /**
         * @brief 提取条目到内存
         * @param index 条目索引
         * @return 数据向量
         * @throws TefPkgException 提取失败时抛出
         */
        [[nodiscard]] std::vector<uint8_t> ExtractEntry(uint32_t index) const;

        /**
         * @brief 提取条目到文件
         * @param index 条目索引
         * @param output_path 输出文件路径
         * @throws TefPkgException 提取失败时抛出
         */
        void ExtractEntryToFile(uint32_t index, const std::string &output_path) const;

        /**
         * @brief 获取条目信息
         * @param index 条目索引
         * @return 条目信息
         * @throws TefPkgException 获取失败时抛出
         */
        [[nodiscard]] std::optional<EntryInfo> GetEntryInfo(uint32_t index) const;

        /**
         * @brief 获取条目数量
         * @return 条目数量
         */
        [[nodiscard]] uint16_t GetEntryCount() const;

        /**
         * @brief 获取预留条目数量
         * @return 预留条目数量
         */
        [[nodiscard]] uint16_t GetReservedEntries() const;

        /**
         * @brief 验证条目完整性
         * @param index 条目索引
         * @return true 验证通过
         * @throws TefPkgException 验证失败时抛出
         */
        [[nodiscard]] bool VerifyEntry(uint32_t index) const;

        /**
         * @brief 验证整个包的完整性
         * @return true 验证通过
         * @throws TefPkgException 验证失败时抛出
         */
        [[nodiscard]] bool VerifyPackage() const;

        /**
         * @brief 验证包签名
         * @param fingerprint 指纹
         * @return true 验证通过
         * @throws TefPkgException 验证失败时抛出
         */
        [[nodiscard]] bool VerifySignature(uint64_t fingerprint) const;

        /**
         * @brief 签名包
         * @param fingerprint 指纹
         * @throws TefPkgException 签名失败时抛出
         */
        void SignPackage(uint64_t fingerprint) const;

        /**
         * @brief 获取访问模式
         * @return 访问模式
         */
        [[nodiscard]] AccessMode GetAccessMode() const;

        /**
         * @brief 检查包是否打开
         * @return true 已打开
         */
        [[nodiscard]] bool IsOpen() const;

        /**
         * @brief 获取C指针（不推荐使用）
         * @return C指针
         */
        [[nodiscard]] tefpkg_t *GetHandle() const;

        /**
         * @brief 释放所有权（返回C指针）
         * @return C指针
         */
        tefpkg_t *Release();

    private:
        tefpkg_t *pkg_;
        bool own_pkg_;
    };

    /**
     * @brief 智能指针类型
     */
    using TefPkgPtr = std::unique_ptr<TefPkg>;

    /**
     * @brief 创建TefPkg智能指针的工厂函数
     */
    TefPkgPtr MakeTefPkg(const std::string &filename);

    TefPkgPtr MakeTefPkg(const std::string &filename, uint16_t reserved_entries);

    TefPkgPtr MakeTefPkg(const std::vector<uint8_t> &data);

    /**
     * @brief 将tefpkg_result_t转换为字符串描述
     */
    std::string ResultToString(tefpkg_result_t result);
}
