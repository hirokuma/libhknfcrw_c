/*
 * @file	HkNfcNdef.c
 * @brief	NDEF
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
#include "HkNfcNdef.h"
#include "hk_misc.h"

#define MB			((uint8_t)0x80)
#define ME			((uint8_t)0x40)
#define CF			((uint8_t)0x20)
#define SR			((uint8_t)0x10)
#define IL			((uint8_t)0x08)
#define TNF_EMPTY	((uint8_t)0x00)
#define TNF_WK		((uint8_t)0x01)


/**
 * NDEF TEXT作成(Short)
 *
 * @param[out]	pMsg		NDEF TEXT
 * @param[in]	pUTF8		テキストデータ(UTF-8)
 * @param[in]	len			テキストデータ長(\0は含まない)
 * @param[in]	LangCode	言語コード
 * @return		作成成功/失敗
 */
bool HkNfcNdef_CreateText(HkNfcNdefMsg* pMsg, const void* pUTF8, uint16_t len, HkNfcNdefLangCode LangCode)
{
	uint16_t pos = 0;
	
	pMsg->Data[pos++] = (uint8_t)(MB | ME | SR | TNF_WK);
	pMsg->Data[pos++] = 1;		//Type Length
	pMsg->Data[pos++] = (uint8_t)(3 + len);		//Payload Length
												//国コードが2byteなので、3
	//ID Lengthは、なし
	pMsg->Data[pos++] = 'T';		//Type=TEXT
	//IDも、なし

	//TEXT本体
	pMsg->Data[pos++] = 0x02;		//UTF-8かつ国コードは2byte
	pMsg->Data[pos++] = (uint8_t)(LangCode >> 8);
	pMsg->Data[pos++] = (uint8_t)LangCode;
	hk_memcpy(pMsg->Data + pos, pUTF8, len);

	pMsg->Length = (uint16_t)(pos + len);

	return true;
}


bool HkNfcNdef_CreateUrl(HkNfcNdefMsg* pMsg, HkNfcHttpType HttpType, const char* pUrl)
{
	uint16_t pos = 0;
	uint8_t len = (uint8_t)hk_strlen(pUrl);
	
	pMsg->Data[pos++] = (uint8_t)(MB | ME | SR | TNF_WK);
	pMsg->Data[pos++] = 1;		//Type Length
	pMsg->Data[pos++] = (uint8_t)(1 + len);		//Payload Length
	//ID Lengthは、なし
	pMsg->Data[pos++] = 'U';		//Type=URI
	//IDも、なし

	//URI本体
	pMsg->Data[pos++] = (uint8_t)HttpType;
	hk_memcpy(pMsg->Data + pos, pUrl, len);

	pMsg->Length = (uint16_t)(pos + len);

	return true;
}
