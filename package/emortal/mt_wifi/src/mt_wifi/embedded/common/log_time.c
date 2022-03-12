#include "rt_config.h"
#ifdef LINUX
#include <linux/time.h>
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
	else if (unit == LOG_TIME_UNIT_US)
		ktime_get_real_ts64(&tl->t);

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
	struct timespec64  t;
#endif

	if (tl->unit == LOG_TIME_UNIT_US) {
#ifdef LINUX
		ktime_get_real_ts64(&t);
		tl->time = (t.tv_sec - tl->t.tv_sec) * 1000000 + (t.tv_nsec / NSEC_PER_USEC) - (tl->t.tv_nsec / NSEC_PER_USEC);
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

	if (dbg_lvl <= DebugLevel) {
		if (tl->unit == LOG_TIME_UNIT_US)
			MTWF_LOG(DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_OFF, ("%s: %lu usec\n", str, tl->time));
		else if (tl->unit == LOG_TIME_UNIT_MS) {
			MTWF_LOG(DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_OFF, ("%s: %lu jiffies", str, tl->time));
#ifdef LINUX
			MTWF_LOG(DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_OFF, (", %u msec\n", jiffies_to_msecs(tl->time)));
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
		MTWF_LOG(DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_OFF, ("\t%s: %lu jiffies", tl->name, tl->time));
#ifdef LINUX
		MTWF_LOG(DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_OFF, (", %u msec\n", jiffies_to_msecs(tl->time)));
#endif
	} else if (tl->unit == LOG_TIME_UNIT_US)
		MTWF_LOG(DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_OFF, ("\t%s: %lu usec\n", tl->name, tl->time));
}



static VOID dump_time_log(
	IN UINT8 dump_cnt,
	IN UINT8 tag)
{
	UINT8 i = all_tl_idx;
	UINT8 j = 0;
	UINT8 bound = (ALL_TL_SIZE > dump_cnt) ? dump_cnt : ALL_TL_SIZE;
	MTWF_LOG(DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_OFF, ("misc time log:\n"));

	do {
		if (i == 0)
			i = ALL_TL_SIZE;

		i--;

		if (tag != LOG_TIME_MISC && all_tl[i].tag != tag)
			continue;

		j++;
		MTWF_LOG(DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_OFF, ("%d:", j));
		print_time_log(&all_tl[i]);
	} while ((j < bound) && (i != all_tl_idx));
}

static VOID sae_dump_time_log(
	IN UINT8 dump_cnt)
{
	UINT8 i = sae_tl_idx;
	UINT8 j = 0;
	UINT8 bound = (SAE_TL_SIZE > dump_cnt) ? dump_cnt : SAE_TL_SIZE;
	MTWF_LOG(DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_OFF, ("sae time log:\n"));

	do {
		if (i == 0)
			i = SAE_TL_SIZE;

		i--;
		j++;
		MTWF_LOG(DBG_CAT_MISC, DBG_SUBCAT_MISC, DBG_LVL_OFF, ("%d:", j));
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

