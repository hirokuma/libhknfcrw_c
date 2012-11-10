/**
 * @file	HkNfcRw.h
 * @brief	NFCのR/W(Sony RC-S956系)アクセスクラス
 */
#ifndef HKNFCRW_H
#define HKNFCRW_H
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
#include "HkNfcId.h"
#include "HkNfcErr.h"

/**
 * @typedef	HkNfcType
 * @brief	Polling時のNFCカードタイプ
 */
typedef uint8_t HkNfcType;
#define HKNFCTYPE_NONE	((HkNfcType)0)		///< 未設定
#define HKNFCTYPE_A		((HkNfcType)1)		///< NFC-A
#define HKNFCTYPE_B		((HkNfcType)2)		///< NFC-B
#define HKNFCTYPE_F		((HkNfcType)3)		///< NFC-F


/// オープン
bool HkNfcRw_Open(void);
/// クローズ
void HkNfcRw_Close(void);

/// 搬送波停止
void HkNfcRw_RfOff(void);

/// リセット
void HkNfcRw_Reset(void);

/// ターゲットの探索
HkNfcType HkNfcRw_Detect(bool bNfcA, bool bNfcB, bool bNfcF);
/// 選択解除
void HkNfcRw_Release(void);

/// NFCID取得
uint8_t HkNfcRw_GetNfcId(uint8_t* pBuf);

/// NFCタイプの取得
HkNfcType HkNfcRw_GetType(void);

#ifdef HKNFCRW_USE_LASTERR
/// 最後に発生したエラー
uint8_t HkNfcRw_GetLastError(void);
#endif	//HKNFCRW_USE_LASTERR

#if 0	//ここはNDEF対応してからじゃないと意味がなさそうだな
/// データ読み込み
virtual bool HkNfcRw_Read(uint8_t* buf, uint8_t blockNo=0x00) { return false; }
/// データ書き込み
virtual bool HkNfcRw_Write(const uint8_t* buf, uint8_t blockNo=0x00) { return false; }
#endif

#endif // HKNFCRW_H
