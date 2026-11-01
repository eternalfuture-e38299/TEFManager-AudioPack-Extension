/*******************************************************************************
 * audiopack_extension - SfxNameResolver
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

#include "SfxNameResolver.hpp"
#include "SfxNameTable.hpp"

#include <string_view>
#include <unordered_map>

namespace {
    constexpr std::string_view kSoundsPrefix = "Content/Sounds/";

    std::string_view Basename(const std::string_view path) {
        const auto pos = path.find_last_of('/');
        return pos == std::string_view::npos ? path : path.substr(pos + 1);
    }

    std::string_view StripExt(const std::string_view name) {
        for (std::string_view ext: {".xnb", ".ogg", ".mp3", ".wav", ".pcm"}) {
            if (name.size() > ext.size() &&
                name.substr(name.size() - ext.size()) == ext) {
                return name.substr(0, name.size() - ext.size());
            }
        }
        return name;
    }

    int ParsePureDigits(const std::string_view &s) {
        if (s.empty()) return -1;
        int v = 0;
        for (const char c: s) {
            if (c < '0' || c > '9') return -1;
            v = v * 10 + (c - '0');
        }
        return v;
    }

    bool ParseDigitsPair(std::string_view s, int &type, int &style) {
        const auto us = s.find('_');
        if (us == std::string_view::npos) {
            const int t = ParsePureDigits(s);
            if (t < 0) return false;
            type = t;
            style = -1;
            return true;
        }
        const int t = ParsePureDigits(s.substr(0, us));
        const int st = ParsePureDigits(s.substr(us + 1));
        if (t < 0 || st < 0) return false;
        type = t;
        style = st;
        return true;
    }

    const std::unordered_map<std::string_view, int> &NameToId() {
        static const auto kMap = [] {
            std::unordered_map<std::string_view, int> m;
            m.reserve(kSfxNameTableSize * 2);
            for (const auto &[name, id]: kSfxNameTable) m.emplace(name, id);
            return m;
        }();
        return kMap;
    }

    struct VariantPrefix {
        std::string_view prefix;
        int base_type;
    };

    constexpr VariantPrefix kVariants[] = {
        {.prefix = "NPC_Hit_", .base_type = 3},
        {.prefix = "NPC_Death_", .base_type = 4},
        {.prefix = "NPCDeath", .base_type = 4},
        {.prefix = "NPCHit", .base_type = 3},
        {.prefix = "Item", .base_type = 2},
    };

    bool TryVariant(const std::string_view &base, int &type, int &style) {
        for (const auto &[prefix, base_type]: kVariants) {
            if (base.size() <= prefix.size()) continue;
            if (base.substr(0, prefix.size()) != prefix) continue;

            auto rest = base.substr(prefix.size());
            /* 跳过可选的下划线：Item_11 / Item11 都接受 */
            if (!rest.empty() && rest.front() == '_') rest.remove_prefix(1);

            /* 变体名后面可能还有 _0 / _1 这种多段后缀，先只取第一段数字 */
            const auto us = rest.find('_');
            auto digits = (us == std::string_view::npos) ? rest : rest.substr(0, us);

            const int n = ParsePureDigits(digits);
            if (n < 0) continue;
            type = base_type;
            style = n;
            return true;
        }
        return false;
    }
} // namespace

bool ResolveSfxName(const std::string& entry_name, int& type, int& style) {
    std::string_view path(entry_name);
    if (const auto sp = path.find(kSoundsPrefix); sp != std::string_view::npos)
        path.remove_prefix(sp + kSoundsPrefix.size());
    else if (path.find('/') != std::string_view::npos)
        path = Basename(path);

    const std::string_view base = StripExt(path);
    if (base.empty()) return false;

    /* 1) 精确名字表 */
    if (const auto it = NameToId().find(base); it != NameToId().end()) {
        type = it->second; style = -1; return true;
    }

    /* 2) 变体：Item_11 / NPC_Hit_26 / chester_close_0 */
    if (TryVariant(base, type, style)) return true;

    /* 3) 去下划线回退：Player_Killed -> PlayerKilled */
    if (base.find('_') != std::string_view::npos) {
        std::string compact;
        compact.reserve(base.size());
        for (const char c : base) if (c != '_') compact.push_back(c);
        if (const auto it = NameToId().find(compact); it != NameToId().end()) {
            type = it->second; style = -1; return true;
        }
    }

    /* 4) sfx_<type>_<style> */
    if (constexpr std::string_view sfxPrefix = "sfx_";
        base.size() > sfxPrefix.size() && base.substr(0, sfxPrefix.size()) == sfxPrefix) {
        if (ParseDigitsPair(base.substr(sfxPrefix.size()), type, style)) return true;
        }

    /* 5) 纯数字 */
    if (const int t = ParsePureDigits(base); t >= 0) {
        type = t; style = -1; return true;
    }

    /* 6) <type>_<style> */
    if (ParseDigitsPair(base, type, style)) return true;
    return false;
}