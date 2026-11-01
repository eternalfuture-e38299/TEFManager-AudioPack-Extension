/*******************************************************************************
 * File: language_manager
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

#pragma once

#include "tefkernel/terraria/language_manager.h"
#include "../patchlib/type.hpp"

namespace TEFKernel::Terraria::LanguageManager {

    /**
     * @brief 语言文化枚举（透传 C 枚举）
     */
    using Culture = language_culture_t;

    inline constexpr Culture English            = LANGUAGE_ENGLISH;
    inline constexpr Culture German             = LANGUAGE_GERMAN;
    inline constexpr Culture Italian            = LANGUAGE_ITALIAN;
    inline constexpr Culture French             = LANGUAGE_FRENCH;
    inline constexpr Culture Spanish            = LANGUAGE_SPANISH;
    inline constexpr Culture Russian            = LANGUAGE_RUSSIAN;
    inline constexpr Culture ChineseSimplified  = LANGUAGE_CHINESE_SIMPLIFIED;
    inline constexpr Culture Portuguese         = LANGUAGE_PORTUGUESE;
    inline constexpr Culture Polish             = LANGUAGE_POLISH;
    inline constexpr Culture Japanese           = LANGUAGE_JAPANESE;
    inline constexpr Culture Korean             = LANGUAGE_KOREAN;
    inline constexpr Culture ChineseTraditional = LANGUAGE_CHINESE_TRADITIONAL;

    /**
     * @brief 自定义本地化查询回调类型
     */
    using GetLocalizedTextCallback = terraria_languager_manager_get_localized_text_t;

    /**
     * @brief 创建一个本地化文本对象
     * @return LocalizedText 句柄，失败返回 nullptr
     */
    [[nodiscard]] patch_handle_t CreateLocalizedText(const char *key, const char *value);

    /**
     * @brief 获取指定键名的本地化文本
     * @return 句柄，未找到返回 nullptr
     */
    [[nodiscard]] patch_handle_t GetLocalizedText(const char *key);

    /**
     * @brief 注册本地化回调
     */
    void RegisterCallback(GetLocalizedTextCallback callback);

}