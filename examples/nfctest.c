#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "HkNfcRw.h"
#include "HkNfcA.h"
#include "HkNfcF.h"

int nfc_test()
{
	bool b;
	int blk;

	printf("Open\n");

	b = HkNfcRw_Open();
	if(!b) {
		printf("open fail\n");
		HkNfcRw_Close();
		return -1;
	}
	printf("Card Detect & read\n");

	HkNfcType type = HkNfcRw_Detect(true, true, true);
	printf("type = %d\n", (int)type);

	if(type == HKNFCTYPE_A) {
		//NFC-A
        printf("[[NFC-A]]\n");

		//Ultralight‘O’ñ
		printf("read\n");
		uint8_t buf[HKNFCA_SZ_BLOCK_R];
		for(blk=0; blk < 16; blk += 4) {
			bool ret = HkNfcA_Read(buf, blk);
			if(ret) {
				int i;
				int j;
				for(j=0; j<4; j++) {
					for(i=0; i<HKNFCA_SZ_BLOCK; i++) {
						printf("%02x ", buf[4*j+i]);
					}
					printf("\n");
				}
			} else {
				printf("fail : %d\n", blk);
				break;
			}
		}
#if 0
		printf("----------------------------\n");
		
		const uint8_t DATA[] = { 12, 13, 14, 15 };
		b = HkNfcA_Write(DATA, 13);
		if(b) {
			b = HkNfcA_Read(buf, 13);
			if(b) {
				int i;
				for(i=0; i<HKNFCA_SZ_BLOCK; i++) {
					printf("%02x ", buf[i]);
				}
				printf("\n");
			} else {
				printf("fail read\n");
			}
		} else {
			printf("fail write\n");
		}
#endif
		
		printf("----------------------------\n");
		
	} else if(type == HKNFCTYPE_F) {
		//NFC-F
        printf("[[NFC-F]]\n");

		printf("read\n");
		b = HkNfcF_Polling(0x12fc);
		if(b) {
			printf("NFC-F card.\n");
		} else {
			printf("not NFC-F card.\n");
		}

		// FeliCa Lite‘O’ñ
		uint8_t buf[HKNFCF_SZ_BLOCK];

		for(blk=0; blk<14; blk++) {
			bool ret = HkNfcF_Read(buf, blk);
			if(ret) {
				int i;
				for(i=0; i<HKNFCF_SZ_BLOCK; i++) {
					printf("%02x ", buf[i]);
				}
				printf("\n");
			} else {
				printf("fail : %d\n", blk);
				break;
			}
		}
		printf("----------------------------\n");

		for(blk=0x80; blk<=0x88; blk++) {
			bool ret = HkNfcF_Read(buf, blk);
			if(ret) {
				int i;
				for(i=0; i<HKNFCF_SZ_BLOCK; i++) {
					printf("%02x ", buf[i]);
				}
				printf("\n");
			} else {
				printf("fail : %d\n", blk);
				break;
			}
		}
#if 0
		printf("----------------------------\n");

		const uint8_t DATA[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		b = HkNfcF_Write(DATA, 10);
		if(b) {
			b = HkNfcF_Read(buf, 10);
			if(b) {
				int i;
				for(i=0; i<HKNFCF_SZ_BLOCK; i++) {
					printf("%02x ", buf[i]);
				}
				printf("\n");
			} else {
				printf("fail read\n");
			}
		} else {
			printf("fail write\n");
		}
#endif

	}
	else {
		printf("not support type\n");
	}

	printf("Close\n");

	HkNfcRw_Close();

	return 0;
}
