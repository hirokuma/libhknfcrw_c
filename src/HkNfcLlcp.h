/**
 * @file	HkNfcLlcp.h
 * @brief	LLCPヘッダ
 */
#ifndef HK_NFCLLCP_H
#define HK_NFCLLCP_H
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

#include <stdint.h>
#include <stdbool.h>


/**
 * @typedef	HkNfcLlcpMode
 * @brief	LLCP動作モード
 */
typedef uint8_t HkNfcLlcpMode;

/* DEPでの通信モード */
/// @def 	HKNFCLLCPMODE_NONE
/// @brief	DEP開始前(取得のみ)
#define HKNFCLLCPMODE_NONE		((HkNfcLlcpMode)0x00)

/// @def 	HKNFCLLCPMODE_ACT_106K
/// @brief	106kbps Active
#define HKNFCLLCPMODE_ACT_106K	((HkNfcLlcpMode)0x0c)

/// @def 	HKNFCLLCPMODE_PSV_106K
/// @brief	106kbps Passive
#define HKNFCLLCPMODE_PSV_106K	((HkNfcLlcpMode)0x04)

/// @def 	HKNFCLLCPMODE_ACT_212K
/// @brief	212kbps Active
#define HKNFCLLCPMODE_ACT_212K	((HkNfcLlcpMode)0x0a)

/// @def 	HKNFCLLCPMODE_PSV_212K
/// @brief	212kbps Passive
#define HKNFCLLCPMODE_PSV_212K	((HkNfcLlcpMode)0x02)

/// @def 	HKNFCLLCPMODE_ACT_424K
/// @brief	424kbps Active
#define HKNFCLLCPMODE_ACT_424K	((HkNfcLlcpMode)0x09)

/// @def 	HKNFCLLCPMODE_PSV_424K
/// @brief	424kbps Passive
#define HKNFCLLCPMODE_PSV_424K	((HkNfcLlcpMode)0x01)



#ifdef HKNFCRW_USE_SNEP_INITIATOR
bool HkNfcLlcp_StartAsIn(HkNfcLlcpMode mode, void (*pRecvCb)(const void* pBuf, uint8_t len));
bool HkNfcLlcp_PollAsIn(void);
#endif	//HKNFCRW_USE_SNEP_INITIATOR

#ifdef HKNFCRW_USE_SNEP_TARGET
bool HkNfcLlcp_StartAsTg(void (*pRecvCb)(const void* pBuf, uint8_t len));
bool HkNfcLlcp_PollAsTg(void);
#endif	//HKNFCRW_USE_SNEP_TARGET

bool HkNfcLlcp_AddSendData(const void* pBuf, uint8_t len);
bool HkNfcLlcp_SendRequest(void);
bool HkNfcLlcp_StopRequest(void);
void HkNfcLlcp_Close(void);

#endif /* HK_NFCLLCP_H */
