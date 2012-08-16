/**
 * @file	HkNfcLlcp.h
 * @brief	LLCPヘッダ
 */
#ifndef HK_NFCLLCP_H
#define HK_NFCLLCP_H

#include <stdint.h>
#include <stdbool.h>
#include "HkNfcDepMode.h"

bool HkNfcLlcpI_Start(HkNfcDepMode mode, void (*pRecvCb)(const void* pBuf, uint8_t len));
bool HkNfcLlcpI_StopRequest(void);
bool HkNfcLlcpI_AddSendData(const void* pBuf, uint8_t len);
bool HkNfcLlcpI_SendRequest(void);
bool HkNfcLlcpI_Poll(void);

bool HkNfcLlcpT_Start(void (*pRecvCb)(const void* pBuf, uint8_t len));
bool HkNfcLlcpT_StopRequest(void);
bool HkNfcLlcpT_AddSendData(const void* pBuf, uint8_t len);
bool HkNfcLlcpT_SendRequest(void);
bool HkNfcLlcpT_Poll(void);

#endif /* HK_NFCLLCP_H */
