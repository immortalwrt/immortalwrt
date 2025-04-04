#ifndef LOG_TIME_H
#define LOG_TIME_H
#ifdef LINUX
#include <linux/time.h>
#endif
enum {
	LOG_TIME_MISC = 0,
	LOG_TIME_SAE,
	LOG_TIME_CONNECTION,
};

enum {
	LOG_TIME_UNIT_MS = 0,
	LOG_TIME_UNIT_US,
};

struct time_log {
#ifdef	LINUX
#if (KERNEL_VERSION(5, 4, 0) < LINUX_VERSION_CODE)
	struct timespec64 t;
#else
	struct timeval t;
#endif
#endif
	ULONG time;
	RTMP_STRING *name;
	UINT8 tag;
	UINT8 unit;
};

struct sae_time_log {
	struct time_log derive_pwe_time;
	struct time_log parse_commit_scalar_time;
	struct time_log parse_commit_element_time;
	struct time_log derive_commit_scalar_time;
	struct time_log derive_commit_element_time;
	struct time_log derive_k_time;
	struct time_log derive_pmk_time;
};

#define ALL_TL_SIZE 40
#define SAE_TL_SIZE 6

VOID log_time_begin(
	IN UINT8 unit,
	INOUT struct time_log *tl);

VOID log_time_end(
	IN UINT8 tag,
	IN UCHAR *str,
	IN INT dbg_lvl,
	INOUT struct time_log *tl);

VOID store_time_log_by_tag(
	IN UINT8 tag,
	IN void *tl);

INT show_time_log_info(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg);

#endif /* LOG_TIME_H */
