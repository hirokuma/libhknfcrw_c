/**
 * @file	HkNfcDep.h
 * @brief	NFC-DEPヘッダ
 * 
 * 本ヘッダは内部用とする.
 * HkNfcLlcp.cの内部にNFC-DEPを取り込んでいるが、NFC-DEPは外部公開しない.
 */
#ifndef HK_NFCDEP_H
#define HK_NFCDEP_H

#include <stdint.h>
#include <stdbool.h>

/// @defgroup	gp_NfcDep	NFC-DEP
/// @{

#define _ACT		((uint8_t)0x08)		///< Active
#define _PSV		((uint8_t)0x00)		///< Passive
#define _AP_MASK	((uint8_t)0x08)		///< Active/Passive用マスク

#define _BR106K		((uint8_t)0x04)		///< 106kbps
#define _BR212K		((uint8_t)0x02)		///< 212kbps
#define _BR424K		((uint8_t)0x01)		///< 424kbps
#define _BR_MASK	((uint8_t)0x07)		///< Baudrate用マスク


/// @addtogroup gp_depinit	NFC-DEP(Initiator)
/// @{
/// InJumpForDEP
bool HkNfcDep_StartAsInitiator(uint8_t mode);
/// InDataExchange
bool HkNfcDep_SendAsInitiator(
		const void* pCommand, uint8_t CommandLen,
		void* pResponse, uint8_t* pResponseLen);
/// RLS_REQ
bool HkNfcDep_StopAsInitiator(void);
/// @}


/// @addtogroup gp_deptgt	NFC-DEP(Target)
/// @{
/// TgInitTarget, TgSetGeneralBytes
bool HkNfcDep_StartAsTarget(void);
/// TgGetData
bool HkNfcDep_RecvAsTarget(void* pCommand, uint8_t* pCommandLen);
/// TgSetData
bool HkNfcDep_RespAsTarget(const void* pResponse, uint8_t ResponseLen);
/// @}

/// @}


void HkNfcDep_Close(void);
uint8_t HkNfcDep_GetDepMode(void);


#endif /* HK_NFCDEP_H */
