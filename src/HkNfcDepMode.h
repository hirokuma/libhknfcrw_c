/**
 * @file	HkNfcDepMode.h
 * @brief	DEPモード定義
 */
#ifndef HK_NFCHKNFCDEPMODE_H
#define HK_NFCHKNFCDEPMODE_H
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
