/**
 * @file	HkNfcF.h
 * @brief	NFC-Fアクセス
 */
#ifndef HKNFC_F_H
#define HKNFC_F_H

#include <stdint.h>
#include <stdbool.h>

#define HKNFCF_SVCCODE_RW		((uint16_t)0x0009)		///< サービスコード:R/W
#define HKNFCF_SVCCODE_RO		((uint16_t)0x000b)		///< サービスコード:RO

#define HKNFCF_SC_NDEF		((uint16_t)0x12fc)		///< NDEFシステムコード


bool HkNfcF_Polling(uint16_t systemCode);
void HkNfcF_Release(void);
bool HkNfcF_Read(uint8_t* buf, uint8_t BlockNo);
bool HkNfcF_Write(const uint8_t* buf, uint8_t BlockNo);

void HkNfcF_SetServiceCode(uint16_t svccode);

#ifdef QHKNFCRW_USE_FELICA
/// Request System Code
bool HkNfcF_ReqSystemCode(uint8_t* pNums);
/// Search Service Code
bool HkNfcF_SearchServiceCode(void);
/// Push
bool HkNfcF_Push(const uint8_t* pData, uint8_t DataLen);
#endif	//QHKNFCRW_USE_FELICA

#endif /* HKNFC_F_H */
