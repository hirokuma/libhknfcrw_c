/*
 * @file	HkNfcNdef.c
 * @brief	NDEF
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
bool HkNfcNdef_CreateText(HkNfcNdefMsg* pMsg, const void* pUTF8, uint16_t len, uint16_t LangCode)
{
	uint16_t pos = 0;
	
	pMsg->Data[pos++] = (uint8_t)(MB | ME | SR | TNF_WK);
	pMsg->Data[pos++] = 1;		//Type Length
	pMsg->Data[pos++] = (uint8_t)len;		//Payload Length
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
