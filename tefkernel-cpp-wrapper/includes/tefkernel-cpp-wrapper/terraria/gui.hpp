/*******************************************************************************
 * File: gui
 * Project: tefkernel_cpp_wrapper
 * Created: 2026/10/29
 * Author: eternalfuture-e38299
 * Github: https://github.com/eternalfuture-e38299
 *
 * MIT License
 *
 * Copyright (c) 2026 eternalfuture-e38299
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/

#pragma once

#include "tefkernel/terraria/gui.h"
#include "../patchlib/type.hpp"

namespace TEFKernel::Terraria::GUI {
    /**
     * @brief 帧信息（对应 tef_gui_frame_t，POD 透传）
     */
    using Frame = tef_gui_frame_t;

    /**
     * @brief 绘制回调类型
     */
    using DrawCallback = tef_gui_draw_cb;

    // ==================== 回调注册 ====================

    /**
     * @brief 注册每帧绘制回调（按注册顺序绘制）
     */
    void AddDraw(DrawCallback cb, void *user);

    /**
     * @brief 移除绘制回调
     */
    void RemoveDraw(DrawCallback cb);

    // ==================== 绘制（仅在绘制回调内有效） ====================

    /**
     * @brief 填充矩形
     * @param rgba 0xAARRGGBB
     */
    void Rect(float x, float y, float w, float h, unsigned rgba);

    /**
     * @brief 画文字
     */
    void Text(const char *text, float x, float y, float scale, unsigned rgba);

    /**
     * @brief 画贴图（Texture2D 句柄）
     */
    void Image(patch_handle_t texture, float x, float y, float w, float h, unsigned rgba);

    /**
     * @brief 用指定字体画文字（SpriteFont 句柄），无描边
     */
    void DrawString(patch_handle_t font, const char *text, float x, float y, float scale,
                    unsigned rgba);

    /**
     * @brief 同 Image，但颜色按游戏 Color 的内存原值传
     */
    void ImageRaw(patch_handle_t texture, float x, float y, float w, float h, unsigned rawColor);

    // ==================== 外部 SpriteBatch 借用 ====================

    /**
     * @brief 借用游戏已开启的 SpriteBatch
     */
    void BeginExternal(void *spriteBatch);

    /**
     * @brief 结束外部 SpriteBatch 借用
     */
    void EndExternal();

}