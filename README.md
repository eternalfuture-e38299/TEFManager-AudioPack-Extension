# TEFManager-AudioPack-Extension

> TEFManager 音频包扩展模块
> 基于 TEFKernel 框架的 Terraria 安卓平台音频包加载系统

---

## 📖 项目介绍

TEFManager 音频包扩展模块，为 Terraria 提供自定义音乐与音效替换能力，兼容创意工坊（Steam Workshop）格式的音频包，实现音频的并行解码、全局索引与优先级覆盖。

本模块是基于 [TEFManager-MusicPack-Extension](https://github.com/lzup333/TEFManager-MusicPack-Extension) 的**重写版本**，并特别感谢 [qingshemg](https://github.com/qingqing114514/qingshemg) 的早期探索与启发。

### ✨ 核心特性

- **🎵 音乐替换** — 按 `Music_{id}` 替换游戏内任意背景音乐
- **🔔 音效替换** — 按音效 `type`/`style` 替换物品、NPC、环境等音效
- **📦 多格式解码** — ogg / mp3 / wav / xnb（音效额外支持 xnb）
- **⚡ 并行加载** — 多线程解码 zip 内音频条目，启动更快
- **🧠 全局索引** — 解码完成后建立无锁只读索引，播放时零查找开销
- **🎯 优先级覆盖** — 高优先级包覆盖低优先级同名条目
- **🔄 回退机制** — 缺失的音频自动使用原版资源

### 支持格式

| 类型         | 说明       | 目录结构                     |
|--------------|------------|------------------------------|
| `Terraria`   | 标准格式   | `Content/Music/`、`Content/Sounds/` |
| `TEFManager` | 预留格式   | 暂未实现                     |

---

## 📦 音频包制作指南

### 目录结构

#### 标准格式 (Terraria)

```text
音频包名称.zip
└── Content/
    ├── Music/
    │   ├── Music_1.ogg
    │   ├── Music_2.ogg
    │   └── Music_98.ogg
    └── Sounds/
        ├── Item_11.xnb
        ├── Player_Killed.xnb
        └── NPC_Hit_1.xnb
```

---

## 🎨 音频规范

### 音乐（Music）

- **命名**：`Content/Music/Music_{id}.{ext}`，`{id}` 为数字
- **格式**：`.ogg` / `.mp3` / `.wav`
- **循环**：音乐自动循环播放
- **示例**：`Content/Music/Music_98.ogg` 替换 Skeletron Prime 战斗曲

### 音效（Sounds）

- **命名**：`Content/Sounds/{名称}.{ext}`，名称按下表规则解析
- **格式**：`.ogg` / `.mp3` / `.wav` / `.xnb`
- **xnb**：支持 Terraria 原版 `SoundEffectReader` 格式（未压缩）

#### 音效命名解析规则（按顺序匹配）

| 规则 | 示例 | 解析结果 |
|------|------|----------|
| 精确名字表 | `Player_Killed` | `type=5, style=-1` |
| 变体前缀 | `Item_11` | `type=2, style=11` |
| 变体前缀 | `NPC_Hit_26` | `type=3, style=26` |
| 变体前缀 | `NPC_Death_1` | `type=4, style=1` |
| 去下划线回退 | `Player_Killed` → `PlayerKilled` | 查名字表 |
| 前缀 `sfx_` | `sfx_2_11` | `type=2, style=11` |
| 纯数字 | `50` | `type=50, style=-1` |
| 数字对 | `2_11` | `type=2, style=11` |

- `style = -1` 表示**通配**：匹配该 `type` 下所有 style
- 精确 style 优先于通配

### 常用音效路径

| 路径 | 说明 |
|------|------|
| `Content/Sounds/Item_{id}.xnb` | 物品使用/击中音效 |
| `Content/Sounds/NPC_Hit_{id}.xnb` | NPC 受击音效 |
| `Content/Sounds/NPC_Death_{id}.xnb` | NPC 死亡音效 |
| `Content/Sounds/Player_Killed.xnb` | 玩家死亡音效 |

### 优先级规则

1. **覆盖顺序**：高优先级音频包覆盖低优先级的同 key 条目
2. **索引构建**：按 `priority` 升序（数值小优先）遍历，先占住同 key
3. **回退机制**：未命中的音乐/音效自动使用原版资源

---

## ⚙️ 配置文件

音频包目录下的 `config.json` 用于声明包列表与优先级：

```json
[
  { "file": "PackA.zip", "enable": true, "priority": 0, "type": 0 },
  { "file": "PackB.zip", "enable": false, "priority": 1, "type": 0 }
]
```

| 字段 | 缺省值 | 说明 |
|------|--------|------|
| `file` | — | zip 文件名（相对 `config.json` 所在目录） |
| `enable` | `true` | 是否启用 |
| `priority` | `0` | 优先级，数值越小越优先 |
| `type` | `0` | 包类型，`0` = Terraria 标准格式 |

- `config.json` 缺失或为空时，自动扫描目录下所有 `.zip`（按文件名排序，priority 均为 0）。

---

## ⚠️ 注意事项

1. **音频尺寸**：音频文件过大（尤其是未压缩 wav）会显著增加内存占用和首次播放延迟
2. **xnb 压缩**：仅支持**未压缩**的 `SoundEffectReader` xnb；LZX/LZ4 压缩的 xnb 需先解压
3. **播放延迟**：首次切换音乐/音效时可能因引擎分配声道产生轻微延迟，后续播放会命中缓存
4. **优先级**：多个音频包覆盖同一音乐/音效时，按 `priority` 数值小的优先
5. **通配 style**：`style = -1` 的音效会匹配该 type 下所有未精确命中的 style
6. **内存管理**：大包（数百个音效）会占用较多内存，请按需启用

---

## ❓ 常见问题

**Q: 音乐/音效没有生效？**
A: 检查命名是否符合 `Music_{id}` / 音效命名规则，查看日志中的 `entry: ... is_sfx=... type=... style=...` 输出。

**Q: xnb 音效解码失败？**
A: 日志若显示 `compressed xnb`，说明该 xnb 被 LZX/LZ4 压缩，暂不支持；若显示 `no SoundEffectReader`，说明该 xnb 不是音效类型。

**Q: 多个音频包如何覆盖？**
A: 通过 `config.json` 的 `priority` 控制，数值越小优先级越高。

**Q: 播放有延迟怎么办？**
A: 优先使用 ogg/mp3 而非 wav；减少单包音效数量；避免超大音频文件。

**Q: 音频包加载失败怎么办？**
A: 检查 ZIP 完整性、目录结构是否符合 `Content/Music/`、`Content/Sounds/`，以及 `config.json` 格式是否正确。

---

## 🤝 贡献指南

1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

---

## 📄 许可证

本项目采用 GNU Affero General Public License v3.0 许可证。

---

## 🔗 相关链接

- [TEFKernel](https://github.com/eternalfuture-e38299/TEFKernel) — 核心框架
- [TEFManager-MusicPack-Extension](https://github.com/lzup333/TEFManager-MusicPack-Extension) — 本模块重写所基于的项目
- [qingshemg](https://github.com/qingqing114514/qingshemg) — 早期探索与启发
- [问题反馈](https://github.com/eternalfuture-e38299/TEFManager-AudioPack-Extension/issues)
