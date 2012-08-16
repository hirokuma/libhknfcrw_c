/**
 * @file	HkNfcB.h
 * @brief	NFC-Bアクセス
 */
#ifndef HKNFCB_H
#define HKNFCB_H

#include <stdint.h>
#include <stdbool.h>

bool HkNfcB_Polling(void);
bool HkNfcB_Read(uint8_t* buf, uint8_t BlockNo);
bool HkNfcB_Write(const uint8_t* buf, uint8_t BlockNo);

#endif /* HKNFCB_H */
