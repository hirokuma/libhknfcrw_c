/*
 * @file	HkNfcA.c
 * @brief	NFC-Aアクセス実装
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
#include "HkNfcA.h"
#include "NfcPcd.h"
#include "hk_misc.h"

#define LOG_TAG "HkNfcA"
#include "nfclog.h"

#define kDEFAULT_TIMEOUT	((uint16_t)(1000 * 2))

static uint16_t			m_SensRes;
static HkNfcASelRes		m_SelRes = HKNFCA_SELRES_UNKNOWN;


//Key A Authentication
#define KEY_A_AUTH		((uint8_t)0x60)
#define KEY_B_AUTH		((uint8_t)0x61)
#define READ			((uint8_t)0x30)
#define UPDATE			((uint8_t)0xa0)
#define WRITE			((uint8_t)0xa2)
#define SECTOR_SELECT	((uint8_t)0xc2)
#define ACK				((uint8_t)0x0a)

/**
 * @brief	ポーリング
 * 
 * NFC-Aのポーリングを行う.
 *
 * @retval		true			成功
 * @retval		false			失敗
 */
bool HkNfcA_Polling(void)
{
	int ret;
	uint8_t responseLen;
	uint8_t* pData;
	const uint8_t cmd[] = { 0x00 };

	ret = NfcPcd_InListPassiveTarget(cmd, 1, &pData, &responseLen);
	if (!ret
	  || (responseLen < 12)
	  || (responseLen <  7 + *(pData + 7))) {
		//LOGE("pollingA fail: ret=%d/len=%d\n", ret, responseLen);
		return false;
	}

	//[0] d5
	//[1] 4b
	//[2] NbTg
	//[3] Tg
	if(*(pData + 3) != 0x01) {
		LOGE("bad TargetNo : %02x\n", *(pData + 3));
		return false;
	}

	m_SensRes = (uint16_t)((*(pData + 4) << 8) | *(pData + 5));
	LOGD("SENS_RES:%04x\n", m_SensRes);

	m_SelRes = (HkNfcASelRes)*(pData + 6);
#ifdef HKNFCRW_ENABLE_DEBUG
	const char* sel_res;
	switch(m_SelRes) {
	case HKNFCA_SELRES_MIFARE_UL:		sel_res = "MIFARE Ultralight";		break;
	case HKNFCA_SELRES_MIFARE_1K:		sel_res = "MIFARE 1K";				break;
	case HKNFCA_SELRES_MIFARE_MINI:		sel_res = "MIFARE MINI";			break;
	case HKNFCA_SELRES_MIFARE_4K:		sel_res = "MIFARE 4K";				break;
	case HKNFCA_SELRES_MIFARE_DESFIRE:	sel_res = "MIFARE DESFIRE";			break;
	case HKNFCA_SELRES_JCOP30:			sel_res = "JCOP30";					break;
	case HKNFCA_SELRES_GEMPLUS_MPCOS:	sel_res = "Gemplus MPCOS";			break;
	default:
		m_SelRes = HKNFCA_SELRES_UNKNOWN;
		sel_res = "???";
	}
	LOGD("SEL_RES:%02x(%s)\n", m_SelRes, sel_res);
#endif	//HKNFCRW_ENABLE_DEBUG

	if(responseLen <  7 + *(pData + 7)) {
		LOGE("bad length\n");
		return false;
	}

	NfcPcd_SetNfcId(pData + 8, *(pData + 7));

	return true;
}


/**
 * @brief	SEL_RES取得
 * 
 * ポーリングで取得したSEL_RES値を返す.
 *
 * @return	SEL_RES値
 */
HkNfcASelRes HkNfcA_GetSelRes(void)
{
	return m_SelRes;
}


/**
 * @brief	読み込み
 * 
 * NFC-A読み込みの指定したブロックを読み込む.
 *
 * @param[out]	pBuf		読み込みデータ(16byte=#HKNFCA_SZ_BLOCK_R)
 * @param[in]	BlockNo		読み込みブロック番号
 * @return		true		成功
 * @return		false		失敗
 */
