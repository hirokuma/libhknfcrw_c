/**
 * @file	HkNfcNdefMsg.h
 * @brief	NDEFメッセージ
 */
#ifndef HK_NFCNDEFMSG_H
#define HK_NFCNDEFMSG_H

#include <stdint.h>

/**
 * @struct	HkNfcNdefMsg
 * @brief	NDEFメッセージ
 */
typedef struct HkNfcNdefMsg {
	uint8_t		Data[256];		///< メッセージ本体
	uint16_t	Length;			///< Data[]の有効データ長
} HkNfcNdefMsg;


#endif /* HK_NFCNDEFMSG_H */
