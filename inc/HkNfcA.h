/**
 * @file	HkNfcA.h
 * @brief	NFC-Aアクセス
 */
#ifndef HKNFCA_H
#define HKNFCA_H
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
 * @typedef		HkNfcASelRes
 * @brief		SEL_RES。HKNFCA_SELRES_xxxと対応する。
 */
typedef uint8_t HkNfcASelRes;

/// @def	HKNFCA_SELRES_MIFARE_UL
/// @brief	MIFARE Ultralight
#define HKNFCA_SELRES_MIFARE_UL			((HkNfcASelRes)0x00)

/// @def	HKNFCA_SELRES_MIFARE_1K
/// @brief	MIFARE 1K
#define HKNFCA_SELRES_MIFARE_1K			((HkNfcASelRes)0x08)

/// @def	HKNFCA_SELRES_MIFARE_MINI
/// @brief	MIFARE MINI
#define HKNFCA_SELRES_MIFARE_MINI		((HkNfcASelRes)0x09)

/// @def	HKNFCA_SELRES_MIFARE_4K
/// @brief	MIFARE 4K
#define HKNFCA_SELRES_MIFARE_4K			((HkNfcASelRes)0x18)

/// @def	HKNFCA_SELRES_MIFARE_DESFIRE
/// @brief	MIFARE DESFIRE
#define HKNFCA_SELRES_MIFARE_DESFIRE	((HkNfcASelRes)0x20)

/// @def	HKNFCA_SELRES_JCOP30
/// @brief	JCOP30
#define HKNFCA_SELRES_JCOP30			((HkNfcASelRes)0x28)

/// @def	HKNFCA_SELRES_GEMPLUS_MPCOS
/// @brief	Gemplus MPCOS
#define HKNFCA_SELRES_GEMPLUS_MPCOS		((HkNfcASelRes)0x98)

/// @def	HKNFCA_SELRES_UNKNOWN
/// @brief	不明
#define HKNFCA_SELRES_UNKNOWN			((HkNfcASelRes)0xff)

/// @def	HKNFCA_SELRES_TPE
/// @brief	SEL_RESのTPEビット
#define HKNFCA_SELRES_TPE				((HkNfcASelRes)0x40)

/// @def	HKNFCA_IS_SELRES_TPE
/// @brief	SEL_RESからTPE端末かどうか判別する
#define HKNFCA_IS_SELRES_TPE(selres)	((selres & HKNFCA_SELRES_TPE) == HKNFCA_SELRES_TPE)

/// @def	HKNFCA_SZ_BLOCK
/// @brief	1ブロックサイズ
#define HKNFCA_SZ_BLOCK		(4)

/// @def	HKNFCA_SZ_BLOCK_R
/// @brief	読込時のブロックサイズ
#define HKNFCA_SZ_BLOCK_R		(16)

/// @def	HKNFCA_SZ_BLOCK_W
/// @brief	書込時のブロックサイズ
#define HKNFCA_SZ_BLOCK_W		(4)


bool HkNfcA_Polling(void);
HkNfcASelRes HkNfcA_GetSelRes(void);

bool HkNfcA_Read(uint8_t* pBuf, uint8_t BlockNo);
bool HkNfcA_Write(const uint8_t* pBuf, uint8_t BlockNo);

#ifdef HKNFCA_USE_CLASSIC
bool HkNfcA_ClassicRead(uint8_t* pBuf, uint8_t BlockNo);
#endif	/* HKNFCA_USE_CLASSIC */

#endif /* HKNFCA_H */
