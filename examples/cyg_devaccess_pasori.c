#include <string.h>
#include <sys/types.h>
#include "libusb-1.0/libusb.h"

#include "hk_devaccess.h"

// デバッグ設定
#define HKNFCRW_ENABLE_DEBUG

#ifdef HKNFCRW_ENABLE_DEBUG
//#define DBG_WRITEDATA
//#define DBG_READDATA
#include <stdio.h>
#define LOGI	printf
#define LOGE	printf
#define printf	printf

#else
#define LOGI(...)
#define LOGE(...)
#define printf(...)

#endif	//HKNFCRW_ENABLE_DEBUG


static const uint16_t VID = 0x054c;
static const uint16_t PID = 0x02e1;
static const unsigned int TIMEOUT = 65535 * 2;

static libusb_context*			m_pContext;
static libusb_device_handle*	m_pHandle;

static uint8_t m_EndPntIn = 0xff;
static uint8_t m_EndPntOut = 0xff;

static int m_ReadSize;
static uint8_t m_ReadBuf[256];
static int m_ReadPtr;

static bool usb_open();
static void usb_close();

static uint8_t usb_write(const uint8_t *pData, uint8_t size);
static uint8_t usb_read(uint8_t *pData, uint8_t size);


static bool usb_open()
{
	//printf("%s\n", __PRETTY_FUNCTION__);

	if(m_pContext) {
		// open済み
		//printf("already opened\n");
		return true;
	}

	int r;

	//printf("--init...\n");
	r = libusb_init(&m_pContext);
	if (r < 0) {
		LOGE("cannot init\n");
		return false;
	}
	//printf("  ==> done!\n");

//	libusb_set_debug(NULL, 3);

#if 0
	libusb_device **devs;
	ssize_t cnt = libusb_get_device_list(m_pContext, &devs);
	if (cnt < 0) {
		LOGE("cannot list\n");
		usb_close();
		return false;
	}

	libusb_device *dev;
	int i = 0;

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			LOGE("failed to get device descriptor\n");
			i = -1;
			break;
		}
		if((desc.idVendor == VID) && (desc.idProduct == PID)) {
			i = 0;
			break;
		}
	}
	if(i != 0) {
		LOGE("cannot find\n");
		usb_close();
		return false;
	}
	r = libusb_open(dev, &m_pHandle);
	if(r != 0) {
		LOGE("cannot open : %d\n", r);
		usb_close();
		return false;
	}
	libusb_free_device_list(devs, 1);

#else
	//printf("--open...\n");
	m_pHandle = libusb_open_device_with_vid_pid(NULL, VID, PID);
	if(m_pHandle == NULL) {
		LOGE("cannot open\n");
		usb_close();
		return false;
	}
	//printf("  ==>done!\n");
	
	libusb_device *dev;
	dev = libusb_get_device(m_pHandle);
#endif

	struct libusb_config_descriptor* pConfig;
	r = libusb_get_config_descriptor(dev, 0, &pConfig);
	if(r != 0) {
		LOGE("cannot getdesc : %d\n", r);
		usb_close();
		return false;
	}
	
	//printf("--find\n");
	int inf;
	int alt;
	int desc;
	for(inf = 0; inf < pConfig->bNumInterfaces; inf++) {
		const struct libusb_interface* inter = &pConfig->interface[inf];
		for(alt = 0; alt < inter->num_altsetting; alt++) {
			const struct libusb_interface_descriptor* idesc = &(inter->altsetting[alt]);
			for(desc = 0; desc < idesc->bNumEndpoints; desc++) {
				const struct libusb_endpoint_descriptor* EndPnt = &(idesc->endpoint[desc]);
				if(EndPnt->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
					//printf("find IN\n");
					m_EndPntIn = EndPnt->bEndpointAddress;
				}
				else {
					//printf("find OUT\n");
					m_EndPntOut = EndPnt->bEndpointAddress;
				}
			}
		}
	}

	r = libusb_claim_interface(m_pHandle, 0);
	if(r != 0) {
		LOGE("cannot claim\n");
		usb_close();
		return false;
	}

	//Debug Out
	libusb_set_debug(m_pContext, 3);

	//printf("open!\n");
	return true;
}


static void usb_close()
{
	if(m_pHandle) {
		libusb_release_interface(m_pHandle, 0);
		libusb_close(m_pHandle);
		m_pHandle = NULL;
	}
	if(m_pContext) {
		libusb_exit(m_pContext);
		m_pContext = NULL;
	}

	printf("%s\n", __PRETTY_FUNCTION__);
}

static uint8_t usb_write(const uint8_t *pData, uint8_t size)
{
	if(m_pHandle == NULL) {
		return 0;
	}

	int transferred = 0;
	int r = libusb_bulk_transfer(m_pHandle, m_EndPntOut, (uint8_t*)pData, size, &transferred, TIMEOUT);
	if(r) {
		LOGE("err : %d / transferred : %d\n", r, transferred);
	}
	return transferred;
}

static uint8_t usb_read(uint8_t *pData, uint8_t size)
{
	if(m_pHandle == NULL) {
		return 0;
	}

	if(m_ReadSize) {
		memcpy(pData, &m_ReadBuf[m_ReadPtr], size);
		m_ReadSize -= size;
		m_ReadPtr += size;
		return size;
	}

	m_ReadPtr = 0;
	int r = libusb_bulk_transfer(m_pHandle, m_EndPntIn, m_ReadBuf, sizeof(m_ReadBuf), &m_ReadSize, TIMEOUT);
	if(r) {
		LOGE("err : %d / transferred : %d\n", r, m_ReadSize);
	}

	memcpy(pData, &m_ReadBuf[m_ReadPtr], size);
	m_ReadSize -= size;
	m_ReadPtr = size;

	return size;
}

/**
 * ポートオープン
 *
 * @retval	true	オープン成功
 */
bool hk_nfcrw_open(void)
{
	return usb_open();
}


/**
 * ポートクローズ
 */
void hk_nfcrw_close(void)
{
	usb_close();
}


/**
 * ポート送信
 *
 * @param[in]	data		送信データ
 * @param[in]	len			dataの長さ
 * @return					送信したサイズ
 */
uint16_t hk_nfcrw_write(const uint8_t* data, uint16_t len)
{
#ifdef DBG_READDATA
	int i;
	printf("---------------------\n");
	for(i=0; i<len; i++) {
		printf("[W]%02x\n", data[i]);
	}
#endif

	return usb_write(data, len);
}

/**
 * 受信
 *
 * @param[out]	data		受信バッファ
 * @param[in]	len			受信サイズ
 *
 * @return					受信したサイズ
 *
 * @attention	- len分のデータを受信するか失敗するまで処理がブロックされる。
 */
uint16_t hk_nfcrw_read(uint8_t* data, uint16_t len)
{
	uint16_t ret_len = 0;

	ret_len = usb_read(data, len);

#ifdef DBG_READDATA
	int i;
	printf("---------------------\n");
	for(i=0; i<ret_len; i++) {
		printf("[R]%02x\n", data[i]);
	}
#endif

	return ret_len;
}


/**
 * ポート受信タイムアウト時間設定
 * 
 * タイムアウト処理が可能な場合、受信タイムアウト時間を設定する。
 * タイムアウトがない場合は、何も処理しないし、#hk_nfcrw_read()にも影響はない。
 *
 * @param[in]	msec		タイムアウト時間(ミリ秒)。0のときはタイムアウト解除。
 */
void hk_nfcrw_read_timeout(uint16_t msec)
{
}
