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

#include "XnbSound.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cmath>

namespace {

constexpr size_t kMaxXnbBytes = 32 * 1024 * 1024;

    bool RdU16(const uint8_t* d, const size_t n, const size_t off, uint16_t& v) {
    if (off + 2 > n) return false;
    v = static_cast<uint16_t>(d[off] | (d[off + 1] << 8));
    return true;
}

 bool RdU32(const uint8_t* d, const size_t n, const size_t off, uint32_t& v) {
    if (off + 4 > n) return false;
    v = static_cast<uint32_t>(d[off])
      | (static_cast<uint32_t>(d[off + 1]) << 8)
      | (static_cast<uint32_t>(d[off + 2]) << 16)
      | (static_cast<uint32_t>(d[off + 3]) << 24);
    return true;
}

 bool RdU32C(const uint8_t* d, const size_t n, size_t& c, uint32_t& v) {
    if (!RdU32(d, n, c, v)) return false;
    c += 4;
    return true;
}

 bool RdI32C(const uint8_t* d, const size_t n, size_t& c, int32_t& v) {
    uint32_t u;
    if (!RdU32C(d, n, c, u)) return false;
    v = static_cast<int32_t>(u);
    return true;
}

 bool Rd7Bit(const uint8_t* d, const size_t n, size_t& c, uint32_t& v) {
    uint32_t r = 0;
    for (int sh = 0; sh < 35; sh += 7) {
        if (c >= n) return false;
        const uint8_t b = d[c++];
        r |= static_cast<uint32_t>(b & 0x7F) << sh;
        if ((b & 0x80) == 0) {
            v = r;
            return true;
        }
    }
    return false;
}

 bool SkipN(const size_t n, size_t& c, const size_t cnt) {
    if (c + cnt > n) return false;
    c += cnt;
    return true;
}

 void SetErr(char* err, const int cap, const char* msg) {
    if (err && cap > 0) {
        std::snprintf(err, static_cast<size_t>(cap), "%s", msg);
    }
}

} // namespace

bool XnbParseSound(const void* data, const size_t size, XnbSound& out,
                   char* err, const int errcap) {
    out = XnbSound{};
    if (err && errcap > 0) err[0] = 0;

    if (!data || size < 10 || size > kMaxXnbBytes) {
        SetErr(err, errcap, "size invalid");
        return false;
    }

    const auto* d = static_cast<const uint8_t*>(data);

    if (d[0] != 0x58 || d[1] != 0x4E || d[2] != 0x42) {
        SetErr(err, errcap, "not xnb");
        return false;
    }
    if (d[5] & 0x80) {
        /* d[5] 高位：0x80=LZX, 0x40=LZ4（Terraria 常见 LZX） */
        char msg[64];
        std::snprintf(msg, sizeof(msg), "compressed xnb (flag=0x%02X)", d[5]);
        SetErr(err, errcap, msg);
        return false;
    }

    size_t c = 6;
    if (uint32_t declared = 0; !RdU32C(d, size, c, declared) || declared != size) {
        SetErr(err, errcap, "declared size mismatch");
        return false;
    }

    uint32_t rc = 0;
    if (!Rd7Bit(d, size, c, rc) || rc == 0 || rc > 64) {
        SetErr(err, errcap, "reader table bad");
        return false;
    }

    bool saw = false;
    for (uint32_t i = 0; i < rc; ++i) {
        uint32_t ns = 0;
        if (!Rd7Bit(d, size, c, ns) || ns > 4096 || !SkipN(size, c, ns)) {
            SetErr(err, errcap, "reader name bad");
            return false;
        }
        if (const size_t noff = c - ns; ns >= 16 && noff + ns <= size) {
            const auto rn = reinterpret_cast<const char*>(d + noff);
            for (size_t k = 0; k + 16 <= ns; ++k) {
                if (std::memcmp(rn + k, "SoundEffectReader", 16) == 0) {
                    saw = true;
                    break;
                }
            }
        }
        if (uint32_t rv = 0; !RdU32C(d, size, c, rv)) {
            SetErr(err, errcap, "reader ver missing");
            return false;
        }
    }
    if (!saw) {
        SetErr(err, errcap, "no SoundEffectReader");
        return false;
    }

    uint32_t prim = 0;
    if (uint32_t shared = 0;
        !Rd7Bit(d, size, c, shared) || !Rd7Bit(d, size, c, prim) || prim == 0) {
        SetErr(err, errcap, "primary header bad");
        return false;
    }

    uint32_t fs = 0;
    if (!RdU32C(d, size, c, fs) || fs < 16 || fs > 256 || !SkipN(size, c, fs)) {
        SetErr(err, errcap, "waveformatex bad");
        return false;
    }
    const size_t fb = c - fs;
    out.format.assign(d + fb, d + fb + fs);

    if (!RdU16(out.format.data(), out.format.size(), 0, out.format_tag) ||
        !RdU16(out.format.data(), out.format.size(), 2, out.channel_count) ||
        !RdU32(out.format.data(), out.format.size(), 4, out.sample_rate) ||
        !RdU32(out.format.data(), out.format.size(), 8, out.avg_bytes_per_sec) ||
        !RdU16(out.format.data(), out.format.size(), 12, out.block_align) ||
        !RdU16(out.format.data(), out.format.size(), 14, out.bits_per_sample) ||
        out.channel_count == 0 || out.sample_rate == 0 || out.block_align == 0) {
        SetErr(err, errcap, "fmt fields invalid");
        return false;
    }

    uint32_t ws = 0;
    if (!RdU32C(d, size, c, ws) || ws == 0 || ws > kMaxXnbBytes || !SkipN(size, c, ws)) {
        SetErr(err, errcap, "waveform bad");
        return false;
    }
    const size_t wb = c - ws;
    out.waveform.assign(d + wb, d + wb + ws);

    if (out.format_tag == 1 && (ws % out.block_align) != 0) {
        SetErr(err, errcap, "not block aligned");
        return false;
    }

    if (!RdI32C(d, size, c, out.loop_start) ||
        !RdI32C(d, size, c, out.loop_length) ||
        !RdI32C(d, size, c, out.duration_ms)) {
        SetErr(err, errcap, "loop meta missing");
        return false;
    }

    return true;
}

