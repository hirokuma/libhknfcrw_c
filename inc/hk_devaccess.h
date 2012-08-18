/**
 * @file	hk_devaccess.h
 * @brief	NFC R/Wへのアクセスインターフェース.
 * 			インターフェースのみ定義し、実装は各自で行う。
 */
#ifndef HK_DEVACCESS_H
#define HK_DEVACCESS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * ポートオープン
 *
 * @retval	true	オープン成功
 */
bool hk_nfcrw_open(void);

/**
 * ポートクローズ
 */
void hk_nfcrw_close(void);

/**
 * ポート送信
 *
 * @param[in]	data		送信データ
 * @param[in]	len			dataの長さ
 * @return					送信したサイズ
 */
uint16_t hk_nfcrw_write(const uint8_t* data, uint16_t len);

/**
 * ポート受信
 * 
 * 事前に#hk_nfcrw_read_timeout()で0以外のタイムアウト時間が設定されていた場合、
 * タイムアウト時間に達した場合は受信サイズが0になる。
 *
 * @param[out]	data		受信バッファ
 * @param[in]	len			受信サイズ
 *
 * @return					受信したサイズ
 *
 * @attention	- len分のデータを受信するか失敗するまで処理がブロックされる。
 */
uint16_t hk_nfcrw_read(uint8_t* data, uint16_t len);


/**
 * ポート受信タイムアウト時間設定
 * 
 * タイムアウト処理が可能な場合、受信タイムアウト時間を設定する。
 * タイムアウトがない場合は、何も処理しないし、#hk_nfcrw_read()にも影響はない。
 *
 * @param[in]	msec		タイムアウト時間(ミリ秒)。0のときはタイムアウト解除。
 */
void hk_nfcrw_read_timeout(uint16_t msec);

#endif // HK_DEVACCESS_H
