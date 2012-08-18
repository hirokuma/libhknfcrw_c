/**
 * @file	HkNfcRw.h
 * @brief	NFCのR/W(Sony RC-S956系)アクセスクラス
 */
#ifndef HKNFCRW_H
#define HKNFCRW_H

#include <stdint.h>
#include <stdbool.h>


/**
 * @typedef	HkNfcType
 * @brief	Polling時のNFCカードタイプ
 */
typedef uint8_t HkNfcType;
#define HKNFCTYPE_NONE	((HkNfcType)0)		///< 未設定
#define HKNFCTYPE_A		((HkNfcType)1)		///< NFC-A
#define HKNFCTYPE_B		((HkNfcType)2)		///< NFC-B
#define HKNFCTYPE_F		((HkNfcType)3)		///< NFC-F



/// 選択解除
void HkNfcRw_Release(void);
#if 0	//ここはNDEF対応してからじゃないと意味がなさそうだな
/// データ読み込み
virtual bool HkNfcRw_Read(uint8_t* buf, uint8_t blockNo=0x00) { return false; }
/// データ書き込み
virtual bool HkNfcRw_Write(const uint8_t* buf, uint8_t blockNo=0x00) { return false; }
#endif

/// オープン
bool HkNfcRw_Open(void);
/// クローズ
void HkNfcRw_Close(void);

/// 搬送波停止
void HkNfcRw_RfOff(void);

/// ターゲットの探索
HkNfcType HkNfcRw_Detect(bool bNfcA, bool bNfcB, bool bNfcF);

/// NFCID取得
uint8_t HkNfcRw_GetNfcId(uint8_t* pBuf);

/// @brief NFCタイプの取得
HkNfcType HkNfcRw_GetType(void);


#endif // HKNFCRW_H
