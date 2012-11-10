/*
 * @file	HkNfcSnep.c
 * @brief	SNEP実装
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
#include "HkNfcSnep.h"
#include "HkNfcLlcp.h"
#include "HkNfcRw.h"
#include "HkNfcRwIn.h"
#include "nfclog.h"




#define ST_INIT				((uint8_t)0)	/**< 初期状態(要求受付可能) */
#define ST_START_PUT		((uint8_t)1)
#define ST_PUT				((uint8_t)2)
#define ST_PUT_RESPONSE		((uint8_t)3)
#define ST_START_GET		((uint8_t)4)
#define ST_GET				((uint8_t)5)
#define ST_GET_RESPONSE		((uint8_t)6)
#define ST_SUCCESS			((uint8_t)7)
#define ST_FAIL				((uint8_t)8)
#define ST_ABORT			((uint8_t)9)

#define SNEP_VERSION		((uint8_t)0x10)		/* SNEP version(1.0) */

#define REQ_CONTINUE	((uint8_t)0x00)		/* Continue */
#define REQ_GET			((uint8_t)0x01)		/* Get */
#define REQ_PUT			((uint8_t)0x02)		/* Put */
#define REQ_REJECT		((uint8_t)0x7f)		/* Reject */

#define RES_CONTINUE	((uint8_t)0x80)		/* Continue */
#define RES_SUCCESS		((uint8_t)0x81)		/* Success */


/* [NFC-A]LLCP動作モード */
#ifndef HKNFCLLCP_DEFAULT_NFCA
#define HKNFCLLCP_DEFAULT_NFCA HKNFCLLCPMODE_ACT_106K
#endif

/* [NFC-F]LLCP動作モード */
#ifndef HKNFCLLCP_DEFAULT_NFCF
#define HKNFCLLCP_DEFAULT_NFCF HKNFCLLCPMODE_ACT_424K
#endif



#ifdef HKNFCRW_USE_SNEP_INITIATOR
static bool pollI(void);
#endif	/* HKNFCRW_USE_SNEP_INITIATOR */

#ifdef HKNFCRW_USE_SNEP_TARGET
static bool pollT(void);
#endif	/* HKNFCRW_USE_SNEP_TARGET */

static void recvCb(const void* pBuf, uint8_t len);


static const HkNfcNdefMsg*	m_pMessage = 0;
static HkNfcSnepMode		m_Mode = HKNFCSNEP_MD_TARGET;
#ifdef HKNFCRW_USE_SNEP_SERVER
static HkNfcNdefMsg* m_pSvrMsg = 0;
#endif	/* HKNFCRW_USE_SNEP_SERVER */

/*
 * 状態(ST_xxx)
 */
static uint8_t				m_Status = ST_INIT;
static bool (*m_PollFunc)(void);


//////////////////////////////////////////////////////////////////////////

/*******************************************************************
 * SNEP PUT開始.
 * NDEFメッセージはPUT完了まで保持するので、解放したり書き換えたりしないこと.
 *
 * @param[in]	Mode	モード
 * @param[in]	pMsg	NDEFメッセージ(送信が終わるまで保持する)
 * @return		開始成功/失敗
 * @note		- #HKNFCSNEP_MD_INITIATORの場合、最後にPollingしたTechnologyでDEPする.
 * @attention	- Initiatorの場合、TargetをPollingしておくこと.
 *******************************************************************/
