/**
 * @file	HkNfcDep.h
 * @brief	NFC-DEPヘッダ
 * 
 * 本ヘッダは内部用とする.
 * HkNfcLlcp.cの内部にNFC-DEPを取り込んでいるが、NFC-DEPは外部公開しない.
 */
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
#ifndef HK_NFCDEP_H
#define HK_NFCDEP_H

#include <stdint.h>
#include <stdbool.h>

/// @defgroup	gp_NfcDep	NFC-DEP
/// @{

#define _ACT		((uint8_t)0x08)		///< Active
#define _PSV		((uint8_t)0x00)		///< Passive
#define _AP_MASK	((uint8_t)0x08)		///< Active/Passive用マスク

#define _BR106K		((uint8_t)0x04)		///< 106kbps
#define _BR212K		((uint8_t)0x02)		///< 212kbps
#define _BR424K		((uint8_t)0x01)		///< 424kbps
#define _BR_MASK	((uint8_t)0x07)		///< Baudrate用マスク


/// @addtogroup gp_depinit	NFC-DEP(Initiator)
/// @{
/// InJumpForDEP
bool HkNfcDep_StartAsInitiator(uint8_t mode);
/// InDataExchange
bool HkNfcDep_SendAsInitiator(
		const void* pCommand, uint8_t CommandLen,
		void* pResponse, uint8_t* pResponseLen);
/// RLS_REQ
bool HkNfcDep_StopAsInitiator(void);
/// @}


/// @addtogroup gp_deptgt	NFC-DEP(Target)
/// @{
/// TgInitTarget, TgSetGeneralBytes
bool HkNfcDep_StartAsTarget(void);
/// TgGetData
bool HkNfcDep_RecvAsTarget(void* pCommand, uint8_t* pCommandLen);
/// TgSetData
bool HkNfcDep_RespAsTarget(const void* pResponse, uint8_t ResponseLen);
/// @}

/// @}


void HkNfcDep_Close(void);
uint8_t HkNfcDep_GetDepMode(void);


#endif /* HK_NFCDEP_H */