namespace {

int NearestRate(const int r) {
    static constexpr int kTab[9] = {
        8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
    };
    int best = kTab[0], bd = std::abs(r - kTab[0]);
    for (int i = 1; i < 9; ++i) {
        if (const int dd = std::abs(r - kTab[i]); dd < bd) { best = kTab[i]; bd = dd; }
    }
    return best;
}

int16_t DecSample(const XnbSound& s, const size_t frame, const int ch, const int bps) {
    const uint8_t* b = s.waveform.data() + frame * s.block_align + ch * bps;
    switch (s.bits_per_sample) {
        case 8:
            return static_cast<int16_t>((static_cast<int>(b[0]) - 128) * 256);
        case 16:
            return static_cast<int16_t>(b[0] | (b[1] << 8));
        case 24: {
            int32_t v = static_cast<int32_t>(b[0])
                      | (static_cast<int32_t>(b[1]) << 8)
                      | (static_cast<int32_t>(b[2]) << 16);
            if (v & 0x00800000) v |= static_cast<int32_t>(0xFF000000);
            return static_cast<int16_t>(v >> 8);
        }
        case 32: {
            const auto v = static_cast<int32_t>(
                static_cast<uint32_t>(b[0])
              | (static_cast<uint32_t>(b[1]) << 8)
              | (static_cast<uint32_t>(b[2]) << 16)
              | (static_cast<uint32_t>(b[3]) << 24));
            return static_cast<int16_t>(v >> 16);
        }
        default:
            return 0;
    }
}

} // namespace

bool XnbNormalizePcm16(const XnbSound& s, std::vector<uint8_t>& out_pcm,
                       int& out_rate, int& out_ch, char* err, const int errcap) {
    out_pcm.clear();
    out_rate = 0;
    out_ch = 0;
    if (err && errcap > 0) err[0] = 0;

    if (s.format_tag != 1) {
        SetErr(err, errcap, "not WAVE_FORMAT_PCM");
        return false;
    }
    if (s.channel_count != 1 && s.channel_count != 2) {
        SetErr(err, errcap, "not mono/stereo");
        return false;
    }
    if (s.bits_per_sample != 8 && s.bits_per_sample != 16 &&
        s.bits_per_sample != 24 && s.bits_per_sample != 32) {
        SetErr(err, errcap, "bits unsupported");
        return false;
    }

    const int bps = s.bits_per_sample / 8;
    if (const size_t exp_align = static_cast<size_t>(s.channel_count) * bps; s.block_align != exp_align || s.waveform.size() % s.block_align != 0) {
        SetErr(err, errcap, "block align mismatch");
        return false;
    }

    const size_t in_frames = s.waveform.size() / s.block_align;
    if (in_frames == 0) {
        SetErr(err, errcap, "no frames");
        return false;
    }
    if (s.sample_rate > 192000) {
        SetErr(err, errcap, "rate too high");
        return false;
    }

    int r = static_cast<int>(s.sample_rate);
    r = std::clamp(r, 8000, 48000);
    const int orate = NearestRate(r);
    const double ratio = static_cast<double>(orate) / static_cast<double>(s.sample_rate);
    auto oframes = static_cast<size_t>(
        std::lround(static_cast<double>(in_frames) * ratio));
    if (oframes < 1) oframes = 1;

    const size_t obytes = oframes * s.channel_count * 2;
    if (obytes > 64 * 1024 * 1024) {
        SetErr(err, errcap, "normalized too large");
        return false;
    }

    out_pcm.resize(obytes);
    for (size_t of = 0; of < oframes; ++of) {
        const double sp = static_cast<double>(of) * s.sample_rate / orate;
        auto fa = static_cast<size_t>(sp);
        if (fa > in_frames - 1) fa = in_frames - 1;
        size_t fb = fa + 1;
        if (fb > in_frames - 1) fb = in_frames - 1;
        const double frac = sp - static_cast<double>(fa);

        for (int ch = 0; ch < s.channel_count; ++ch) {
            const double a = DecSample(s, fa, ch, bps);
            const double b = DecSample(s, fb, ch, bps);
            const double m = a + (b - a) * frac;
            auto v = static_cast<int32_t>(m < 0 ? m - 0.5 : m + 0.5);
            v = std::clamp<int32_t>(v, -32768, 32767);
            const size_t off = (of * s.channel_count + ch) * 2;
            out_pcm[off] = static_cast<uint8_t>(v & 0xFF);
            out_pcm[off + 1] = static_cast<uint8_t>((static_cast<uint16_t>(v) >> 8) & 0xFF);
        }
    }

    out_rate = orate;
    out_ch = static_cast<int>(s.channel_count);
    return true;
}