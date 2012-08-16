/**
 * @file	HkNfcDepMode.h
 * @brief	DEPモード定義
 */
#ifndef HK_NFCHKNFCDEPMODE_H
#define HK_NFCHKNFCDEPMODE_H

#include <stdint.h>

/**
 * @typedef	HkNfcDepMode
 * @brief	NFC-DEPモード
 */
typedef uint8_t	HkNfcDepMode;

/* DEPでの通信モード */
/// @def 	HKNFCDEPMODE_NONE
/// @brief	DEP開始前(取得のみ)
#define HKNFCDEPMODE_NONE		((HkNfcDepMode)0x00)

/// @def 	HKNFCDEPMODE_ACT_106K
/// @brief	106kbps Active
#define HKNFCDEPMODE_ACT_106K	((HkNfcDepMode)0x0c)

/// @def 	HKNFCDEPMODE_PSV_106K
/// @brief	106kbps Passive
#define HKNFCDEPMODE_PSV_106K	((HkNfcDepMode)0x04)

/// @def 	HKNFCDEPMODE_ACT_212K
/// @brief	212kbps Active
#define HKNFCDEPMODE_ACT_212K	((HkNfcDepMode)0x0a)

/// @def 	HKNFCDEPMODE_PSV_212K
/// @brief	212kbps Passive
#define HKNFCDEPMODE_PSV_212K	((HkNfcDepMode)0x02)

/// @def 	HKNFCDEPMODE_ACT_424K
/// @brief	424kbps Active
#define HKNFCDEPMODE_ACT_424K	((HkNfcDepMode)0x09)

/// @def 	HKNFCDEPMODE_PSV_424K
/// @brief	424kbps Passive
#define HKNFCDEPMODE_PSV_424K	((HkNfcDepMode)0x01)


#endif /* HK_NFCHKNFCDEPMODE_H */
