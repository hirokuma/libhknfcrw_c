/**
 * @file	hk_misc.h
 * @brief	環境依存インターフェース.
 * 			インターフェース定義のみで、実装は各自で行う。
 */
#ifndef HK_MISC_H
#define HK_MISC_H

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
