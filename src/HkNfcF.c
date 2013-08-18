/*
 * @file	HkNfcF.c
 * @brief	NFC-Fアクセス実装
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
#include "HkNfcF.h"
#include "NfcPcd.h"
#include "hk_misc.h"

#define LOG_TAG "HkNfcF"
#include "nfclog.h"

#define kSC_BROADCAST		((uint16_t)0xffff)		///<
#define kDEFAULT_TIMEOUT	((uint16_t)(1000 * 2))
#define kPUSH_TIMEOUT		((uint16_t)(2100 * 2))


static uint16_t		m_SystemCode = kSC_BROADCAST;		///< システムコード
#if 0
static uint16_t		m_SvcCode = HKNFCF_SVCCODE_RW;		///< サービスコード
#endif

#ifdef QHKNFCRW_USE_FELICA
static uint16_t		m_syscode[16];
static uint8_t		m_syscode_num;
#endif	//QHKNFCRW_USE_FELICA


typedef enum AccessModeType {
	AM_NORMAL = 0x00,
} AccessModeType;

typedef enum SCOrderType {
	SCO_NORMAL = 0x00,
} SCOrderType;

static inline uint16_t _create_blocklist2(uint16_t BlockNo, AccessModeType am, SCOrderType sco)
{
	return (uint16_t)(0x8000U | (am << 12) | (sco << 8) | BlockNo);
}


/**
 * pollingしたカードの解放
 */
void HkNfcF_Release(void)
{
	m_SystemCode = kSC_BROADCAST;
}


/**
 * @brief	ポーリング
 * 
 * NFC-Fのポーリングを行う.
 *
 * @param[in]		systemCode		システムコード
 * @retval			true			取得成功
 * @retval			false			取得失敗
 *
 * @retval		true			成功
 * @retval		false			失敗
 *
 * @attention	- 取得失敗は、主にカードが認識できない場合である。
 */
bool HkNfcF_Polling(uint16_t systemCode)
{
	//InListPassiveTarget
	const uint8_t INLISTPASSIVETARGET[] = {
		0x02,				// 0x01:212Kbps  0x02:424Kbps
		0x00,
		0xff, 0xff,			// SystemCode
		0x01,				// opt:0x01...get SystemCode
		0x0f				// Time Slot
	};
	const uint8_t INLISTPASSIVETARGET_RES[] = {
		0x01,				// System Number
		0x14,				// length(opt:+2)
		0x01				// response code
	};

	bool ret;
	uint8_t responseLen = 0;
	uint8_t* pData;
	uint8_t* pCmd = NfcPcd_CommandBuf();

	// 424Kbps
	hk_memcpy(pCmd, INLISTPASSIVETARGET, sizeof(INLISTPASSIVETARGET));
	pCmd[2] = h16(systemCode);
	pCmd[3] = l16(systemCode);

	ret = NfcPcd_InListPassiveTarget(
				pCmd, sizeof(INLISTPASSIVETARGET),
				&pData, &responseLen);
	if (!ret
	  || (hk_memcmp(&pData[3], INLISTPASSIVETARGET_RES, sizeof(INLISTPASSIVETARGET_RES)) != 0)) {
//		LOGE("pollingF fail(424Kbps): ret=%d/len=%d\n", ret, responseLen);

		//212Kbps
		pCmd[0] = 0x01;
		ret = NfcPcd_InListPassiveTarget(
				pCmd, sizeof(INLISTPASSIVETARGET),
				&pData, &responseLen);
		if (!ret
		  || (hk_memcmp(&pData[3], INLISTPASSIVETARGET_RES, sizeof(INLISTPASSIVETARGET_RES)) != 0)) {
//			LOGE("pollingF fail(212Kbps): ret=%d/len=%d\n", ret, responseLen);
			m_SystemCode = kSC_BROADCAST;
			return false;
		}
	}
	//[0] d5
	//[1] 4b
	//[2] NbTg
	//[3] Tg

	NfcPcd_SetNfcId(pData + 6, NFCID2_LEN);
	m_SystemCode = (uint16_t)(*(pData + 22) << 8 | *(pData + 23));

	return true;
}


/**
 * @brief	読み込み
 * 
 * NFC-Fの指定したブロックを読み込む(1ブロック=16byte).
 *
 * @param[out]	pBuf		読み込みデータ(16byte=#HKNFCF_SZ_BLOCK)
 * @param[in]	BlockNo		読み込みブロック番号
 * @return		true		成功
 * @return		false		失敗
 */
