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


#define SNEP_SUCCESS		((uint8_t)0x81)


static bool pollI(void);
static void recvCbI(const void* pBuf, uint8_t len);

static bool pollT(void);
static void recvCbT(const void* pBuf, uint8_t len);


#define ST_INIT				((uint8_t)0)

#define ST_START_PUT		((uint8_t)1)
#define ST_PUT				((uint8_t)2)
#define ST_PUT_RESPONSE		((uint8_t)3)

#define ST_START_GET		((uint8_t)4)
#define ST_GET				((uint8_t)5)
#define ST_GET_RESPONSE		((uint8_t)6)

#define ST_SUCCESS			((uint8_t)7)
#define ST_ABORT			((uint8_t)8)


static const HkNfcNdefMsg*	m_pMessage = 0;
static HkNfcSnepMode		m_Mode = HKNFCSNEP_MD_TARGET;
static uint8_t				m_Status = ST_INIT;
static bool (*m_PollFunc)(void);


//////////////////////////////////////////////////////////////////////////
/**
 * SNEP実行結果取得
 *
 * @return	HKNFCSNEP_xxx
 */
uint8_t HkNfcSnep_GetResult(void)
{
	switch(m_Status) {
	case ST_INIT:
	case ST_SUCCESS:
		return HKNFCSNEP_SUCCESS;
	
	case ST_ABORT:
		return HKNFCSNEP_FAIL;
	
	default:
		return HKNFCSNEP_PROCESSING;
	}
}


/**
 * SNEP PUT開始.
 * NDEFメッセージはPUT完了まで保持するので、解放したり書き換えたりしないこと.
 *
 * @param[in]	Mode	モード
 * @param[in]	pMsg	NDEFメッセージ(送信が終わるまで保持する)
 * @return		開始成功/失敗
 * @note		- #HKNFCSNEP_MD_INITIATORの場合、最後にPollingしたTechnologyでDEPする.
 * @attention	- Initiatorの場合、TargetをPollingしておくこと.
 */
bool HkNfcSnep_PutStart(HkNfcSnepMode Mode, const HkNfcNdefMsg* pMsg)
{
	if(m_pMessage != 0) {
		return false;
	}
	m_Status = ST_START_PUT;

	if(Mode == HKNFCSNEP_MD_INITIATOR) {
		// Initiatorとして動作
#ifdef HKNFCRW_USE_SNEP_INITIATOR
		HkNfcType type = HkNfcRw_GetType();
		if((type != HKNFCTYPE_A) && (type != HKNFCTYPE_F)) {
			/* ポーリングの結果がNFC-AかNFC-Fだけ */
			HkNfcRw_SetLastError(HKNFCERR_SNEP_PUT);
			return false;
		}
		m_PollFunc = pollI;
#else
		HkNfcRw_SetLastError(HKNFCERR_SNEP_NOTSUP);
		return false;
#endif	//HKNFCRW_USE_SNEP_INITIATOR
	} else {
		// Targetとして動作
#ifdef HKNFCRW_USE_SNEP_TARGET
		Mode = HKNFCSNEP_MD_TARGET;
		m_PollFunc = pollT;
#else
		HkNfcRw_SetLastError(HKNFCERR_SNEP_NOTSUP);
		return false;
#endif	//HKNFCRW_USE_SNEP_TARGET
	}

	m_pMessage = pMsg;
	return true;
}


/**
 * 定期処理
 * 
 * @return	true:残処理あり。続けてHkNfcSnep_Poll()を呼び出すこと。
 */
bool HkNfcSnep_Poll(void)
{
	return (*m_PollFunc)();
}


void HkNfcSnepStop(void)
{
	LOGD("%s\n", __PRETTY_FUNCTION__);

	if(m_Mode == HKNFCSNEP_MD_INITIATOR) {
		HkNfcDep_StopAsInitiator();
	} else {
	}
	HkNfcDep_Close();
	HkNfcRw_SetLastError(HKNFCERR_NONE);
}

