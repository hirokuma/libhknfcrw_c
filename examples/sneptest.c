#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "HkNfcRw.h"
#include "HkNfcA.h"
#include "HkNfcF.h"
#include "HkNfcNdef.h"
#include "HkNfcSnep.h"

const char* STR_NFCA = 			"NFC-A";
const char* STR_NFCB = 			"NFC-B";
const char* STR_NFCF = 			"NFC-F";

static void _snep(const char* pStr);

int nfc_test()
{
	int count = 0;
	HkNfcType type;
	bool b;

	printf("Open\n");

	b = HkNfcRw_Open();
	if(!b) {
		printf("open fail\n");
		HkNfcRw_Close();
		return -1;
	}

#if 0
	printf("Card Detect & read\n");

	while(1) {
		//カード探索して・・・
		type = HkNfcRw_Detect(false, false, true);
		//すぐにRFは止めて・・・
		HkNfcRw_RfOff();

		switch(type) {

		// NFC-Aの場合、TPEだったらSNEP(text)
		case HKNFCTYPE_A:
			printf("[%d]NFC-A\n", count++);
			{
				HkNfcASelRes selres = HkNfcA_GetSelRes();
				if(HKNFCA_IS_SELRES_TPE(selres)) {
					_snep(STR_NFCA);
				}
			}
			break;

		// NFC-Bはカード
		case HKNFCTYPE_B:
			printf("[%d]NFC-B\n", count++);
			break;

		// NFC-Aの場合、TPEだったらSNEP(text)
		case HKNFCTYPE_F:
			printf("[%d]NFC-F\n", count++);
			{
				uint8_t idm[NFCID2_LEN];
				if(HkNfcRw_GetNfcId(idm) != 0) {
					if(HKNFCF_IS_NFCID_TPE(idm)) {
						_snep(STR_NFCF);
					}
				}
			}
			break;
		default:
			break;
		}
		
		hk_msleep(1000);
	}
#else
	while(1) {
		_snep(STR_NFCF);
		HkNfcSnepStop();
		hk_msleep(1000);
		HkNfcRw_Reset();
		printf("-----------------------\n");
	}
#endif

	printf("Close\n");

	HkNfcRw_Close();

	return 0;
}


static void _snep(const char* pStr)
{
	HkNfcNdefMsg msg;
	bool b;

#if 1
//	b = HkNfcNdef_CreateText(&msg, pStr, _strlen(pStr), LANGCODE_EN);
	b = HkNfcNdef_CreateUrl(&msg, HKNFCNDEF_URI_HTTP, "google.com");
	if(!b) {
		printf("ndef fail\n");
		return;
	}
#endif

	
	/* Initiatorの場合、最後にPollingしたTechnologyでDEPする */
//	b = HkNfcSnep_PutStart(HKNFCSNEP_MD_INITIATOR, &msg);
	b = HkNfcSnep_PutStart(HKNFCSNEP_MD_TARGET, &msg);
	if(!b) {
		printf("putstart fail : %02x\n", HkNfcRw_GetLastError());
		return;
	}
	
	while(HkNfcSnep_Poll()) {
		;
	}
	
	if(HkNfcSnep_GetResult() != HKNFCSNEP_SUCCESS) {
		printf("putresult fail : %02x\n", HkNfcRw_GetLastError());
		return;
	}

	printf("success\n");
	hk_msleep(3000);
}

