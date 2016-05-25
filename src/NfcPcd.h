/**
 * @file	nfcpcd.h
 * @brief	NFCのPCD(Proximity Coupling Device)アクセス用ヘッダ
 */
#ifndef NFCPCD_H
#define NFCPCD_H
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

#include "HkNfcId.h"

/**
 * @class		NfcPcd
 * @brief		NFCのPCDアクセスクラス
 * @defgroup	gp_NfcPcd	NfcPcdクラス
 *
 * NFCのPCDにアクセスするPHY部
 */

#define DATA_MAX		((uint16_t)265)		///< データ部最大長

/// GetFirmware
#define GF_IC		(0)			///< GetFirmware:IC
#define GF_VER		(1)			///< GetFirmware:Ver
#define GF_REV		(2)			///< GetFirmware:Rev
#define GF_SUPPORT	(3)			///< GetFirmware:Support

/// GetGeneralStatus
#define GGS_ERR			(0)			///< GetGeneralStatus:??
#define GGS_FIELD		(1)			///< GetGeneralStatus:??
#define GGS_NBTG		(2)			///< GetGeneralStatus:??
#define GGS_TG			(3)			///< GetGeneralStatus:??
#define GGS_TXMODE		(4)			///< GetGeneralStatus:??
#define GGS_TXMODE_DEP	(0x03)		///< GetGeneralStatus:DEP
#define GGS_TXMODE_FALP	(0x05)		///< GetGeneralStatus:FALP


/// @enum	ActPass
/// @brief	inJumpForDep(void)参照
typedef enum ActPass {
	AP_PASSIVE	= 0x00,		///< パッシブモード(受動通信)
	AP_ACTIVE	= 0x01		///< アクティブモード(能動通信)
} ActPass;

/// @enum	BaudRate
/// @brief	inJumpForDep(void)参照
typedef enum BaudRate {
	BR_106K		= 0x00,		///< 106kbps(MIFAREなど)
	BR_212K		= 0x01,		///< 212kbps(FeliCa)
	BR_424K		= 0x02		///< 424kbps(FeliCa)
} BaudRate;

/// @struct	DepInitiatorParam
/// @brief	NFC-DEPイニシエータパラメータ
typedef struct DepInitiatorParam {
	ActPass Ap;						///< Active/Passive
	BaudRate Br;					///< 通信速度
	const uint8_t*		pNfcId3;	///< NFCID3(不要ならnull)
	const uint8_t*		pGb;		///< GeneralBytes(GbLenが0:未使用)
	uint8_t				GbLen;		///< pGbサイズ(不要なら0)
	uint8_t*			pResponse;		///< [out]Targetからの戻り値(不要なら0)
	uint8_t				ResponseLen;	///< [out]pResponseのサイズ(不要なら0)
} DepInitiatorParam;

/// @struct	TargetParam
/// @brief	ターゲットパラメータ
typedef struct TargetParam {
	//		uint16_t			SystemCode;	///< [NFC-F]システムコード
	//		const uint8_t*		pUId;		///< [NFC-A]UID(3byte)。
	//										///  UID 4byteの先頭は強制的に0x04。
	//										///  null時には乱数生成する。
	//		const uint8_t*		pIDm;		///< [NFC-F]IDm
	//										///  null時には6byteを乱数生成する(先頭は01fe)。
	const uint8_t*		pGb;		///< GeneralBytes(GbLenが0:未使用)
	uint8_t				GbLen;		///< pGbサイズ(不要なら0)
	uint8_t*			pCommand;		///< [out]Initiatorからの送信データ(不要なら0)
	uint8_t				CommandLen;		///< [out]pCommandのサイズ(不要なら0)
} TargetParam;


/// @addtogroup gp_port	Device Port Control
/// @ingroup gp_NfcPcd
/// @{

/// オープン
bool NfcPcd_PortOpen(void);
/// オープン済みかどうか
bool NfcPcd_IsOpened(void);
/// クローズ
void NfcPcd_PortClose(void);
/// @}


/// @addtogroup gp_commoncmd	Common Command
/// @ingroup gp_NfcPcd
/// @{