bool HkNfcF_Read(uint8_t* pBuf, uint8_t BlockNo)
{
	uint8_t len;
	uint8_t* pCmd = NfcPcd_CommandBuf();
	uint8_t* pRes = NfcPcd_ResponseBuf();

	pCmd[0] = 16;
	pCmd[1] = 0x06;		// Read w/o Enc.
	hk_memcpy(pCmd + 2, NfcPcd_NfcId(), NFCID2_LEN);
	pCmd[10] = 0x01;				//サービス数
#if 1
	pCmd[11] = l16(HKNFCF_SVCCODE_RO);
	pCmd[12] = h16(HKNFCF_SVCCODE_RO);
#else
	pCmd[11] = l16(m_SvcCode);
	pCmd[12] = h16(m_SvcCode);
#endif
	pCmd[13] = 0x01;			//ブロック数
	uint16_t blist = _create_blocklist2(BlockNo, AM_NORMAL, SCO_NORMAL);
	pCmd[14] = h16(blist);
	pCmd[15] = l16(blist);
	bool ret = NfcPcd_CommunicateThruEx(
					kDEFAULT_TIMEOUT,
					pCmd, pCmd[0],
					pRes, &len);
	if (!ret || (len != pRes[0]) || (pRes[0] != 13 + HKNFCF_SZ_BLOCK) || (pRes[1] != 0x07)
	  || (hk_memcmp(pRes + 2, NfcPcd_NfcId(), NFCID2_LEN) != 0)
	  || (pRes[10] != 0x00)
	  || (pRes[11] != 0x00)) {
		LOGE("read : ret=%d / %d / %d / %02x / %02x\n", ret, len, pRes[0], pRes[10], pRes[11]);
		return false;
	}
	hk_memcpy(pBuf, &pRes[13], HKNFCF_SZ_BLOCK);

	return true;
}


/**
 * @brief	書き込み
 * 
 * NFC-Fの指定したブロックに書き込む(1ブロック=16byte).
 *
 * @param[in]	pBuf		書き込みデータ(16byte=#HKNFCF_SZ_BLOCK)
 * @param[in]	BlockNo		書き込みブロック番号
 * @return		true		成功
 * @return		false		失敗
 */
bool HkNfcF_Write(const uint8_t* pBuf, uint8_t BlockNo)
{
	uint8_t len;
	uint8_t* pCmd = NfcPcd_CommandBuf();
	uint8_t* pRes = NfcPcd_ResponseBuf();
	
	pCmd[0] = (uint8_t)(14 + 2 + HKNFCF_SZ_BLOCK);		//len
	pCmd[1] = 0x08;		// Read w/o Enc.
	hk_memcpy(pCmd + 2, NfcPcd_NfcId(), NFCID2_LEN);
	pCmd[10] = 0x01;				//サービス数
#if 1
	pCmd[11] = l16(HKNFCF_SVCCODE_RW);
	pCmd[12] = h16(HKNFCF_SVCCODE_RW);
#else
	pCmd[11] = l16(m_SvcCode);
	pCmd[12] = h16(m_SvcCode);
#endif
	pCmd[13] = 0x01;			//ブロック数
	uint16_t blist = _create_blocklist2(BlockNo, AM_NORMAL, SCO_NORMAL);
	pCmd[14] = h16(blist);
	pCmd[15] = l16(blist);
	hk_memcpy(pCmd + 16, pBuf, HKNFCF_SZ_BLOCK);
	bool ret = NfcPcd_CommunicateThruEx(
					kDEFAULT_TIMEOUT,
					pCmd, pCmd[0],
					pRes, &len);
	if (!ret || (len != pRes[0]) || (pRes[0] != 12) || (pRes[1] != 0x09)
	  || (hk_memcmp(pRes + 2, NfcPcd_NfcId(), NFCID2_LEN) != 0)
	  || (pRes[10] != 0x00)
	  || (pRes[11] != 0x00)) {
		LOGE("read : ret=%d / %d / %d / %02x / %02x\n", ret, len, pRes[0], pRes[10], pRes[11]);
		return false;
	}

	return true;
}

#if 0
/**
 * @brief	サービスコード指定
 * 
 * アクセスするサービスコードの指定
 *
 * @param[in]	svccode		指定するサービスコード
 */
void HkNfcF_SetServiceCode(uint16_t svccode)
{
	m_SvcCode = svccode;
}
#endif

#ifdef HKNFCF_USE_FELICA
/**
 *
 */
bool HkNfcF_ReqSystemCode(uint8_t* pNums)
{
	// Request System Codeのテスト
	uint8_t len;
	uint8_t* pCmd = NfcPcd_CommandBuf();
	uint8_t* pRes = NfcPcd_ResponseBuf();

	pCmd[0] = 10;
	pCmd[1] = 0x0c;
	hk_memcpy(pCmd + 2, NfcPcd_NfcId(), NFCID2_LEN);
	bool ret = NfcPcd_CommunicateThruEx(
						kDEFAULT_TIMEOUT,
						pCmd, 10,
						pRes, &len);
	if (!ret || (pRes[0] != 0x0d)
	  || (memcmp(pRes + 1, NfcPcd_NfcId(), NFCID2_LEN) != 0)) {
		LOGE("req_sys_code : ret=%d\n", ret);
		return false;
	}

	*pNums = *(pRes + 9);

	m_syscode_num = *pNums;
	for(int i=0; i<m_syscode_num; i++) {
		m_syscode[i] = (uint16_t)(*(pRes + 10 + i * 2) << 8 | *(pRes + 10 + i * 2 + 1));
		LOGD("sys[%d] : %04x\n", i, m_syscode[i]);
	}

	return true;
}


