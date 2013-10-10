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
 *  @brief	�~���b�X���[�v
 *
 * @param	msec	�҂�����[msec]
 */
void hk_msleep(uint16_t msec) {
	usleep(msec * 1000);
}


/**
 * �^�C�}�J�n
 *
 * @param[in]	tmval	�^�C���A�E�g����[msec]
 */
void hk_start_timer(uint16_t tmval)
{
	gettimeofday(&s_TimeVal, 0);
	s_Timeout = tmval;
	//LOGD("  startTimer : (%ld, %ld)\n", s_TimeVal.tv_sec, s_TimeVal.tv_usec);
}


/**
 *  �^�C���A�E�g�Ď�
 * 
 * @retval	true	�^�C���A�E�g����
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


/**
 * ��������r
 *
 * @param[in]	s1		��r��1
 * @param[in]	s2		��r��2
 * @param[in]	n		��r�T�C�Y
 * @retval		0			��v
 * @retval		��L�ȊO	�s��v
 */
int   hk_memcmp(const void *s1, const void *s2, uint16_t n)
{
	return memcmp(s1, s2, (size_t)n);
}

/**
 * �������R�s�[
 *
 * @param[out]	dst		�R�s�[��
 * @param[in]	src		�R�s�[��
 * @param[in]	len		�R�s�[�T�C�Y
 * @return				�R�s�[��A�h���X(dst)
 */
void* hk_memcpy(void* dst, const void* src, uint16_t len)
{
	return memcpy(dst, src, (size_t)len);
}

/**
 * ��������������
 *
 * @param[out]	dst		�������ݐ�
 * @param[in]	dat		�������݃f�[�^
 * @param[in]	len		�������݃T�C�Y
 * @return				�������ݐ�A�h���X(dst)
 */
void* hk_memset(void* dst, uint8_t dat, uint16_t len)
{
	return memset(dst, dat, len);
}

uint8_t hk_strlen(const char* pStr)
{
	return strlen(pStr);
}
