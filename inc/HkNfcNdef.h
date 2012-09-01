/**
 * @file	HkNfcNdef.h
 * @brief	NDEF
 */
#ifndef HK_NFCNDEF_H
#define HK_NFCNDEF_H
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

#include <stdbool.h>
#include "HkNfcNdefMsg.h"

typedef uint16_t HkNfcNdefLangCode;

#define HKNFCNDEF_LC(c1,c2) ((HkNfcNdefLangCode)(((c1)<<8) | c2))

/**
 * @enum	HkNfcNdefRecordType
 * @brief	レコードタイプ
 */
typedef enum HkNfcNdefRecordType {
	RTD_NONE,
	RTD_TEXT,
	RTD_URI,
	RTD_SP,
} HkNfcNdefRecordType;

/*
 * 言語コード
 * RFC-3066
 * http://www.cybergarden.net/references/langcode/
 */

/**
 * @def		LANGCODE_EN
 * @brief	[言語コード]英語
 */
#define LANGCODE_EN		HKNFCNDEF_LC('e', 'n')

/**
 * @def		LANGCODE_JP
 * @brief	[言語コード]日本語
 */
#define LANGCODE_JP		HKNFCNDEF_LC('j', 'a')

bool HkNfcNdef_CreateText(HkNfcNdefMsg* pMsg,
			const void* pUTF8, uint16_t len, HkNfcNdefLangCode LangCode);

#endif /* HK_NFCNDEF_H */
