/*******************************************************************************
 * File: audio_manager
 * Project: tefkernel
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

#ifndef TEFKERNEL_AUDIO_MANAGER_H
#define TEFKERNEL_AUDIO_MANAGER_H

#include "../../tef_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct terraria_audio_t {
    const char *source_name;         /* 仅用于调试/归属标识，可为 NULL */
    int sample_rate;                 /* 44100 */
    int channels;                    /* 1 或 2 */
    int format;                      /* ma_format：u8=1, s16=2, s24=3, s32=4, f32=5 */
    const void *pcm_data;            /* 解码好的 PCM 数据 */
    unsigned long long pcm_frames;   /* 总帧数 = 字节数 / (channels * bytes_per_sample) */
    bool looping;                    /* true = 循环 */
    float volume;                    /* 0.0 ~ 1.0，默认 1.0 */
} terraria_audio_t;

/**
 * @brief 登记音源（内核只保存指针，不分配 ID）
 * @return 成功返回 true
 */
DEFINE_FUNCTION(bool, terraria_audio_manager_register, terraria_audio_t *audio)

/**
 * @brief 注销音源（会先停止其所有 voice）
 */
DEFINE_FUNCTION(bool, terraria_audio_manager_unregister, terraria_audio_t *audio)

/**
 * @brief 播放，返回 voice_id（失败 -1）
 * @param volume 0.0 ~ 1.0，传 -1 用 audio->volume
 * @param pitch  1.0 = 原速
 */
DEFINE_FUNCTION(int, terraria_audio_manager_play, terraria_audio_t *audio,
                float volume, float pitch)

DEFINE_FUNCTION(void, terraria_audio_manager_pause, int voice_id)
DEFINE_FUNCTION(void, terraria_audio_manager_resume, int voice_id)
DEFINE_FUNCTION(void, terraria_audio_manager_stop, int voice_id)
DEFINE_FUNCTION(void, terraria_audio_manager_set_volume, int voice_id, float volume)
DEFINE_FUNCTION(bool, terraria_audio_manager_is_playing, int voice_id)
DEFINE_FUNCTION(void, terraria_audio_manager_stop_all, terraria_audio_t *audio)
DEFINE_FUNCTION(void, terraria_audio_manager_set_master_volume, float volume)

#if IS_TEFKERNEL_BUILD
/** @brief 内核在进程退出 */
void terraria_audio_manager_shutdown();
#endif

#ifdef __cplusplus
}
#endif

#endif //TEFKERNEL_AUDIO_MANAGER_H