/// デバイス初期化
bool NfcPcd_Init(void);
/// RF出力停止
bool NfcPcd_RfOff(void);
/// RFConfiguration
bool NfcPcd_RfConfiguration(uint8_t cmd, const uint8_t* pCommand, uint8_t CommandLen);
/// Reset
bool NfcPcd_Reset(void);
/// Diagnose
bool NfcPcd_Diagnose(
		uint8_t cmd, const uint8_t* pCommand, uint8_t CommandLen,
		uint8_t* pResponse, uint8_t* pResponseLen);
/// SetParameters
bool NfcPcd_SetParameters(uint8_t val);
/// WriteRegister
bool NfcPcd_WriteRegister(const uint8_t* pCommand, uint8_t CommandLen);
/// PowerDown
bool NfcPcd_PowerDown(uint8_t wake);
/// GetFirmware
bool NfcPcd_GetFirmwareVersion(uint8_t* pResponse);
/// GetGeneralStatus
bool NfcPcd_GetGeneralStatus(uint8_t* pResponse);
/// CommunicateThruEX
bool NfcPcd_CommunicateThruEx0(void);
/// CommunicateThruEX
bool NfcPcd_CommunicateThruEx2(
		const uint8_t* pCommand, uint8_t CommandLen,
		uint8_t* pResponse, uint8_t* pResponseLen);
/// CommunicateThruEX
bool NfcPcd_CommunicateThruEx(
		uint16_t Timeout,
		const uint8_t* pCommand, uint8_t CommandLen,
		uint8_t* pResponse, uint8_t* pResponseLen);
/// @}


/// @addtogroup gp_initiatorcmd	Initiator Command
/// @ingroup gp_NfcPcd
/// @{

/// InJumpForDEP
bool NfcPcd_InJumpForDep(DepInitiatorParam* pParam);
/// InJumpForPSL
bool NfcPcd_InJumpForPsl(DepInitiatorParam* pParam);
/// InListPassiveTarget
bool NfcPcd_InListPassiveTarget(
		const uint8_t* pInitData, uint8_t InitLen,
		uint8_t** ppTgData, uint8_t* pTgLen);
/// InDataExchange
bool NfcPcd_InDataExchange(
		const uint8_t* pCommand, uint8_t CommandLen,
		uint8_t* pResponse, uint8_t* pResponseLen/*, bool bCoutinue=false*/);
/// InCommunicateThru
bool NfcPcd_InCommunicateThru(
		const uint8_t* pCommand, uint8_t CommandLen,
		uint8_t* pResponse, uint8_t* pResponseLen);
/// @}


/// @addtogroup gp_targetcmd	Target Command
/// @ingroup gp_NfcPcd
/// @{

/// TgInitAsTarget
bool NfcPcd_TgInitAsTarget(TargetParam* pParam);
/// TgSetGeneralBytes
bool NfcPcd_TgSetGeneralBytes(const TargetParam* pParam);
/// TgResponseToInitiator
bool NfcPcd_TgResponseToInitiator(
		const uint8_t* pData, uint8_t DataLen,
		uint8_t* pResponse, uint8_t* pResponseLen);
/// TgGetInitiatorCommand
bool NfcPcd_TgGetInitiatorCommand(uint8_t* pResponse, uint8_t* pResponseLen);
/// TgGetData
bool NfcPcd_TgGetData(uint8_t* pCommand, uint8_t* pCommandLen);
/// TgSetData
bool NfcPcd_TgSetData(const uint8_t* pResponse, uint8_t ResponseLen);
/// InRelease
bool NfcPcd_InRelease(void);
/// @}


/// @addtogroup gp_buf		shared buffer
/// @ingroup gp_NfcPcd
/// @{

/// コマンドバッファ取得.
uint8_t* NfcPcd_CommandBuf(void);

/// レスポンスバッファ取得.
uint8_t* NfcPcd_ResponseBuf(void);
/// @}


/// @addtogroup gp_nfcid	NFCID
/// @ingroup gp_NfcPcd
/// @{

/// 保持しているNFCIDサイズ取得.
uint8_t NfcPcd_NfcIdLen(void);

/// 保持しているNFCID取得
const uint8_t* NfcPcd_NfcId(void);

/// 保持しているNFCIDの設定.
void NfcPcd_SetNfcId(const uint8_t* pNfcId, uint8_t len);

/// 保持しているNFCIDのクリア
void NfcPcd_ClrNfcId(void);
/// @}


#endif /* NFCPCD_H */
