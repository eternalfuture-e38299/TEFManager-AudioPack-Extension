/*******************************************************************************
 * audiopack_extension - XnbSound
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

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @brief 从 .xnb（SoundEffectReader）解析出来的音频描述。
 */
struct XnbSound {
    std::vector<uint8_t> format;      // WaveFormatEx 原始字节
    std::vector<uint8_t> waveform;    // 压缩波形数据
    uint16_t format_tag = 0;
    uint16_t channel_count = 0;
    uint32_t sample_rate = 0;
    uint32_t avg_bytes_per_sec = 0;
    uint16_t block_align = 0;
    uint16_t bits_per_sample = 0;
    int32_t loop_start = 0;
    int32_t loop_length = 0;
    int32_t duration_ms = 0;
};

/**
 * @brief 从内存解析 .xnb。
 * @param data   .xnb 字节
 * @param size   字节数
 * @param out    解析结果
 * @param err    错误信息缓冲（可为 nullptr）
 * @param errcap 缓冲大小
 * @return 成功返回 true
 */
bool XnbParseSound(const void* data, size_t size, XnbSound& out,
                   char* err = nullptr, int errcap = 0);

/**
 * @brief 把 XnbSound 归一化为 s16 交错 PCM。
 * @param s          解析结果
 * @param out_pcm    输出 PCM
 * @param out_rate   输出采样率
 * @param out_ch     输出声道数
 */
bool XnbNormalizePcm16(const XnbSound& s, std::vector<uint8_t>& out_pcm,
                       int& out_rate, int& out_ch,
                       char* err = nullptr, int errcap = 0);