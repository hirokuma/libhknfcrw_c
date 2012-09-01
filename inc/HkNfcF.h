/**
 * @file	HkNfcF.h
 * @brief	NFC-Fアクセス
 */
#ifndef HKNFC_F_H
#define HKNFC_F_H
/*
 * Copyright (c) 2012-2012, hiro99ma(uenokuma@gmail.com)
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

#define HKNFCF_SVCCODE_RW		((uint16_t)0x0009)		///< サービスコード:R/W
#define HKNFCF_SVCCODE_RO		((uint16_t)0x000b)		///< サービスコード:RO

#define HKNFCF_SC_NDEF		((uint16_t)0x12fc)		///< NDEFシステムコード


/// @def	HKNFCF_IS_NFCID_TPE()
/// @brief	NFCIDからTPE端末かどうか判別する
#define HKNFCF_IS_NFCID_TPE(nfcid)	((nfcid[0] == 0x01) && (nfcid[1] == 0xfe))


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
