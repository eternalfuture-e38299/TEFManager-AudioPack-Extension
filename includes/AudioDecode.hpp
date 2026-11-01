/*******************************************************************************
 * audiopack_extension - AudioDecode
 * Copyright (C) 2026 eternalfuture-e38299
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Author: eternalfuture-e38299
 * GitHub: https://github.com/eternalfuture-e38299
 * Created: 2026/10/29
 *******************************************************************************/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief 把内存中的 ogg/mp3/wav 解码为 s16 交错 PCM。
 *
 * @param data      压缩数据
 * @param size      数据长度
 * @param name      条目名（用于按扩展名分派解码器，并写日志）
 * @param out_pcm   输出 PCM
 * @param out_rate  输出采样率
 * @param out_ch    输出声道数
 * @return 成功返回 true
 */
bool DecodeMemoryToS16(const void* data, size_t size, const std::string& name,
                       std::vector<uint8_t>& out_pcm, int& out_rate, int& out_ch);

/**
 * @brief 把内存中的原始 s16 PCM 包装成输出（用于 .pcm 快路径）。
 */
bool WrapPcmS16(const void* data, size_t size, int rate, int channels,
                std::vector<uint8_t>& out_pcm, int& out_rate, int& out_ch);

/**
 * @brief 解码 .xnb（SoundEffectReader）为 s16 交错 PCM。
 *        需要 xnb_parse_sound / xnb_normalize_pcm16。
 */
bool DecodeXnbToS16(const void* data, size_t size,
                    std::vector<uint8_t>& out_pcm, int& out_rate, int& out_ch);