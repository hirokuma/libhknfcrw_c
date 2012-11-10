#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "HkNfcRw.h"
#include "HkNfcA.h"
#include "HkNfcF.h"
#include "HkNfcNdef.h"
#include "HkNfcSnep.h"

#define SNEP_INITIATOR		//�Ȃ�������^�[�Q�b�g
//#define SNEP_PUT			//�Ȃ�������T�[�o

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
		//�J�[�h�T�����āE�E�E
		type = HkNfcRw_Detect(true, false, true);
		//������RF�͎~�߂āE�E�E
		HkNfcRw_RfOff();

		switch(type) {

		// NFC-A�̏ꍇ�ATPE��������SNEP
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

		// NFC-B�̓J�[�h
		case HKNFCTYPE_B:
			printf("[%d]NFC-B\n", count++);
			break;

		// NFC-A�̏ꍇ�ATPE��������SNEP(text)
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
//�f�[�^�]������

	if(b) {
# ifdef SNEP_INITIATOR
		/* Initiator�̏ꍇ�A�Ō��Polling����Technology��DEP���� */
		b = HkNfcSnep_PutStart(HKNFCSNEP_MD_INITIATOR, &msg);
# else
		b = HkNfcSnep_PutStart(HKNFCSNEP_MD_TARGET, &msg);
# endif
	}

#else
//SNEP�T�[�o

	if(b) {
# ifdef SNEP_INITIATOR
		/* Initiator�̏ꍇ�A�Ō��Polling����Technology��DEP���� */
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

