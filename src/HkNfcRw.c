/*
 * @file	HkNfcRw.c
 * @brief	NFCアクセスのインターフェース実装.
 */
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
#include "HkNfcRw.h"
#include "HkNfcRwIn.h"
#include "HkNfcA.h"
#include "HkNfcB.h"
#include "HkNfcF.h"
#include "NfcPcd.h"
#include "hk_misc.h"

// ログ用
#define LOG_TAG "HkNfcRw"
#include "nfclog.h"

static HkNfcType	m_Type = HKNFCTYPE_NONE;	///< アクティブなNFCタイプ
#ifdef HKNFCRW_USE_LASTERR
static uint8_t		m_LastError;	///< 最後に発生したエラー番号
#endif	//HKNFCRW_USE_LASTERR


/**
 * NFCデバイスオープン
 *
 * @retval	true		成功
 * @retval	false		失敗
 *
 * @attention	- 初回に必ず呼び出すこと。
 * 				- falseの場合、呼び出し側が#HkNfcRw_Close()すること。
 * 				- 終了時には#HkNfcRw_Close()を呼び出すこと。
 *
 * @note		- 呼び出しただけでは搬送波を出力しない。
 */
bool HkNfcRw_Open(void)
{
	bool ret = NfcPcd_PortOpen();
	if(!ret) {
		return false;
	}

	ret = NfcPcd_Init();
	if(ret) {
		HkNfcRw_SetLastError(HKNFCERR_NONE);
	} else {
		HkNfcRw_Close();
	}
	
	return ret;
}


/**
 * NFCデバイスをクローズする。
 *
 * @attention	- 使用が終わったら、必ず呼び出すこと。
 * 				- 呼び出さない場合、搬送波を出力したままとなる。
 * 				- 呼び出し後、再度使用する場合は#open()を呼び出すこと。
 */
void HkNfcRw_Close(void)
{
	if(NfcPcd_IsOpened()) {
		NfcPcd_Reset();
		NfcPcd_RfOff();
		NfcPcd_PortClose();
	}
	HkNfcRw_SetLastError(HKNFCERR_NONE);
}


/**
 * 搬送波停止。
 */
void HkNfcRw_RfOff(void)
{
//	LOGD("\n");

	if(!NfcPcd_RfOff()) {
		NfcPcd_Reset();
	}
}


/**
 *
 */
void HkNfcRw_Reset(void)
{
	LOGD("\n");

	if(!NfcPcd_Reset()) {
		//なんか致命的
		HkNfcRw_Close();
		HkNfcRw_Open();
	}
}


/**
 * ターゲットの探索
 *
 * @param[in]	bNfcA	NfcA(null時は探索しない)
 * @param[in]	bNfcB	NfcB(null時は探索しない)
 * @param[in]	bNfcF	NfcF(null時は探索しない)
 *
 * @retval	true		成功
 * @retval	false		失敗
 *
 * @note		- #HkNfcRw_Open()後に呼び出すこと。
 * 				- アクティブなカードが変更された場合に呼び出すこと。
 */
HkNfcType HkNfcRw_Detect(bool bNfcA, bool bNfcB, bool bNfcF)
{
	m_Type = HKNFCTYPE_NONE;

	if(bNfcF) {
        bool ret = HkNfcF_Polling(0xffff);
		if(ret) {
			LOGD("PollingF\n");
			m_Type = HKNFCTYPE_F;
		}
	}
	if ((m_Type == HKNFCTYPE_NONE) && (bNfcA)) {
        bool ret = HkNfcA_Polling();
		if(ret) {
			LOGD("PollingA\n");
			m_Type = HKNFCTYPE_A;
		}
	}
	if ((m_Type == HKNFCTYPE_NONE) && (bNfcB)) {
        bool ret = HkNfcB_Polling();
		if(ret) {
			LOGD("PollingB\n");
			m_Type = HKNFCTYPE_B;
		}
	}
    else {
    	LOGD("Detect fail\n");
    }

	return m_Type;
}


/**
 * 選択しているカードを内部的に解放する。
 *
 * @attention	RLS_REQ/RLS_RESとは関係ない
 */
void HkNfcRw_Release(void)
{
	NfcPcd_ClrNfcId();
	m_Type = HKNFCTYPE_NONE;
}


/**
 * @brief UIDの取得
 *
 * UIDを取得する
 *
 * @param[out]	pBuf		UID値
 * @return					pBufの長さ
 *
 * @note		- 戻り値が0の場合、UIDは未取得
 */
uint8_t HkNfcRw_GetNfcId(uint8_t* pBuf)
{
	hk_memcpy(pBuf, NfcPcd_NfcId(), NfcPcd_NfcIdLen());
	return NfcPcd_NfcIdLen();
}


/**
 * @brief NFCタイプの取得
 *
 * NFCタイプを取得する
 *
 * @return		HkNfcType 参照
 */
HkNfcType HkNfcRw_GetType(void)
{
	return m_Type;
}


#ifdef HKNFCRW_USE_LASTERR
/**
 * @brief 最終発生エラー取得
 * 
 * 最後に発生したエラー値の取得
 *
 * @return	エラー値
 */
uint8_t HkNfcRw_GetLastError(void)
{
	return m_LastError;
}


/**
 * @brief 最終発生エラー設定
 * 
 * 最後に発生したエラー値の設定
 *
 * @param[in]	err		設定するエラー値
 */
void HkNfcRw_SetLastError(uint8_t err)
{
	m_LastError = err;
}
#endif	//HKNFCRW_USE_LASTERR
