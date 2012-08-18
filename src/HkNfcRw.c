/*
 * @file	HkNfcRw.c
 * @brief	NFCアクセスのインターフェース実装.
 */
#include "HkNfcRw.h"
#include "HkNfcA.h"
#include "HkNfcB.h"
#include "HkNfcF.h"
#include "NfcPcd.h"
#include "hk_misc.h"

// ログ用
#define LOG_TAG "HkNfcRw"
#include "nfclog.h"

static HkNfcType	m_Type = HKNFCTYPE_NONE;	///< アクティブなNFCタイプ



/**
 * NFCデバイスオープン
 *
 * @retval	true		成功
 * @retval	false		失敗
 *
 * @attention	- 初回に必ず呼び出すこと。
 * 				- falseの場合、呼び出し側が#close()すること。
 * 				- 終了時には#close()を呼び出すこと。
 *
 * @note		- 呼び出しただけでは搬送波を出力しない。
 */
bool HkNfcRw_Open(void)
{
	LOGD("%s\n", __PRETTY_FUNCTION__);

	bool ret = NfcPcd_PortOpen();
	if(!ret) {
		return false;
	}

	ret = NfcPcd_Init();
	if(!ret) {
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
			return m_Type;
		}
	}

	if(bNfcA) {
        bool ret = HkNfcA_Polling();
		if(ret) {
			LOGD("PollingA\n");
			m_Type = HKNFCTYPE_A;
			return m_Type;
		}
	}

	if(bNfcB) {
        bool ret = HkNfcB_Polling();
		if(ret) {
			LOGD("PollingB\n");
			m_Type = HKNFCTYPE_B;
			return m_Type;
		}
	}
	LOGD("Detect fail\n");

	return HKNFCTYPE_NONE;
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

