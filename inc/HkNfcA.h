/**
 * @file	HkNfcA.h
 * @brief	NFC-Aアクセス
 */
#ifndef HKNFCA_H
#define HKNFCA_H

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
#define HKNFCA_IS_SELRES_TPE(selres)	((sel_res & HKNFCA_SELRES_TPE) == HKNFCA_SELRES_TPE)

#if 0
enum Error {
	SUCCESS				= 0x00,

	ERR_TIMEOUT			= 0x01,
	ERR_CRC				= 0x02,
	ERR_PARITY			= 0x03,
	ERR_ANTICOL			= 0x04,
	ERR_FRAMING			= 0x05,
	ERR_BITCOL			= 0x06,
	ERR_LESSBUF			= 0x07,
	ERR_RFBUF			= 0x08,
	ERR_
};
#endif

bool HkNfcA_Polling(void);
HkNfcASelRes HkNfcA_GetSelRes(void);

bool HkNfcA_Read(uint8_t* pBuf, uint8_t BlockNo);
bool HkNfcA_Write(const uint8_t* pBuf, uint8_t BlockNo);

#endif /* HKNFCA_H */
