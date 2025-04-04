#include "rt_config.h"
#ifdef LINUX
#include <linux/time.h>
#ifndef CONFIG_64BIT
#include <asm/div64.h>	/* For do_div(). */
#endif
#endif

static struct time_log all_tl[ALL_TL_SIZE];
static UCHAR all_tl_idx;
static struct sae_time_log sae_tl[SAE_TL_SIZE];
static UCHAR sae_tl_idx;


VOID log_time_begin(
	IN UINT8 unit,
	INOUT struct time_log *tl)
{
	tl->unit = unit;

	if (unit == LOG_TIME_UNIT_MS)
		NdisGetSystemUpTime(&tl->time);

#ifdef LINUX
	else if (unit == LOG_TIME_UNIT_US) {
#if (KERNEL_VERSION(5, 4, 0) < LINUX_VERSION_CODE)
		ktime_get_real_ts64(&tl->t);
#else
		do_gettimeofday(&tl->t);
#endif
	}

#endif
}

VOID log_time_end(
	IN UINT8 tag,
	IN UCHAR *str,
	IN INT dbg_lvl,
	INOUT struct time_log *tl)
{
	ULONG temp;
#ifdef LINUX
#if (KERNEL_VERSION(5, 4, 0) < LINUX_VERSION_CODE)
	struct timespec64 t;
#else
	struct timeval t;
#endif
#endif

	if (tl->unit == LOG_TIME_UNIT_US) {
#ifdef LINUX
#if (KERNEL_VERSION(5, 4, 0) < LINUX_VERSION_CODE)
		ktime_get_real_ts64(&t);
#ifdef CONFIG_64BIT
		tl->time = ((t.tv_sec - tl->t.tv_sec) * 1000000000 + t.tv_nsec - tl->t.tv_nsec)/1000;
#else
		u64 time_interval = ((t.tv_sec - tl->t.tv_sec) * 1000000000 + t.tv_nsec - tl->t.tv_nsec);
		do_div(time_interval, 1000);
		tl->time = time_interval;
#endif	/* CONFIG_64BIT */
#else
		do_gettimeofday(&t);
		tl->time = (t.tv_sec - tl->t.tv_sec) * 1000000 + t.tv_usec - tl->t.tv_usec;
#endif
#endif
	} else if (tl->unit == LOG_TIME_UNIT_MS) {
		NdisGetSystemUpTime(&temp);
		tl->time = temp - tl->time;
	}

	tl->tag = tag;
	tl->name = str;
	NdisCopyMemory(&all_tl[all_tl_idx], tl, sizeof(struct time_log));
	all_tl_idx++;
	all_tl_idx = all_tl_idx % ALL_TL_SIZE;

	if ((dbg_lvl <= DebugLevel) && (dbg_lvl >= 0)) {
		if (tl->unit == LOG_TIME_UNIT_US)
			MTWF_DBG(NULL, DBG_CAT_MISC, DBG_SUBCAT_MISC, dbg_lvl, "%s: %lu usec\n", str, tl->time);
		else if (tl->unit == LOG_TIME_UNIT_MS) {
			MTWF_DBG(NULL, DBG_CAT_MISC, DBG_SUBCAT_MISC, dbg_lvl, "%s: %lu jiffies", str, tl->time);
#ifdef LINUX
			MTWF_DBG(NULL, DBG_CAT_MISC, DBG_SUBCAT_MISC, dbg_lvl, ", %u msec\n", jiffies_to_msecs(tl->time));
#endif
		}
	}
}


VOID store_time_log_by_tag(
	IN UINT8 tag,
	IN void *tl)
{
	void *tl_array = NULL;
	UCHAR *idx;
	UINT32 size;
	UINT32 array_size;

	switch (tag) {
	case LOG_TIME_SAE:
		idx = &sae_tl_idx;
		tl_array = &sae_tl[*idx];
		size = sizeof(struct sae_time_log);
		array_size = SAE_TL_SIZE;
		break;

	default:
		break;
	}

	if (tl_array != NULL) {
		NdisCopyMemory(tl_array, tl, size);
		(*idx)++;
		*idx %= array_size;
	}
}

static VOID print_time_log(
	IN struct time_log *tl)
{
	if (tl->unit == LOG_TIME_UNIT_MS) {
		MTWF_PRINT("\t%s: %lu jiffies", tl->name, tl->time);
#ifdef LINUX
		MTWF_PRINT(", %u msec\n", jiffies_to_msecs(tl->time));
#endif
	} else if (tl->unit == LOG_TIME_UNIT_US)
		MTWF_PRINT("\t%s: %lu usec\n", tl->name, tl->time);
}



static VOID dump_time_log(
	IN UINT8 dump_cnt,
	IN UINT8 tag)
{
	UINT8 i = all_tl_idx;
	UINT8 j = 0;
	UINT8 bound = (ALL_TL_SIZE > dump_cnt) ? dump_cnt : ALL_TL_SIZE;
	MTWF_PRINT("misc time log:\n");

	do {
		if (i == 0)
			i = ALL_TL_SIZE;

		i--;

		if (tag != LOG_TIME_MISC && all_tl[i].tag != tag)
			continue;

		j++;
		MTWF_PRINT("%d:", j);
		print_time_log(&all_tl[i]);
	} while ((j < bound) && (i != all_tl_idx));
}

static VOID sae_dump_time_log(
	IN UINT8 dump_cnt)
{
	UINT8 i = sae_tl_idx;
	UINT8 j = 0;
	UINT8 bound = (SAE_TL_SIZE > dump_cnt) ? dump_cnt : SAE_TL_SIZE;
	MTWF_PRINT("sae time log:\n");

	do {
		if (i == 0)
			i = SAE_TL_SIZE;

		i--;
		j++;
		MTWF_PRINT("%d:", j);
		print_time_log(&sae_tl[i].derive_pwe_time);
		print_time_log(&sae_tl[i].parse_commit_scalar_time);
		print_time_log(&sae_tl[i].parse_commit_element_time);
		print_time_log(&sae_tl[i].derive_commit_scalar_time);
		print_time_log(&sae_tl[i].derive_commit_element_time);
		print_time_log(&sae_tl[i].derive_k_time);
		print_time_log(&sae_tl[i].derive_pmk_time);
	} while (j < bound);
}

INT show_time_log_info(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING * arg)
{
	UINT8 tag = LOG_TIME_MISC;
	RTMP_STRING *fp  = NULL;
	UINT8 dump_cnt = 0xff;
	fp = strsep(&arg, ":");

	if (fp != NULL)
		tag = os_str_toul(fp, 0, 10);

	if (arg != NULL)
		dump_cnt = os_str_toul(arg, 0, 10);

	switch (tag) {
	case LOG_TIME_SAE:
		sae_dump_time_log(dump_cnt);
		break;

	case LOG_TIME_MISC:
	default:
		dump_time_log(dump_cnt, tag);
		break;
	}

	return TRUE;
}

