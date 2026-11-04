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
#include <thread>
#include <vector>

#include "libs/json.hpp"

namespace {

/* 并发解码线程数：按 CPU 核数折半，最多 8。
 * 每个线程都会整包解码到内存，16 线程在手机上会造成明显的内存/CPU 峰值。 */
constexpr size_t kMaxThreads = 8;

/**
 * @brief 读取 private_dir/config.json，按 priority 升序填充 entries。
 *
 * config.json 格式：
 * [
 *   { "file": "PackA.zip", "enable": true, "priority": 0, "type": 0 },
 *   { "file": "PackB.zip", "enable": false, "priority": 1, "type": 0 }
 * ]
 *
 * 字段缺省值：enable=true, priority=0, type=0
 * file 相对 audio_packs/ 解析（也接受 "audio_packs/PackA.zip" 这种写法）。
 *
 * @return 配置文件存在且成功解析返回 true；打开失败或 JSON 非法返回 false。
 *         enable=false 的条目会被跳过，missing 的文件只告警不启用。
 */
bool LoadConfig(const std::filesystem::path& cfg_path,
                const std::filesystem::path& packs_dir,
                std::vector<AudioPackEntry>& entries) {
    std::ifstream file(cfg_path);
    if (!file.is_open()) {
        LOGE("config.json open failed: %s", cfg_path.string().c_str());
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        if (!j.is_array()) {
            LOGE("config.json root must be an array");
            return false;
        }

        for (const auto& item : j) {
            if (!item.value("enable", true)) continue;

            AudioPackEntry e;
            const auto rel = item.value("file", "");
            if (rel.empty()) continue;

            /* 标准约定：file 相对 audio_packs/。
             * 兼容两种常见写法：纯文件名，或带 audio_packs/ 前缀。 */
            auto path = packs_dir / rel;
            if (!std::filesystem::is_regular_file(path)) {
                const auto alt = packs_dir / std::filesystem::path(rel).filename();
                if (std::filesystem::is_regular_file(alt)) path = alt;
            }
            if (!std::filesystem::is_regular_file(path)) {
                LOGW("config: pack not found: %s (resolved %s)",
                     rel.c_str(), path.string().c_str());
                continue;
            }

            e.file = path.string();
            e.priority = item.value("priority", 0);
            e.type = static_cast<AudioPackType>(item.value("type", 0));
            entries.push_back(std::move(e));
        }
    } catch (const std::exception& ex) {
        LOGE("config.json parse error: %s", ex.what());
        entries.clear();
        return false;
    }

    /* priority 升序；同 priority 按文件名，保证结果稳定 */
    std::ranges::sort(entries, [](const AudioPackEntry& a, const AudioPackEntry& b) {
        if (a.priority != b.priority) return a.priority < b.priority;
        return a.file < b.file;
    });
    return true;
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
    const std::filesystem::path packs_dir(dir);
    /* 管理器约定：config.json 在 private_dir，zip 在 private_dir/audio_packs/ */
    const auto cfg_path = packs_dir.parent_path() / "config.json";

    /* 只有配置文件不存在时才回退到目录扫描。
     * 配置存在但解析失败/条目无效时保持空：否则会把玩家禁用的包全部重新启用。 */
    if (!std::filesystem::is_regular_file(cfg_path)) {
        LOGW("config.json missing: %s, scanning directory", cfg_path.string().c_str());
        auto entries = ScanDirectory(dir);
        LOGI("directory scan: %zu packs", entries.size());
        return entries;
    }

    std::vector<AudioPackEntry> entries;
    if (!LoadConfig(cfg_path, packs_dir, entries)) {
        LOGE("config.json invalid, no audio pack loaded");
        return {};
    }
    LOGI("config.json loaded: %zu packs", entries.size());
    for (const auto& e : entries) {
        LOGI("  pack priority=%d type=%d file=%s",
             e.priority, static_cast<int>(e.type), e.file.c_str());
    }
    if (entries.empty())
        LOGW("config.json has no enabled pack");
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

    size_t threads = std::thread::hardware_concurrency();
    if (threads == 0) threads = 4;
    threads = std::min(threads / 2 + 1, kMaxThreads);
    ThreadPool pool(threads);
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
        try {
            if (auto p = f.get(); p && p->Size() > 0) built.push_back(std::move(p));
        } catch (const std::exception& ex) {
            /* 单个包解码时抛异常（如 bad_alloc）不应让整个模块初始化失败 */
            LOGE("audio pack decode failed: %s", ex.what());
        } catch (...) {
            LOGE("audio pack decode failed: unknown exception");
        }
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