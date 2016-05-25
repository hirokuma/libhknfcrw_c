#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "hk_misc.h"
#define LOGI(...)
#define LOGE(...)
#define LOGD(...)

static struct timeval s_TimeVal;
static uint16_t s_Timeout = 0;

/**
 *  @brief	ミリ秒スリープ
 *
 * @param	msec	待ち時間[msec]
 */
void hk_msleep(uint16_t msec) {
	usleep(msec * 1000);
}


/**
 * タイマ開始
 *
 * @param[in]	tmval	タイムアウト時間[msec]
 */
void hk_start_timer(uint16_t tmval)
{
	gettimeofday(&s_TimeVal, 0);
	s_Timeout = tmval;
	//LOGD("  startTimer : (%ld, %ld)\n", s_TimeVal.tv_sec, s_TimeVal.tv_usec);
}


/**
 *  タイムアウト監視
 * 
 * @retval	true	タイムアウト発生
 */
bool hk_is_timeout()
{
	if(s_Timeout == 0) {
		return false;
	}

	struct timeval now;
	struct timeval res;
	
	gettimeofday(&now, 0);
	timersub(&now, &s_TimeVal, &res);
	uint32_t tt = res.tv_sec * 1000 + res.tv_usec / 1000;
	bool b = tt >= s_Timeout;
	if(b) {
		LOGD("  isTimeout : (%ld, %ld) - (%ld, %ld)\n", now.tv_sec, now.tv_usec, s_TimeVal.tv_sec, s_TimeVal.tv_usec);
	}
	s_Timeout = 0;
	return b;
}


#ifndef HK_USE_STRING_H
/**
 * メモリ比較
 *
 * @param[in]	s1		比較元1
 * @param[in]	s2		比較元2
 * @param[in]	n		比較サイズ
 * @retval		0			一致
 * @retval		上記以外	不一致
 */
int   hk_memcmp(const void *s1, const void *s2, uint16_t n)
{
	return memcmp(s1, s2, (size_t)n);
}

/**
 * メモリコピー
 *
 * @param[out]	dst		コピー先
 * @param[in]	src		コピー元
 * @param[in]	len		コピーサイズ
 * @return				コピー先アドレス(dst)
 */
void* hk_memcpy(void* dst, const void* src, uint16_t len)
{
	return memcpy(dst, src, (size_t)len);
}

/**
 * メモリ書き込み
 *
 * @param[out]	dst		書き込み先
 * @param[in]	dat		書き込みデータ
 * @param[in]	len		書き込みサイズ
 * @return				書き込み先アドレス(dst)
 */
void* hk_memset(void* dst, uint8_t dat, uint16_t len)
{
	return memset(dst, dat, len);
}

uint8_t hk_strlen(const char* pStr)
{
	return strlen(pStr);
}
#endif  //HK_USE_STRING_H
