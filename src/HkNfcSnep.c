/*
 * @file	HkNfcSnep.c
 * @brief	SNEP実装
 */
#include "HkNfcSnep.h"
#include "HkNfcLlcp.h"
#include "HkNfcRw.h"


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
 */
bool HkNfcSnep_PutStart(HkNfcSnepMode Mode, const HkNfcNdefMsg* pMsg)
{
	if((m_pMessage != 0) || (pMsg == 0)) {
		return false;
	}
	m_Status = ST_START_PUT;
	m_pMessage = pMsg;

	if(Mode == HKNFCSNEP_MD_INITIATOR) {
		// Initiatorとして動作
#ifdef USE_SNEP_INITIATOR
		HkNfcType type = HkNfcRw_GetType();
		if((type != HKNFCTYPE_A) && (type != HKNFCTYPE_F)) {
			/* ポーリングの結果がNFC-AかNFC-Fだけ */
			return false;
		}
		m_PollFunc = pollI;
#else
		return false;
#endif	//USE_SNEP_INITIATOR
	} else {
		// Targetとして動作
#ifdef USE_SNEP_TARGET
		Mode = HKNFCSNEP_MD_TARGET;
		m_PollFunc = pollT;
#else
		return false;
#endif	//USE_SNEP_TARGET
	}

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


//////////////////////////////////////////////////////////////////////////
#ifdef USE_SNEP_INITIATOR
static bool pollI(void)
{
	bool b = false;
	HkNfcType type;
	HkNfcDepMode mode;

	switch(m_Status) {
	case ST_START_PUT:
		type = HkNfcRw_GetType();
		switch(type) {
		case HKNFCTYPE_A:
			mode = HKNFCDEPMODE_ACT_106K;
			break;
		case HKNFCTYPE_F:
			mode = HKNFCDEPMODE_ACT_424K;
			break;
		default:
			/* こうはならないはず */
			m_Status = ST_ABORT;
			return false;
		} 
		b = HkNfcLlcpI_Start(mode, recvCbI);
		if(b) {
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
			m_Status = ST_PUT;
		} else {
			m_Status = ST_ABORT;
			HkNfcLlcpI_StopRequest();
		}
		break;

	case ST_PUT:
		b = HkNfcLlcpI_SendRequest();
		if(b) {
			m_Status = ST_PUT_RESPONSE;
		} else {
			m_Status = ST_ABORT;
			HkNfcLlcpI_StopRequest();
		}
		break;

	case ST_PUT_RESPONSE:
	case ST_SUCCESS:
	case ST_ABORT:
		b = HkNfcLlcpI_Poll();
		break;
	}

	return b;
}


static void recvCbI(const void* pBuf, uint8_t len)
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
		HkNfcLlcpI_StopRequest();
		break;
	}
}
#endif	//USE_SNEP_INITIATOR


//////////////////////////////////////////////////////////////////////////
#ifdef USE_SNEP_TARGET
static bool pollT(void)
{
	bool b = false;

	switch(m_Status) {
	case ST_START_PUT:
		b = HkNfcLlcpT_Start(recvCbT);
		if(b) {
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
			m_Status = ST_PUT;
		} else {
			m_Status = ST_ABORT;
			HkNfcLlcpT_StopRequest();
		}
		break;

	case ST_PUT:
		b = HkNfcLlcpT_SendRequest();
		if(b) {
			m_Status = ST_PUT_RESPONSE;
		} else {
			m_Status = ST_ABORT;
			HkNfcLlcpT_StopRequest();
		}
		break;
	case ST_PUT_RESPONSE:
	case ST_SUCCESS:
	case ST_ABORT:
		b = HkNfcLlcpT_Poll();
		break;
	}

	return b;
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
		HkNfcLlcpT_StopRequest();
		break;
	}
}
#endif	//USE_SNEP_TARGET
