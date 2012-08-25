/**
 * @file	HkNfcNdef.h
 * @brief	NDEF
 */
#ifndef HK_NFCNDEF_H
#define HK_NFCNDEF_H

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
