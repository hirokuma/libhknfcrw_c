/**
 * @file	HkNfcSnep.h
 * @brief	SNEPアクセス
 */
#ifndef HK_NFCSNEP_H
#define HK_NFCSNEP_H

#include <stdint.h>
#include <stdbool.h>
#include "HkNfcNdefMsg.h"


#define HKNFCSNEP_SUCCESS		((uint8_t)0)
#define HKNFCSNEP_PROCESSING	((uint8_t)1)
#define HKNFCSNEP_FAIL			((uint8_t)2)

typedef uint8_t HkNfcSnepMode;
#define HKNFCSNEP_MD_INITIATOR		((HkNfcSnepMode)1)
#define HKNFCSNEP_MD_TARGET			((HkNfcSnepMode)2)


uint8_t HkNfcSnep_GetResult(void);
bool HkNfcSnep_PutStart(HkNfcSnepMode Mode, const HkNfcNdefMsg* pMsg);
bool HkNfcSnep_Poll(void);

#endif /* HK_NFCSNEP_H */