/**
 *
 */
bool HkNfcF_SearchServiceCode(void)
{
	// Search Service Code
	uint8_t len;
	uint16_t loop = 0x0000;
	uint8_t* pCmd = NfcPcd_CommandBuf();
	uint8_t* pRes = NfcPcd_ResponseBuf();

	pCmd[0] = 12;
	pCmd[1] = 0x0a;
	hk_memcpy(pCmd + 2, NfcPcd_NfcId(), NFCID2_LEN);

	do {
		pCmd[10] = l16(loop);
		pCmd[11] = h16(loop);
		bool ret = NfcPcd_CommunicateThruEx(
							kDEFAULT_TIMEOUT,
							pCmd, 12,
							NfcPcd_ResponseBuf, &len);
		if (!ret || (pRes[0] != 0x0b)
		  || (memcmp(NfcPcd_ResponseBuf + 1, NfcPcd_NfcId(), NFCID2_LEN) != 0)) {
			LOGE("searchServiceCode : ret=%d\n", ret);
			return false;
		}

		len -= 9;
		const uint8_t* p = &pRes[9];
		if(len) {
			uint16_t svc = uint16_t((*(p+1) << 8) | *p);
#ifdef HKNFCRW_ENABLE_DEBUG
			uint16_t code = uint16_t((svc & 0xffc0) >> 6);
			uint8_t  attr = svc & 0x003f;
			if(len == 2) {
				LOGD("%04x(code:%04x : attr:%02x)\n", svc, code, attr);
			} else {
				LOGD("%04x(%s)\n", svc, (attr) ? "can" : "cannot");
			}
#endif

			//おしまいチェック
			if(svc == 0xffff) {
				break;
			}
			loop++;
		} else {
			break;
		}
	} while(true);

	return true;
}


/**
 * [FeliCa]PUSHコマンド
 *
 * @param[in]	pData		PUSHデータ
 * @param[in]	DataLen		pDataの長さ
 *
 * @retval		true			成功
 * @retval		false			失敗
 *
 * @attention	- pDataはそのまま送信するため、上位層で加工しておくこと。
 */
bool HkNfcF_Push(const uint8_t* pData, uint8_t DataLen)
{
	int ret;
	uint8_t responseLen;
	uint8_t* pCmd = NfcPcd_CommandBuf();
	uint8_t* pRes = NfcPcd_ResponseBuf();

	LOGD("%s", __FUNCTION__);

	if (DataLen > 224) {
		LOGE("bad len\n");
		return false;
	}

	pCmd[0] = (uint8_t)(10 + DataLen);
	pCmd[1] = 0xb0;			//PUSH
	hk_memcpy(pCmd + 2, NfcPcd_NfcId(), NFCID2_LEN);
	pCmd[10] = DataLen;
	hk_memcpy(pCmd + 11, pData, DataLen);

	// xx:IDm
	// [cmd]b0 xx xx xx xx xx xx xx xx len (push pData...)
	ret = NfcPcd_CommunicateThruEx(
						kPUSH_TIMEOUT,
						pCmd, 10 + DataLen,
						NfcPcd_ResponseBuf, &responseLen);
	if (!ret || (responseLen != 10) || (pRes[0] != 0xb1) ||
	  (memcmp(NfcPcd_ResponseBuf + 1, NfcPcd_NfcId(), NFCID2_LEN) != 0) ||
	  (pRes[9] != DataLen)) {

		LOGE("push1 : ret=%d\n", ret);
		return false;
	}

	// xx:IDm
	// [cmd]a4 xx xx xx xx xx xx xx xx 00
	pCmd[0] = 11;
	pCmd[1] = 0xa4;			//inactivate? activate2?
	hk_memcpy(pCmd + 2, NfcPcd_NfcId(), NFCID2_LEN);
	pCmd[10] = 0x00;

	ret = NfcPcd_CommunicateThruEx(
						kDEFAULT_TIMEOUT,
						pCmd, 11,
						NfcPcd_ResponseBuf, &responseLen);
	if (!ret || (responseLen != 10) || (pRes[0] != 0xa5) ||
	  (memcmp(NfcPcd_ResponseBuf + 1, NfcPcd_NfcId(), NFCID2_LEN) != 0) ||
	  (pRes[9] != 0x00)) {

		LOGE("push2 : ret=%d\n", ret);
		return false;
	}

	hk_msleep(1000);

	return true;
}
#endif	//HKNFCF_USE_FELICA

