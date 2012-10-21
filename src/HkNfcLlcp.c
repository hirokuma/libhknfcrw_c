/*
 * @file	HkNfcLlcp.c
 * @brief	LLCP実装
 */
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
#include "HkNfcRw.h"
#include "HkNfcDep.h"
#include "HkNfcLlcp.h"
#include "NfcPcd.h"
#include "hk_misc.h"

#define LOG_TAG "HkNfcLlcp"
#include "nfclog.h"

//#define USE_DEBUG
#define USE_DEBUG_PDU
#define USE_DEBUG_PDUI

/*
 * definition
 */
#define PL_VERSION		((uint8_t)0x01)
#define PL_MIUX			((uint8_t)0x02)
#define PL_WKS			((uint8_t)0x03)
#define PL_LTO			((uint8_t)0x04)
#define PL_RW			((uint8_t)0x05)
#define PL_SN			((uint8_t)0x06)
#define PL_OPT			((uint8_t)0x07)

// VERSION(1.0)
#define VER_MAJOR		((uint8_t)0x01)
#define VER_MINOR		((uint8_t)0x00)

// http://www.nfc-forum.org/specs/nfc_forum_assigned_numbers_register
#define WKS_LMS	 		(uint16_t)(1 << 0)
#define WKS_SDP 		(uint16_t)(1 << SAP_SDP)
#define WKS_IP 			(uint16_t)(1 << 2)	//nfcpyより
#define WKS_OBEX		(uint16_t)(1 << 3)	//nfcpyより
#define WKS_SNEP 		(uint16_t)(1 << SAP_SNEP)

static const char SN_SDP[] = "\x06\x0eurn:nfc:sn:sdp";
#define LEN_SN_SDP		((uint8_t)16)
static const char SN_SNEP[] = "\x06\x0furn:nfc:sn:snep";
#define LEN_SN_SNEP		((uint8_t)17)


#define PDU_INFOPOS		(2)		///< PDUパケットのInformation開始位置


/* PDU種別 */
#define PDU_SYMM	((uint8_t)0x00)			///< SYMM
#define PDU_PAX		((uint8_t)0x01)			///< PAX
#define PDU_AGF		((uint8_t)0x02)			///< AGF
#define PDU_UI		((uint8_t)0x03)			///< UI
#define PDU_CONN	((uint8_t)0x04)			///< CONNECT
#define PDU_DISC	((uint8_t)0x05)			///< DISC
#define PDU_CC		((uint8_t)0x06)			///< CC
#define PDU_DM		((uint8_t)0x07)			///< DM
#define PDU_FRMR	((uint8_t)0x08)			///< FRMR
#define PDU_RESV1	((uint8_t)0x09)			// reserved
#define PDU_RESV2	((uint8_t)0x0a)			// reserved
#define PDU_RESV3	((uint8_t)0x0b)			// reserved
#define PDU_I		((uint8_t)0x0c)			///< I
#define PDU_RR		((uint8_t)0x0d)			///< RR
#define PDU_RNR		((uint8_t)0x0e)			///< RNR
#define PDU_RESV4	((uint8_t)0x0f)			///< reserved
#define PDU_LAST	PDU_RESV4				//
#define PDU_NONE	((uint8_t)0xff)			// PDU範囲外


/* LLCP状態 */
#define LSTAT_NONE			((uint8_t)0)		///< 未接続
#define LSTAT_NOT_CONNECT	((uint8_t)1)		///< ATR交換後、CONNECT前
#define LSTAT_CONNECTING	((uint8_t)2)		///< CONNECT要求
#define LSTAT_NORMAL		((uint8_t)3)		///< CONNECT/CC交換後
#define LSTAT_BUSY			((uint8_t)4)		///< Receiver Busy
#define LSTAT_TERM			((uint8_t)5)		///< Connection Termination
						// 送信する番になったら、DISCを送信し、#LSTAT_DMに遷移.
						// 受信する場合は、特に何もしない.
#define LSTAT_DM			((uint8_t)6)		///< DM送信待ち
						// 送信する番になったら、DMを送信し、#LSTAT_NONEに遷移.
						// 受信する場合は、特に何もしない.
#define LSTAT_WAIT_DM		((uint8_t)7)		///< DM受信待ち
						// DMを受信したら、#LSTAT_NONEに遷移.
						// DM以外の受信は破棄
						// 送信は破棄

#define SAP_MNG			((uint8_t)0)		///< LLC Link Management
#define SAP_SDP			((uint8_t)1)		///< SDP
#define SAP_SNEP		((uint8_t)4)		///< SNEP
#define LLCP_MIU		((uint8_t)128)		///< MIU

// PDU解析の戻り値で使用する。
// 「このPDUのデータ部はService Data Unitなので、末尾までデータです」という意味。
#define SDU				((uint8_t)0xff)

#define DEFAULT_LTO		((uint16_t)100)	// 100msec

#define ACT_TARGET		((uint8_t)0)
#define ACT_INITIATOR	((uint8_t)1)

#define SR_RECEIVER		((uint8_t)0)
#define SR_SENDER		((uint8_t)1)

#define PRI_NONE		((uint8_t)0)
#define PRI_RECV		((uint8_t)1)


/*
 * const
 */
/// LLCPのGeneralBytes
static const uint8_t LlcpGb[] = {
	// LLCP Magic Number
	0x46, 0x66, 0x6d,

	// TLV0:VERSION[MUST]
	0x01, 0x01, (uint8_t)((VER_MAJOR << 4) | VER_MINOR),

	// TLV1:WKS[SHOULD]
	0x03, 0x02, 0x00, 0x13,
					// bit4 : SNEP
					// bit1 : SDP
					// bit0 : LLC Link Management Service(MUST)

	// TLV2:LTO[MAY] ... 10ms x 200 = 2000ms
	0x04, 0x01, 200,
							// LTO > RWT
							// RWTはRFConfigurationで決定(gbyAtrResTo)

	// TLV3:OPT
	0x07, 0x01, 0x02		//Class 2 (Connection-oriented only)
};


/*
 * variables
 */
static uint8_t		m_DepMode = HKNFCDEPMODE_NONE;	///< 現在のDepMode
static uint8_t		m_Initiator = ACT_TARGET;		///< Initiator / Target or not DEP mode