bool HkNfcSnep_PutStart(HkNfcSnepMode Mode, const HkNfcNdefMsg* pMsg)
{
	bool ret = true;

	if((m_Status != ST_INIT) || (pMsg == 0) || (m_pMessage != 0)) {
		LOGE("bad param\n");
		return false;
	}

	if(Mode == HKNFCSNEP_MD_INITIATOR) {
		// Initiatorとして動作
#ifdef HKNFCRW_USE_SNEP_INITIATOR
		HkNfcType type = HkNfcRw_GetType();
		if((type == HKNFCTYPE_A) || (type == HKNFCTYPE_F)) {
			/* ポーリングの結果がNFC-AかNFC-Fだけ */
			m_PollFunc = pollI;
		} else {
			HkNfcRw_SetLastError(HKNFCERR_SNEP_PUT);
			ret = false;
		}
#else
		/* 未サポート */
		HkNfcRw_SetLastError(HKNFCERR_SNEP_NOTSUP);
		ret = false;
#endif	//HKNFCRW_USE_SNEP_INITIATOR
	} else {
		// Targetとして動作
#ifdef HKNFCRW_USE_SNEP_TARGET
		Mode = HKNFCSNEP_MD_TARGET;
		m_PollFunc = pollT;
#else
		/* 未サポート */
		HkNfcRw_SetLastError(HKNFCERR_SNEP_NOTSUP);
		ret = false;
#endif	//HKNFCRW_USE_SNEP_TARGET
	}

	if(ret) {
		m_Status = ST_START_PUT;
		m_pMessage = pMsg;
	}
	
	return ret;
}


#ifdef HKNFCRW_USE_SNEP_SERVER
/*******************************************************************
 * SNEP PUT開始.
 * NDEFメッセージはPUT完了まで保持するので、解放したり書き換えたりしないこと.
 *
 * @param[in]	Mode	モード
 * @param[out]	pMsg	NDEFメッセージ(送信が終わるまで保持する)
 * @return		開始成功/失敗
 * @note		- #HKNFCSNEP_MD_INITIATORの場合、最後にPollingしたTechnologyでDEPする.
 * @attention	- Initiatorの場合、TargetをPollingしておくこと.
 *******************************************************************/
bool HkNfcSnep_PutServer(HkNfcSnepMode Mode, HkNfcNdefMsg* pMsg)
{
	bool ret = true;

	if((m_Status != ST_INIT) || (pMsg == 0) || (m_pMessage != 0)) {
		LOGE("bad param\n");
		return false;
	}

	if(Mode == HKNFCSNEP_MD_INITIATOR) {
		// Initiatorとして動作
#ifdef HKNFCRW_USE_SNEP_INITIATOR
		HkNfcType type = HkNfcRw_GetType();
		if((type == HKNFCTYPE_A) || (type == HKNFCTYPE_F)) {
			/* ポーリングの結果がNFC-AかNFC-Fだけ */
			m_PollFunc = pollI;
		} else {
			HkNfcRw_SetLastError(HKNFCERR_SNEP_PUT);
			ret = false;
		}
#else
		/* 未サポート */
		HkNfcRw_SetLastError(HKNFCERR_SNEP_NOTSUP);
		ret = false;
#endif	//HKNFCRW_USE_SNEP_INITIATOR
	} else {
		// Targetとして動作
#ifdef HKNFCRW_USE_SNEP_TARGET
		Mode = HKNFCSNEP_MD_TARGET;
		m_PollFunc = pollT;
#else
		/* 未サポート */
		HkNfcRw_SetLastError(HKNFCERR_SNEP_NOTSUP);
		ret = false;
#endif	//HKNFCRW_USE_SNEP_TARGET
	}

	if(ret) {
		m_Status = ST_START_PUT;
		m_pSvrMsg = pMsg;
	}
	
	return ret;
}
#endif	/* HKNFCRW_USE_SNEP_SERVER */


/*******************************************************************
 * 定期処理
 * 
 * @return	true:残処理あり。続けてHkNfcSnep_Poll()を呼び出すこと。
 *******************************************************************/
bool HkNfcSnep_Poll(void)
{
	return (*m_PollFunc)();
}


/*******************************************************************
 * 停止
 * 
 * SNEP中であっても、処理を停止させる。
 *******************************************************************/
void HkNfcSnep_Stop(void)
{
	LOGD("%s\n", __PRETTY_FUNCTION__);

	HkNfcLlcp_Close();
	
	m_Status = ST_INIT;
	m_pMessage = 0;
}


/*******************************************************************
 * SNEP実行結果取得
 *
 * @return	HKNFCSNEP_xxx
 *******************************************************************/
