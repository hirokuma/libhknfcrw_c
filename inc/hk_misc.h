/**
 * @file	hk_misc.h
 * @brief	環境依存インターフェース.
 * 			インターフェース定義のみで、実装は各自で行う。
 */
#ifndef HK_MISC_H
#define HK_MISC_H
/*
 * Copyright (c) 2012-2012, hiro99ma
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *         this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *         this list of conditions and the following disclaimer
 *         in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stdbool.h>

#define h16(u16)		((uint8_t)(u16 >> 8))
#define l16(u16)		((uint8_t)u16)
#define ul16(h8,l8)		((uint16_t)((h8<<8) | l8))

/**
 *  ミリ秒スリープ
 *
 * @param	msec	待ち時間[msec]
 */
void hk_msleep(uint16_t msec);

/**
 * タイマ開始
 *
 * @param[in]	tmval	タイムアウト時間[msec]
 */
void hk_start_timer(uint16_t tmval);

/**
 *  タイムアウト監視
 * 
 * @retval	true	タイムアウト発生
 */
bool hk_is_timeout(void);

/**
 * メモリ比較
 *
 * @param[in]	s1		比較元1
 * @param[in]	s2		比較元2
 * @param[in]	n		比較サイズ
 * @retval		0			一致
 * @retval		上記以外	不一致
 */
int   hk_memcmp(const void *s1, const void *s2, uint16_t n);

/**
 * メモリコピー
 *
 * @param[out]	dst		コピー先
 * @param[in]	src		コピー元
 * @param[in]	len		コピーサイズ
 * @return				コピー先アドレス(dst)
 */
void* hk_memcpy(void* dst, const void* src, uint16_t len);

/**
 * メモリ書き込み
 *
 * @param[out]	dst		書き込み先
 * @param[in]	dat		書き込みデータ
 * @param[in]	len		書き込みサイズ
 * @return				書き込み先アドレス(dst)
 */
void* hk_memset(void* dst, uint8_t dat, uint16_t len);

#endif // HK_MISC_H