bool HkNfcA_Read(uint8_t* pBuf, uint8_t BlockNo)
{
	uint8_t* pCmd = NfcPcd_CommandBuf();
	uint8_t* pRes = NfcPcd_ResponseBuf();

	pCmd[0] = READ;
	pCmd[1] = BlockNo;

	uint8_t len;
	bool ret;

	// Read
#if 0
	ret = NfcPcd_CommunicateThruEx(
					kDEFAULT_TIMEOUT,
					pCmd, 2,
					pRes, &len);
#else
	ret = NfcPcd_InDataExchange(
					pCmd, 2,
					pRes, &len);
#endif
	if(ret && (len == HKNFCA_SZ_BLOCK_R)) {
		hk_memcpy(pBuf, pRes, len);
	} else {
		LOGE("read fail : %d / %d / %02x\n", ret, len, (len > 0) ? pRes[0] : 0xff);
		ret = false;
	}

	return ret;
}


/**
 * @brief	書き込み
 * 
 * NFC-Aの指定したブロックに書き込む.
 *
 * @param[in]	pBuf		書き込みデータ(4byte=#HKNFCA_SZ_BLOCK_W)
 * @param[in]	BlockNo		書き込みブロック番号
 * @return		true		成功
 * @return		false		失敗
 */
bool HkNfcA_Write(const uint8_t* pBuf, uint8_t BlockNo)
{
	uint8_t* pCmd = NfcPcd_CommandBuf();
	uint8_t* pRes = NfcPcd_ResponseBuf();

	pCmd[0] = WRITE;
	pCmd[1] = BlockNo;
	hk_memcpy(pCmd + 2, pBuf, HKNFCA_SZ_BLOCK_W);

	uint8_t len;
	bool ret;

	// Read
#if 0
	ret = NfcPcd_CommunicateThruEx(
					kDEFAULT_TIMEOUT,
					pCmd, (uint8_t)(2 + HKNFCA_SZ_BLOCK_W),
					pRes, &len);
#else
	ret = NfcPcd_InDataExchange(
					pCmd, (uint8_t)(2 + HKNFCA_SZ_BLOCK_W),
					pRes, &len);
#endif
	if(ret && (len == 0)) {
		ret = true;
	} else {
		LOGE("write fail\n");
		ret = false;
	}

	return ret;
}


#ifdef HKNFCA_USE_CLASSIC
/**
 * @brief	読み込み
 * 
 * NFC-A読み込みの指定したブロックを読み込む.
 *
 * @param[out]	pBuf		読み込みデータ(4byte=#HKNFCA_SZ_BLOCK)
 * @param[in]	BlockNo		読み込みブロック番号
 * @return		true		成功
 * @return		false		失敗
 */
bool HkNfcA_ClassicRead(uint8_t* pBuf, uint8_t BlockNo)
{
	uint8_t* pCmd = NfcPcd_CommandBuf();
	pCmd[0] = 0x01;
	pCmd[2] = BlockNo;
	pCmd[3] = 0xff;
	pCmd[4] = 0xff;
	pCmd[5] = 0xff;
	pCmd[6] = 0xff;
	pCmd[7] = 0xff;
	pCmd[8] = 0xff;
	hk_memcpy(pCmd + 9, NfcPcd_NfcId(), NfcPcd_NfcIdLen());

	uint8_t len;
	bool ret;

	// Key A Authentication
	pCmd[1] = KEY_A_AUTH;
	ret = NfcPcd_CommunicateThruEx(
					kDEFAULT_TIMEOUT,
					pCmd, 9 + NfcPcd_NfcIdLen(),
					NfcPcd_ResponseBuf(), &len);
	if(!ret) {
		LOGE("read fail1\n");
		return false;
	}

	// Key B Authentication
	pCmd[1] = KEY_B_AUTH;
	ret = NfcPcd_CommunicateThruEx(
					kDEFAULT_TIMEOUT,
					pCmd, 9 + NfcPcd_NfcIdLen(),
					NfcPcd_ResponseBuf(), &len);
	if(!ret) {
		LOGE("read fail2\n");
		return false;
	}

	// Read
	pCmd[1] = READ;
	ret = NfcPcd_CommunicateThruEx(
					kDEFAULT_TIMEOUT,
					pCmd, 3,
					NfcPcd_ResponseBuf(), &len);
	if(ret) {
		hk_memcpy(pBuf, NfcPcd_ResponseBuf(), len);
	} else {
		LOGE("read fail3\n");
	}

	return ret;
}
#endif	/* HKNFCA_USE_CLASSIC */
