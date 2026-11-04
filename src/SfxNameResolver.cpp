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
#include "SfxNameFamilies.h"

#include <string>
#include <string_view>

namespace {
    std::string_view Basename(const std::string_view path) {
        const auto pos = path.find_last_of('/');
        return pos == std::string_view::npos ? path : path.substr(pos + 1);
    }

    /* 去掉任意扩展名（不限定大小写/白名单） */
    std::string_view StripExt(std::string_view name) {
        const auto dot = name.find_last_of('.');
        return dot == std::string_view::npos ? name : name.substr(0, dot);
    }

    int ParsePureDigits(const std::string_view s) {
        if (s.empty()) return -1;
        int v = 0;
        for (const char c: s) {
            if (c < '0' || c > '9') return -1;
            v = v * 10 + (c - '0');
        }
        return v;
    }

    /* 与 SoundID const 名不一致的常见写法（归一化后） */
    struct NameAlias { const char *name; int type; };
    constexpr NameAlias kAliases[] = {
        {"dooropened", 8},
        {"doorclose", 9},
        {"dooropen", 8},
    };

    bool LookupAlias(const std::string &norm, int &type) {
        for (const auto &a: kAliases) {
            if (norm == a.name) { type = a.type; return true; }
        }
        return false;
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
} // namespace

bool ResolveSfxName(const std::string& entry_name, int& type, int& style) {
    std::string_view path(entry_name);
    /* 允许 Content/Sounds/ 或任意 .../Sounds/ 前缀；其它情况退回文件名 */
    if (const auto sp = path.find("Sounds/"); sp != std::string_view::npos)
        path.remove_prefix(sp + 7);
    path = Basename(path);

    const std::string_view base = StripExt(path);
    if (base.empty()) return false;

    /* 1) 归一化名字表（小写+去符号）：const / 变体字段 / 家族前缀+数字。
     *    覆盖 Player_Killed、Door_Closed、NPC_Hit_26、Drip_0、Female_Hit_0 等。 */
    const std::string base_str(base);
    if (sfx_name_lookup(base_str.c_str(), &type, &style)) return true;

    /* 1b) 归一化后的别名（例如 Door_Opened -> const DoorOpen） */
    {
        std::string norm;
        norm.reserve(base.size());
        for (const char c: base) {
            if (c >= 'A' && c <= 'Z') norm.push_back(static_cast<char>(c + 32));
            else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) norm.push_back(c);
        }
        if (LookupAlias(norm, type)) { style = -1; return true; }
    }

    /* 2) sfx_<type>_<style>（该写法不是 SoundID 名字，走数字解析） */
    if (base.size() > 4 && base.substr(0, 4) == "sfx_")
        if (ParseDigitsPair(base.substr(4), type, style)) return true;

    /* 3) 纯数字 / <type>_<style> */
    if (const int t = ParsePureDigits(base); t >= 0) {
        type = t;
        style = -1;
        return true;
    }
    return ParseDigitsPair(base, type, style);
}
