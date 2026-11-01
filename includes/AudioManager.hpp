/*******************************************************************************
 * audiopack_extension - AudioManager
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

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "AudioPack.hpp"

class AudioManager final {
public:
    static AudioManager& Instance();

    void LoadAllBlocking(const std::string& packs_dir);

    [[nodiscard]] const AudioClip* FindMusic(int music_id) const;
    [[nodiscard]] const AudioClip* FindSfx(int type, int style) const;

    void Clear();

private:
    AudioManager() = default;

    std::vector<std::unique_ptr<AudioPack>> _packs;
    std::unordered_map<int, const AudioClip*> _music_index;
    std::unordered_map<uint64_t, const AudioClip*> _sfx_index;
    mutable std::mutex _mtx;
};