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

#include "AudioManager.hpp"

#include <algorithm>

#include "ThreadPool.hpp"
#include "Log.hpp"

#include <filesystem>
#include <fstream>
#include <future>
#include <vector>

#include "libs/json.hpp"

namespace {

constexpr size_t kThreads = 16;

/**
 * @brief 读取 audio_packs/config.json，按 priority 升序返回条目。
 *
 * config.json 格式：
 * [
 *   { "file": "PackA.zip", "enable": true, "priority": 0, "type": 0 },
 *   { "file": "PackB.zip", "enable": false, "priority": 1, "type": 0 }
 * ]
 *
 * 字段缺省值：enable=true, priority=0, type=0
 * 文件不存在或解析失败时返回空 vector。
 */
std::vector<AudioPackEntry> LoadConfig(const std::filesystem::path& cfg_path) {
    std::vector<AudioPackEntry> entries;

    std::ifstream file(cfg_path);
    if (!file.is_open()) {
        LOGW("config.json not found: %s", cfg_path.string().c_str());
        return entries;
    }

    try {
        nlohmann::json j;
        file >> j;
        if (!j.is_array()) {
            LOGE("config.json root must be an array");
            return entries;
        }

        const auto base_dir = cfg_path.parent_path();
        for (const auto& item : j) {
            if (!item.value("enable", true)) continue;

            AudioPackEntry e;
            const auto rel = item.value("file", "");
            if (rel.empty()) continue;

            e.file = (base_dir / rel).string();
            e.priority = item.value("priority", 0);
            e.type = static_cast<AudioPackType>(item.value("type", 0));

            if (!std::filesystem::is_regular_file(e.file)) {
                LOGW("config: file missing: %s", e.file.c_str());
                continue;
            }
            entries.push_back(std::move(e));
        }
    } catch (const std::exception& ex) {
        LOGE("config.json parse error: %s", ex.what());
        entries.clear();
    }

    /* priority 升序；同 priority 按文件名，保证结果稳定 */
    std::ranges::sort(entries, [](const AudioPackEntry& a, const AudioPackEntry& b) {
        if (a.priority != b.priority) return a.priority < b.priority;
        return a.file < b.file;
    });
    return entries;
}

/**
 * @brief 扫描目录下的 .zip（config 缺失时的兜底）。
 */
std::vector<AudioPackEntry> ScanDirectory(const std::string& dir) {
    std::vector<AudioPackEntry> entries;
    std::error_code ec;
    for (auto& p : std::filesystem::directory_iterator(dir, ec)) {
        if (!p.is_regular_file()) continue;
        if (p.path().extension() != ".zip") continue;
        AudioPackEntry e;
        e.file = p.path().string();
        e.priority = 0;
        e.type = AudioPackType::Terraria;
        entries.push_back(std::move(e));
    }
    if (ec) LOGE("ScanDirectory: %s", ec.message().c_str());

    std::ranges::sort(entries, [](const AudioPackEntry& a, const AudioPackEntry& b) {
        return a.file < b.file;
    });
    return entries;
}

std::vector<AudioPackEntry> ScanPacks(const std::string& dir) {
    const auto cfg_path = std::filesystem::path(dir).parent_path() / "config.json";
    auto entries = LoadConfig(cfg_path);
    if (!entries.empty()) {
        LOGI("config.json loaded: %zu packs", entries.size());
        for (const auto&[file, priority, type] : entries) {
            LOGI("  pack priority=%d type=%d file=%s",
                 priority, static_cast<int>(type), file.c_str());
        }
        return entries;
    }
    LOGW("config.json empty or missing, falling back to directory scan");
    entries = ScanDirectory(dir);
    LOGI("directory scan: %zu packs", entries.size());
    return entries;
}

} // namespace

AudioManager& AudioManager::Instance() {
    static AudioManager instance;
    return instance;
}

void AudioManager::LoadAllBlocking(const std::string& packs_dir) {
    auto entries = ScanPacks(packs_dir);
    if (entries.empty()) {
        LOGW("No audio packs in %s", packs_dir.c_str());
        return;
    }

    ThreadPool pool(kThreads);
    std::vector<std::future<std::unique_ptr<AudioPack>>> futures;
    futures.reserve(entries.size());

    for (auto& e : entries) {
        futures.push_back(pool.Enqueue([entry = std::move(e)]() mutable {
            auto pack = std::make_unique<AudioPack>(std::move(entry));
            if (!pack->BuildIndex()) {
                LOGW("BuildIndex failed: %s", pack->File().c_str());
            }
            return pack;
        }));
    }

    /*
     * 按提交顺序收集结果：futures 是按 entries（已按 priority 排序）的顺序
     * push_back 的，f.get() 按同样顺序返回，所以 built 天然保持 priority 顺序。
     * 不要在这里重新排序，否则 priority 会被文件名顺序覆盖。
     */
    std::vector<std::unique_ptr<AudioPack>> built;
    for (auto& f : futures) {
        if (auto p = f.get(); p && p->Size() > 0) built.push_back(std::move(p));
    }

    std::lock_guard lock(_mtx);
    _packs = std::move(built);

    /*
     * 合并全局索引：按 _packs 顺序（priority 升序）遍历，
     * !contains 才写入，保证高优先级包先占住同 key。
     */
    for (const auto& pack : _packs) {
        for (auto& clip : pack->GetClips()) {
            if (clip->music_id >= 0 && !_music_index.contains(clip->music_id)) {
                _music_index[clip->music_id] = clip.get();
            }
            if (clip->type >= 0) {
                uint64_t key = static_cast<uint64_t>(static_cast<uint32_t>(clip->type)) << 32
                             | static_cast<uint32_t>(clip->style + 1);
                if (!_sfx_index.contains(key)) _sfx_index[key] = clip.get();
            }
        }
    }
    LOGI("Audio global index: music=%zu sfx=%zu",
         _music_index.size(), _sfx_index.size());
}

const AudioClip* AudioManager::FindMusic(const int music_id) const {
    std::lock_guard lock(_mtx);
    const auto it = _music_index.find(music_id);
    return it == _music_index.end() ? nullptr : it->second;
}

const AudioClip* AudioManager::FindSfx(const int type, const int style) const {
    std::lock_guard lock(_mtx);
    if (const auto exact = _sfx_index.find(
            static_cast<uint64_t>(static_cast<uint32_t>(type)) << 32
          | static_cast<uint32_t>(style + 1)); exact != _sfx_index.end()) {
        return exact->second;
    }
    const auto wild = _sfx_index.find(
        static_cast<uint64_t>(static_cast<uint32_t>(type)) << 32);
    return wild == _sfx_index.end() ? nullptr : wild->second;
}

void AudioManager::Clear() {
    std::lock_guard lock(_mtx);
    for (const auto& p : _packs) p->Clear();
    _packs.clear();
    _music_index.clear();
    _sfx_index.clear();
}