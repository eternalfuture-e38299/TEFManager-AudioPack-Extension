#!/usr/bin/env python3
"""
从 SoundID_zh.csv 和 MusicID_zh.txt 生成两个头文件：

    SfxNameTable.hpp   - SfxNameEntry{name, id}
    MusicNameTable.hpp - MusicNameEntry{id, name, zh}

用法：
    python3 gen_sfx_music_tables.py \
        --sfx-csv SoundID_zh.csv \
        --music-txt MusicID_zh.txt \
        --out-dir .
"""

import argparse
import csv
import sys
from pathlib import Path

SFX_GUARD = "TEFKERNEL_SFX_NAME_TABLE_HPP"
MUSIC_GUARD = "TEFKERNEL_MUSIC_NAME_TABLE_HPP"


def escape_cpp_string(s: str) -> str:
    out = []
    for ch in s:
        if ch == '\\':
            out.append('\\\\')
        elif ch == '"':
            out.append('\\"')
        elif ch == '\n':
            out.append('\\n')
        elif ch == '\t':
            out.append('\\t')
        elif ch == '\r':
            out.append('\\r')
        else:
            out.append(ch)
    return ''.join(out)


# ---------------- SoundID_zh.csv ----------------

def load_sfx(csv_path: Path):
    rows = []
    with csv_path.open(newline='', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        if reader.fieldnames is None:
            raise SystemExit(f"{csv_path}: 空文件或没有表头")

        field_map = {name.strip().lower(): name for name in reader.fieldnames}
        id_col = field_map.get('id')
        name_col = field_map.get('name')
        if id_col is None or name_col is None:
            raise SystemExit(
                f"{csv_path}: 需要 'ID' 和 'Name' 两列，实际表头: {reader.fieldnames}"
            )

        for line_no, row in enumerate(reader, start=2):
            raw_id = (row.get(id_col) or '').strip()
            raw_name = (row.get(name_col) or '').strip()
            if not raw_id or not raw_name:
                continue
            try:
                sid = int(raw_id)
            except ValueError:
                print(f"跳过 {csv_path} 第 {line_no} 行: ID 不是整数: {raw_id!r}",
                      file=sys.stderr)
                continue
            rows.append((raw_name, sid))

    rows.sort(key=lambda x: x[1])
    return rows


def write_sfx_header(rows, out_path: Path):
    lines = []
    lines.append("/*******************************************************************************")
    lines.append(" * SfxNameTable.hpp - 自动生成，请勿手工修改")
    lines.append(" *")
    lines.append(" * 由 gen_sfx_music_tables.py 从 SoundID_zh.csv 生成。")
    lines.append(" * 每项为 {Name, SoundID}，SoundID 即 LegacySoundPlayer.PlaySound 的 type。")
    lines.append(" *******************************************************************************/")
    lines.append("")
    lines.append(f"#ifndef {SFX_GUARD}")
    lines.append(f"#define {SFX_GUARD}")
    lines.append("")
    lines.append("#include <cstddef>")
    lines.append("")
    lines.append("struct SfxNameEntry {")
    lines.append("    const char* name;")
    lines.append("    int         id;")
    lines.append("};")
    lines.append("")
    lines.append("constexpr SfxNameEntry kSfxNameTable[] = {")
    for name, sid in rows:
        lines.append(f'    {{"{escape_cpp_string(name)}", {sid}}},')
    lines.append("};")
    lines.append("")
    lines.append("constexpr std::size_t kSfxNameTableSize =")
    lines.append("    sizeof(kSfxNameTable) / sizeof(kSfxNameTable[0]);")
    lines.append("")
    lines.append(f"#endif // {SFX_GUARD}")
    lines.append("")
    out_path.write_text("\n".join(lines), encoding='utf-8')
    print(f"生成 {out_path}: {len(rows)} 条音效记录")


# ---------------- MusicID_zh.txt ----------------

def load_music(txt_path: Path):
    """
    行格式：ID,Name,ZH
    允许 Name/ZH 含逗号（简单按前两个逗号切分）。
    """
    rows = []
    for line_no, raw in enumerate(txt_path.read_text(encoding='utf-8-sig').splitlines(),
                                  start=1):
        line = raw.strip()
        if not line or line.startswith('#'):
            continue
        parts = line.split(',', 2)
        if len(parts) < 3:
            print(f"跳过 {txt_path} 第 {line_no} 行: 需要 3 列: {raw!r}",
                  file=sys.stderr)
            continue
        raw_id, name, zh = (p.strip() for p in parts)
        if not raw_id:
            continue
        try:
            mid = int(raw_id)
        except ValueError:
            print(f"跳过 {txt_path} 第 {line_no} 行: ID 不是整数: {raw_id!r}",
                  file=sys.stderr)
            continue
        rows.append((mid, name, zh))

    rows.sort(key=lambda x: x[0])
    return rows


def write_music_header(rows, out_path: Path):
    lines = []
    lines.append("/*******************************************************************************")
    lines.append(" * MusicNameTable.hpp - 自动生成，请勿手工修改")
    lines.append(" *")
    lines.append(" * 由 gen_sfx_music_tables.py 从 MusicID_zh.txt 生成。")
    lines.append(" * 每项为 {MusicID, Name, ZH}，MusicID 对应 Content/Music/Music_<ID>。")
    lines.append(" *******************************************************************************/")
    lines.append("")
    lines.append(f"#ifndef {MUSIC_GUARD}")
    lines.append(f"#define {MUSIC_GUARD}")
    lines.append("")
    lines.append("#include <cstddef>")
    lines.append("")
    lines.append("struct MusicNameEntry {")
    lines.append("    int         id;")
    lines.append("    const char* name;")
    lines.append("    const char* zh;")
    lines.append("};")
    lines.append("")
    lines.append("constexpr MusicNameEntry kMusicNameTable[] = {")
    for mid, name, zh in rows:
        lines.append(
            f'    {{{mid}, "{escape_cpp_string(name)}", "{escape_cpp_string(zh)}"}},'
        )
    lines.append("};")
    lines.append("")
    lines.append("constexpr std::size_t kMusicNameTableSize =")
    lines.append("    sizeof(kMusicNameTable) / sizeof(kMusicNameTable[0]);")
    lines.append("")
    lines.append(f"#endif // {MUSIC_GUARD}")
    lines.append("")
    out_path.write_text("\n".join(lines), encoding='utf-8')
    print(f"生成 {out_path}: {len(rows)} 条音乐记录")


# ---------------- 入口 ----------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--sfx-csv", type=Path, help="SoundID_zh.csv 路径")
    ap.add_argument("--music-txt", type=Path, help="MusicID_zh.txt 路径")
    ap.add_argument("--out-dir", type=Path, default=Path("."),
                    help="输出目录（默认当前目录）")
    args = ap.parse_args()

    if not args.sfx_csv and not args.music_txt:
        ap.error("至少指定 --sfx-csv 或 --music-txt 之一")

    args.out_dir.mkdir(parents=True, exist_ok=True)

    if args.sfx_csv:
        if not args.sfx_csv.is_file():
            print(f"找不到 {args.sfx_csv}", file=sys.stderr)
            return 1
        rows = load_sfx(args.sfx_csv)
        write_sfx_header(rows, args.out_dir / "SfxNameTable.hpp")

    if args.music_txt:
        if not args.music_txt.is_file():
            print(f"找不到 {args.music_txt}", file=sys.stderr)
            return 1
        rows = load_music(args.music_txt)
        write_music_header(rows, args.out_dir / "MusicNameTable.hpp")

    return 0


if __name__ == "__main__":
    sys.exit(main())