//////////////////////////////////////////////////////////////////////////
#ifdef HKNFCRW_USE_SNEP_INITIATOR
static bool pollI(void)
{
	bool b;
	HkNfcType type;
	HkNfcDepMode mode;

	switch(m_Status) {
	case ST_START_PUT:
		b = true;
		type = HkNfcRw_GetType();
		switch(type) {
		case HKNFCTYPE_A:
			mode = HKNFCDEPMODE_ACT_106K;
			break;
		case HKNFCTYPE_F:
			mode = HKNFCDEPMODE_ACT_424K;
			//mode = HKNFCDEPMODE_PSV_424K;
			break;
		default:
			/* こうはならないはず */
			m_Status = ST_ABORT;
			b = false;
			HkNfcRw_SetLastError(HKNFCERR_SNEP_ERR);
			break;
		} 
		b = b && HkNfcLlcpI_Start(mode, recvCbI);
		if(b && m_pMessage) {
			uint8_t snep_head[6];
			snep_head[0] = 0x10;
			snep_head[1] = 0x02;	//PUT
			snep_head[2] = (uint8_t)((m_pMessage->Length) >> 24);
			snep_head[3] = (uint8_t)((m_pMessage->Length) >> 16);
			snep_head[4] = (uint8_t)((m_pMessage->Length) >> 8);
			snep_head[5] = (uint8_t)(m_pMessage->Length);
			b = HkNfcLlcpI_AddSendData(snep_head, sizeof(snep_head));
			b = b && HkNfcLlcpI_AddSendData(m_pMessage->Data, m_pMessage->Length);
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
			LOGD("==>pollI : ST_START_PUT==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_PUT:
		b = HkNfcLlcpI_SendRequest();
		if(b) {
			LOGD("==>pollI : ST_PUT==>ST_PUT_RESPONSE\n");
			m_Status = ST_PUT_RESPONSE;
		} else {
			LOGD("==>pollI : ST_PUT==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_PUT_RESPONSE:
		b = HkNfcLlcpI_Poll();
		if(b) {
			LOGD("==>pollI : ST_PUT_RESPONSE\n");
		} else {
			LOGD("==>pollI : ST_PUT_RESPONSE==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_SUCCESS:
	case ST_ABORT:
		LOGD("==>pollI : %02x\n", m_Status);
		m_pMessage = 0;
		HkNfcLlcpI_StopRequest();
		return false;
	}

	return true;
}


static void recvCbI(const void* pBuf, uint8_t len)
{
	const uint8_t* pData = (const uint8_t*)pBuf;

	switch(m_Status) {
	case ST_PUT_RESPONSE:
		//PUT後の応答
		if(pData[1] == SNEP_SUCCESS) {
			m_Status = ST_SUCCESS;
			LOGD("==>recvCbI : ST_PUT_RESPONSE==>ST_SUCCESS\n");
		} else {
			m_Status = ST_ABORT;
			LOGD("==>recvCbI : ST_PUT_RESPONSE==>abort\n");
		}
		break;
	}
}
#endif	//HKNFCRW_USE_SNEP_INITIATOR


//////////////////////////////////////////////////////////////////////////
#ifdef HKNFCRW_USE_SNEP_TARGET
static bool pollT(void)
{
	bool b;

	switch(m_Status) {
	case ST_START_PUT:
		b = HkNfcLlcpT_Start(recvCbT);
		if(b && m_pMessage) {
			uint8_t snep_head[6];
			snep_head[0] = 0x10;
			snep_head[1] = 0x02;	//PUT
			snep_head[2] = (uint8_t)((m_pMessage->Length) >> 24);
			snep_head[3] = (uint8_t)((m_pMessage->Length) >> 16);
			snep_head[4] = (uint8_t)((m_pMessage->Length) >> 8);
			snep_head[5] = (uint8_t)(m_pMessage->Length);
			b = HkNfcLlcpT_AddSendData(snep_head, sizeof(snep_head));
			b = b && HkNfcLlcpT_AddSendData(m_pMessage->Data, m_pMessage->Length);
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
			LOGD("==>pollT : ST_START_PUT==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_PUT:
		b = HkNfcLlcpT_SendRequest();
		if(b) {
			m_Status = ST_PUT_RESPONSE;
		} else {
			LOGD("==>pollT : ST_PUT==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_PUT_RESPONSE:
		b = HkNfcLlcpT_Poll();
		if(!b) {
			LOGD("==>pollT : ST_PUT_RESPONSE==>abort\n");
			m_Status = ST_ABORT;
		}
		break;

	case ST_SUCCESS:
	case ST_ABORT:
		LOGD("==>pollT : %02x\n", m_Status);
		m_pMessage = 0;
		HkNfcLlcpT_StopRequest();
		return false;
	}

	return true;
}


static void recvCbT(const void* pBuf, uint8_t len)
{
	const uint8_t* pData = (const uint8_t*)pBuf;

	switch(m_Status) {
	case ST_PUT_RESPONSE:
		//PUT後の応答
		if(pData[1] == SNEP_SUCCESS) {
			m_Status = ST_SUCCESS;
		} else {
			m_Status = ST_ABORT;
		}
		break;
	}
}
#endif	//HKNFCRW_USE_SNEP_TARGET
