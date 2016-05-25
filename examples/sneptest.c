#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "HkNfcRw.h"
#include "HkNfcA.h"
#include "HkNfcF.h"
#include "HkNfcNdef.h"
#include "HkNfcSnep.h"
#include "hk_misc.h"

#define SNEP_INITIATOR		//なかったらターゲット
//#define SNEP_PUT			//なかったらサーバ

const char* STR_NFCA = 			"NFC-A";
const char* STR_NFCB = 			"NFC-B";
const char* STR_NFCF = 			"NFC-F";

static bool _snep(const char* pStr);

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

#ifdef SNEP_INITIATOR
	printf("Card Detect & read\n");

	bool loop = true;
	while(loop) {
		//カード探索して・・・
		type = HkNfcRw_Detect(true, false, true);
		//すぐにRFは止めて・・・
		HkNfcRw_RfOff();

		switch(type) {

		// NFC-Aの場合、TPEだったらSNEP
		case HKNFCTYPE_A:
			printf("[%d]NFC-A\n", count++);
			{
				HkNfcASelRes selres = HkNfcA_GetSelRes();
				if(HKNFCA_IS_SELRES_TPE(selres)) {
					if(_snep(STR_NFCA)) {
						//success
						loop = false;
					}
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
						if(_snep(STR_NFCF)) {
							//success
							loop = false;
						}
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
	printf("wait initiator\n");

	while(1) {
		if(_snep(STR_NFCF)) {
			hk_msleep(3000);
			break;
		} else {
			hk_msleep(1000);
		}
		printf("-----------------------\n");
	}
#endif

	printf("Close\n");

	HkNfcRw_Close();

	return 0;
}


static bool _snep(const char* pStr)
{
	HkNfcNdefMsg msg;
	bool b;

//	b = HkNfcNdef_CreateText(&msg, pStr, _strlen(pStr), LANGCODE_EN);
	b = HkNfcNdef_CreateUrl(&msg, HKNFCNDEF_URI_HTTP_WWW, "google.com");
	if(!b) {
		printf("ndef fail\n");
	}

	
#ifdef SNEP_PUT
//データ転送あり

	if(b) {
# ifdef SNEP_INITIATOR
		/* Initiatorの場合、最後にPollingしたTechnologyでDEPする */
		b = HkNfcSnep_PutStart(HKNFCSNEP_MD_INITIATOR, &msg);
# else
		b = HkNfcSnep_PutStart(HKNFCSNEP_MD_TARGET, &msg);
# endif
	}

#else
//SNEPサーバ

	if(b) {
# ifdef SNEP_INITIATOR
		/* Initiatorの場合、最後にPollingしたTechnologyでDEPする */
		b = HkNfcSnep_PutServer(HKNFCSNEP_MD_INITIATOR, &msg);
# else
		b = HkNfcSnep_PutServer(HKNFCSNEP_MD_TARGET, &msg);
# endif
	}

#endif
	
	if(b) {
		while(HkNfcSnep_Poll()) {
			;
		}
	
		if(HkNfcSnep_GetResult() == HKNFCSNEP_SUCCESS) {
#ifdef SNEP_PUT
			printf("put success\n");
#else
			printf("[svr]len = %d\n", msg.Length);
			int loop;
			for(loop=0; loop<msg.Length; loop++) {
				printf("[g]%02x\n", msg.Data[loop]);
			}
#endif
		} else {
			printf("put result fail : %02x\n", HkNfcRw_GetLastError());
			b = false;
		}
		HkNfcSnep_Stop();
	}
	
	return b;
}

