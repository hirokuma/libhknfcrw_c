/*
 * @file	HkNfcB.c
 * @brief	NFC-Bアクセス実装
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
#include "HkNfcB.h"
#include "NfcPcd.h"

#define LOG_TAG "HkNfcB"
#include "nfclog.h"


//Polling
static const uint8_t INLISTPASSIVETARGET[] = { 0x03, 0x00 };
static const uint8_t INLISTPASSIVETARGET_RES = 0x01;


/**
 * @brief	ポーリング
 * 
 * NFC-Bのポーリングを行う.
 *
 * @retval		true			成功
 * @retval		false			失敗
 */
bool HkNfcB_Polling(void)
{
	int ret;
	int i;
	uint8_t responseLen;
	uint8_t* pData;

	ret = NfcPcd_InListPassiveTarget(
					INLISTPASSIVETARGET, sizeof(INLISTPASSIVETARGET),
					&pData, &responseLen);
	if (!ret || (responseLen < 17)) {
		//LOGE("pollingB fail: ret=%d/len=%d\n", ret, responseLen);
		return false;
	}

	//[0] d5
	//[1] 4b
	//[2] NbTg
	//[3] Tg
	if(*(pData + 3) != 0x01) {
		LOGE("bad TargetNo : %02x", *(pData + 3));
		return false;
	}

	//[4] 50
	if(*(pData + 4) != 0x50) {
		LOGE("not ATQB : %02x", *(pData + 4));
		return false;
	}
	
	//[5..8]NFCID0
	NfcPcd_SetNfcId(pData + 5, NFCID0_LEN);
	LOGD("NFCID0 : %02x%02x%02x%02x\n", *(pData + 5), *(pData + 6), *(pData + 7), *(pData + 8));

	//Application Data
	//[9]AFI
	LOGD("AFI : %02x\n", *(pData + 9));
	
	//[10]CRC_B(AID)[0]
	//[11]CRC_B(AID)[1]
	LOGD("CRC_B(AID) : %02x%02x\n", *(pData + 10), *(pData + 11));
	
	//[12]Number of application
	LOGD("Number of application : %02x\n", *(pData + 12));

	//Protocol Info
	//[13]Bit_Rate_Capability
	LOGD("Bit_Rate_Capability : %02x\n", *(pData + 13));
	
	//[14][FSCI:4][Protocol_Type:4]
	LOGD("FSCI : %x\n", *(pData + 14) >> 4);
	LOGD("Protocol_Type : %x\n", *(pData + 14) & 0x0f);
	
	//[15][FWI:4][ADC:2][FO:2]
	LOGD("FWI : %x\n", *(pData + 15) >> 4);
	LOGD("ADC : %x\n", (*(pData + 15) >> 2) & 0x03);
	LOGD("FO  : %x\n", *(pData + 15) & 0x03);
	//[??]optional
	
	//ATTRIB_RES
	//[16]Length
	for(i=0; i<*(pData + 16); i++) {
		LOGD("[ATTRIB_RES]%02x\n", *(pData + 17 + i));
	}

	return true;
}


/**
 * @brief	(未実装)読み込み
 * 
 * NFC-Bの指定したブロックを読み込む.
 *
 * @param[out]	pBuf		読み込みデータ
 * @param[in]	BlockNo		読み込みブロック番号
 * @return		true		成功
 * @return		false		失敗
 */
bool HkNfcB_Read(uint8_t* pBuf, uint8_t BlockNo)
{
	return false;
}


/**
 * @brief	(未実装)書き込み
 * 
 * NFC-Bの指定したブロックに書き込む.
 *
 * @param[in]	pBuf		書き込みデータ
 * @param[in]	BlockNo		書き込みブロック番号
 * @return		true		成功
 * @return		false		失敗
 */
bool HkNfcB_Write(const uint8_t* pBuf, uint8_t BlockNo)
{
	return false;
}