HkNfcSnepRet HkNfcSnep_GetResult(void)
{
	switch(m_Status) {
	case ST_INIT:
	case ST_SUCCESS:
		return HKNFCSNEP_SUCCESS;
	
	case ST_FAIL:
	case ST_ABORT:
		return HKNFCSNEP_FAIL;
	
	default:
		return HKNFCSNEP_PROCESSING;
	}
}


//////////////////////////////////////////////////////////////////////////
#ifdef HKNFCRW_USE_SNEP_INITIATOR

/*******************************************************************
 * ポーリング動作(Initiator)
 *******************************************************************/
static bool pollI(void)
{
	bool ret = true;
	bool b;
	HkNfcType type;
	HkNfcLlcpMode mode;

	switch(m_Status) {
	case ST_START_PUT:
		/* PUT開始 */
		b = true;
		type = HkNfcRw_GetType();
		switch(type) {
		case HKNFCTYPE_A:
			mode = HKNFCLLCP_DEFAULT_NFCA;
			break;
		case HKNFCTYPE_F:
			mode = HKNFCLLCP_DEFAULT_NFCF;
			break;
		default:
			/* こうはならないはず */
			m_Status = ST_ABORT;
			b = false;
			HkNfcRw_SetLastError(HKNFCERR_SNEP_ERR);
			break;
		}
		/* LLCP開始 */
		b = b && HkNfcLlcp_StartAsIn(mode, recvCb);
		if(b && m_pMessage) {
			uint8_t snep_head[6];
			snep_head[0] = SNEP_VERSION;
			snep_head[1] = REQ_PUT;	//PUT
			snep_head[2] = (uint8_t)((m_pMessage->Length) >> 24);
			snep_head[3] = (uint8_t)((m_pMessage->Length) >> 16);
			snep_head[4] = (uint8_t)((m_pMessage->Length) >> 8);
			snep_head[5] = (uint8_t)( m_pMessage->Length);
			b = HkNfcLlcp_AddSendData(snep_head, sizeof(snep_head));
			b = b && HkNfcLlcp_AddSendData(m_pMessage->Data, m_pMessage->Length);
		} else {
			/*
			 * m_pMessageが0の場合は、SYMMをやりとりするだけになる(テスト用)。
			 * 特にエラーとはしない。
			 */
		}
		if(b) {
			if(m_pMessage) {
				LOGD("==>pollI : ST_START_PUT==>ST_PUT\n");
				m_Status = ST_PUT;
			} else {
				LOGD("==>pollI[test] : ST_START_PUT==>ST_PUT_RESPONSE\n");
				m_Status = ST_PUT_RESPONSE;
			}
		} else {
			LOGE("==>pollI : ST_START_PUT==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_PUT:
		/* PUT開始 */
		b = HkNfcLlcp_SendRequest();
		if(b) {
			LOGD("==>pollI : ST_PUT==>ST_PUT_RESPONSE\n");
			m_Status = ST_PUT_RESPONSE;
		} else {
			LOGE("==>pollI : ST_PUT==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_PUT_RESPONSE:
		b = HkNfcLlcp_PollAsIn();
		if(!b) {
			LOGE("==>pollI : ST_PUT_RESPONSE==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_SUCCESS:
	case ST_FAIL:
		ret = HkNfcLlcp_PollAsIn();
		break;
	
	case ST_ABORT:
	default:
		ret = false;
		break;
	}

	return ret;
}
#endif	//HKNFCRW_USE_SNEP_INITIATOR


//////////////////////////////////////////////////////////////////////////
#ifdef HKNFCRW_USE_SNEP_TARGET

/*******************************************************************
 * ポーリング動作(Target)
 *******************************************************************/
static bool pollT(void)
{
	bool ret = true;
	bool b;

	switch(m_Status) {
	case ST_START_PUT:
		b = HkNfcLlcp_StartAsTg(recvCb);
		if(b && m_pMessage) {
			uint8_t snep_head[6];
			snep_head[0] = SNEP_VERSION;
			snep_head[1] = REQ_PUT;	//PUT
			snep_head[2] = (uint8_t)((m_pMessage->Length) >> 24);
			snep_head[3] = (uint8_t)((m_pMessage->Length) >> 16);
			snep_head[4] = (uint8_t)((m_pMessage->Length) >> 8);
			snep_head[5] = (uint8_t)(m_pMessage->Length);
			b = HkNfcLlcp_AddSendData(snep_head, sizeof(snep_head));
			b = b && HkNfcLlcp_AddSendData(m_pMessage->Data, m_pMessage->Length);
		}
		if(b) {
			if(m_pMessage) {
				LOGD("==>pollT : ST_START_PUT==>ST_PUT\n");
				m_Status = ST_PUT;
			} else {
				LOGD("==>pollT[test] : ST_START_PUT==>ST_PUT_RESPONSE\n");
				m_Status = ST_PUT_RESPONSE;
			}
		} else {
			LOGE("==>pollT : ST_START_PUT==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_PUT:
		b = HkNfcLlcp_SendRequest();
		if(b) {
			m_Status = ST_PUT_RESPONSE;
		} else {
			LOGE("==>pollT : ST_PUT==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_PUT_RESPONSE:
		b = HkNfcLlcp_PollAsTg();
		if(!b) {
			LOGE("==>pollT : ST_PUT_RESPONSE==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_SUCCESS:
	case ST_FAIL:
		ret = HkNfcLlcp_PollAsTg();
		break;
	
	case ST_ABORT:
	default:
		ret = false;
		break;
	}

	return ret;
}
#endif	//HKNFCRW_USE_SNEP_TARGET


/*******************************************************************
 * LLCP I PDU受信通知
 * 
 * @param[in]	pBuf	受信データ
 * @param[in]	len		受信サイズ
 *******************************************************************/
static void recvCb(const void* pBuf, uint8_t len)
{
	const uint8_t* pData = (const uint8_t*)pBuf;

	switch(m_Status) {
	case ST_PUT_RESPONSE:
		//PUT後の応答
		if(m_pMessage) {
			/* SNEP PUTデータあり */
			if(pData[1] == RES_SUCCESS) {
				LOGD("==>recvCb : ST_PUT_RESPONSE==>ST_SUCCESS\n");
				m_Status = ST_SUCCESS;
			} else {
				LOGE("==>recvCb : ST_PUT_RESPONSE==>ST_FAIL\n");
				m_Status = ST_FAIL;
			}
		} else {
#ifdef HKNFCRW_USE_SNEP_SERVER
			/* SNEP PUTデータなし→SNEPサーバになって受信した */
			if(pData[1] == REQ_PUT) {
				/* Put */
				uint32_t len = ((((uint32_t)pData[2]) << 24)
								| (((uint32_t)pData[3]) << 16)
								| (((uint32_t)pData[4]) << 8)
								| ((uint32_t)pData[5]));
				if(len < 0x100) {
					LOGD("==>recvCb : ST_PUT_RESPONSE==>ST_SUCCESS(svr)\n");
					m_Status = ST_SUCCESS;
					m_pSvrMsg->Length = (uint8_t)len;
					hk_memcpy(m_pSvrMsg->Data, &pData[6], len);
				} else {
					LOGE("too large...\n");
				}
			} else {
				/* 今の私には受けとることができない */
				m_Status = ST_FAIL;
			}
#else	/* HKNFCRW_USE_SNEP_SERVER */
			LOGE("==>recvCb : ST_PUT_RESPONSE==>ST_ABORT(not support)\n");
			m_Status = ST_ABORT;
#endif	/* HKNFCRW_USE_SNEP_SERVER */
		}

		//いずれにせよ、終わらせる
		HkNfcLlcp_StopRequest();
		break;
		
	default:
		LOGE("==>recvCb : ST_PUT_RESPONSE==>ST_ABORT\n");
		m_Status = ST_ABORT;
		break;
	}
}
