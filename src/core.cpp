/*******************************************************************************
 * audiopack_extension - core
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

#include "tefkernel/module/module_core.h"
#include "AudioManager.hpp"
#include "Log.hpp"
#include "MusicNameTable.hpp"
#include "tefkernel-cpp-wrapper/patchlib/type.hpp"
#include "tefkernel-cpp-wrapper/patchlib/method.hpp"
#include "tefkernel-cpp-wrapper/patchlib/field.hpp"

#include <chrono>
#include <filesystem>

static constexpr module_info_t g_module_info = {
    .pkg_id = "eternal.future.audiopackextension",
    .name = "AudioPackExtension",
    .author = "eternalfuture-e38299",
    .version = "2.0.0",
    .version_code = 200,
    .api_version = 1,
    .plugin_dependencies_sizes = 0,
    .plugin_dependencies = nullptr,
};

static patch_handle_t g_f_soundVol = nullptr, g_f_musicVol = nullptr;
static int g_volfields_ok = 0;
static int g_music_voice = -1;
static volatile float g_music_vol = 1.0f;
static volatile float g_sound_vol = 1.0f;

static const MusicNameEntry* MusicNameOf(const int id) {
    static const auto kById = [] {
        std::unordered_map<int, const MusicNameEntry*> m;
        for (const auto& e : kMusicNameTable) m[e.id] = &e;
        return m;
    }();
    const auto it = kById.find(id);
    return it == kById.end() ? nullptr : it->second;
}

static void RefreshVolumes() {
    if (!g_volfields_ok) return;
    float v = 0.0f;
    patchlib_field_get_value(g_f_soundVol, nullptr, &v);
    g_sound_vol = v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v;
    patchlib_field_get_value(g_f_musicVol, nullptr, &v);
    g_music_vol = v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v;
}

static int g_current_music_id = -1;   // 当前我接管的 music id，-1 表示没接管
static bool MusicPrefix(patch_handle_t instance, void** args,
                        const patch_method_signature_t* sig, void* result) {
    (void)instance; (void)sig; (void)result;
    if (!args || !args[1]) return false;
    int idx = *static_cast<int *>(args[1]);
    if (idx == 50) idx = 51;

    RefreshVolumes();

    const AudioClip* clip = AudioManager::Instance().FindMusic(idx);
    const bool can_takeover = (clip != nullptr);

    /* 情况 3：这首我不管 → 释放接管，放行原版 */
    if (!can_takeover) {
        if (g_music_voice >= 0) {
            terraria_audio_manager_stop(g_music_voice);
            g_music_voice = -1;
        }
        g_current_music_id = -1;
        /* 不动 args[2]，原版音量保持 */
        return false;
    }

    /* 情况 1/2：我要接管这一首 */

    /* id 变了：停自己的旧 voice，准备换曲 */
    if (idx != g_current_music_id) {
        if (g_music_voice >= 0) {
            terraria_audio_manager_stop(g_music_voice);
            g_music_voice = -1;
        }
        g_current_music_id = idx;
    }

    /* 接管期间压掉原版这一路（只在这一首被接管时压） */
    if (args[2]) *static_cast<float *>(args[2]) = 0.0f;

    if (g_music_vol <= 0.0f) return false;
    if (g_music_voice >= 0) return false;   // 同一首已在播，别重启

    g_music_voice = terraria_audio_manager_play(
        const_cast<terraria_audio_t *>(&clip->desc), g_music_vol, 1.0f);
    if (const auto* entry = MusicNameOf(idx); entry) {
        LOGI("HIT music=%d (%s) voice=%d", idx, entry->name, g_music_voice);
    }
    return false;
}

static bool SfxPrefix(patch_handle_t instance, void** args,
                      const patch_method_signature_t* sig, void* result) {
    (void)instance; (void)sig; (void)result;
    if (!args || !args[0]) return false;
    const int type = *static_cast<int *>(args[0]);
    const int style = args[3] ? *static_cast<int *>(args[3]) : 1;
    float vol = 1.0f;
    if (args[4]) {
        if (const float v = *static_cast<float *>(args[4]); v >= 0.0f && v <= 1.0f) vol = v;
    }

    RefreshVolumes();
    if (g_volfields_ok && g_sound_vol <= 0.0f) return false;
    if (g_volfields_ok) vol *= g_sound_vol;

    const AudioClip* clip = AudioManager::Instance().FindSfx(type, style);
    if (!clip) return false;
    return terraria_audio_manager_play(const_cast<terraria_audio_t *>(&clip->desc), vol, 1.0f) >= 0;
}

static bool init_module(module_entry_t* entry) {
    const auto t0 = std::chrono::steady_clock::now();

    const std::filesystem::path packs_dir =
        std::filesystem::path(entry->private_dir) / "audio_packs";
    std::error_code ec;
    std::filesystem::create_directories(packs_dir, ec);

    AudioManager::Instance().LoadAllBlocking(packs_dir.string());

    const TEFKernel::PatchLib::Type Main("Terraria", "Main");
    g_f_soundVol = Main.GetField("soundVolume").GetHandle();
    g_f_musicVol = Main.GetField("musicVolume").GetHandle();
    g_volfields_ok = patchlib_is_valid(g_f_soundVol) && patchlib_is_valid(g_f_musicVol);
    LOGI("vol fields: sound=%d music=%d",
         static_cast<int>(patchlib_is_valid(g_f_soundVol)),
         static_cast<int>(patchlib_is_valid(g_f_musicVol)));

    const TEFKernel::PatchLib::Type LAS("Terraria.Audio", "LegacyAudioSystem");
    if (const auto m = LAS.GetMethod("UpdateCommonTrack"); m) {
        patchlib_install_prepost_hook(m.GetHandle(), MusicPrefix, nullptr);
    }

    const TEFKernel::PatchLib::Type LSP("Terraria.Audio", "LegacySoundPlayer");
    if (const auto m = LSP.GetMethod("PlaySound", 6); m) {
        patchlib_install_prepost_hook(m.GetHandle(), SfxPrefix, nullptr);
    }

    const auto t1 = std::chrono::steady_clock::now();
    LOGI("init_module done in %ld ms",
         std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
    return true;
}

static bool cleanup_module(module_entry_t* entry) {
    (void)entry;
    /*if (g_music_voice >= 0) {
        terraria_audio_manager_stop(g_music_voice);
        g_music_voice = -1;
    }
    AudioManager::Instance().Clear();
    */
    return true;
}

static void hot_reload(module_entry_t* entry) { (void)entry; }
static const module_info_t* get_info() { return &g_module_info; }

static constexpr module_ops_t g_module_ops = {
    .init_module = init_module,
    .cleanup_module = cleanup_module,
    .hot_reload = hot_reload,
    .get_info = get_info,
};

API_EXPORT const module_ops_t* API_CALL module_create() {
    return &g_module_ops;
}