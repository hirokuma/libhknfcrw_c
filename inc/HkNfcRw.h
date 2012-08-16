/**
 * @file	HkNfcRw.h
 * @brief	NFCのR/W(Sony RC-S956系)アクセスクラス
 */
#ifndef HKNFCRW_H
#define HKNFCRW_H

#include <stdint.h>
#include <stdbool.h>


/**
 * @enum	HkNfcRwType
 * @brief	Polling時のNFCカードタイプ
 */
typedef enum HkNfcRwType {
	NFC_NONE,		///< 未設定
	NFC_A,			///< NFC-A
	NFC_B,			///< NFC-B
    NFC_F			///< NFC-F
} HkNfcRwType;



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

/// ターゲットの探索
HkNfcRwType HkNfcRw_Detect(bool bNfcA, bool bNfcB, bool bNfcF);

/// NFCID取得
uint8_t HkNfcRw_GetNfcId(uint8_t* pBuf);

/// @brief NFCタイプの取得
HkNfcRwType HkNfcRw_GetType(void);


#endif // HKNFCRW_H
