/*******************************************************************************
 * audiopack_extension - AudioPack
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

#include "AudioPack.hpp"
#include "AudioDecode.hpp"
#include "SfxNameResolver.hpp"
#include "Log.hpp"
#include "libs/miniz.h"

#include <cstring>
#include <string_view>

namespace {

constexpr auto MUSIC_PREFIX = "Content/Music/Music_";
constexpr auto SOUNDS_PREFIX = "Content/Sounds/";

int ParseMusicId(const std::string& name) {
    auto pos = name.find(MUSIC_PREFIX);
    if (pos == std::string::npos) return -1;
    pos += std::strlen(MUSIC_PREFIX);
    int v = 0; bool any = false;
    while (pos < name.size() && name[pos] >= '0' && name[pos] <= '9') {
        v = v * 10 + (name[pos] - '0'); ++pos; any = true;
    }
    if (!any) return -1;
    if (const std::string_view rest(name.data() + pos, name.size() - pos); rest != ".ogg" && rest != ".mp3" && rest != ".wav") return -1;
    return v;
}

} // namespace

AudioPack::AudioPack(AudioPackEntry entry) : _entry(std::move(entry)) {}

bool AudioPack::BuildIndex() {
    switch (_entry.type) {
        case AudioPackType::Terraria: return BuildZipIndex();
        case AudioPackType::TEFManager:
            LOGW("TEFManager audio format not implemented");
            return false;
        default: return false;
    }
}

const AudioClip* AudioPack::FindMusic(const int music_id) const {
    const auto it = _music_lookup.find(music_id);
    return it == _music_lookup.end() ? nullptr : _clips[it->second].get();
}

const AudioClip* AudioPack::FindSfx(const int type, const int style) const {
    if (const auto exact = _sfx_lookup.find(static_cast<uint64_t>(static_cast<uint32_t>(type)) << 32 | static_cast<uint32_t>(style + 1)); exact != _sfx_lookup.end()) return _clips[exact->second].get();
    const auto wild = _sfx_lookup.find(static_cast<uint64_t>(static_cast<uint32_t>(type)) << 32);
    return wild == _sfx_lookup.end() ? nullptr : _clips[wild->second].get();
}

void AudioPack::Clear() {
    for (const auto& c : _clips) terraria_audio_manager_unregister(&c->desc);
    _clips.clear();
    _music_lookup.clear();
    _sfx_lookup.clear();
}

bool AudioPack::BuildZipIndex() {
    mz_zip_archive zip{};
    if (!mz_zip_reader_init_file(&zip, _entry.file.c_str(), 0)) {
        LOGE("Failed to open audio zip: %s", _entry.file.c_str());
        return false;
    }

    const auto n = mz_zip_reader_get_num_files(&zip);
    for (int i = 0; i < n; ++i) {
        if (mz_zip_reader_is_file_a_directory(&zip, i)) continue;

        mz_zip_archive_file_stat stat;
        if (!mz_zip_reader_file_stat(&zip, i, &stat)) continue;
        std::string name(stat.m_filename);

        const int mid = ParseMusicId(name);
        int type = -1, style = -1;
        bool is_sfx = false;

        if (mid < 0 && name.starts_with(SOUNDS_PREFIX)) {
            is_sfx = ResolveSfxName(name, type, style);
        }

        /* 解析后再打印，避免打印初始值误导排查 */
        /*LOGI("entry: %s mid=%d is_sfx=%d type=%d style=%d",
             name.c_str(), mid, static_cast<int>(is_sfx), type, style);
             */

        if (mid < 0 && !is_sfx) continue;

        if (mid >= 0 && _music_lookup.contains(mid)) continue;
        if (is_sfx) {
            const uint64_t key =
                static_cast<uint64_t>(static_cast<uint32_t>(type)) << 32
              | static_cast<uint32_t>(style + 1);
            if (_sfx_lookup.contains(key)) continue;
        }

        size_t sz = 0;
        void* p = mz_zip_reader_extract_to_heap(&zip, i, &sz, 0);
        if (!p) { LOGE("Extract failed: %s", name.c_str()); continue; }

        auto clip = std::make_unique<AudioClip>();
        if (!DecodeMemoryToS16(p, sz, name, clip->pcm, clip->sample_rate, clip->channels)) {
            mz_free(p);
            LOGE("Decode failed: %s", name.c_str());
            continue;
        }
        mz_free(p);

        clip->music_id = mid;
        clip->type = type;
        clip->style = style;
        clip->desc.source_name = nullptr;
        clip->desc.sample_rate = clip->sample_rate;
        clip->desc.channels    = clip->channels;
        clip->desc.format      = 2;
        clip->desc.pcm_data    = clip->pcm.data();
        clip->desc.pcm_frames  = clip->pcm.size() / (clip->channels * 2);
        clip->desc.looping     = (mid >= 0);
        clip->desc.volume      = 1.0f;

        if (!terraria_audio_manager_register(&clip->desc)) {
            LOGE("Register failed: %s", name.c_str());
            continue;
        }

        const size_t idx = _clips.size();
        if (mid >= 0) {
            _music_lookup[mid] = idx;
        } else {
            const uint64_t key =
                static_cast<uint64_t>(static_cast<uint32_t>(type)) << 32
              | static_cast<uint32_t>(style + 1);
            _sfx_lookup[key] = idx;
        }
        _clips.push_back(std::move(clip));
    }

    mz_zip_reader_end(&zip);
    LOGI("AudioPack %s: music=%zu sfx=%zu", _entry.file.c_str(),
         _music_lookup.size(), _sfx_lookup.size());
    return true;
}