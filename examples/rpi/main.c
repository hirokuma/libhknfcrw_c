#include <stdio.h>

int main(int argc, char *argv[])
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

	if (type != HKNFCTYPE_NONE) {
		uint8_t nfcid[HKNFCID_MAX];
		
		uint8_t len = HkNfcRw_GetNfcId(nfcid);
		
		printf("UID : ");
		for(int i=0; i<len; i++) {
			printf("%02x ", nfcid[i]);
		}
		printf("\n");
	}

	return 0;
}
