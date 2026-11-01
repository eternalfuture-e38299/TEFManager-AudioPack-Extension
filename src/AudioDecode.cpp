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

#include "AudioDecode.hpp"
#include "Log.hpp"

#include <string_view>

#include "libs/stb_vorbis.c"

#define DR_MP3_IMPLEMENTATION
#include "libs/dr_mp3.h"

#define DR_WAV_IMPLEMENTATION
#include "XnbSound.hpp"
#include "libs/dr_wav.h"

namespace {

std::string_view ExtOf(const std::string_view name) {
    const auto dot = name.find_last_of('.');
    if (dot == std::string_view::npos) return {};
    return name.substr(dot);
}

    bool LowerEquals(const std::string_view& a, const std::string_view& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        const char ca = static_cast<char>(a[i] | 0x20);
        if (const char cb = static_cast<char>(b[i] | 0x20); ca != cb) return false;
    }
    return true;
}

bool DecodeOgg(const void* data, const size_t size, std::vector<uint8_t>& out,
               int& rate, int& ch) {
    short* pcm = nullptr;
    const int frames = stb_vorbis_decode_memory(
        static_cast<const unsigned char*>(data), static_cast<int>(size),
        &ch, &rate, &pcm);
    if (frames < 0 || !pcm) return false;
    const size_t bytes = static_cast<size_t>(frames) * ch * sizeof(short);
    out.assign(reinterpret_cast<uint8_t*>(pcm),
               reinterpret_cast<uint8_t*>(pcm) + bytes);
    free(pcm);
    return true;
}

bool DecodeMp3(const void* data, const size_t size, std::vector<uint8_t>& out,
               int& rate, int& ch) {
    const drmp3_uint64 frames = drmp3_get_pcm_frame_count(
        nullptr);
    (void)frames;
    /* dr_mp3 没有直接从内存拿总帧数的便捷接口，用 decoder 逐块读 */
    drmp3 dec;
    if (!drmp3_init_memory(&dec, data, size, nullptr)) return false;
    rate = dec.sampleRate; // NOLINT(*-narrowing-conversions)
    ch = dec.channels; // NOLINT(*-narrowing-conversions)
    const drmp3_uint64 total = drmp3_get_pcm_frame_count(&dec);
    if (total == 0) {
        /* 某些流式 mp3 拿不到总帧数，退化为逐块追加 */
        out.clear();
        constexpr drmp3_uint64 kBlock = 4096;
        std::vector<short> block(kBlock * ch);
        drmp3_uint64 read;
        while ((read = drmp3_read_pcm_frames_s16(&dec, kBlock, block.data())) > 0) {
            const size_t bytes = static_cast<size_t>(read) * ch * sizeof(short);
            out.insert(out.end(),
                       reinterpret_cast<uint8_t*>(block.data()),
                       reinterpret_cast<uint8_t*>(block.data()) + bytes);
        }
        drmp3_uninit(&dec);
        return !out.empty();
    }
    out.resize(static_cast<size_t>(total) * ch * sizeof(short));
    const drmp3_uint64 got = drmp3_read_pcm_frames_s16(
        &dec, total, reinterpret_cast<short*>(out.data()));
    out.resize(static_cast<size_t>(got) * ch * sizeof(short));
    drmp3_uninit(&dec);
    return got > 0;
}

bool DecodeWav(const void* data, const size_t size, std::vector<uint8_t>& out,
               int& rate, int& ch) {
    drwav wav;
    if (!drwav_init_memory(&wav, data, size, nullptr)) return false;
    rate = static_cast<int>(wav.sampleRate);
    ch = static_cast<int>(wav.channels);
    const drwav_uint64 total = wav.totalPCMFrameCount;
    out.resize(static_cast<size_t>(total) * ch * sizeof(short));
    const drwav_uint64 got = drwav_read_pcm_frames_s16(
        &wav, total, reinterpret_cast<short*>(out.data()));
    out.resize(static_cast<size_t>(got) * ch * sizeof(short));
    drwav_uninit(&wav);
    return got > 0;
}

} // namespace

bool DecodeMemoryToS16(const void* data, const size_t size, const std::string& name,
                       std::vector<uint8_t>& out_pcm, int& out_rate, int& out_ch) {
    if (!data || size == 0) return false;
    const auto ext = ExtOf(name);
    if (LowerEquals(ext, ".ogg")) return DecodeOgg(data, size, out_pcm, out_rate, out_ch);
    if (LowerEquals(ext, ".mp3")) return DecodeMp3(data, size, out_pcm, out_rate, out_ch);
    if (LowerEquals(ext, ".wav")) return DecodeWav(data, size, out_pcm, out_rate, out_ch);
    if (LowerEquals(ext, ".pcm")) {
        /* .pcm 需要外部提供 rate/ch，这里不支持裸调用 */
        LOGE("DecodeMemoryToS16: raw .pcm needs WrapPcmS16");
        return false;
    }
    if (LowerEquals(ext, ".xnb")) return DecodeXnbToS16(data, size, out_pcm, out_rate, out_ch);
    LOGE("DecodeMemoryToS16: unsupported ext %.*s (%s)",
         static_cast<int>(ext.size()), ext.data(), name.c_str());
    return false;
}

bool WrapPcmS16(const void* data, const size_t size, const int rate, const int channels,
                std::vector<uint8_t>& out_pcm, int& out_rate, int& out_ch) {
    if (!data || size == 0 || rate <= 0 || channels <= 0) return false;
    if (size % (sizeof(short) * channels) != 0) return false;
    out_pcm.assign(static_cast<const uint8_t*>(data),
                   static_cast<const uint8_t*>(data) + size);
    out_rate = rate;
    out_ch = channels;
    return true;
}

bool DecodeXnbToS16(const void* data, const size_t size,
                    std::vector<uint8_t>& out_pcm, int& out_rate, int& out_ch) {
    XnbSound snd;
    char err[128] = {};
    if (!XnbParseSound(data, size, snd, err, sizeof(err))) {
        LOGW("DecodeXnbToS16: XnbParseSound failed: %s", err);
        return false;
    }
    if (!XnbNormalizePcm16(snd, out_pcm, out_rate, out_ch, err, sizeof(err))) {
        LOGW("DecodeXnbToS16: XnbNormalizePcm16 failed: %s "
             "(tag=%u ch=%u rate=%u bps=%u align=%u wave=%zu)",
             err,
             static_cast<unsigned>(snd.format_tag),
             static_cast<unsigned>(snd.channel_count),
             static_cast<unsigned>(snd.sample_rate),
             static_cast<unsigned>(snd.bits_per_sample),
             static_cast<unsigned>(snd.block_align),
             snd.waveform.size());
        return false;
    }
    return true;
}