static uint16_t		m_LinkTimeout;					///< Link Timeout値[msec](デフォルト:100ms)
static uint8_t		m_SendRecv = SR_RECEIVER;		///< 送信側 / 受信側
static uint8_t		m_LlcpStat = LSTAT_NONE;		///< LLCP状態
static uint8_t		m_PrevRecvI = PRI_NONE;			///< 直前にI PDUを受信したか
static uint8_t		m_DSAP = 0;						///< DSAP
static uint8_t		m_SSAP = 0;						///< SSAP
static uint8_t		m_LastSentPdu = PDU_NONE;		///< 最後に送信したPDU
static uint8_t		m_CommandLen = 0;		///< 次に送信するデータ長
static uint8_t		m_SendBuf[LLCP_MIU];	///< 送信データバッファ
static uint8_t		m_SendLen = 0;			///< 送信データサイズ
static uint8_t		m_ValueS = 0;			///< V(S)
static uint8_t		m_ValueR = 0;			///< V(R)
static uint8_t		m_ValueSA = 0;			///< V(SA)
static uint8_t		m_ValueRA = 0;			///< V(SA)

static void (*m_pRecvCb)(const void* pBuf, uint8_t len) = 0;


/*
 * prototype
 */
static uint8_t analyzePdu(const uint8_t* pBuf, uint8_t len, uint8_t* pResPdu);
static uint8_t analyzeSymm(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzePax(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeAgf(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeUi(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeConn(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeDisc(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeCc(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeDm(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeFrmr(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeI(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeRr(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeRnr(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeDummy(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap);
static uint8_t analyzeParamList(const uint8_t *pBuf);
static void createPdu(uint8_t type);
static void killConnection(void);
static bool addSendData(const void* pBuf, uint8_t len);
static bool connect(void);
#ifdef USE_DEBUG_PDU
static const char* string_pdu(uint8_t type);
#endif

/////////////////////////////////////////////////////////////////////////

#ifdef HKNFCRW_USE_SNEP_INITIATOR
/*
 * NFC-DEP開始(Initiator)
 * 
 * [in]		mode	開始するHKNFCDEPMODE_xxx
 * @retval	true	成功
 * @retval	false	失敗
 */
bool HkNfcDep_StartAsInitiator(uint8_t mode)
{
	LOGD("%s\n", __PRETTY_FUNCTION__);

	if(m_DepMode != HKNFCDEPMODE_NONE) {
		LOGE("Already DEP mode\n");
		HkNfcRw_SetLastError(HKNFCERR_PCD_GETSTAT);
		return false;
	}

	DepInitiatorParam prm;
	
	prm.Ap = (mode & _AP_MASK) ? AP_ACTIVE : AP_PASSIVE;

	uint32_t br = mode & _BR_MASK;
	switch(br) {
	case _BR106K:
		prm.Br = BR_106K;
		break;
	case _BR212K:
		prm.Br = BR_212K;
		break;
	case _BR424K:
	default:
		prm.Br = BR_424K;
		break;
	}
	
	prm.pNfcId3 = 0;		// R/Wで自動生成
	prm.pResponse = NfcPcd_ResponseBuf();
	prm.ResponseLen = 0;

	prm.pGb = LlcpGb;
	prm.GbLen = (uint8_t)sizeof(LlcpGb);

	if(!NfcPcd_InJumpForDep(&prm)) {
		LOGE("inJumpForDep\n");
		return false;
	}

	m_DepMode = mode;
	m_Initiator = ACT_INITIATOR;

	const uint8_t* pRecv = prm.pResponse;

	if(prm.ResponseLen < 19) {
		// TgInitTarget Res(16) + Magic Number(3)
		LOGE("small len : %d\n", prm.ResponseLen);
		return false;
	}

	int pos = 0;

	// Tg
	if(pRecv[pos++] != 0x01) {
		LOGE("bad Tg\n");
		return false;
	}

	// NFCID3(skip)
	pos += NFCID3_LEN;

	//DIDt, BSt, BRt
	if((pRecv[pos] == 0x00) && (pRecv[pos+1] == 0x00) && (pRecv[pos+2] == 0x00)) {
		//OK
	} else {
		LOGE("bad DID/BS/BR\n");
		return false;
	}
	pos += 3;

	//TO(skip)
	pos++;

	//PPt
	if(pRecv[pos++] != 0x32) {
		LOGE("bad PP\n");
		return false;
	}

	// Gt

	// Magic Number
	if((pRecv[pos] == 0x46) && (pRecv[pos+1] == 0x66) && (pRecv[pos+2] == 0x6d)) {
		//OK
	} else {
		LOGE("bad Magic Number\n");
		return false;
	}
	pos += 3;

	//Link activation
	m_LinkTimeout = DEFAULT_LTO;
	bool bVERSION = false;
	while(pos < prm.ResponseLen) {
		//ここでParameter List解析
		uint8_t pdu;
		if(pRecv[pos] == PL_VERSION) {
			bVERSION = true;
		}
		pos += analyzeParamList(&pRecv[pos]);
	}
	if(!bVERSION || (m_DepMode == HKNFCDEPMODE_NONE)) {
		//だめ
		return false;
	}

	return true;
}


/*
 * [DEP-Initiator]データ送信
 * 
 * @param	[in]	pCommand		Targetへの送信データ
 * @param	[in]	CommandLen		Targetへの送信データサイズ
 * @param	[out]	pResponse		Targetからの返信データ
 * @param	[out]	pResponseLen	Targetからの返信データサイズ
 * @retval	true	成功
 * @retval	false	失敗(pResponse/pResponseLenは無効)
 */
bool HkNfcDep_SendAsInitiator(
			const void* pCommand, uint8_t CommandLen,
			void* pResponse, uint8_t* pResponseLen)
{
	bool b = NfcPcd_InDataExchange(
					(const uint8_t*)pCommand, CommandLen,
					(uint8_t*)pResponse, pResponseLen);
	return b;
}


/*
 * [DEP-Initiator]データ交換終了(RLS_REQ送信)
 * @retval	true	成功
 * @retval	false	失敗
 */
bool HkNfcDep_StopAsInitiator(void)
{
	LOGD("%s\n", __PRETTY_FUNCTION__);

	hk_msleep(100);

	const uint16_t timeout = 2000;
	uint8_t* pCommandBuf = NfcPcd_CommandBuf();
	uint8_t* pResponseBuf = NfcPcd_ResponseBuf();
	
	pCommandBuf[0] = 3;
	pCommandBuf[1] = 0xd4;
	pCommandBuf[2] = 0x0a;		// RLS_REQ
	pCommandBuf[3] = 0x00;		// DID
	uint8_t res_len;
	bool b = NfcPcd_CommunicateThruEx(timeout,
					pCommandBuf, 4,
					pResponseBuf, &res_len);
	return b;
}
#endif	//HKNFCRW_USE_SNEP_INITIATOR


/////////////////////////////////////////////////////////////////////////
#ifdef HKNFCRW_USE_SNEP_TARGET
/*
 * NFC-DEP開始(Target)
 * 
 * [in]		pGb			GeneralBytes(未使用の場合、0)
 * [in]		GbLen		pGb長(最大48byteだが、チェックしない)
 * @retval	true	成功
 * @retval	false	失敗
 */
bool HkNfcDep_StartAsTarget(void)
{
	LOGD("%s\n", __PRETTY_FUNCTION__);

	if(m_DepMode != HKNFCDEPMODE_NONE) {
		LOGE("Already DEP mode\n");
		return false;
	}

	TargetParam prm;
	prm.pCommand = NfcPcd_ResponseBuf();
	prm.CommandLen = 0;

	prm.pGb = LlcpGb;
	prm.GbLen = (uint8_t)sizeof(LlcpGb);
	//fAutomaticATR_RES=0
	NfcPcd_SetParameters(0x18);

	// Target時の通信性能向上
	const uint8_t target[] = { 0x63, 0x0d, 0x08 };
	NfcPcd_WriteRegister(target, sizeof(target));

	//Initiator待ち
    if(!NfcPcd_TgInitAsTarget(&prm)) {
		LOGE("tgInitAsTarget\n");
		return false;
	}
	uint8_t* pResponseBuf = NfcPcd_ResponseBuf();
	uint8_t br = pResponseBuf[0] & 0x70;
	switch(br) {
	case 0x00:
		LOGD("106kbps\n");
		m_DepMode = _BR106K;	//あとでorして完成する
		break;
	case 0x10:
		LOGD("212kbps\n");
		m_DepMode = _BR212K;	//あとでorして完成する
		break;
	case 0x20:
		LOGD("424kbps\n");
		m_DepMode = _BR424K;	//あとでorして完成する
		break;
	default:
		LOGE("unknown bps : %02x\n", br);
		m_DepMode = HKNFCDEPMODE_NONE;
		break;
	}

	uint8_t comm = pResponseBuf[0] & 0x03;
	switch(comm) {
	case 0x00:
		LOGD("106kbps passive\n");
		m_DepMode = (uint8_t)(m_DepMode | _PSV);
		break;
	case 0x01:
		LOGD("Active\n");
		m_DepMode = (uint8_t)(m_DepMode | _ACT);
		break;
	case 0x02:
		LOGD("212k/424kbps passive\n");
		m_DepMode = (uint8_t)(m_DepMode | _PSV);
		break;
	default:
		LOGE("unknown comm : %02x\n", comm);
		m_DepMode = HKNFCDEPMODE_NONE;
		break;
	}

	if(pResponseBuf[1] != prm.CommandLen - 1) {
		LOGE("bad size, %d, %d\n", pResponseBuf[1], prm.CommandLen);
		return false;
	}

	uint8_t* pCommandBuf = NfcPcd_CommandBuf();
	const uint8_t* pIniCmd = pResponseBuf + 2;
	uint8_t IniCmdLen = prm.CommandLen - 2;

	if((pIniCmd[0] >= 3) && (pIniCmd[1] == 0xd4) && (pIniCmd[2] == 0x0a)) {
		// RLS_REQなら、終わらせる
		const uint16_t timeout = 2000;
		pCommandBuf[0] = 3;
		pCommandBuf[1] = 0xd5;
		pCommandBuf[2] = 0x0b;			// RLS_REQ
		pCommandBuf[3] = pIniCmd[3];	// DID
		uint8_t res_len;
		NfcPcd_CommunicateThruEx(timeout,
						pCommandBuf, 4,
						pResponseBuf, &res_len);
		m_DepMode = HKNFCDEPMODE_NONE;
	}

	if(m_DepMode == HKNFCDEPMODE_ACT_106K) {
		// 106k active

		// 特性を元に戻す
		const uint8_t normal[] = { 0x63, 0x0d, 0x00 };
		NfcPcd_WriteRegister(normal, sizeof(normal));
	} else if(m_DepMode == HKNFCDEPMODE_PSV_106K) {
		// 106k passive

		// 特性を元に戻す
		const uint8_t normal[] = { 0x63, 0x0d, 0x00, 0x63, 0x01, 0x3b };
		NfcPcd_WriteRegister(normal, sizeof(normal));
	} else if(m_DepMode == HKNFCDEPMODE_NONE) {
		LOGE("invalid response\n");
		return false;
	}
	
	if(IniCmdLen < 19) {
		// ATR_REQ(16) + Magic Number(3)
		LOGE("small len : %d\n", IniCmdLen);
		return false;
	}

	// ATR_REQチェック
	int pos = 0;

	// ATR_REQ
	if((pIniCmd[pos] == 0xd4) && (pIniCmd[pos+1] == 0x00)) {
		//OK
	} else {
		LOGE("not ATR_REQ\n");
		return false;
	}
	pos += 2;
	
	// NFCID3(skip)
	pos += NFCID3_LEN;

	//DIDi, BSi, BRi
	if((pIniCmd[pos] == 0x00) && (pIniCmd[pos+1] == 0x00) && (pIniCmd[pos+2] == 0x00)) {
		//OK
	} else {
		LOGE("bad DID/BS/BR\n");
		return false;
	}
	pos += 3;

	//PPi
	if(pIniCmd[pos++] != 0x32) {
		LOGE("bad PP\n");
		return false;
	}

	// Gi

	// Magic Number
	if((pIniCmd[pos] == 0x46) && (pIniCmd[pos+1] == 0x66) && (pIniCmd[pos+2] == 0x6d)) {
		//OK
	} else {
		LOGE("bad Magic Number\n");
		return false;
	}
	pos += 3;

	//Link activation
	bool bVERSION = false;
	while(pos < IniCmdLen) {
		//ここでPDU解析
		uint8_t pdu;
		if(pIniCmd[pos] == PL_VERSION) {
			bVERSION = true;
		}
		pos += analyzeParamList(&pIniCmd[pos]);
	}
	if(!bVERSION || (m_DepMode == HKNFCDEPMODE_NONE)) {
		//だめ
		return false;
	}

	bool ret = NfcPcd_TgSetGeneralBytes(&prm);
	if(ret) {
		//モード確認
		ret = NfcPcd_GetGeneralStatus(pResponseBuf);
	}
	if(ret) {
		if(pResponseBuf[GGS_TXMODE] != GGS_TXMODE_DEP) {
			//DEPではない
			LOGE("not DEP mode\n");
			ret = false;
		}
	}
	
	return true;
}


/*
 * [DEP-Target]データ受信
 * 
 * @param	[out]	pCommand	Initiatorからの送信データ
 * @param	[out]	pCommandLen	Initiatorからの送信データサイズ
 * @retval	true	成功
 * @retval	false	失敗(pCommand/pCommandLenは無効)
 */
bool HkNfcDep_RecvAsTarget(void* pCommand, uint8_t* pCommandLen)
{
	uint8_t* p = (uint8_t*)pCommand;
	bool b = NfcPcd_TgGetData(p, pCommandLen);
	return b;
}

/*
 * [DEP-Target]データ送信
 * 
 * @param	[in]	pResponse		Initiatorへの返信データ
 * @param	[in]	ResponseLen		Initiatorへの返信データサイズ
 * @retval	true	成功
 * @retval	false	失敗
 */
bool HkNfcDep_RespAsTarget(const void* pResponse, uint8_t ResponseLen)
{
	bool b = NfcPcd_TgSetData(
					(const uint8_t*)pResponse, ResponseLen);
	return b;
}
#endif	//HKNFCRW_USE_SNEP_TARGET


/////////////////////////////////////////////////////////////////////////

/*
 * [DEP]クローズ
 */
void HkNfcDep_Close(void)
{
	LOGD("%s\n", __PRETTY_FUNCTION__);

	m_SendRecv = SR_RECEIVER;
	m_LlcpStat = LSTAT_NONE;
	m_DepMode = HKNFCDEPMODE_NONE;
	m_DSAP = 0;
	m_SSAP = 0;
	m_CommandLen = 0;
	m_SendLen = 0;
}


/*
 * [DEP]現在のDEPモード
 */
uint8_t HkNfcDep_GetDepMode(void)
{
	return m_DepMode;
}


/*******************************************************************
 * PDU処理
 *******************************************************************/

/* PDU解析の関数テーブル */
static uint8_t (*sAnalyzePdu[])(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap) = {
	analyzeSymm,
	analyzePax,
	analyzeAgf,
	analyzeUi,
	analyzeConn,
	analyzeDisc,
	analyzeCc,
	analyzeDm,
	analyzeFrmr,
	analyzeDummy,		//0x09
	analyzeDummy,		//0x0a
	analyzeDummy,		//0x0b
	analyzeI,
	analyzeRr,
	analyzeRnr,
	analyzeDummy		//0x0f
};


/**
 * PDU解析.
 * 指定されたアドレスから1つ分のPDU解析を行う。
 *
 * @param[in]	pBuf		解析対象のデータ
 * @param[out]	pResPdu		PDU種別
 * @retval		SDU			これ以降のPDUはない
 * @retval		上記以外	PDUサイズ
 */
static uint8_t analyzePdu(const uint8_t* pBuf, uint8_t len, uint8_t* pResPdu)
{
	m_PrevRecvI = PRI_NONE;

	*pResPdu = (uint8_t)(((*pBuf & 0x03) << 2) | (*(pBuf+1) >> 6));
	if(*pResPdu > PDU_LAST) {
		LOGE("BAD PDU\n");
		*pResPdu = PDU_NONE;
		return SDU;
	}
#ifdef USE_DEBUG_PDU
	LOGD("analyzePdu : %s[len=%d]\n", string_pdu(*pResPdu), len);
#endif

	// 5.6.6 Connection Termination(disconnecting phase)
	uint8_t next;
	if((m_LlcpStat == LSTAT_WAIT_DM) && (*pResPdu != PDU_DM)) {
		LOGD("analyzePdu...Connection Termination(%d)\n", *pResPdu);
		killConnection();
		next = SDU;
	} else {
		uint8_t dsap = *pBuf >> 2;
		uint8_t ssap = *(pBuf + 1) & 0x3f;
		LOGD("[D:0x%02x/S:0x%02x]\n", dsap, ssap);
		next = (*sAnalyzePdu[*pResPdu])(pBuf, len, dsap, ssap);
	}
	return next;
}


/**
 * SYMM
 *
 * @param[in]		解析対象
 * @return
 */
static uint8_t analyzeSymm(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_SYMM\n");
	return PDU_INFOPOS;
}

static uint8_t analyzePax(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_PAX\n");

	pBuf += PDU_INFOPOS;
	len -= PDU_INFOPOS;
	while(len) {
		//ここでPDU解析
		uint8_t next = analyzeParamList(pBuf);
		if(len > next) {
			len -= next;
			pBuf += next;
		} else {
			break;
		}
	}
	return SDU;
}

static uint8_t analyzeAgf(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_AGF\n");
	return (uint8_t)(*(pBuf + PDU_INFOPOS) + 1);
}

static uint8_t analyzeUi(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_UI\n");
	return SDU;		//終わりまでデータが続く
}

static uint8_t analyzeConn(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_CONN\n");
	m_DSAP = *pBuf >> 2;

	pBuf += PDU_INFOPOS;
	len -= PDU_INFOPOS;
	while(len) {
		//ここでPDU解析
		uint8_t next = analyzeParamList(pBuf);
		if(len > next) {
			len -= next;
			pBuf += next;
		} else {
			break;
		}
	}

	if((dsap == SAP_SNEP) && (ssap == SAP_SNEP)) {
		LOGD("... accept\n");
		m_DSAP = dsap;
		m_SSAP = ssap;
		m_ValueS = 0;
		m_ValueR = 0;
		m_ValueSA = 0;
		m_ValueRA = 0;
		
		//CC返信
		createPdu(PDU_CC);
		hk_memcpy(NfcPcd_CommandBuf() + PDU_INFOPOS, 0, 0);
		m_CommandLen = PDU_INFOPOS + 0;
		m_LlcpStat = LSTAT_CONNECTING;
		
		return SDU;
	} else {
		LOGD("... unmatch SAP(D:0x%02x / S:0x%02x)\n", dsap, ssap);
		m_DSAP = SAP_MNG;
		m_SSAP = SAP_MNG;
		m_LlcpStat = LSTAT_DM;
		return SDU;
	}
}

static uint8_t analyzeDisc(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_DISC\n");
	if((dsap == 0) && (ssap == 0)) {
		//5.4.1 Intentional Link Deactivation
		LOGD("-- Link Deactivation\n");
		killConnection();
	} else {
		//5.6.6 Connection Termination
		LOGD("-- Connection Termination\n");
		m_LlcpStat = LSTAT_DM;
		uint8_t* pCommandBuf = NfcPcd_CommandBuf();
		pCommandBuf[0] = 0x00;		//DISC受信による切断
	}
	return PDU_INFOPOS;
}


static uint8_t analyzeCc(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_CC\n");
	if(m_LlcpStat == LSTAT_CONNECTING) {
		//OK
		LOGD("LSTAT_CONNECTING==>LSTAT_NORMAL\n");
		m_LlcpStat = LSTAT_NORMAL;
		m_ValueS = 0;
		m_ValueR = 0;
		m_ValueSA = 0;
		m_ValueRA = 0;
		m_DSAP = ssap;

		pBuf += PDU_INFOPOS;
		len -= PDU_INFOPOS;
		while(len) {
			//ここでPDU解析
			uint8_t next = analyzeParamList(pBuf);
			if(len > next) {
				len -= next;
				pBuf += next;
			} else {
				break;
			}
		}
		return SDU;
	} else {
		LOGD("reject %d\n", m_LlcpStat);
		m_DSAP = SAP_MNG;
		m_SSAP = SAP_MNG;
		m_LlcpStat = LSTAT_DM;
		return SDU;
	}
}

static uint8_t analyzeDm(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_DM : %d\n", *(pBuf + PDU_INFOPOS));
	if(m_LlcpStat == LSTAT_WAIT_DM) {
		//切断シーケンスの終わり
		LOGD("LSTAT_WAIT_DM\n");
	}
	LOGD("==>LSTAT_NONE\n");
	m_LlcpStat = LSTAT_NONE;
#ifdef HKNFCRW_USE_SNEP_INITIATOR
	if(m_Initiator == ACT_INITIATOR) {
		HkNfcDep_StopAsInitiator();
	}
#endif	//HKNFCRW_USE_SNEP_INITIATOR
	HkNfcDep_Close();
	NfcPcd_Reset();
	return PDU_INFOPOS + 1;
}

static uint8_t analyzeFrmr(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_FRMR\n");
	return 0;
}

static uint8_t analyzeI(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	uint8_t NowS = *(pBuf+PDU_INFOPOS) >> 4;
	uint8_t NowR = *(pBuf+PDU_INFOPOS) & 0x0f;

	LOGD("PDU_I(NS:%d / NR:%d))\n", NowS, NowR);
	if(NowS == m_ValueR) {
		//OK
		m_ValueR++;
		
		pBuf += PDU_INFOPOS + 1;
		len -= PDU_INFOPOS + 1;
#ifdef USE_DEBUG_PDUI
		int i;
		for(i=0; i<len; i++) {
			LOGD("  [I]%02x\n", *(pBuf + i));
		}
#endif	//USE_DEBUG_PDUI
		m_PrevRecvI = PRI_RECV;
		(*m_pRecvCb)(pBuf, len);
	} else {
		LOGD("bad sequence(NS:%d / VR:%d)\n", NowS, m_ValueR);
	}
	return SDU;
}

static uint8_t analyzeRr(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_RR : N(R)=%d\n", *(pBuf + PDU_INFOPOS));
	return 0;
}

static uint8_t analyzeRnr(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("PDU_RNR : N(R)=%d\n", *(pBuf + PDU_INFOPOS));
	return 0;
}

static uint8_t analyzeDummy(const uint8_t* pBuf, uint8_t len, uint8_t dsap, uint8_t ssap)
{
	LOGD("dummy dummy dummy\n");
	int i;
	for(i=0; i<len; i++) {
		LOGD("[D]%02x\n", *(pBuf + i));
	}
	return 0;
}

static uint8_t analyzeParamList(const uint8_t *pBuf)
{
	LOGD("Parameter List\n");
	uint8_t next = (uint8_t)(PDU_INFOPOS + *(pBuf + 1));

	switch(*pBuf) {
	case PL_VERSION:
		// 5.2.2 LLCP Version Number Agreement Procedure
		LOGD("VERSION : %02x\n", *(pBuf + PDU_INFOPOS));
		{
			uint8_t major = *(pBuf + PDU_INFOPOS) >> 4;
			uint8_t minor = *(pBuf + PDU_INFOPOS) & 0x0f;
			if(major == VER_MAJOR) {
				if(minor == VER_MINOR) {
					LOGD("agree : same version\n");
				} else if(minor > VER_MINOR) {
					LOGD("agree : remote>local ==> local\n");
				} else {
					LOGD("agree : remote<local ==> remote\n");
				}
			} else if(major < VER_MAJOR) {
				//自分のバージョンが上→今回は受け入れないことにする
				LOGD("not agree : remote<local\n");
				killConnection();
			} else {
				//相手のバージョンが上→判定を待つ
				LOGD("remote>local\n");
			}
		}
		break;
	case PL_MIUX:
		//5.2.3 Link MIU Determination Procedure
		//今回は無視する
		{
			uint16_t miux = (uint16_t)(*(pBuf + PDU_INFOPOS) << 8) | *(pBuf + PDU_INFOPOS + 1);
			LOGD("MIUX : %d\n", miux);
		}
		break;
	case PL_WKS:
		{
			uint16_t wks = (uint16_t)((*(pBuf + PDU_INFOPOS) << 8) | *(pBuf + 3));
			if(wks & (WKS_LMS | WKS_SNEP)) {
				//OK
				LOGD("WKS : have SNEP\n");
			} else {
				//invalid
				LOGD("WKS : not have SNEP\n");
				killConnection();
			}
		}
		break;
	case PL_LTO:
		m_LinkTimeout = (uint16_t)(*(pBuf + PDU_INFOPOS) * 10);
		LOGD("LTO : %d\n", m_LinkTimeout);
		break;
	case PL_RW:
		// RWサイズが0の場合はI PDUを受け付けないので、切る
		if(*(pBuf + PDU_INFOPOS) > 0) {
			LOGD("RW : %d\n", *(pBuf + 2));
		} else {
			LOGD("RW == 0\n");
			killConnection();
		}
		break;
	case PL_SN:
		// SNEPするだけなので、無視
#ifdef USE_DEBUG
		{
			uint8_t sn[100];	//そげんなかろう
			hk_memcpy(sn, pBuf + PDU_INFOPOS, *(pBuf + 1));
			sn[*(pBuf + 1) + 1] = '\0';
			LOGD("SN(%s)\n", sn);
		}
#endif	//USE_DEBUG
		break;
	case PL_OPT:
		//SNEPはConnection-orientedのみ
		switch(*(pBuf + PDU_INFOPOS) & 0x03) {
		case 0x00:
			LOGD("OPT(LSC) : unknown\n");
			break;
		case 0x01:
			LOGD("OPT(LSC) : Class 1\n");
			killConnection();
			break;
		case 0x02:
			LOGD("OPT(LSC) : Class 2\n");
			break;
		case 0x03:
			LOGD("OPT(LSC) : Class 3\n");
			break;
		}
		break;
	default:
		LOGE("unknown\n");
		next = 0xff;
		break;
	}
	return next;
}


static void createPdu(uint8_t type)
{
#ifdef USE_DEBUG_PDU
	LOGD("createPdu : %s\n", string_pdu(type));
#endif

	uint8_t ssap;
	uint8_t dsap;
	
	switch(type) {
	case PDU_UI:
	case PDU_CONN:
	case PDU_DISC:
	case PDU_CC:
	case PDU_DM:
	case PDU_FRMR:
	case PDU_I:
	case PDU_RR:
	case PDU_RNR:
		ssap = m_SSAP;
		dsap = m_DSAP;
		break;
	default:
		ssap = 0;
		dsap = 0;
	}
	uint8_t* pCommandBuf = NfcPcd_CommandBuf();
	pCommandBuf[0] = (uint8_t)((dsap << 2) | ((type & 0x0c) >> 2));
	pCommandBuf[1] = (uint8_t)(((type & 0x03) << 6) | ssap);
	
	//とりあえず
	m_LastSentPdu = type;
}


/**
 * 接続を強制的に切る.
 * Resetコマンドまで発行する.
 */
static void killConnection(void)
{
	LOGD("---n");
	LOGD("%s\n", __PRETTY_FUNCTION__);
	HkNfcDep_Close();
	NfcPcd_Reset();
	LOGD("---n");
}


/**
 * 送信データ設定
 *
 * @param[in]	pBuf		送信データ(コピーする)
 * @param[in]	len			送信データサイズ
 * @return		true		データ受け入れ
 * @return		false		データ拒否
 */
static bool addSendData(const void* pBuf, uint8_t len)
{
	LOGD("%s\n", __PRETTY_FUNCTION__);

	if((m_LlcpStat < LSTAT_NOT_CONNECT) || (LSTAT_BUSY < m_LlcpStat)) {
		HkNfcRw_SetLastError(HKNFCERR_LLCP_ADDDT);
		return false;
	}
	if(m_SendLen + len > LLCP_MIU) {
		HkNfcRw_SetLastError(HKNFCERR_LLCP_ADDDT);
		return false;
	}

	hk_memcpy(m_SendBuf + m_SendLen, pBuf, len);
	m_SendLen += len;

	return true;
}


static bool connect(void)
{
	bool b = false;

	//CONNECT前は、まずCONNECTする
	if(m_LlcpStat == LSTAT_NOT_CONNECT) {
#if 0
		m_SSAP = SAP_SNEP;
		m_DSAP = SAP_SNEP;
		m_CommandLen = PDU_INFOPOS;
#else
		m_SSAP = SAP_SNEP;
		m_DSAP = SAP_SDP;
		hk_memcpy(NfcPcd_CommandBuf() + PDU_INFOPOS, SN_SNEP, LEN_SN_SNEP);
		m_CommandLen = PDU_INFOPOS + LEN_SN_SNEP;
#endif
		createPdu(PDU_CONN);

		m_LlcpStat = LSTAT_CONNECTING;
		
		b = true;
	} else {
		HkNfcRw_SetLastError(HKNFCERR_LLCP_CONN);
	}

	return b;
}

#ifdef USE_DEBUG_PDU
static const char* string_pdu(uint8_t type)
{
	switch(type) {
	case PDU_SYMM	:
		return "PDU_SYMM";
	case PDU_PAX	:	
		return "PDU_PAX";
	case PDU_AGF	:	
		return "PDU_AGF";
	case PDU_UI		:
		return "PDU_UI";
	case PDU_CONN	:
		return "PDU_CONN";
	case PDU_DISC	:
		return "PDU_DISC";
	case PDU_CC		:
		return "PDU_CC";
	case PDU_DM		:
		return "PDU_DM";
	case PDU_FRMR	:
		return "PDU_FRMR";
	case PDU_RESV1	:
		return "PDU_RESV1";
	case PDU_RESV2	:
		return "PDU_RESV2";
	case PDU_RESV3	:
		return "PDU_RESV3";
	case PDU_I		:
		return "PDU_I";
	case PDU_RR		:
		return "PDU_RR";
	case PDU_RNR	:	
		return "PDU_RNR";
	case PDU_RESV4	:
		return "PDU_RESV4";
	case PDU_NONE	:
		return "PDU_NONE";
	default:
		return "unknown PDU";
	}
}
#endif

/////////////////////////////////////////////////////////////////////////

#ifdef HKNFCRW_USE_SNEP_INITIATOR
/**
 * @brief	LLCP(Initiator)開始
 * 
 * Initiator LLCPを開始する.
 *
 * @param[in]	mode		開始するDepMode
 * @param[in]	pRecvCb		コールバック関数
 * @return		成功/失敗
 */
bool HkNfcLlcpI_Start(uint8_t mode, void (*pRecvCb)(const void* pBuf, uint8_t len))
{
	LOGD("%s\n", __PRETTY_FUNCTION__);
	
	bool ret = HkNfcDep_StartAsInitiator(mode);
	if(ret) {
		//PDU送信側
		m_SendRecv = SR_SENDER;
		m_LlcpStat = LSTAT_NOT_CONNECT;
		m_pRecvCb = pRecvCb;
	} else {
		HkNfcRw_SetLastError(HKNFCERR_LLCP_START);
		killConnection();
	}

	return ret;
}


/**
 * @brief LLCP(Initiator)終了要求
 * 
 * Initiator LLCPを終了要求する.
 *
 * @return		true:要求受け入れ
 */
bool HkNfcLlcpI_StopRequest(void)
{
	LOGD("%s(%d)\n", __PRETTY_FUNCTION__, m_LlcpStat);
	
	switch(m_LlcpStat) {
	case LSTAT_NOT_CONNECT:
	case LSTAT_CONNECTING:
		LOGD("==>LSTAT_DM\n");
		m_LlcpStat = LSTAT_DM;
		break;
	
	case LSTAT_NONE:
	case LSTAT_TERM:
	case LSTAT_DM:
	case LSTAT_WAIT_DM:
		break;
	
	case LSTAT_NORMAL:
	case LSTAT_BUSY:
		LOGD("==>LSTAT_TERM\n");
		m_LlcpStat = LSTAT_TERM;
		break;
	}
	
	return true;
}



/**
 * @brief	LLCP(Initiator)送信データ追加
 * 
 * Initiator LLCPの送信データを追加する.
 *
 * @param[in]	pBuf	送信データ。最大#LLCP_MIU[byte]
 * @param[in]	len		送信データ長。最大#LLCP_MIU.
 * @retval		true	送信データ受け入れ
 */
bool HkNfcLlcpI_AddSendData(const void* pBuf, uint8_t len)
{
	LOGD("%s(%d)\n", __PRETTY_FUNCTION__, m_LlcpStat);
	
	bool b;
	b = addSendData(pBuf, len);
	
	return b;
}


/**
 * @brief	LLCP(Initiator)データ送信要求
 * 
 * Initiator LLCPのデータ送信を要求する.
 *
 * @retval		true	送信受け入れ
 */
bool HkNfcLlcpI_SendRequest(void)
{
	LOGD("%s(%d)\n", __PRETTY_FUNCTION__, m_LlcpStat);
	
	bool b;
	b = connect();
	
	return b;
}


/**
 * @brief	LLCP(Initiator)定期処理.
 * 
 * Initiator LLCPの定期処理を行う.
 * 開始後、やることがない間は #HkNfcLlcpI_Poll() を呼び出し続けること.
 * 
 * @retval	true	定期処理継続
 * @retval	false	定期処理終了
 */
bool HkNfcLlcpI_Poll(void)
{
	uint8_t* pCommandBuf = NfcPcd_CommandBuf();
	uint8_t* pResponseBuf = NfcPcd_ResponseBuf();

	if(HkNfcDep_GetDepMode() == HKNFCDEPMODE_NONE) {
		return false;
	}

	if(m_SendRecv == SR_SENDER) {
		//PDU送信時
		switch(m_LlcpStat) {
		case LSTAT_NONE:
			//
			break;
		
		case LSTAT_NOT_CONNECT:
			//CONNECT前はSYMMを投げる
			m_CommandLen = PDU_INFOPOS;
			LOGD("*");
			createPdu(PDU_SYMM);
			break;
		
		case LSTAT_CONNECTING:
			//CONNECT or CCはデータ設定済み
			if(m_CommandLen) {
				LOGD("send CONNECT or CC\n");
			}
			break;

		case LSTAT_NORMAL:
			//
			if(m_SendLen) {
				//送信データあり
				LOGD("send I(VR:%d / VS:%d)\n", m_ValueR, m_ValueS);
				m_CommandLen = PDU_INFOPOS + 1 + m_SendLen;
				createPdu(PDU_I);
				pCommandBuf[PDU_INFOPOS] = (uint8_t)((m_ValueS << 4) | m_ValueR);
				hk_memcpy(pCommandBuf + PDU_INFOPOS + 1, m_SendBuf, m_SendLen);
				m_SendLen = 0;
				m_ValueS++;
			} else {
				if(m_PrevRecvI == PRI_RECV) {
					//直前がI PDUだった
					m_CommandLen = PDU_INFOPOS + 1;
					createPdu(PDU_RR);
					pCommandBuf[PDU_INFOPOS] = m_ValueR;		//N(R)
				} else {
					//それ以外
					m_CommandLen = PDU_INFOPOS;
					LOGD("*");
					createPdu(PDU_SYMM);
				}
			}
			break;

		case LSTAT_TERM:
			//切断シーケンス
			m_CommandLen = PDU_INFOPOS;
			createPdu(PDU_DISC);
			LOGD("send DISC\n");
			break;
		
		case LSTAT_DM:
			//切断
			pCommandBuf[PDU_INFOPOS] = pCommandBuf[0];
			m_CommandLen = PDU_INFOPOS + 1;
			createPdu(PDU_DM);
			LOGD("send DM\n");
			break;
		}
		if(m_CommandLen == 0) {
			//SYMMでしのぐ
			m_CommandLen = PDU_INFOPOS;
			createPdu(PDU_SYMM);
			LOGD("*");
		}
		
		uint8_t len;
		hk_start_timer(m_LinkTimeout);
		bool b = HkNfcDep_SendAsInitiator(pCommandBuf, m_CommandLen, pResponseBuf, &len);
		if(m_LlcpStat == LSTAT_DM) {
			//DM送信後は強制終了する
			LOGD("DM sent\n");
			HkNfcDep_StopAsInitiator();
			killConnection();
		} else if(b) {
			if(hk_is_timeout()) {
				//相手から通信が返ってこない
				LOGE("Link timeout\n");
				m_SendRecv = SR_SENDER;
				m_LlcpStat = LSTAT_TERM;
				m_DSAP = SAP_MNG;
				m_SSAP = SAP_MNG;
				HkNfcRw_SetLastError(HKNFCERR_LLCP_TIMEOUT);
			} else {
				if((m_LlcpStat == LSTAT_CONNECTING) && (m_LastSentPdu == PDU_CC)) {
					m_LlcpStat = LSTAT_NORMAL;
				}

				//受信は済んでいるので、次はPDU送信側になる
				m_SendRecv = SR_SENDER;
				m_CommandLen = 0;

				uint8_t type;
				uint8_t pdu = analyzePdu(pResponseBuf, len, &type);
			}
		} else {
			LOGE("error\n");
			//もうだめ
			killConnection();
			HkNfcRw_SetLastError(HKNFCERR_LLCP_ERR);
		}
	}
	
	return true;
}

#endif	//HKNFCRW_USE_SNEP_INITIATOR


//////////////////////////////////////////////////////////////////////

#ifdef HKNFCRW_USE_SNEP_TARGET
/**
 * @brief	LLCP(Target)開始.
 * 
 * Target LLCPを開始する.
 * 呼び出すと、Initiatorから要求が来るまで戻らない.
 * 成功すると、ATR_RESの返信まで終わっている.
 *
 * @retval	true	開始成功
 * @retval	false	開始失敗
 */
bool HkNfcLlcpT_Start(void (*pRecvCb)(const void* pBuf, uint8_t len))
{
	LOGD("%s\n", __PRETTY_FUNCTION__);
	
	bool ret = HkNfcDep_StartAsTarget();
	if(ret) {
		LOGD("%s -- success\n", __PRETTY_FUNCTION__);
		
		//PDU受信側
		m_SendRecv = SR_RECEIVER;
		m_LlcpStat = LSTAT_NOT_CONNECT;
		m_pRecvCb = pRecvCb;

		hk_start_timer(m_LinkTimeout);
	} else {
		LOGE("-- fail\n");
		HkNfcRw_SetLastError(HKNFCERR_LLCP_START);
	}
	
	return ret;
}


/**
 * @brief	LLCP(Target)終了要求
 *
 * Target LLCPの終了要求を行う.
 * 
 * @return		true:要求受け入れ
 */
bool HkNfcLlcpT_StopRequest(void)
{
	LOGD("%s(%d)\n", __PRETTY_FUNCTION__, m_LlcpStat);
	
	switch(m_LlcpStat) {
	case LSTAT_NOT_CONNECT:
	case LSTAT_CONNECTING:
		LOGD("==>LSTAT_DM\n");
		m_LlcpStat = LSTAT_DM;
		break;
	
	case LSTAT_NONE:
	case LSTAT_TERM:
	case LSTAT_DM:
	case LSTAT_WAIT_DM:
		break;
	
	case LSTAT_NORMAL:
	case LSTAT_BUSY:
		LOGD("==>LSTAT_TERM\n");
		m_LlcpStat = LSTAT_TERM;
		break;
	}
	
	return true;
}


/**
 * @brief	LLCP(Target)送信データ追加
 * 
 * Target LLCPの送信データを追加する.
 *
 * @param[in]	pBuf	送信データ。最大#LLCP_MIU[byte]
 * @param[in]	len		送信データ長。最大#LLCP_MIU.
 * @retval		true	送信データ受け入れ
 */
bool HkNfcLlcpT_AddSendData(const void* pBuf, uint8_t len)
{
	LOGD("%s(%d)\n", __PRETTY_FUNCTION__, m_LlcpStat);
	
	bool b;
	b = addSendData(pBuf, len);
	
	return b;
}


/**
 * @brief	LLCP(Target)データ送信要求
 * 
 * Target LLCPのデータ送信を要求する.
 *
 * @retval		true	送信受け入れ
 */
bool HkNfcLlcpT_SendRequest(void)
{
	LOGD("%s(%d)\n", __PRETTY_FUNCTION__, m_LlcpStat);
	
	bool b;
	b = connect();
	
	return b;
}


/**
 * @brief	LLCP(Target)定期処理.
 * 
 * Target LLCPの定期処理を行う.
 * 開始後、やることがない間は #HkNfcLlcpT_Poll() を呼び出し続けること.
 * 
 * @retval	true	定期処理継続
 * @retval	false	定期処理終了
 */
bool HkNfcLlcpT_Poll(void)
{
	uint8_t* pCommandBuf = NfcPcd_CommandBuf();
	uint8_t* pResponseBuf = NfcPcd_ResponseBuf();

	if(HkNfcDep_GetDepMode() == HKNFCDEPMODE_NONE) {
		return false;
	}

	if(m_SendRecv == SR_RECEIVER) {
		//PDU受信側
		uint8_t len;
		bool b = HkNfcDep_RecvAsTarget(pResponseBuf, &len);
		if(hk_is_timeout()) {
			//相手から通信が返ってこない
			LOGE("Link timeout\n");
			m_SendRecv = SR_SENDER;
			m_LlcpStat = LSTAT_TERM;
			m_DSAP = SAP_MNG;
			m_SSAP = SAP_MNG;
			HkNfcRw_SetLastError(HKNFCERR_LLCP_TIMEOUT);
		} else if(b) {
			uint8_t type;
			uint8_t pdu = analyzePdu(pResponseBuf, len, &type);
			//PDU送信側になる
			m_SendRecv = SR_SENDER;
		} else {
			//もうだめだろう
			LOGE("recv error\n");
			killConnection();
			HkNfcRw_SetLastError(HKNFCERR_LLCP_ERR);
		}
	} else {
		//PDU送信側
		uint8_t len;
		switch(m_LlcpStat) {
		case LSTAT_NONE:
			//
			break;
		
		case LSTAT_NOT_CONNECT:
			//CONNECT前はSYMMを投げる
			m_CommandLen = PDU_INFOPOS;
			createPdu(PDU_SYMM);
			//LOGD("send SYMM\n");
			LOGD("*");
			break;
		
		case LSTAT_CONNECTING:
			//CONNECT or CCはデータ設定済み
			if(m_CommandLen) {
				LOGD("send CONNECT or CC\n");
			}
			break;

		case LSTAT_NORMAL:
			//
			if(m_SendLen) {
				//送信データあり
				LOGD("send I(VR:%d / VS:%d)\n", m_ValueR, m_ValueS);
				m_CommandLen = PDU_INFOPOS + 1 + m_SendLen;
				createPdu(PDU_I);
				pCommandBuf[PDU_INFOPOS] = (uint8_t)((m_ValueS << 4) | m_ValueR);
				hk_memcpy(pCommandBuf + PDU_INFOPOS + 1, m_SendBuf, m_SendLen);
				m_SendLen = 0;
				m_ValueS++;
			} else {
				if(m_PrevRecvI == PRI_RECV) {
					//直前がI PDUだった
					m_CommandLen = PDU_INFOPOS + 1;
					createPdu(PDU_RR);
					pCommandBuf[PDU_INFOPOS] = m_ValueR;		//N(R)
				} else {
					//それ以外
					m_CommandLen = PDU_INFOPOS;
					LOGD("*");
					createPdu(PDU_SYMM);
				}
			}
			break;

		case LSTAT_TERM:
			//切断シーケンス
			LOGD("send DISC\n");
			m_CommandLen = PDU_INFOPOS;
			createPdu(PDU_DISC);
			break;
		
		case LSTAT_DM:
			//切断
			LOGD("send DM\n");
			pCommandBuf[PDU_INFOPOS] = pCommandBuf[0];
			m_CommandLen = PDU_INFOPOS + 1;
			createPdu(PDU_DM);
			break;
		}
		if(m_CommandLen == 0) {
			//SYMMでしのぐ
			m_CommandLen = PDU_INFOPOS;
			createPdu(PDU_SYMM);
		}
		bool b = HkNfcDep_RespAsTarget(pCommandBuf, m_CommandLen);
		if(m_LlcpStat == LSTAT_DM) {
			//DM送信後は強制終了する
			LOGD("DM sent\n");
			killConnection();
		} else if(b) {
			if(m_LlcpStat == LSTAT_TERM) {
				//DISC送信後
				if((m_DSAP == 0) && (m_SSAP == 0)) {
					LOGD("fin : Link Deactivation\n");
					killConnection();
				} else {
					LOGD("wait DM\n");
					m_LlcpStat = LSTAT_WAIT_DM;

					//PDU受信側になる
					m_SendRecv = SR_RECEIVER;
					m_CommandLen = 0;
					
					hk_start_timer(m_LinkTimeout);
				}
			} else {
				if((m_LlcpStat == LSTAT_CONNECTING) && (m_LastSentPdu == PDU_CC)) {
					m_LlcpStat = LSTAT_NORMAL;
				}
				//PDU受信側になる
				m_SendRecv = SR_RECEIVER;
				m_CommandLen = 0;
				
				hk_start_timer(m_LinkTimeout);
			}
		} else {
			LOGE("send error\n");
			//もうだめ
			killConnection();
			HkNfcRw_SetLastError(HKNFCERR_LLCP_ERR);
		}
	}
	
	return true;
}
#endif	//HKNFCRW_USE_SNEP_TARGET
