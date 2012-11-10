/**
 * @file	HkNfcSnep.h
 * @brief	SNEPアクセス
 */
#ifndef HK_NFCSNEP_H
#define HK_NFCSNEP_H
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
#include "HkNfcNdefMsg.h"


typedef uint8_t HkNfcSnepRet;
#define HKNFCSNEP_SUCCESS		((HkNfcSnepRet)0)
#define HKNFCSNEP_PROCESSING	((HkNfcSnepRet)1)
#define HKNFCSNEP_FAIL			((HkNfcSnepRet)2)

typedef uint8_t HkNfcSnepMode;
#define HKNFCSNEP_MD_INITIATOR		((HkNfcSnepMode)1)
#define HKNFCSNEP_MD_TARGET			((HkNfcSnepMode)2)


bool HkNfcSnep_PutStart(HkNfcSnepMode Mode, const HkNfcNdefMsg* pMsg);
#ifdef HKNFCRW_USE_SNEP_SERVER
bool HkNfcSnep_PutServer(HkNfcSnepMode Mode, HkNfcNdefMsg* pMsg);
#endif	/* HKNFCRW_USE_SNEP_SERVER */

bool HkNfcSnep_Poll(void);
void HkNfcSnep_Stop(void);
HkNfcSnepRet HkNfcSnep_GetResult(void);

#endif /* HK_NFCSNEP_H */
