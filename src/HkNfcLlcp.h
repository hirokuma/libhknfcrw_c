/**
 * @file	HkNfcLlcp.h
 * @brief	LLCPヘッダ
 */
#ifndef HK_NFCLLCP_H
#define HK_NFCLLCP_H
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
#include "HkNfcDepMode.h"

bool HkNfcLlcpI_Start(HkNfcDepMode mode, void (*pRecvCb)(const void* pBuf, uint8_t len));
bool HkNfcLlcpI_StopRequest(void);
bool HkNfcLlcpI_AddSendData(const void* pBuf, uint8_t len);
bool HkNfcLlcpI_SendRequest(void);
bool HkNfcLlcpI_Poll(void);

bool HkNfcLlcpT_Start(void (*pRecvCb)(const void* pBuf, uint8_t len));
bool HkNfcLlcpT_StopRequest(void);
bool HkNfcLlcpT_AddSendData(const void* pBuf, uint8_t len);
bool HkNfcLlcpT_SendRequest(void);
bool HkNfcLlcpT_Poll(void);

#endif /* HK_NFCLLCP_H */
