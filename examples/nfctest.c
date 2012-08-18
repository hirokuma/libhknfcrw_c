#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "HkNfcRw.h"
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

	if(type != HKNFCTYPE_F) {
		printf("detect fail\n");
		HkNfcRw_Close();
		return -1;
	}

	printf("\nread\n");
	b = HkNfcF_Polling(0x12fc);
	if(b) {
		printf("NFC-F card.\n");
	} else {
		printf("not NFC-F card.\n");
		HkNfcRw_Close();
		return -1;
	}

	uint8_t buf[16];

	for(blk=0; blk<14; blk++) {
		bool ret = HkNfcF_Read(buf, blk);
		if(ret) {
			int i;
			for(i=0; i<16; i++) {
                printf("%02x ", buf[i]);
			}
            printf("\n");
		} else {
			printf("fail : %d\n", blk);
			break;
		}
	}
	printf("----------------------------\n");

	HkNfcF_SetServiceCode(HKNFCF_SVCCODE_RO);
	for(blk=0x80; blk<=0x88; blk++) {
		bool ret = HkNfcF_Read(buf, blk);
		if(ret) {
			int i;
			for(i=0; i<16; i++) {
                printf("%02x ", buf[i]);
			}
            printf("\n");
		} else {
			printf("fail : %d\n", blk);
			break;
		}
	}

	printf("\nClose\n");

	HkNfcRw_Close();

	return 0;
}
