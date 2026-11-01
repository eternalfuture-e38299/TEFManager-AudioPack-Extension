/*******************************************************************************
 * musicpack_extension - AudioPack
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
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "tefkernel/terraria/content/audio_manager.h"

/**
 * @brief 一个已解码的 PCM 音源。
 *
 * 由 unique_ptr 持有，地址稳定，desc.pcm_data 不会因容器扩容悬空。
 */
struct AudioClip {
    std::vector<uint8_t> pcm;      // s16 交错
    int sample_rate = 0;
    int channels = 0;
    int music_id = -1;             // >=0 表示音乐
    int type = -1;                 // >=0 表示音效
    int style = -1;                // -1 = 通配
    terraria_audio_t desc{};
};

enum class AudioPackType { Terraria = 0, TEFManager };

struct AudioPackEntry {
    std::string file;
    int priority = 0;
    AudioPackType type = AudioPackType::Terraria;
};

class AudioPack {
public:
    explicit AudioPack(AudioPackEntry entry);

    bool BuildIndex();

    [[nodiscard]] const AudioClip* FindMusic(int music_id) const;
    [[nodiscard]] const AudioClip* FindSfx(int type, int style) const;

    [[nodiscard]] const std::vector<std::unique_ptr<AudioClip>>& GetClips() const { return _clips; }
    [[nodiscard]] size_t Size() const { return _clips.size(); }
    [[nodiscard]] const std::string& File() const { return _entry.file; }

    void Clear();

private:
    AudioPackEntry _entry;
    std::vector<std::unique_ptr<AudioClip>> _clips;
    std::unordered_map<int, size_t> _music_lookup;
    std::unordered_map<uint64_t, size_t> _sfx_lookup;

    bool BuildZipIndex();
};