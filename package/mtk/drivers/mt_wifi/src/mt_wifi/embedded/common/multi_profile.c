/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:
	multi_profile.c
*/

#ifdef MULTI_PROFILE
#include "rt_config.h"

/*Local definition*/
#define FIRST_AP_EXT_PROFILE_PATH	"/etc/Wireless/RT2860/RT2860_5G.dat"
#define FIRST_AP_MERGE_PROFILE_PATH ""
#define FIRST_AP_EXT_DEVNAME "rax0"
#define FIRST_MBSSID_EXT_DEVNAME "rax"
#define FIRST_APCLI_EXT_DEVNAME "apclix"
#define SECOND_AP_EXT_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap_5G.dat"
#define SECOND_AP_MERGE_PROFILE_PATH ""
#define SECOND_AP_EXT_DEVNAME "ray0"
#define SECOND_MBSSID_EXT_DEVNAME "ray"
#define SECOND_APCLI_EXT_DEVNAME "apcliiy"
#define THIRD_AP_EXT_PROFILE_PATH	"/etc/Wireless/WIFI3/RT2870AP_5G.dat"
#define THIRD_AP_MERGE_PROFILE_PATH ""
#define THIRD_AP_EXT_DEVNAME "raz0"
#define THIRD_MBSSID_EXT_DEVNAME "raz"
#define THIRD_APCLI_EXT_DEVNAME "apcliez"


#define TEMP_STR_SIZE 256
/* A:B:C:D -> 4 (Channel group A, B, C, D) + 3 ':' */
#define LEN_BITMAP_CHGRP 7

struct mpf_data {
	UCHAR enable;
	UCHAR specific_dname;
	UCHAR pf1_num;
	UCHAR pf2_num;
	UCHAR total_num;
};

struct mpf_table {
	UCHAR profile_ext[L2PROFILE_PATH_LEN];
	UCHAR merge[L2PROFILE_PATH_LEN];
	struct dev_type_name_map_t dev_name_map[MAX_INT_TYPES+1];
};

static struct mpf_table mtb[] = {
	{ FIRST_AP_EXT_PROFILE_PATH, FIRST_AP_MERGE_PROFILE_PATH,
		{{INT_MAIN, FIRST_AP_EXT_DEVNAME}, {INT_MBSSID, FIRST_MBSSID_EXT_DEVNAME},
			{INT_WDS, FIRST_AP_EXT_DEVNAME}, {INT_APCLI, FIRST_APCLI_EXT_DEVNAME},
			{INT_MESH, FIRST_AP_EXT_DEVNAME}, {INT_P2P, FIRST_AP_EXT_DEVNAME},
			{INT_MONITOR, FIRST_AP_EXT_DEVNAME}, {INT_MSTA, FIRST_AP_EXT_DEVNAME}, {0} } },
	{ SECOND_AP_EXT_PROFILE_PATH, SECOND_AP_MERGE_PROFILE_PATH,
		{{INT_MAIN, SECOND_AP_EXT_DEVNAME}, {INT_MBSSID, SECOND_MBSSID_EXT_DEVNAME},
			{INT_WDS, SECOND_AP_EXT_DEVNAME}, {INT_APCLI, SECOND_APCLI_EXT_DEVNAME},
			{INT_MESH, SECOND_AP_EXT_DEVNAME}, {INT_P2P, SECOND_AP_EXT_DEVNAME},
			{INT_MONITOR, SECOND_AP_EXT_DEVNAME}, {INT_MSTA, SECOND_AP_EXT_DEVNAME}, {0} } },
	{ THIRD_AP_EXT_PROFILE_PATH, THIRD_AP_MERGE_PROFILE_PATH,
		{{INT_MAIN, THIRD_AP_EXT_DEVNAME}, {INT_MBSSID, THIRD_MBSSID_EXT_DEVNAME},
			{INT_WDS, THIRD_AP_EXT_DEVNAME}, {INT_APCLI, THIRD_APCLI_EXT_DEVNAME},
			{INT_MESH, THIRD_AP_EXT_DEVNAME}, {INT_P2P, THIRD_AP_EXT_DEVNAME},
			{INT_MONITOR, THIRD_AP_EXT_DEVNAME}, {INT_MSTA, THIRD_AP_EXT_DEVNAME}, {0} } },
};

/* merge rule of multi-profile */
typedef enum _MPF_MERGE_RULES_T {
	MPF_APPEND_0       = 0,		/* append 0 */
	MPF_APPEND_1       = 1,		/* append 1 */
	MPF_APPEND_BSS0    = 2		/* follow BSS 0 of each profile */
} MPF_MERGE_RULES;

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
#if defined(WSC_V2_SUPPORT) || defined(WSC_AP_SUPPORT)
static INT multi_profile_merge_wsc(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final);
#endif
#endif /*MBSS_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/

#ifdef DBDC_ONE_BAND1_SUPPORT
UCHAR *get_dbdcdev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type)
#else
static UCHAR *get_dbdcdev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type)
#endif
{
	struct dev_type_name_map_t *map;
	INT type_idx = 0, dev_idx = get_dev_config_idx(pAd);

	if (dev_idx < 0 || dev_idx >= MAX_NUM_OF_INF) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): invalid dev_idx(%d)!\n",
				 __func__, dev_idx);
		return NULL;
	}

	if (type_idx >= (MAX_INT_TYPES+1)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): invalid type_idx(%d)!\n",
				 __func__, type_idx);
		return NULL;
	}

	do {
		map = &(mtb[dev_idx].dev_name_map[type_idx]);

		if (map->type == dev_type) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s(): dev_idx = %d, dev_name_prefix=%s\n", __func__, dev_idx, map->prefix);
			return map->prefix;
		}

		type_idx++;
	} while (mtb[dev_idx].dev_name_map[type_idx].type != 0);

	return NULL;
}


NDIS_STATUS update_mtb_value(struct _RTMP_ADAPTER *pAd, UCHAR data_id, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	INT dev_idx = get_dev_config_idx(pAd);
	UINT8 str_len = 0;

	if (dev_idx < 0 || dev_idx >= MAX_NUM_OF_INF) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): invalid dev_idx(%d)!\n",
				 __func__, dev_idx);
		return NDIS_STATUS_FAILURE;
	}

	switch (data_id) {
	case MTB_EXT_PROFILE:
		if (value) {
			str_len = strlen(value);
			if (value && strlen(value) && strcmp(value, mtb[dev_idx].profile_ext) && (str_len < L2PROFILE_PATH_LEN)) {
				strncpy(mtb[dev_idx].profile_ext, value, sizeof(mtb[dev_idx].profile_ext));
				mtb[dev_idx].profile_ext[str_len] = '\0';
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"mtb[%d].profile_ext updated as %s!\n", dev_idx, mtb[dev_idx].profile_ext);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"mtb[%d].profile_ext remain %s!\n", dev_idx, mtb[dev_idx].profile_ext);
			}
		} else
			mtb[dev_idx].profile_ext[0] = '\0';
		break;
	case MTB_DEV_PREFIX:
		{
			RTMP_STRING *prefix = get_dbdcdev_name_prefix(pAd, extra);
			if (prefix == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"prefix is NULL.\n");
				retVal = NDIS_STATUS_FAILURE;
				break;
			}

			if (value && strlen(value) && strcmp(prefix, value)) {
				strncpy(prefix, value, IFNAMSIZ);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"mtb[%d].prefix updated as %s!\n", dev_idx, prefix);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"mtb[%d].prefix remain %s!\n", dev_idx, prefix);
			}
		}
		break;
	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown data_id(%d)\n", data_id);
		retVal = NDIS_STATUS_FAILURE;
		break;
	}

	return retVal;
}

static UINT32 parent_dir_len(UCHAR *path)
{
	UINT32 length = 0;
	UCHAR *slash = strstr(path, "/");

	if (slash) {
		length = slash - path;
		length += (parent_dir_len(slash+1)+1);
	}

	return length;
}

/*Local function body*/
/*
*
*/
static UCHAR *multi_profile_fname_get(struct _RTMP_ADAPTER *pAd, UCHAR profile_id)
{
	UCHAR *src_path = NULL, *main_profile_path = NULL;
	INT card_idx = 0, ret = 0;
	UCHAR src_path_tmp[L2PROFILE_PATH_LEN] = {0};
#if defined(CONFIG_RT_FIRST_CARD) || defined(CONFIG_RT_SECOND_CARD) || defined(CONFIG_RT_THIRD_CARD)
	card_idx = get_dev_config_idx(pAd);
#endif /* CONFIG_RT_FIRST_CARD || CONFIG_RT_SECOND_CARD */

	main_profile_path = get_dev_l2profile(pAd);

	if (card_idx < 0 || card_idx >= MAX_NUM_OF_INF) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s(): invalid card_idx(%d)!\n",
				 __func__, card_idx);
		return NULL;
	}


	if (main_profile_path) {
		if (profile_id == MTB_MERGE_PROFILE) {
			src_path = mtb[card_idx].merge;

			if (strlen(src_path) == 0) {
				strncat(src_path, main_profile_path, parent_dir_len(main_profile_path));
				os_move_mem(src_path_tmp, src_path, strlen(src_path) + 1);
				ret = snprintf(src_path, L2PROFILE_PATH_LEN,
					"%sDBDC_card%d.dat", src_path_tmp, card_idx);

				if (ret < 0 || ret >= L2PROFILE_PATH_LEN)
					return NULL;
			}

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Open file \"%s\" to store DBDC cfg! (%d)\n",
				  src_path, parent_dir_len(main_profile_path));
		} else
			src_path = (profile_id == MTB_EXT_PROFILE) ? mtb[card_idx].profile_ext : main_profile_path;
	}

	return src_path;
}

/*
* open & read profile
*/
static INT multi_profile_read(CHAR *fname, CHAR *buf)
{
	INT retval = NDIS_STATUS_FAILURE;
	RTMP_OS_FD_EXT srcf;

	if (!fname || (strlen(fname) == 0))
		return retval;

	srcf = os_file_open(fname, O_RDONLY, 0);

	if (srcf.Status) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Open file \"%s\" failed!\n", fname);
		return retval;
	}

	if (buf) {
		os_zero_mem(buf, MAX_INI_BUFFER_SIZE);
		retval = os_file_read(srcf, buf, MAX_INI_BUFFER_SIZE - 1);

		if (retval > 0)
			retval = NDIS_STATUS_SUCCESS;
		else {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Read file \"%s\" failed(errCode=%d)!\n", fname, retval);
			retval = NDIS_STATUS_FAILURE;
		}
	} else
		retval = NDIS_STATUS_FAILURE;

	if (os_file_close(srcf) != 0) {
		retval = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Close file \"%s\" failed(errCode=%d)!\n", fname, retval);
	}

	return retval;
}

/*
* write merge profile for check
*/
static INT multi_profile_write(CHAR *fname, CHAR *buf)
{
	INT retval = NDIS_STATUS_FAILURE;
	RTMP_OS_FD_EXT srcf;
	UINT32 writeLen;

	if (!fname)
		return retval;

	srcf = os_file_open(fname, O_WRONLY | O_CREAT, 0);

	if (srcf.Status) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Open file \"%s\" failed!\n", fname);
		return retval;
	}

	if (buf) {
		writeLen = strlen(buf);
		if (writeLen > MAX_INI_BUFFER_SIZE) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "ERROR!! exceeded buffer size %d (> %d)!\n", writeLen, MAX_INI_BUFFER_SIZE);
			writeLen = MAX_INI_BUFFER_SIZE;
		}

		retval = os_file_write(srcf, buf, writeLen);

		if (retval > 0) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Write file \"%s\" success (size=%d)!\n", fname, writeLen);
			retval = NDIS_STATUS_SUCCESS;
		} else
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Write file \"%s\" failed(errCode=%d)!\n", fname, retval);
	} else
		retval = NDIS_STATUS_FAILURE;

	if (os_file_close(srcf) != 0) {
		retval = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Close file \"%s\" failed(errCode=%d)!\n", fname, retval);
	}

	return retval;
}
/*
* replace function
*/
INT multi_profile_replace(CHAR *cha, CHAR *rep, CHAR *value)
{
	CHAR *token = NULL;

	while ((token = strstr(value, cha)) != NULL)
		strncpy(token, rep, strlen(rep));

	return NDIS_STATUS_SUCCESS;
}


/*
* Separate
*/
static INT multi_profile_merge_separate(
	UCHAR *parm,
	UCHAR *buf1,
	UCHAR *buf2,
	UCHAR *final)
{
	CHAR *tmpbuf = NULL;
	CHAR *tmpbuf2 = NULL;
	CHAR *value = NULL;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	if (!buf1 || !buf2)
		return NDIS_STATUS_FAILURE;

	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf2, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!tmpbuf || !tmpbuf2 || !value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(tmpbuf, TEMP_STR_SIZE);
	os_zero_mem(tmpbuf2, TEMP_STR_SIZE);
	os_zero_mem(value, TEMP_STR_SIZE);

	if (RTMPGetKeyParameter(parm, tmpbuf, TEMP_STR_SIZE, buf1, TRUE) != TRUE) {
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	if (RTMPGetKeyParameter(parm, tmpbuf2, TEMP_STR_SIZE, buf2, TRUE) != TRUE) {
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", tmpbuf, tmpbuf2);
	if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"value snprintf error!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}
	RTMPSetKeyParameter(parm, value, TEMP_STR_SIZE, final, TRUE);

end:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	if (tmpbuf2)
		os_free_mem(tmpbuf2);
	if (value)
		os_free_mem(value);
	return ret;
}

#ifdef APCLI_SUPPORT
/* without space trim */
static INT multi_profile_merge_separate_space(
	UCHAR *parm,
	UCHAR *buf1,
	UCHAR *buf2,
	UCHAR *final)
{
	CHAR *tmpbuf;
	CHAR *value;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	if (!buf1 || !buf2)
		return NDIS_STATUS_FAILURE;

	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!tmpbuf || !value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(tmpbuf, TEMP_STR_SIZE);
	os_zero_mem(value, TEMP_STR_SIZE);

	if (RTMPGetKeyParameter("ApCliSsid", tmpbuf, TEMP_STR_SIZE, buf1, FALSE) == TRUE) {
		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);

		if (ret_tmp < 0 || ret_tmp >= TEMP_STR_SIZE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}
	} else {
		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", "");

		if (ret_tmp < 0 || ret_tmp >= TEMP_STR_SIZE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}
	}

	if (RTMPGetKeyParameter("ApCliSsid", tmpbuf, TEMP_STR_SIZE, buf2, FALSE) == TRUE) {
		CHAR tmp[TEMP_STR_SIZE] = {0};

		os_move_mem(tmp, value, strlen(value) + 1);
		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", tmp, tmpbuf);

		if (ret_tmp < 0 || ret_tmp >= TEMP_STR_SIZE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}
	}

	RTMPSetKeyParameter("ApCliSsid", value, TEMP_STR_SIZE, final, FALSE);

end:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	if (value)
		os_free_mem(value);
	return ret;
}

/* initially there is no value in dat file for ApCliEnable, hence causing the issue
	in DBDC case while parsing from merged dat file*/

static INT multi_profile_merge_separate_no_initial_value(
	UCHAR *parm,
	UCHAR *buf1,
	UCHAR *buf2,
	UCHAR *final)
{
	CHAR *tmpbuf = NULL;
	CHAR *tmpbuf2 = NULL;
	CHAR *value = NULL;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	if (!buf1 || !buf2)
		return NDIS_STATUS_FAILURE;

	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf2, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!tmpbuf || !tmpbuf2 || !value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(tmpbuf, TEMP_STR_SIZE);
	os_zero_mem(tmpbuf2, TEMP_STR_SIZE);
	os_zero_mem(value, TEMP_STR_SIZE);

	if (RTMPGetKeyParameter(parm, tmpbuf, TEMP_STR_SIZE, buf1, TRUE) != TRUE) {
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	if (RTMPGetKeyParameter(parm, tmpbuf2, TEMP_STR_SIZE, buf2, TRUE) != TRUE) {
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	if (strlen(tmpbuf) == 0)
		tmpbuf[0] = '0';
	if (strlen(tmpbuf2) == 0)
		tmpbuf2[0] = '0';
	ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", tmpbuf, tmpbuf2);
	if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"value snprintf error!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}
	RTMPSetKeyParameter(parm, value, TEMP_STR_SIZE, final, TRUE);

end:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	if (tmpbuf2)
		os_free_mem(tmpbuf2);
	if (value)
		os_free_mem(value);
	return ret;
}
#endif

/*
*per bss
*/
static INT multi_profile_merge_perbss(
	struct mpf_data *data,
	UCHAR *parm,
	UCHAR *buf1,
	UCHAR *buf2,
	UCHAR *final,
	MPF_MERGE_RULES merge_rule)
{
	UCHAR *value;
	UCHAR i, j;
	UCHAR *ptok = NULL;
	UCHAR pf_def_value[32];
	UCHAR mpfidx;
	struct mpf_list {
		UCHAR  *name;
		UCHAR  *buf;
		UCHAR  bss_num;
		UCHAR  pf[TEMP_STR_SIZE];
		INT    status;
	} *mpf[2];
	UINT mpf_list_num = 2;
	INT bCreateKey = FALSE;
	UCHAR tmp[TEMP_STR_SIZE * 2] = {0};
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, &value, TEMP_STR_SIZE * 2);
	os_alloc_mem_suspend(NULL, (UCHAR **)&mpf[0], sizeof(struct mpf_list));
	os_alloc_mem_suspend(NULL, (UCHAR **)&mpf[1], sizeof(struct mpf_list));

	if (!mpf[0] || !mpf[1] || !value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(value, TEMP_STR_SIZE * 2);
	os_zero_mem(mpf[0], sizeof(struct mpf_list));
	os_zero_mem(mpf[1], sizeof(struct mpf_list));

	mpf[0]->name = "pf1";
	mpf[0]->buf = buf1;
	mpf[0]->bss_num = data->pf1_num;
	mpf[0]->status = FALSE;
	mpf[1]->name = "pf2";
	mpf[1]->buf = buf2;
	mpf[1]->bss_num = data->pf2_num;
	mpf[1]->status = FALSE;

	for (mpfidx = 0; mpfidx < mpf_list_num; mpfidx++) {
		if (!mpf[mpfidx]->buf) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "ERROR: [%s] is NULL!!!\n", mpf[mpfidx]->name);
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		/* get profile key */
		mpf[mpfidx]->status = RTMPGetKeyParameter(parm, mpf[mpfidx]->pf,
						TEMP_STR_SIZE, mpf[mpfidx]->buf, TRUE);
		if (mpf[mpfidx]->status)
			bCreateKey = TRUE;
	}

	if (bCreateKey) {
		for (mpfidx = 0; mpfidx < mpf_list_num; mpfidx++) {
			i = 0;
			if (merge_rule == MPF_APPEND_0)
				strncpy(pf_def_value, "0", 2);
			else if (merge_rule == MPF_APPEND_1)
				strncpy(pf_def_value, "1", 2);
			else
				strncpy(pf_def_value, "", 2);

			if (mpf[mpfidx]->status) {
				for (i = 0, ptok = rstrtok(mpf[mpfidx]->pf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
					if (i >= mpf[mpfidx]->bss_num)
						break;

					/* get default value (bss0) */
					if ((merge_rule == MPF_APPEND_BSS0) && (i == 0))
						strncpy(pf_def_value, ptok, sizeof(pf_def_value));

					os_move_mem(tmp, value, strlen(value) + 1);
					ret_tmp = snprintf(value, TEMP_STR_SIZE * 2, "%s%s;", tmp, ptok);

					if (ret_tmp < 0 || ret_tmp >= TEMP_STR_SIZE * 2) {
						ret = NDIS_STATUS_FAILURE;
						goto end;
					}
				}
			}
			/* append default value */
			if (i < mpf[mpfidx]->bss_num) {
				for (j = i; j < mpf[mpfidx]->bss_num; j++) {
					os_move_mem(tmp, value, strlen(value) + 1);
					ret_tmp = snprintf(value, TEMP_STR_SIZE * 2, "%s%s;", tmp, pf_def_value);

					if (ret_tmp < 0 || ret_tmp >= TEMP_STR_SIZE * 2) {
						ret = NDIS_STATUS_FAILURE;
						goto end;
					}
				}
			}
		}
		RTMPSetKeyParameter(parm, value, TEMP_STR_SIZE * 2, final, TRUE);

	}

end:
	if (mpf[0])
		os_free_mem(mpf[0]);
	if (mpf[1])
		os_free_mem(mpf[1]);
	if (value)
		os_free_mem(value);
	return ret;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
/*
* MACAddress(Idx)
*/
static INT multi_profile_merge_mac_address(
	struct mpf_data *mpf,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[25] = "";
	CHAR tok_str[25] = "";
	UCHAR i = 0;
	UCHAR j = 0;
	INT ret;

	/* set file parameter to portcfg*/
	if (RTMPGetKeyParameter("MacAddress", tmpbuf, 25, buf2, TRUE)) {
		ret = snprintf(tok_str, sizeof(tok_str), "MacAddress%d", mpf->pf1_num);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "tok_str snprintf error!!!\n");
			return NDIS_STATUS_FAILURE;
		}
		RTMPAddKeyParameter(tok_str, tmpbuf, 25, final);
	}

	for (i = 1; i <= mpf->pf2_num ; i++) {
		ret = snprintf(tok_str, sizeof(tok_str), "MacAddress%d", i);

		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "tok_str snprintf error!!!\n");
			return NDIS_STATUS_FAILURE;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 25, buf2, TRUE)) {
			j = i + mpf->pf1_num;
			ret = snprintf(tok_str, sizeof(tok_str), "MacAddress%d", j);
			if (os_snprintf_error(sizeof(tok_str), ret)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "tok_str snprintf error!!!\n");
				return NDIS_STATUS_FAILURE;
			}
			RTMPAddKeyParameter(tok_str, tmpbuf, 25, final);
		}
	}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
	/* apcli mac addr */
	if (RTMPGetKeyParameter("ApcliMacAddress", tmpbuf, 25, buf2, TRUE)) {
		ret = snprintf(tok_str, sizeof(tok_str), "ApcliMacAddress1");
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "tok_str snprintf error!!!\n");
			return NDIS_STATUS_FAILURE;
		}
		RTMPAddKeyParameter(tok_str, tmpbuf, 25, final);
	}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}

/*
* For increase from 1
*/
static INT multi_profile_merge_increase(
	struct mpf_data *mpf,
	UCHAR start_idx,
	CHAR *parm,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR *tmpbuf;
	CHAR *tok_str;
	UCHAR i = 0;
	UCHAR j = 0;
	UCHAR k = 0;
	BOOLEAN isNeedTrim = TRUE;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&tok_str, TEMP_STR_SIZE);

	if (strcmp(parm, "WPAPSK") == 0) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"parm is WPAPSK, no need trim space\n");
		isNeedTrim = FALSE;
	}
	if (strcmp(parm, "SSID") == 0) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"parm is SSID, no need trim space\n");
		isNeedTrim = FALSE;
	}

	if (!tmpbuf || !tok_str) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(tmpbuf, TEMP_STR_SIZE);
	os_zero_mem(tok_str, TEMP_STR_SIZE);

	/* set file parameter to portcfg*/
	for (i = 0; i < mpf->pf2_num; i++) {
		k = start_idx + i;
		ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "%s%d", parm, k);

		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "tok_str snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAM_BUFFER_SIZE, buf2, isNeedTrim)) {
			j = k + mpf->pf1_num;
			ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "%s%d", parm, j);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "tok_str snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter(tok_str, tmpbuf, MAX_PARAM_BUFFER_SIZE, final, isNeedTrim);
		}
	}

end:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	if (tok_str)
		os_free_mem(tok_str);
	return ret;
}

static INT multi_profile_merge_radius_key(
	struct mpf_data *mpf,
	UCHAR start_idx,
	CHAR *parm,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR *tmpbuf = NULL;
	CHAR *tmpbuf1 = NULL;
	CHAR *tok_str = NULL;
	UCHAR i = 0;
	UCHAR j = 0;
	UCHAR k = 0;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf1, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&tok_str, TEMP_STR_SIZE);

	if (!tmpbuf || !tmpbuf1 || !tok_str) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(tmpbuf, TEMP_STR_SIZE);
	os_zero_mem(tmpbuf1, TEMP_STR_SIZE);
	os_zero_mem(tok_str, TEMP_STR_SIZE);

	ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "%s%d", parm, start_idx);

	if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "tok_str snprintf error!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	if (!RTMPGetKeyParameter(tok_str, tmpbuf1, TEMP_STR_SIZE, buf1, TRUE)) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"RTMPGetKeyParameter fail!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}
	for (i = 0; i < mpf->pf1_num; i++) {
		k = start_idx + i;
		ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "%s%d", parm, k);
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "tok_str snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		if (!RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, buf1, TRUE))
			RTMPSetKeyParameter(tok_str, tmpbuf1, TEMP_STR_SIZE, final, TRUE);
	}

	ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "%s%d", parm, start_idx);
	if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "tok_str snprintf error!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}
	if (!RTMPGetKeyParameter(tok_str, tmpbuf1, TEMP_STR_SIZE, buf2, TRUE)) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"RTMPGetKeyParameter fail!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	for (i = 0; i < mpf->pf2_num; i++) {
		k = start_idx + i;
		ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "%s%d", parm, k);
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "tok_str snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, buf2, TRUE)) {
			j = k + mpf->pf1_num;
			ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "%s%d", parm, j);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "tok_str snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, final, TRUE);
		} else {
			j = k + mpf->pf1_num;
			ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "%s%d", parm, j);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "tok_str snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter(tok_str, tmpbuf1, TEMP_STR_SIZE, final, TRUE);
		}
	}

end:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	if (tmpbuf1)
		os_free_mem(tmpbuf1);
	if (tok_str)
		os_free_mem(tok_str);
	return ret;
}

/*
* Key%dType  & Key%dStr%
*/
static INT multi_profile_merge_keytype(
	struct mpf_data *mpf,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR *tmpbuf;
	CHAR *tok_str;
	UCHAR i = 0;
	UCHAR j = 0;
	UCHAR k = 0;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&tok_str, TEMP_STR_SIZE);

	if (!tmpbuf || !tok_str) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		goto end;
	}

	os_zero_mem(tmpbuf, TEMP_STR_SIZE);
	os_zero_mem(tok_str, TEMP_STR_SIZE);

	for (k = 1; k <= 4; k++) {
		ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "Key%dType", k);

		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "tok_str snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_merge_separate(tok_str, buf1, buf2, final);
	}

	/* set file parameter to keytype*/
	for (i = 1; i <= mpf->pf2_num; i++) {
		j = i + mpf->pf1_num;

		for (k = 1; k <= 4; k++) {
			ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "Key%dStr%d", k, i);

			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "tok_str snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}

			if (RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, buf2, TRUE)) {
				ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "Key%dStr%d", k, j);

				if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "tok_str snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
					goto end;
				}

				RTMPSetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, final, TRUE);
			}
		}
	}

end:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	if (tok_str)
		os_free_mem(tok_str);
	return ret;
}

/*
* Security
*/
static INT multi_profile_merge_security(
	struct mpf_data *mpf,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*IEEE8021X*/
	multi_profile_merge_perbss(mpf, "IEEE8021X", buf1, buf2, final, MPF_APPEND_BSS0);
	/*PreAuth*/
	multi_profile_merge_perbss(mpf, "PreAuth", buf1, buf2, final, MPF_APPEND_BSS0);
	/*AuthMode*/
	multi_profile_merge_perbss(mpf, "AuthMode", buf1, buf2, final, MPF_APPEND_BSS0);
	/*EncrypType*/
	multi_profile_merge_perbss(mpf, "EncrypType", buf1, buf2, final, MPF_APPEND_BSS0);
	/*RekeyMethod*/
	multi_profile_merge_perbss(mpf, "RekeyMethod", buf1, buf2, final, MPF_APPEND_BSS0);
	/*RekeyInterval*/
	multi_profile_merge_perbss(mpf, "RekeyInterval", buf1, buf2, final, MPF_APPEND_BSS0);
	/*PMKCachePeriod*/
	multi_profile_merge_perbss(mpf, "PMKCachePeriod", buf1, buf2, final, MPF_APPEND_BSS0);
	/*WPAPSK*/
	multi_profile_merge_increase(mpf, 1, "WPAPSK", buf1, buf2, final);
	/*DefaultKeyID*/
	multi_profile_merge_perbss(mpf, "DefaultKeyID", buf1, buf2, final, MPF_APPEND_BSS0);
	/*KeyType & KeyStr*/
	multi_profile_merge_keytype(mpf, buf1, buf2, final);
	/*AccessPolicy */
	multi_profile_merge_increase(mpf, 0, "AccessPolicy", buf1, buf2, final);
	/*AccessControlList*/
	multi_profile_merge_increase(mpf, 0, "AccessControlList", buf1, buf2, final);
	/*RADIUS_Server*/
	multi_profile_merge_perbss(mpf, "RADIUS_Server", buf1, buf2, final, MPF_APPEND_BSS0);
	/*RADIUS_Port*/
	multi_profile_merge_perbss(mpf, "RADIUS_Port", buf1, buf2, final, MPF_APPEND_BSS0);
	/*RADIUS_Key*/
	multi_profile_merge_perbss(mpf, "RADIUS_Key", buf1, buf2, final, MPF_APPEND_BSS0);
	/*RADIUS Key%d*/
	multi_profile_merge_radius_key(mpf, 1, "RADIUS_Key", buf1, buf2, final);
#ifdef RADIUS_ACCOUNTING_SUPPORT
	/*Accounting Interim_Interval*/
	multi_profile_merge_perbss(mpf, "acct_interim_interval", buf1, buf2, final, MPF_APPEND_BSS0);
	/*Accounting RADIUS_Server*/
	multi_profile_merge_perbss(mpf, "RADIUS_Acct_Server", buf1, buf2, final, MPF_APPEND_BSS0);
	/*Accounting RADIUS_Port*/
	multi_profile_merge_perbss(mpf, "RADIUS_Acct_Port", buf1, buf2, final, MPF_APPEND_BSS0);
	/*Accounting RADIUS_Key*/
	multi_profile_merge_perbss(mpf, "RADIUS_Acct_Key", buf1, buf2, final, MPF_APPEND_BSS0);
	/*Accounting RADIUS_Acct_Key%d*/
	multi_profile_merge_radius_key(mpf, 1, "RADIUS_Acct_Key", buf1, buf2, final);
#endif
	/*EAPifname*/
	multi_profile_merge_perbss(mpf, "EAPifname", buf1, buf2, final, MPF_APPEND_BSS0);
	/*PreAuthifname*/
	multi_profile_merge_perbss(mpf, "PreAuthifname", buf1, buf2, final, MPF_APPEND_BSS0);

	/*PMFMFPC*/
	multi_profile_merge_perbss(mpf, "PMFMFPC", buf1, buf2, final, MPF_APPEND_BSS0);
	/*PMFMFPR*/
	multi_profile_merge_perbss(mpf, "PMFMFPR", buf1, buf2, final, MPF_APPEND_BSS0);
	/*PMFSHA256*/
	multi_profile_merge_perbss(mpf, "PMFSHA256", buf1, buf2, final, MPF_APPEND_BSS0);
#ifdef BCN_PROTECTION_SUPPORT
	/* BcnProt */
	multi_profile_merge_perbss(mpf, "BcnProt", buf1, buf2, final, MPF_APPEND_BSS0);
#endif
#ifndef HOSTAPD_WPA3_SUPPORT
#ifdef DOT11_SAE_SUPPORT
	/*PWDID*/
	multi_profile_merge_increase(mpf, 1, "PWDID", buf1, buf2, final);
	/*PWDIDR*/
	multi_profile_merge_perbss(mpf, "PWDIDR", buf1, buf2, final, MPF_APPEND_BSS0);
	/*PweMethod*/
	multi_profile_merge_perbss(mpf, "PweMethod", buf1, buf2, final, MPF_APPEND_BSS0);
	/*SAEPK*/
	multi_profile_merge_perbss(mpf, "SAEPK", buf1, buf2, final, MPF_APPEND_BSS0);
	/*SAEPKInputMode*/
	multi_profile_merge_perbss(mpf, "SAEPKInputMode", buf1, buf2, final, MPF_APPEND_BSS0);
	/*SAEPKKeyFilePath*/
	multi_profile_merge_increase(mpf, 1, "SAEPKKeyFilePath", buf1, buf2, final);
	/*SAEPKCfg*/
	multi_profile_merge_perbss(mpf, "SAEPKCfg", buf1, buf2, final, MPF_APPEND_BSS0);
	/*SAEPKKey*/
	multi_profile_merge_increase(mpf, 1, "SAEPKKey", buf1, buf2, final);
	/*SAEPKStartM*/
	multi_profile_merge_increase(mpf, 1, "SAEPKStartM", buf1, buf2, final);
	/*SAEPKSec*/
	multi_profile_merge_perbss(mpf, "SAEPKSec", buf1, buf2, final, MPF_APPEND_BSS0);
	/*SAEPKLambda*/
	multi_profile_merge_perbss(mpf, "SAEPKLambda", buf1, buf2, final, MPF_APPEND_BSS0);
	/*SAEPKGroup*/
	multi_profile_merge_perbss(mpf, "SAEPKGroup", buf1, buf2, final, MPF_APPEND_BSS0);
#endif
#endif /*HOSTAPD_WPA3_SUPPORT*/
	/* OCVSupport */
	multi_profile_merge_perbss(mpf, "OCVSupport", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(mpf, "TransitionDisable", buf1, buf2, final, MPF_APPEND_BSS0);
	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
/* TODO: related to multi_profile_merge_apedca */
static INT multi_profile_merge_default_edca(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR i;
	CHAR aifs[32] = "";
	CHAR cwmin[32] = "";
	CHAR cwmax[32] = "";
	CHAR txop[32] = "";
	CHAR acm[32] = "";
	CHAR *buf = NULL;
	CHAR *value;
	UCHAR idx;
	CHAR tok_str[25] = "";
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	for (i = 0; i < 2; i++) {
		buf = (i == 0) ? buf1 : buf2;
		os_zero_mem(value, TEMP_STR_SIZE);

		/*APAifsn*/
		if (RTMPGetKeyParameter("APAifsn", aifs, sizeof(aifs), buf, FALSE) != TRUE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_replace(";", ",", aifs);

		/*APCwmin*/
		if (RTMPGetKeyParameter("APCwmin", cwmin, sizeof(cwmin), buf, FALSE) != TRUE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_replace(";", ",", cwmin);

		/*APCwmax*/
		if (RTMPGetKeyParameter("APCwmax", cwmax, sizeof(cwmax), buf, FALSE) != TRUE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_replace(";", ",", cwmax);

		/*APTxop*/
		if (RTMPGetKeyParameter("APTxop", txop, sizeof(txop), buf, FALSE) != TRUE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_replace(";", ",", txop);

		/*APACM*/
		if (RTMPGetKeyParameter("APACM", acm, sizeof(acm), buf, FALSE) != TRUE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_replace(";", ",", acm);
		/*merge*/
		ret_tmp = snprintf(value, TEMP_STR_SIZE, "1;%s;%s;%s;%s;%s", aifs, cwmin, cwmax, txop, acm);

		if (ret_tmp < 0 || ret_tmp >= TEMP_STR_SIZE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		/*set*/
		ret_tmp = snprintf(tok_str, sizeof(tok_str), "APEdca%d", i);

		if (ret_tmp < 0 || ret_tmp >= sizeof(tok_str)) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		RTMPSetKeyParameter(tok_str, value, TEMP_STR_SIZE, final, TRUE);
	}

	os_zero_mem(value, TEMP_STR_SIZE);

	for (i = 0; i < data->total_num; i++) {
		CHAR tmp[TEMP_STR_SIZE] = {0};

		idx = (i < data->pf1_num) ? 0 : 1;
		os_move_mem(tmp, value, strlen(value) + 1);
		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s%d;", tmp, idx);

		if (ret_tmp < 0 || ret_tmp >= TEMP_STR_SIZE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}
	}

	RTMPSetKeyParameter("EdcaIdx", value, TEMP_STR_SIZE, final, TRUE);

end:
	if (value)
		os_free_mem(value);
	return ret;
}

/*
* apedca
*/
static INT multi_profile_merge_apedca(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR *tmpbuf;
	CHAR *tmpbuf2;
	UCHAR edca_idx = 0;
	CHAR *macptr;
	UCHAR i;
	UCHAR j;
	UCHAR edca_own[4] = { 0, 0, 0, 0 };
	CHAR tok_str[25] = "";
	CHAR tmp[TEMP_STR_SIZE] = {0};
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf2, TEMP_STR_SIZE);

	if (!tmpbuf || !tmpbuf2) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		goto end;
	}

	os_zero_mem(tmpbuf, TEMP_STR_SIZE);
	os_zero_mem(tmpbuf2, TEMP_STR_SIZE);

	/*for seach 2.4G band EdcaIdx*/
	if (RTMPGetKeyParameter("EdcaIdx", tmpbuf, TEMP_STR_SIZE, buf1, FALSE) != TRUE) {
		/*default EDCA parameters*/
		multi_profile_merge_default_edca(data, buf1, buf2, final);
		goto end;
	}

	for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		edca_idx = simple_strtol(macptr, 0, 10);

		if (edca_idx < 4)
			edca_own[edca_idx] = (0x10 | edca_idx);
	}

	/*for seach 5G band EdcaIdx*/
	if (RTMPGetKeyParameter("EdcaIdx", tmpbuf2, TEMP_STR_SIZE, buf2, FALSE) != TRUE)
		goto end;

	for (i = 0, macptr = rstrtok(tmpbuf2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		edca_idx = simple_strtol(macptr, 0, 10);

		if (edca_idx >= 4) {
			os_move_mem(tmp, tmpbuf, strlen(tmpbuf) + 1);
			ret_tmp = snprintf(tmpbuf, TEMP_STR_SIZE, "%s;%d", tmp, 0);

			if (ret_tmp < 0 || ret_tmp >= TEMP_STR_SIZE) {
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}

			continue;
		}

		for (j = 0; j < 4; j++) {
			if (edca_own[j] == 0)
				break;
		}

		if (j < 4)
			edca_own[j] = (0x20 | edca_idx);

		os_move_mem(tmp, tmpbuf, strlen(tmpbuf) + 1);
		ret_tmp = snprintf(tmpbuf, TEMP_STR_SIZE, "%s;%d", tmp, j);

		if (ret_tmp < 0 || ret_tmp >= TEMP_STR_SIZE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

	}

	RTMPSetKeyParameter("EdcaIdx", tmpbuf, TEMP_STR_SIZE, final, TRUE);

	/*merge  ApEdca%*/
	for (i = 0; i < 4; i++) {
		if (edca_own[i] & 0x10)
			macptr = buf1;
		else if (edca_own[i] & 0x20)
			macptr = buf2;
		else
			continue;

		j = (edca_own[i] & 0x3);
		ret_tmp = snprintf(tok_str, sizeof(tok_str), "APEdca%d", j);

		if (ret_tmp < 0 || ret_tmp >= sizeof(tok_str)) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, macptr, TRUE)) {
			ret_tmp = snprintf(tok_str, sizeof(tok_str), "APEdca%d", i);
			if (os_snprintf_error(sizeof(tok_str), ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "tok_str snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, final, TRUE);
		}
	}

end:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	if (tmpbuf2)
		os_free_mem(tmpbuf2);
	return ret;
}

/*
* bssedca
*/
static INT multi_profile_merge_bssedca(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR i;
	CHAR aifs[32] = "";
	CHAR cwmin[32] = "";
	CHAR cwmax[32] = "";
	CHAR txop[32] = "";
	CHAR acm[32] = "";
	CHAR *buf = NULL;
	CHAR *value;
	CHAR tok_str[25] = "";
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	for (i = 0; i < data->total_num; i++) {
		buf = (i < data->pf1_num) ? buf1 : buf2;
		os_zero_mem(value, TEMP_STR_SIZE);

		/*BSSAifsn*/
		if (RTMPGetKeyParameter("BSSAifsn", aifs, sizeof(aifs), buf, FALSE) != TRUE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_replace(";", ",", aifs);

		/*BSSCwmin*/
		if (RTMPGetKeyParameter("BSSCwmin", cwmin, sizeof(cwmin), buf, FALSE) != TRUE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_replace(";", ",", cwmin);

		/*BSSCwmax*/
		if (RTMPGetKeyParameter("BSSCwmax", cwmax, sizeof(cwmax), buf, FALSE) != TRUE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_replace(";", ",", cwmax);

		/*BSSTxop*/
		if (RTMPGetKeyParameter("BSSTxop", txop, sizeof(txop), buf, FALSE) != TRUE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_replace(";", ",", txop);

		/*BSSACM*/
		if (RTMPGetKeyParameter("BSSACM", acm, sizeof(acm), buf, FALSE) != TRUE) {
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		multi_profile_replace(";", ",", acm);
		/*merge*/
		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s;%s;%s;%s", aifs, cwmin, cwmax, txop, acm);

		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		/*set*/
		ret_tmp = snprintf(tok_str, sizeof(tok_str), "BSSEdca%d", i);

		if (os_snprintf_error(sizeof(tok_str), ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "tok_str snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		RTMPSetKeyParameter(tok_str, value, TEMP_STR_SIZE, final, TRUE);
	}

end:
	if (value)
		os_free_mem(value);
	return ret;
}

static INT multi_profile_merge_chgrp(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR *chgrp1 = NULL;
	UCHAR *chgrp2 = NULL;
	UCHAR *value = NULL;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, (UCHAR **)&chgrp1, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&chgrp2, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!chgrp1 || !chgrp2 || !value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(chgrp1, TEMP_STR_SIZE);
	os_zero_mem(chgrp2, TEMP_STR_SIZE);
	os_zero_mem(value, TEMP_STR_SIZE);

	if (RTMPGetKeyParameter("ChannelGrp", chgrp1, TEMP_STR_SIZE, buf1, TRUE) != TRUE) {
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}
	if (strlen(chgrp1) < LEN_BITMAP_CHGRP) {
		ret_tmp = snprintf(chgrp1, TEMP_STR_SIZE, "0:0:0:0");
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}
	}

	if (RTMPGetKeyParameter("ChannelGrp", chgrp2, TEMP_STR_SIZE, buf2, TRUE) != TRUE) {
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}
	if (strlen(chgrp2) < LEN_BITMAP_CHGRP) {
		ret_tmp = snprintf(chgrp2, TEMP_STR_SIZE, "0:0:0:0");
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}
	}

	os_zero_mem(value, TEMP_STR_SIZE);
	ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", chgrp1, chgrp2);
	if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "value snprintf error!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	RTMPSetKeyParameter("ChannelGrp", value, TEMP_STR_SIZE, final, TRUE);

end:
	if (chgrp1)
		os_free_mem(chgrp1);
	if (chgrp2)
		os_free_mem(chgrp2);
	if (value)
		os_free_mem(value);
	return ret;
}

/*
* VOW_BW_Ctrl
*/
static INT multi_profile_merge_vow_bw_ctrl(
	struct mpf_data *data,
	CHAR *parm,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR *group1;
	UCHAR *group2;
	UCHAR *value;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, (UCHAR **)&group1, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&group2, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!group1 || !group2 || !value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(group1, TEMP_STR_SIZE);
	os_zero_mem(group2, TEMP_STR_SIZE);
	os_zero_mem(value, TEMP_STR_SIZE);

	if (RTMPGetKeyParameter(parm, group1, TEMP_STR_SIZE, buf1, TRUE) != TRUE) {
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	if (RTMPGetKeyParameter(parm, group2, TEMP_STR_SIZE, buf2, TRUE) != TRUE) {
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(value, TEMP_STR_SIZE);

	ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", group1, group2);
	if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"value snprintf error!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	RTMPSetKeyParameter(parm, value, TEMP_STR_SIZE, final, TRUE);

end:
	if (group1)
		os_free_mem(group1);
	if (group2)
		os_free_mem(group2);
	if (value)
		os_free_mem(value);
	return ret;
}

/*
*
*/
static INT multi_profile_merge_ack_policy(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	UCHAR i;
	UCHAR idx;
	CHAR tmpbuf[32];
	CHAR tok_str[25] = "";
	INT ret_tmp;

	/*read 2.4G profile*/
	if (RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buf1, TRUE)) {
		for (i = 0 ; i < data->pf1_num ; i++) {
			ret_tmp = snprintf(tok_str, sizeof(tok_str), "APAckPolicy%d", i);
			if (os_snprintf_error(sizeof(tok_str), ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "tok_str snprintf error!!!\n");
				return NDIS_STATUS_FAILURE;
			}
			RTMPSetKeyParameter(tok_str, tmpbuf, sizeof(tmpbuf), final, TRUE);
		}
	}

	/*read 5G profile*/
	if (RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buf2, TRUE)) {
		for (i = 0 ; i < data->pf2_num ; i++) {
			idx = i + data->pf1_num;
			ret_tmp = snprintf(tok_str, sizeof(tok_str), "APAckPolicy%d", idx);
			if (os_snprintf_error(sizeof(tok_str), ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "tok_str snprintf error!!!\n");
				return NDIS_STATUS_FAILURE;
			}
			RTMPSetKeyParameter(tok_str, tmpbuf, sizeof(tmpbuf), final, TRUE);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

/*
* Country Region
*/
static INT multi_profile_merge_country_region(
	struct mpf_data *data,
	CHAR *parm,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT Status = FALSE;
	CHAR *value;
	CHAR *tmpbuf;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!tmpbuf || !value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(tmpbuf, TEMP_STR_SIZE);
	os_zero_mem(value, TEMP_STR_SIZE);

#ifdef DEFAULT_5G_PROFILE
	Status = RTMPGetKeyParameter(parm, tmpbuf, TEMP_STR_SIZE, buf1, TRUE);
#else
	Status = RTMPGetKeyParameter(parm, tmpbuf, TEMP_STR_SIZE, buf2, TRUE);
#endif

	if (Status == TRUE) {
		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}
		RTMPSetKeyParameter(parm, value, TEMP_STR_SIZE, final, TRUE);
	}

end:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	if (value)
		os_free_mem(value);
	return ret;
}

/*
* mbss related merge function
*/
static INT multi_profile_merge_mbss(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge AP/APCLI MACAddress*/
	multi_profile_merge_mac_address(data, buf1, buf2, final);
	/*merge SSID*/
	multi_profile_merge_increase(data, 1, "SSID", buf1, buf2, final);
	/*merge FragThreshold*/
	multi_profile_merge_perbss(data, "FragThreshold", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge DLSCapable*/
	multi_profile_merge_separate("DLSCapable", buf1, buf2, final);
	/* MAP config */
#ifdef CONFIG_MAP_SUPPORT
	multi_profile_merge_separate("MapEnable", buf1, buf2, final);
	multi_profile_merge_separate("MAP_Turnkey", buf1, buf2, final);
	multi_profile_merge_separate("MAP_Ext", buf1, buf2, final);
#endif
	/*merge WirelessMode*/
	multi_profile_merge_perbss(data, "WirelessMode", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge Channel*/
	multi_profile_merge_separate("Channel", buf1, buf2, final);
	/*merge ChannelGrp*/
	multi_profile_merge_chgrp(buf1, buf2, final);
	/*merge AutoChannelSelect*/
	multi_profile_merge_separate("AutoChannelSelect", buf1, buf2, final);
	/*merge AutoChannelSkipList*/
	multi_profile_merge_separate("PSC_ACS", buf1, buf2, final);
	/*merge AutoChannelSkipList*/
	multi_profile_merge_separate("AutoChannelSkipList", buf1, buf2, final);
	/*merge ACSCheckTime*/
	multi_profile_merge_separate("ACSCheckTime", buf1, buf2, final);
	/*merge ACSCheckTimeMin*/
	multi_profile_merge_separate("ACSCheckMinTime", buf1, buf2, final);
	/*merge security*/
	multi_profile_merge_security(data, buf1, buf2, final);
	/*merge WmmCapable*/
	multi_profile_merge_separate("WmmCapable", buf1, buf2, final);
	/*merge NoForwarding*/
	multi_profile_merge_separate("NoForwarding", buf1, buf2, final);
	/*merge StationKeepAlive*/
	multi_profile_merge_perbss(data, "StationKeepAlive", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge HideSSID*/
	multi_profile_merge_perbss(data, "HideSSID", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge HT_EXTCHA*/
	multi_profile_merge_perbss(data, "HT_EXTCHA", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge HT_TxStream*/
	multi_profile_merge_separate("HT_TxStream", buf1, buf2, final);
	/*merge HT_RxStream*/
	multi_profile_merge_separate("HT_RxStream", buf1, buf2, final);
	/*merge HT_MCS*/
	multi_profile_merge_separate("HT_MCS", buf1, buf2, final);
	/*merge HT_BW*/
	multi_profile_merge_perbss(data, "HT_BW", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge HT_STBC*/
	multi_profile_merge_perbss(data, "HT_STBC", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge HT_LDPC*/
	multi_profile_merge_perbss(data, "HT_LDPC", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge HT_AMSDU*/
	multi_profile_merge_perbss(data, "HT_AMSDU", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge AMSDU_NUM*/
	multi_profile_merge_perbss(data, "AMSDU_NUM", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge PPDUTxType*/
	multi_profile_merge_perbss(data, "PPDUTxType", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge OFDMA*/
	multi_profile_merge_perbss(data, "OFDMA", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge NumUsersOFDMA*/
	multi_profile_merge_perbss(data, "NumUsersOFDMA", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge MuEdcaOverride*/
	multi_profile_merge_perbss(data, "MuEdcaOverride", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge NonTxBSSIndex*/
	multi_profile_merge_perbss(data, "NonTxBSSIndex", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge HE_TXOP_RTS_THLD*/
	multi_profile_merge_perbss(data, "HE_TXOP_RTS_THLD", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge Fixed MCS*/
	multi_profile_merge_perbss(data, "FixedMcs", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge VHT_STBC*/
	multi_profile_merge_perbss(data, "VHT_STBC", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge VHT_LDPC*/
	multi_profile_merge_perbss(data, "VHT_LDPC", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge HeLdpc*/
	multi_profile_merge_perbss(data, "HeLdpc", buf1, buf2, final, MPF_APPEND_BSS0);
#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
	/*merge FgiFltf*/
	multi_profile_merge_perbss(data, "FgiFltf", buf1, buf2, final, MPF_APPEND_BSS0);
#endif
	/*merge MbssMaxStaNum*/
	multi_profile_merge_separate("MbssMaxStaNum", buf1, buf2, final);
	/*merge UAPSDCapable*/
	multi_profile_merge_perbss(data, "UAPSDCapable", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_separate("APSDCapable", buf1, buf2, final); /* backward compatible with old SDK */
	/*merge APEdcaIdx*/
	multi_profile_merge_apedca(data, buf1, buf2, final);
	/*merge BSSEdcaIdx*/
	multi_profile_merge_bssedca(data, buf1, buf2, final);
	/*merge AckPolicy*/
	multi_profile_merge_ack_policy(data, buf1, buf2, final);
	/*merge CountryRegionABand*/
	multi_profile_merge_country_region(data, "CountryRegionABand", buf1, buf2, final);
	/*merge VOW BW_Ctrl related profile*/
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Min_Rate", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Max_Rate", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Min_Ratio", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Max_Ratio", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Airtime_Ctrl_En", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Rate_Ctrl_En", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Min_Rate_Bucket_Size", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Max_Rate_Bucket_Size", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Min_Airtime_Bucket_Size", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Max_Airtime_Bucket_Size", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Backlog", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_Max_Wait_Time", buf1, buf2, final);
	multi_profile_merge_vow_bw_ctrl(data, "VOW_Group_DWRR_Quantum", buf1, buf2, final);
#if defined(WSC_V2_SUPPORT) || defined(WSC_AP_SUPPORT)
	multi_profile_merge_wsc(data, buf1, buf2, final);
#endif
#ifdef TXBF_SUPPORT
	/*merge ETxBfEnCond*/
	multi_profile_merge_perbss(data, "ETxBfEnCond", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge ITxBfEn*/
	multi_profile_merge_perbss(data, "ITxBfEn", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge ITxBfEn*/
	multi_profile_merge_perbss(data, "BfSmthIntlBbypass", buf1, buf2, final, MPF_APPEND_BSS0);
	/* merge HeraStbcPriority */
	multi_profile_merge_separate("HeraStbcPriority", buf1, buf2, final);
#endif /* TXBF_SUPPORT */
	multi_profile_merge_perbss(data, "MuOfdmaDlEnable", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "MuOfdmaUlEnable", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "MuMimoDlEnable", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "MuMimoUlEnable", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "HeOmiUlMuDataDisableRx", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "HeErSuRxDisable", buf1, buf2, final, MPF_APPEND_BSS0);
#ifdef CFG_SUPPORT_FALCON_MURU
	multi_profile_merge_separate("Disable160RuMu", buf1, buf2, final);
	multi_profile_merge_separate("MaxRuOfdma", buf1, buf2, final);
	multi_profile_merge_separate("MaxDLMuMimo", buf1, buf2, final);
	multi_profile_merge_separate("MaxULMuMimo", buf1, buf2, final);
#endif /* CFG_SUPPORT_FALCON_MURU*/
#ifdef CFG_SUPPORT_FALCON_SR
	/* Spatial Reuse */
	/* merge SREnable */
	multi_profile_merge_separate("SREnable", buf1, buf2, final);
	/* merge SRMode */
	multi_profile_merge_separate("SRMode", buf1, buf2, final);
	/* merge SRSDEnable */
	multi_profile_merge_separate("SRSDEnable", buf1, buf2, final);
	/* merge SRDPDEnable */
	multi_profile_merge_separate("SRDPDEnable", buf1, buf2, final);
	/* merge SRDropMinMcs */
	multi_profile_merge_separate("SRDropMinMcs", buf1, buf2, final);
	/* merge SRDPDThreshold */
	multi_profile_merge_separate("SRDPDThreshold", buf1, buf2, final);
#endif /* CFG_SUPPORT_FALCON_SR */

#ifdef CFG_SUPPORT_FALCON_PP
	/* Preamble puncture  */
	/* merge PPEnable  */
	multi_profile_merge_separate("PPEnable", buf1, buf2, final);
#endif /* CFG_SUPPORT_FALCON_PP */

#ifdef DOT11V_MBSSID_SUPPORT
	multi_profile_merge_perbss(data, "Dot11vMbssid", buf1, buf2, final, MPF_APPEND_BSS0);
#endif
	/*BssMaxIdle*/
	multi_profile_merge_perbss(data, "BssMaxIdle", buf1, buf2, final, MPF_APPEND_BSS0);

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	multi_profile_merge_separate("TWTSupport", buf1, buf2, final);
	multi_profile_merge_separate("TWTInfoFrame", buf1, buf2, final);
#endif
	/* HeDynSmps */
	multi_profile_merge_perbss(data, "HeDynSmps", buf1, buf2, final, MPF_APPEND_BSS0);
#ifdef CONFIG_AP_SUPPORT
	multi_profile_merge_separate("BSSColorValue", buf1, buf2, final);
#endif

#endif

	multi_profile_merge_separate("Wifi6gCap", buf1, buf2, final);

#ifdef DYNAMIC_WMM_SUPPORT
	multi_profile_merge_separate("DynWmmEnable", buf1, buf2, final);
#endif /* DYNAMIC_WMM_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
	multi_profile_merge_separate("SRMeshUlMode", buf1, buf2, final);
#endif /* CONFIG_MAP_SUPPORT */

#ifdef WIFI_CSI_CN_INFO_SUPPORT
	multi_profile_merge_separate("EnableCNInfo", buf1, buf2, final);
#endif /* WIFI_CSI_CN_INFO_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}

/*
* wsc related merge function
*/
#if defined(WSC_V2_SUPPORT) || defined(WSC_AP_SUPPORT)
static INT multi_profile_merge_wsc(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{

#if defined(WSC_V2_SUPPORT) || defined(WSC_AP_SUPPORT)
	UCHAR WscConMode[32] = "";
	UCHAR WscConMode2[32] = "";
	UCHAR *value;
	UCHAR DeviceName[128] = "";
	UCHAR DeviceName2[128] = "";
	UCHAR WscPin[128] = "";
	UCHAR WscPin2[128] = "";
	INT	i, ret_tmp;
	INT ret = NDIS_STATUS_SUCCESS;
	CHAR *macptr;

	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		goto end;
	}

	os_zero_mem(value, TEMP_STR_SIZE);
#endif
#ifdef WSC_V2_SUPPORT
	/*merge WscV2Support*/
	{
		/*merge WscConfMode*/
		if (RTMPGetKeyParameter("WscV2Support", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_wsc_v2_done;

		for (i = 0, macptr = rstrtok(WscConMode, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscV2Support", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_wsc_v2_done;

		if (data->pf1_num > i) {/* need to append default value */
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(WscConMode));
			os_zero_mem(tmp, sizeof(WscConMode));

			while (append_cnt) {
				os_move_mem(tmp, WscConMode, strlen(WscConMode) + 1);
				ret_tmp = snprintf(WscConMode, sizeof(WscConMode), "%s; ", tmp);
				if (os_snprintf_error(sizeof(WscConMode), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "WscConMode snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf1_num < i)
			goto label_wsc_v2_done;

		if (RTMPGetKeyParameter("WscV2Support", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_wsc_v2_done;

		for (i = 0, macptr = rstrtok(WscConMode2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscV2Support", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_wsc_v2_done;

		if (data->pf2_num > i) {/* need to append default value */
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(WscConMode2));
			os_zero_mem(tmp, sizeof(WscConMode2));

			while (append_cnt) {
				os_move_mem(tmp, WscConMode2, strlen(WscConMode2) + 1);
				ret_tmp = snprintf(WscConMode2, sizeof(WscConMode2), "%s; ", tmp);
				if (os_snprintf_error(sizeof(WscConMode2), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "WscConMode snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf2_num < i)
			goto label_wsc_v2_done;

		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", WscConMode, WscConMode2);
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
		}
		RTMPSetKeyParameter("WscV2Support", value, TEMP_STR_SIZE, final, TRUE);
	}
label_wsc_v2_done:
#endif /*WSC_V2_SUPPORT*/
#ifdef WSC_AP_SUPPORT
	{
		/*merge WscConfMode*/
		if (RTMPGetKeyParameter("WscConfMode", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_WscConfMode_done;

		for (i = 0, macptr = rstrtok(WscConMode, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscConfMode", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_WscConfMode_done;

		if (data->pf1_num > i) {/* need to append default value */
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(WscConMode));
			os_zero_mem(tmp, sizeof(WscConMode));

			while (append_cnt) {
				os_move_mem(tmp, WscConMode, strlen(WscConMode) + 1);
				ret_tmp = snprintf(WscConMode, sizeof(WscConMode), "%s; ", tmp);
				if (os_snprintf_error(sizeof(WscConMode), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "WscConMode snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf1_num < i)
			goto label_WscConfMode_done;

		if (RTMPGetKeyParameter("WscConfMode", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_WscConfMode_done;

		for (i = 0, macptr = rstrtok(WscConMode2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscConfMode", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_WscConfMode_done;

		if (data->pf2_num > i) {/* need to append default value */
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(WscConMode2));
			os_zero_mem(tmp, sizeof(WscConMode2));

			while (append_cnt) {
				os_move_mem(tmp, WscConMode2, strlen(WscConMode2) + 1);
				ret_tmp = snprintf(WscConMode2, sizeof(WscConMode2), "%s; ", tmp);
				if (os_snprintf_error(sizeof(WscConMode2), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "WscConMode2 snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf2_num < i)
			goto label_WscConfMode_done;

		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", WscConMode, WscConMode2);
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
		}
		if (RTMPSetKeyParameter("WscConfMode", value, TEMP_STR_SIZE, final, TRUE) != TRUE)
			;
label_WscConfMode_done:

		/*merge WscConfStatus*/
		if (RTMPGetKeyParameter("WscConfStatus", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_WscConfStatus_done;

		for (i = 0, macptr = rstrtok(WscConMode, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscConfStatus", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_WscConfStatus_done;

		if (data->pf1_num > i) {/* need to append default value */
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(WscConMode));
			os_zero_mem(tmp, sizeof(WscConMode));

			while (append_cnt) {
				os_move_mem(tmp, WscConMode, strlen(WscConMode) + 1);
				ret_tmp = snprintf(WscConMode, sizeof(WscConMode), "%s; ", tmp);
				if (os_snprintf_error(sizeof(WscConMode), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "WscConMode snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf1_num < i)
			goto label_WscConfStatus_done;

		if (RTMPGetKeyParameter("WscConfStatus", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_WscConfStatus_done;

		for (i = 0, macptr = rstrtok(WscConMode2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscConfStatus", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_WscConfStatus_done;

		if (data->pf2_num > i) {/* need to append default value */
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(WscConMode2));
			os_zero_mem(tmp, sizeof(WscConMode2));

			while (append_cnt) {
				os_move_mem(tmp, WscConMode2, strlen(WscConMode2) + 1);
				ret_tmp = snprintf(WscConMode2, sizeof(WscConMode2), "%s; ", tmp);
				if (os_snprintf_error(sizeof(WscConMode2), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "WscConMode2 snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf2_num < i)
			goto label_WscConfStatus_done;

		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", WscConMode, WscConMode2);
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
		}
		if (RTMPSetKeyParameter("WscConfStatus", value, TEMP_STR_SIZE, final, TRUE) != TRUE)
			;
label_WscConfStatus_done:

		/*merge WscConfMethods*/
		if (RTMPGetKeyParameter("WscConfMethods", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_WscConfMethods_done;

		for (i = 0, macptr = rstrtok(WscConMode, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscConfMethods", WscConMode, sizeof(WscConMode), buf1, TRUE) != TRUE)
			goto label_WscConfMethods_done;

		if (data->pf1_num > i) {/* need to append default value */
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(WscConMode));
			os_zero_mem(tmp, sizeof(WscConMode));

			while (append_cnt) {
				os_move_mem(tmp, WscConMode, strlen(WscConMode) + 1);
				ret_tmp = snprintf(WscConMode, sizeof(WscConMode), "%s; ", tmp);
				if (os_snprintf_error(sizeof(WscConMode), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "WscConMode snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf1_num < i)
			goto label_WscConfMethods_done;

		if (RTMPGetKeyParameter("WscConfMethods", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_WscConfMethods_done;

		for (i = 0, macptr = rstrtok(WscConMode2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscConfMethods", WscConMode2, sizeof(WscConMode2), buf2, TRUE) != TRUE)
			goto label_WscConfMethods_done;

		if (data->pf2_num > i) {/* need to append default value */
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(WscConMode2));
			os_zero_mem(tmp, sizeof(WscConMode2));

			while (append_cnt) {
				os_move_mem(tmp, WscConMode2, strlen(WscConMode2) + 1);
				ret_tmp = snprintf(WscConMode2, sizeof(WscConMode2), "%s; ", tmp);
				if (os_snprintf_error(sizeof(WscConMode2), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "WscConMode2 snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf2_num < i)
			goto label_WscConfMethods_done;

		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", WscConMode, WscConMode2);
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
		}
		if (RTMPSetKeyParameter("WscConfMethods", value, TEMP_STR_SIZE, final, TRUE) != TRUE)
			goto label_WscConfMethods_done;
	}
label_WscConfMethods_done:

		if (RTMPGetKeyParameter("WscDeviceName", DeviceName, sizeof(DeviceName), buf1, TRUE) != TRUE)
			goto label_WscdeviceName_done;

		for (i = 0, macptr = rstrtok(DeviceName, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscDeviceName", DeviceName, sizeof(DeviceName), buf1, TRUE) != TRUE)
			goto label_WscdeviceName_done;

		if (data->pf1_num > i) {/* need to append default value */
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(DeviceName));
			os_zero_mem(tmp, sizeof(DeviceName));

			while (append_cnt) {
				os_move_mem(tmp, DeviceName, strlen(DeviceName) + 1);
				ret_tmp = snprintf(DeviceName, sizeof(DeviceName), "%s; ", tmp);
				if (os_snprintf_error(sizeof(DeviceName), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "DeviceName snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf1_num < i)
				goto label_WscdeviceName_done;

		if (RTMPGetKeyParameter("WscDeviceName", DeviceName2, sizeof(DeviceName2), buf2, TRUE) != TRUE)
			goto label_WscdeviceName_done;

		for (i = 0, macptr = rstrtok(DeviceName2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscDeviceName", DeviceName2, sizeof(DeviceName2), buf2, TRUE) != TRUE)
			goto label_WscdeviceName_done;

		if (data->pf2_num > i) {/* need to append default value */
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(DeviceName2));
			os_zero_mem(tmp, sizeof(DeviceName2));

			while (append_cnt) {
				os_move_mem(tmp, DeviceName2, strlen(DeviceName2) + 1);
				ret_tmp = snprintf(DeviceName2, sizeof(DeviceName2), "%s; ", tmp);
				if (os_snprintf_error(sizeof(DeviceName2), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "DeviceName2 snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf2_num < i)
				goto label_WscdeviceName_done;

		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", DeviceName, DeviceName2);
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
		}

		if (RTMPSetKeyParameter("WscDeviceName", value, TEMP_STR_SIZE, final, TRUE) != TRUE)
			;
label_WscdeviceName_done:

		if (RTMPGetKeyParameter("WscVendorPinCode", WscPin, sizeof(WscPin), buf1, TRUE) != TRUE)
			goto label_WscVendorPinCode_done;

		for (i = 0, macptr = rstrtok(WscPin, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscVendorPinCode", WscPin, sizeof(WscPin), buf1, TRUE) != TRUE)
			goto label_WscVendorPinCode_done;

		if (data->pf1_num > i) {/* need to append default value */
			INT append_cnt = data->pf1_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(WscPin));
			os_zero_mem(tmp, sizeof(WscPin));

			while (append_cnt) {
				os_move_mem(tmp, WscPin, strlen(WscPin) + 1);
				ret_tmp = snprintf(WscPin, sizeof(WscPin), "%s; ", tmp);
				if (os_snprintf_error(sizeof(WscPin), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "WscPin snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf1_num < i)
				goto label_WscVendorPinCode_done;

		if (RTMPGetKeyParameter("WscVendorPinCode", WscPin2, sizeof(WscPin2), buf2, TRUE) != TRUE)
			goto label_WscVendorPinCode_done;

		for (i = 0, macptr = rstrtok(WscPin2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			;/*do nothing*/

		if (RTMPGetKeyParameter("WscVendorPinCode", WscPin2, sizeof(WscPin2), buf2, TRUE) != TRUE)
			goto label_WscVendorPinCode_done;

		if (data->pf2_num > i) {/* need to append default value */
			INT append_cnt = data->pf2_num - i;
			INT loop_cnt = 0;
			UCHAR *tmp = NULL;

			os_alloc_mem_suspend(NULL, (UCHAR **)&tmp, sizeof(WscPin2));
			os_zero_mem(tmp, sizeof(WscPin2));

			while (append_cnt) {
				os_move_mem(tmp, WscPin2, strlen(WscPin2) + 1);
				ret_tmp = snprintf(WscPin2, sizeof(WscPin2), "%s; ", tmp);
				if (os_snprintf_error(sizeof(WscPin2), ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "WscPin2 snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
				}
				append_cnt--;
				loop_cnt++;
			}
			if (tmp)
				os_free_mem(tmp);
		} else if (data->pf2_num < i)
				goto label_WscVendorPinCode_done;

		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s;%s", WscPin, WscPin2);
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
		}

		if (RTMPSetKeyParameter("WscVendorPinCode", value, TEMP_STR_SIZE, final, TRUE) != TRUE)
			goto label_WscdeviceName_done;
label_WscVendorPinCode_done:

#endif /*WSC_AP_SUPPORT*/
#if defined(WSC_V2_SUPPORT) || defined(WSC_AP_SUPPORT)
end:
	if (value)
		os_free_mem(value);
#endif
	return ret;
}
#endif
#endif /*MBSS_SUPPORT*/

#ifdef APCLI_SUPPORT
/*
* apcli related merge function
*/
static INT multi_profile_merge_apcli(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT status = FALSE;
	CHAR *tmpbuf;
	CHAR *value;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!tmpbuf || !value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(tmpbuf, TEMP_STR_SIZE);
	os_zero_mem(value, TEMP_STR_SIZE);

#ifdef MAC_REPEATER_SUPPORT
	/*MACRepeaterEn is enabled by any band profile*/
	multi_profile_merge_separate_no_initial_value("MACRepeaterEn", buf1, buf2, final);

	/*MACRepeaterOuiMode, use profile 1*/
	status = RTMPGetKeyParameter("MACRepeaterOuiMode", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

	if (status == TRUE) {
		ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "value snprintf error!!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}
		RTMPSetKeyParameter("MACRepeaterOuiMode", value, TEMP_STR_SIZE, final, TRUE);
	}

#endif
	/*merge ApCliEnable*/
	multi_profile_merge_separate_no_initial_value("ApCliEnable", buf1, buf2, final);
	/*merge ApCliSsid*/
	/*multi_profile_merge_separate("ApCliSsid", buf1, buf2, final);*/
	/*read SSID with space allowed*/
	multi_profile_merge_separate_space("ApCliSsid", buf1, buf2, final);

	/*merge ApCliWirelessMode*/
	multi_profile_merge_separate("ApCliWirelessMode", buf1, buf2, final);
	/*merge ApCliBssid*/
	multi_profile_merge_separate("ApCliBssid", buf1, buf2, final);
	/*merge ApCliAuthMode*/
	multi_profile_merge_separate("ApCliAuthMode", buf1, buf2, final);
	/*merge ApCliEncrypType*/
	multi_profile_merge_separate("ApCliEncrypType", buf1, buf2, final);
	{
		/*merge apcli0 ApCliWPAPSK*/
		status = RTMPGetKeyParameter("ApCliWPAPSK", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

		if (status == TRUE) {
			ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter("ApCliWPAPSK", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*tansfer apcli1 ApCliWPAPSK to ApCliWPAPSK1*/
		status = RTMPGetKeyParameter("ApCliWPAPSK", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);

		if (status == TRUE) {
			ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter("ApCliWPAPSK1", value, TEMP_STR_SIZE, final, TRUE);
		}
	}
	/*merge ApCliDefaultKeyID*/
	multi_profile_merge_separate("ApCliDefaultKeyID", buf1, buf2, final);
	/*merge ApCliKey1Type*/
	multi_profile_merge_separate("ApCliKey1Type", buf1, buf2, final);
	/*merge ApCliKey2Type*/
	multi_profile_merge_separate("ApCliKey2Type", buf1, buf2, final);
	/*merge ApCliKey3Type*/
	multi_profile_merge_separate("ApCliKey3Type", buf1, buf2, final);
	/*merge ApCliKey4Type*/
	multi_profile_merge_separate("ApCliKey4Type", buf1, buf2, final);
	{
		/*merge apcli0 ApCliKey1Str*/
		status = RTMPGetKeyParameter("ApCliKey1Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

		if (status == TRUE) {
			ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter("ApCliKey1Str", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*tansfer apcli1 ApCliKey1Str to ApCliKey1Str1*/
		status = RTMPGetKeyParameter("ApCliKey1Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);

		if (status == TRUE) {
			ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter("ApCliKey1Str1", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*merge apcli0 ApCliKey2Str*/
		status = RTMPGetKeyParameter("ApCliKey2Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

		if (status == TRUE) {
			ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter("ApCliKey2Str", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*tansfer apcli1 ApCliKey2Str to ApCliKey2Str1*/
		status = RTMPGetKeyParameter("ApCliKey2Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);

		if (status == TRUE) {
			ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter("ApCliKey2Str1", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*merge apcli0 ApCliKey3Str*/
		status = RTMPGetKeyParameter("ApCliKey3Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

		if (status == TRUE) {
			ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter("ApCliKey3Str", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*tansfer apcli1 ApCliKey3Str to ApCliKey3Str1*/
		status = RTMPGetKeyParameter("ApCliKey3Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);

		if (status == TRUE) {
			ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter("ApCliKey3Str1", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*merge apcli0 ApCliKey4Str*/
		status = RTMPGetKeyParameter("ApCliKey4Str", tmpbuf, TEMP_STR_SIZE, buf1, TRUE);

		if (status == TRUE) {
			ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter("ApCliKey4Str", value, TEMP_STR_SIZE, final, TRUE);
		}

		/*tansfer apcli1 ApCliKey4Str to ApCliKey4Str1*/
		status = RTMPGetKeyParameter("ApCliKey4Str", tmpbuf, TEMP_STR_SIZE, buf2, TRUE);

		if (status == TRUE) {
			ret_tmp = snprintf(value, TEMP_STR_SIZE, "%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			RTMPSetKeyParameter("ApCliKey4Str1", value, TEMP_STR_SIZE, final, TRUE);
		}
	}
#ifdef DOT11_SAE_SUPPORT
	/*merge ApCliPweMethod*/
	multi_profile_merge_separate("ApCliPweMethod", buf1, buf2, final);
	/*merge ApCliSAEPK*/
	multi_profile_merge_separate("ApCliSAEPK", buf1, buf2, final);
	/*merge ApCliSAEGroup*/
	multi_profile_merge_separate("ApCliSAEGroup", buf1, buf2, final);
#endif
	/* merge ApCliPESupport */
	multi_profile_merge_separate("ApCliPESupport", buf1, buf2, final);
	/* merge ApCliOCVSupport */
	multi_profile_merge_separate("ApCliOCVSupport", buf1, buf2, final);
	/* merge ApCliTransDisableSupported */
	multi_profile_merge_separate("ApCliTransDisableSupported", buf1, buf2, final);
	/*merge ApCliTxMode*/
	multi_profile_merge_separate("ApCliTxMode", buf1, buf2, final);
	/*merge ApCliTxMcs*/
	multi_profile_merge_separate("ApCliTxMcs", buf1, buf2, final);
#ifdef BCN_PROTECTION_SUPPORT
	/* merge ApCliBcnProt */
	multi_profile_merge_separate("ApCliBcnProt", buf1, buf2, final);
#endif
#ifdef WSC_AP_SUPPORT
	/*merge ApCli_Wsc4digitPinCode*/
	multi_profile_merge_separate("ApCli_Wsc4digitPinCode", buf1, buf2, final);
	/*merge ApCliWscScanMode*/
	multi_profile_merge_separate("ApCliWscScanMode", buf1, buf2, final);
#endif /*WSC_AP_SUPPORT*/
#ifdef UAPSD_SUPPORT
	/*merge ApCliUAPSDCapable*/
	multi_profile_merge_separate("ApCliUAPSDCapable", buf1, buf2, final);
	multi_profile_merge_separate("ApCliAPSDCapable", buf1, buf2, final); /* backward compatible with old SDK */
#endif /*UAPSD_SUPPORT*/
	/*merge ApCliPMFMFPC*/
	multi_profile_merge_separate("ApCliPMFMFPC", buf1, buf2, final);
	/*merge ApCliPMFMFPR*/
	multi_profile_merge_separate("ApCliPMFMFPR", buf1, buf2, final);
	/*merge ApCliPMFSHA256*/
	multi_profile_merge_separate("ApCliPMFSHA256", buf1, buf2, final);
	/*merge ApCliMuOfdmaDlEnable*/
	multi_profile_merge_separate("ApCliMuOfdmaDlEnable", buf1, buf2, final);
	/*merge ApCliMuOfdmaDlEnable*/
	multi_profile_merge_separate("ApCliMuOfdmaUlEnable", buf1, buf2, final);
	/*merge ApCliMuOfdmaDlEnable*/
	multi_profile_merge_separate("ApCliMuMimoDlEnable", buf1, buf2, final);
	/*merge ApCliMuOfdmaDlEnable*/
	multi_profile_merge_separate("ApCliMuMimoUlEnable", buf1, buf2, final);

end:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	if (value)
		os_free_mem(value);
	return ret;
}
#endif /*APCLI_SUPPORT*/

#ifdef BAND_STEERING
static INT multi_profile_merge_bandsteering(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/* multi_profile_merge_separate("BndStrgBssIdx",buf1,buf2,final); */
	/* return NDIS_STATUS_SUCCESS; */

	CHAR *tmpbuf = NULL;
	CHAR *tmpbuf2 = NULL;
	CHAR *value = NULL;
	RTMP_STRING *macptr = NULL;
	int i = 0;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	if (!buf1 || !buf2)
		return NDIS_STATUS_FAILURE;

	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf2, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&value, TEMP_STR_SIZE);

	if (!tmpbuf || !tmpbuf2 || !value) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(tmpbuf, TEMP_STR_SIZE);
	os_zero_mem(tmpbuf2, TEMP_STR_SIZE);
	os_zero_mem(value, TEMP_STR_SIZE);

	if (RTMPGetKeyParameter("BndStrgBssIdx", tmpbuf, TEMP_STR_SIZE, buf1, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i == data->pf1_num)
				break;

			if (i == 0) {
				ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), "%s", macptr);
				if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "value snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
					goto end;
				}
			} else {
				ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), ";%s", macptr);
				if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "value snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
					goto end;
				}
			}
		}
		if (i < data->pf1_num) {
			for (; i < data->pf1_num; i++) {
				if (i == 0) {
					ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), "%s", "1");
					if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
						MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "value snprintf error!!!\n");
						ret = NDIS_STATUS_FAILURE;
						goto end;
					}
				} else {
					ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), ";%s", "0");
					if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
						MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "value snprintf error!!!\n");
						ret = NDIS_STATUS_FAILURE;
						goto end;
					}
				}
			}
		}
	} else{
		for (i = 0; i < data->pf1_num; i++) {
			if (i == 0) {
				ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), "%s", "1");

				if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "value snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
					goto end;
				}
			} else {
				ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), ";%s", "0");

				if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "value snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
					goto end;
				}
			}
		}
	}

	if (RTMPGetKeyParameter("BndStrgBssIdx", tmpbuf2, TEMP_STR_SIZE, buf2, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf2, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i == data->pf2_num)
				break;
			ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), ";%s", macptr);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "value snprintf error!!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
		}
		if (i < data->pf2_num) {
			for (; i < data->pf2_num; i++) {
				if (i == 0) {
					ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), ";%s", "1");
					if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
						MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "value snprintf error!!!\n");
						ret = NDIS_STATUS_FAILURE;
						goto end;
					}
				} else {
					ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), ";%s", "0");
					if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
						MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "value snprintf error!!!\n");
						ret = NDIS_STATUS_FAILURE;
						goto end;
					}
				}
			}
		}
	} else{
		for (i = 0; i < data->pf2_num; i++) {
			if (i == 0) {
				ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), ";%s", "1");

				if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "value snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
					goto end;
				}
			} else {
				ret_tmp = snprintf((value + strlen(value)), TEMP_STR_SIZE - strlen(value), ";%s", "0");
				if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "value snprintf error!!!\n");
					ret = NDIS_STATUS_FAILURE;
					goto end;
				}
			}
		}
	}
	RTMPSetKeyParameter("BndStrgBssIdx", value, TEMP_STR_SIZE, final, TRUE);

end:
	if (tmpbuf)
		os_free_mem(tmpbuf);
	if (tmpbuf2)
		os_free_mem(tmpbuf2);
	if (value)
		os_free_mem(value);
	return ret;
}
#endif

/*
* BssidNum
*/
static INT multi_profile_merge_bssidnum(struct mpf_data *data, CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[25] = "";
	UCHAR num1 = 0;
	UCHAR num2 = 0;
	UCHAR total;
	INT ret;

	if (RTMPGetKeyParameter("BssidNum", tmpbuf, 25, buf1, TRUE))
		num1 = (UCHAR) simple_strtol(tmpbuf, 0, 10);

	if (RTMPGetKeyParameter("BssidNum", tmpbuf, 25, buf2, TRUE))
		num2 = (UCHAR) simple_strtol(tmpbuf, 0, 10);

	total = num1 + num2;
	ret = snprintf(tmpbuf, sizeof(tmpbuf), "%d", total);

	if (os_snprintf_error(sizeof(tmpbuf), ret)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "tmpbuf snprintf error!!!\n");
		return NDIS_STATUS_FAILURE;
	}

	RTMPSetKeyParameter("BssidNum", tmpbuf, 25, final, TRUE);
	/*assign bss number*/
	data->pf1_num = num1;
	data->pf2_num = num2;
	data->total_num = total;
	return NDIS_STATUS_SUCCESS;
}

#ifdef ICAP_FW_SUPPORT
static INT multi_profile_merge_is_icap_fw(struct mpf_data *data, CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[25] = "";

	if (RTMPGetKeyParameter("IsICAPFW", tmpbuf, 25, buf1, TRUE))
		RTMPSetKeyParameter("IsICAPFW", tmpbuf, 25, final, TRUE);

	return NDIS_STATUS_SUCCESS;
}
#endif

static INT multi_profile_merge_edcca(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	/*merge EDCCA related profile*/
	multi_profile_merge_separate("EDCCAEnable", buf1, buf2, final);
	multi_profile_merge_separate("EDCCAThreshold", buf1, buf2, final);
	multi_profile_merge_separate("EDCCA_Mode", buf1, buf2, final);
	multi_profile_merge_separate("EDCCACfgMode", buf1, buf2, final);
	multi_profile_merge_separate("CBP_COMPENSATION", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_SCS(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	/*merge EDCCA related profile*/
	multi_profile_merge_separate("SCSEnable", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}

#endif /*CONFIG_AP_SUPPORT*/

/*
 * protections: including HT_PROTECT / RTS_THRESHOLD
 */
static INT multi_profile_merge_protection(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*RTSPktThreshold*/
	multi_profile_merge_perbss(data, "RTSPktThreshold", buf1, buf2, final, MPF_APPEND_BSS0);
	/*RTSThreshold*/
	multi_profile_merge_perbss(data, "RTSThreshold", buf1, buf2, final, MPF_APPEND_BSS0);
	/*HT_PRORTECT*/
	multi_profile_merge_perbss(data, "HT_PROTECT", buf1, buf2, final, MPF_APPEND_BSS0);
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_frag(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*Fragment Threshold*/
	multi_profile_merge_perbss(data, "FragThreshold", buf1, buf2, final, MPF_APPEND_BSS0);
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_gi(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*HT_GI*/
	multi_profile_merge_perbss(data, "HT_GI", buf1, buf2, final, MPF_APPEND_BSS0);
	/*VHT_SGI*/
	/*multi_profile_merge_separate(data, "VHT_SGI", buf1, buf2, final);*/
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_mpdu_density(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*HT_MpduDensity*/
	multi_profile_merge_perbss(data, "HT_MpduDensity", buf1, buf2, final, MPF_APPEND_BSS0);
	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_ht_mode(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*HT_OpMode*/
	multi_profile_merge_perbss(data, "HT_OpMode", buf1, buf2, final, MPF_APPEND_BSS0);
	return NDIS_STATUS_SUCCESS;
}

#ifdef MBO_SUPPORT
static INT multi_profile_merge_mbo(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*MboSupport*/
	multi_profile_merge_perbss(data, "MboSupport", buf1, buf2, final, MPF_APPEND_BSS0);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef OCE_SUPPORT
static INT multi_profile_merge_oce(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/* OCE_SUPPORT */
	multi_profile_merge_perbss(data, "OCE_SUPPORT", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "OCE_FD_FRAME", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "OCE_FILS_HLP", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "OCE_FILS_DhcpServer", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "OCE_FILS_DhcpServerPort", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "OCE_FILS_REALMS", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "OCE_FILS_CACHE", buf1, buf2, final, MPF_APPEND_BSS0);
	multi_profile_merge_perbss(data, "OCE_RNR_SUPPORT", buf1, buf2, final, MPF_APPEND_BSS0);
	return NDIS_STATUS_SUCCESS;
}
#endif

static INT multi_profile_merge_ba_related(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*HT_BADecline*/
	multi_profile_merge_perbss(data, "HT_BADecline", buf1, buf2, final, MPF_APPEND_BSS0);
	/*HT_AutoBA*/
	multi_profile_merge_perbss(data, "HT_AutoBA", buf1, buf2, final, MPF_APPEND_BSS0);
	/*HT_BAWinSize*/
	multi_profile_merge_perbss(data, "HT_BAWinSize", buf1, buf2, final, MPF_APPEND_BSS0);

	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_bcn_dtim_period(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*BeaconPeriod*/
	/*multi_profile_merge_perbss(data, "BeaconPeriod", buf1, buf2, final, MPF_APPEND_BSS0);*/
	multi_profile_merge_separate("BeaconPeriod", buf1, buf2, final);

	/*DtimPeriod*/
	multi_profile_merge_perbss(data, "DtimPeriod", buf1, buf2, final, MPF_APPEND_BSS0);


	return NDIS_STATUS_SUCCESS;
}


#ifdef CONFIG_RA_PHY_RATE_SUPPORT
static INT multi_profile_merge_rate_config(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_perbss(data, "mgmrateset1", buf1, buf2, final, MPF_APPEND_BSS0);

	multi_profile_merge_perbss(data, "mgmrateset2", buf1, buf2, final, MPF_APPEND_BSS0);

	multi_profile_merge_perbss(data, "suprateset", buf1, buf2, final, MPF_APPEND_BSS0);

	multi_profile_merge_perbss(data, "htsuprateset", buf1, buf2, final, MPF_APPEND_BSS0);

	multi_profile_merge_perbss(data, "vhtsuprateset", buf1, buf2, final, MPF_APPEND_BSS0);

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


#ifdef MCAST_RATE_SPECIFIC
static INT multi_profile_merge_mcast(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_perbss(data, "McastPhyMode", buf1, buf2, final, MPF_APPEND_BSS0);

	multi_profile_merge_perbss(data, "McastMcs", buf1, buf2, final, MPF_APPEND_BSS0);

	return NDIS_STATUS_SUCCESS;
}
#endif /* MCAST_RATE_SPECIFIC */


/*
* merge 5G only related
*/
static INT multi_profile_merge_5g_only(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	CHAR tmpbuf[64] = "";
	CHAR *buf_mu = buf2;
	UCHAR len = sizeof(tmpbuf);
#ifndef DEFAULT_5G_PROFILE
	UCHAR tmpbuf_dbdc[25] = "";
	UCHAR dbdc_mode = 0;
#endif /* n DEFAULT_5G_PROFILE*/

	/*merge VHT_BW*/
	if (multi_profile_merge_perbss(data, "VHT_BW", buf1, buf2, final, MPF_APPEND_BSS0) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	/*merge VHT_SGI*/
	if (multi_profile_merge_perbss(data, "VHT_SGI", buf1, buf2, final, MPF_APPEND_BSS0) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	/*merge VHT_BW_SIGNAL*/
	if (multi_profile_merge_perbss(data, "VHT_BW_SIGNAL", buf1, buf2, final, MPF_APPEND_BSS0) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	/*merge VHT_Sec80_Channel*/
	if (multi_profile_merge_perbss(data, "VHT_Sec80_Channel", buf1, buf2, final, MPF_APPEND_BSS0) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

#ifndef DEFAULT_5G_PROFILE
	/*check dbdc mode is enable*/
	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf_dbdc, 25, buf1, TRUE)) {
		dbdc_mode = (UCHAR) simple_strtol(tmpbuf_dbdc, 0, 10);

		if (dbdc_mode == ENUM_DBDC_5G5G)
			buf_mu = buf1; /* 5G+5G, use 1st profile's MUTxRxEnable */

		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "DBDC_MODE=%d\n", dbdc_mode);
	}
#endif /* n DEFAULT_5G_PROFILE*/

	/*MUTxRxEnable*/
	if (RTMPGetKeyParameter("MUTxRxEnable", tmpbuf, len, buf_mu, TRUE) == TRUE)
		RTMPSetKeyParameter("MUTxRxEnable", tmpbuf, len, final, TRUE);

#ifdef DEFAULT_5G_PROFILE
	buf_mu = buf1;
#endif
	/*merge AutoChannelSkipList6G*/
	if (RTMPGetKeyParameter("AutoChannelSkipList6G", tmpbuf, len, buf_mu, TRUE) == TRUE)
		RTMPSetKeyParameter("AutoChannelSkipList6G", tmpbuf, len, final, TRUE);

	/*IEEE80211H*/
	if (RTMPGetKeyParameter("IEEE80211H", tmpbuf, len, buf_mu, TRUE) == TRUE)
		RTMPSetKeyParameter("IEEE80211H", tmpbuf, len, final, TRUE);

#ifdef DBDC_ONE_BAND1_SUPPORT
	if (RTMPGetKeyParameter("CountryCode", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("CountryCode", tmpbuf, len, final, TRUE);
#endif

	/*DFS related params is 5G only, use profile 2*/
#ifdef MT_DFS_SUPPORT

	/*DfsCalibration*/
	if (RTMPGetKeyParameter("DfsCalibration", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DfsCalibration", tmpbuf, len, final, TRUE);

	/*DfsEnable*/
	if (RTMPGetKeyParameter("DfsEnable", tmpbuf, len, buf_mu, TRUE) == TRUE)
		RTMPSetKeyParameter("DfsEnable", tmpbuf, len, final, TRUE);

	/* DfsFalseAlarmPrevent */
	if (RTMPGetKeyParameter("DfsFalseAlarmPrevent", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DfsFalseAlarmPrevent", tmpbuf, len, final, TRUE);

	/* DfsZeroWait */
	if (RTMPGetKeyParameter("DfsZeroWait", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DfsZeroWait", tmpbuf, len, final, TRUE);

	/* DfsZeroWaitCacTime */
	if (RTMPGetKeyParameter("DfsZeroWaitCacTime", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DfsZeroWaitCacTime", tmpbuf, len, final, TRUE);

	/* DfsDedicatedZeroWait */
	if (RTMPGetKeyParameter("DfsDedicatedZeroWait", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DfsDedicatedZeroWait", tmpbuf, len, final, TRUE);

	/*DfsZeroWaitDefault*/
	if (RTMPGetKeyParameter("DfsZeroWaitDefault", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DfsZeroWaitDefault", tmpbuf, len, final, TRUE);

	/*DfsTargetCh*/
	if (RTMPGetKeyParameter("DfsTargetCh", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DfsTargetCh", tmpbuf, len, final, TRUE);

	if (RTMPGetKeyParameter("Ch144Support", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("Ch144Support", tmpbuf, len, final, TRUE);
	/*DfsPreferType*/
	if (RTMPGetKeyParameter("DfsChSelPrefer", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DfsChSelPrefer", tmpbuf, len, final, TRUE);
#endif

	/*RDRegion*/
	if (RTMPGetKeyParameter("RDRegion", tmpbuf, len, buf_mu, TRUE) == TRUE)
		RTMPSetKeyParameter("RDRegion", tmpbuf, len, final, TRUE);

	/*VHT 1024QAM Support*/
	if (RTMPGetKeyParameter("Vht1024QamSupport", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("Vht1024QamSupport", tmpbuf, len, final, TRUE);

	return NDIS_STATUS_SUCCESS;
}

#ifdef DEFAULT_5G_PROFILE
/*
* merge 2G only related
*/
static INT multi_profile_merge_2g_only(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[64] = "";
	UCHAR len = sizeof(tmpbuf);

	/*merge CountryRegion*/
	if (RTMPGetKeyParameter("CountryRegion", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("CountryRegion", tmpbuf, len, final, TRUE);

	/*merge DisableOLBC*/
	if (RTMPGetKeyParameter("DisableOLBC", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DisableOLBC", tmpbuf, len, final, TRUE);

	/*merge G_BAND_256QAM*/
	if (RTMPGetKeyParameter("G_BAND_256QAM", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("G_BAND_256QAM", tmpbuf, len, final, TRUE);

	/*merge HT_BSSCoexistence*/
	if (RTMPGetKeyParameter("HT_BSSCoexistence", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("HT_BSSCoexistence", tmpbuf, len, final, TRUE);

	/*BGProtection*/
	if (RTMPGetKeyParameter("BGProtection", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("BGProtection", tmpbuf, len, final, TRUE);

	/*TxPreamble*/
	if (RTMPGetKeyParameter("TxPreamble", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("TxPreamble", tmpbuf, len, final, TRUE);
#ifdef GN_MIXMODE_SUPPORT
	/*merge GNMixMode*/
	if (RTMPGetKeyParameter("GNMixMode", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("GNMixMode", tmpbuf, len, final, TRUE);
#endif /*GN_MIXMODE_SUPPORT*/

	return NDIS_STATUS_SUCCESS;
}
/*
* merge global setting only related
*/
static INT multi_profile_merge_global_setting_only(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	CHAR tmpbuf[64] = "";
	UCHAR len = sizeof(tmpbuf);

	/*merge CountryCode*/
	if (RTMPGetKeyParameter("CountryCode", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("CountryCode", tmpbuf, len, final, TRUE);

	/*merge NoForwardingBTNBSSID*/
	if (RTMPGetKeyParameter("NoForwardingBTNBSSID", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("NoForwardingBTNBSSID", tmpbuf, len, final, TRUE);

	/*merge GreenAP*/
	if (RTMPGetKeyParameter("GreenAP", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("GreenAP", tmpbuf, len, final, TRUE);

	/*merge PcieAspm*/
	if (RTMPGetKeyParameter("PcieAspm", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("PcieAspm", tmpbuf, len, final, TRUE);

	/*DBDC_MODE*/
	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DBDC_MODE", tmpbuf, len, final, TRUE);

	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, len, buf1, TRUE) == TRUE)
		RTMPSetKeyParameter("DBDC_MODE", tmpbuf, len, final, TRUE);

	/*IcapMode*/
	if (RTMPGetKeyParameter("IcapMode", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("IcapMode", tmpbuf, len, final, TRUE);

	/*CarrierDetect*/
	if (RTMPGetKeyParameter("CarrierDetect", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("CarrierDetect", tmpbuf, len, final, TRUE);

	/*DebugFlags*/
	if (RTMPGetKeyParameter("DebugFlags", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("DebugFlags", tmpbuf, len, final, TRUE);

	/*E2pAccessMode*/
	if (RTMPGetKeyParameter("E2pAccessMode", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("E2pAccessMode", tmpbuf, len, final, TRUE);

	/*EfuseBufferMode*/
	if (RTMPGetKeyParameter("EfuseBufferMode", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("EfuseBufferMode", tmpbuf, len, final, TRUE);

	/*WCNTest*/
	if (RTMPGetKeyParameter("WCNTest", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("WCNTest", tmpbuf, len, final, TRUE);

	/*HT_RDG*/
	if (RTMPGetKeyParameter("HT_RDG", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("HT_RDG", tmpbuf, len, final, TRUE);

	/*TxBurst*/
	if (RTMPGetKeyParameter("TxBurst", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("TxBurst", tmpbuf, len, final, TRUE);

	/*PktAggregate*/
	if (RTMPGetKeyParameter("PktAggregate", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("PktAggregate", tmpbuf, len, final, TRUE);

	/*VOW_WATF_Enable*/
	if (RTMPGetKeyParameter("VOW_WATF_Enable", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_WATF_Enable", tmpbuf, len, final, TRUE);

	/*VOW_WATF_MAC_LV1*/
	if (RTMPGetKeyParameter("VOW_WATF_MAC_LV1", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_WATF_MAC_LV1", tmpbuf, len, final, TRUE);

	/*VOW_WATF_MAC_LV2*/
	if (RTMPGetKeyParameter("VOW_WATF_MAC_LV2", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_WATF_MAC_LV2", tmpbuf, len, final, TRUE);

	/*VOW_WATF_MAC_LV3*/
	if (RTMPGetKeyParameter("VOW_WATF_MAC_LV3", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_WATF_MAC_LV3", tmpbuf, len, final, TRUE);

	/*VOW_WATF_MAC_LV4*/
	if (RTMPGetKeyParameter("VOW_WATF_MAC_LV4", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_WATF_MAC_LV4", tmpbuf, len, final, TRUE);

	/*VOW_Airtime_Fairness_En*/
	if (RTMPGetKeyParameter("VOW_Airtime_Fairness_En", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_Airtime_Fairness_En", tmpbuf, len, final, TRUE);

	/*VOW_BW_Ctrl*/
	if (RTMPGetKeyParameter("VOW_BW_Ctrl", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_BW_Ctrl", tmpbuf, len, final, TRUE);

	/* VOW_RX_En */
	if (RTMPGetKeyParameter("VOW_RX_En", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("VOW_RX_En", tmpbuf, len, final, TRUE);

	/*RED_Enable*/
	if (RTMPGetKeyParameter("RED_Enable", tmpbuf, len, buf2, TRUE) == TRUE)
		RTMPSetKeyParameter("RED_Enable", tmpbuf, len, final, TRUE);


	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef IGMP_SNOOP_SUPPORT
/*
* merge igmp related
*/
static INT multi_profile_merge_igmp(CHAR *buf1, CHAR *buf2, CHAR *final)
{
	/*merge VHT_BW*/
	multi_profile_merge_separate("IgmpSnEnable", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif /* IGMP_SNOOP_SUPPORT */

/*
*
*/
static INT multi_profile_dbdc_mode_sanity(CHAR *buf1, CHAR *buf2, CHAR *final)
{
#ifdef DEFAULT_5G_PROFILE
	UCHAR tmpbuf[25] = "";
	UCHAR dbdc_mode = 0;

	/*check dbdc mode is enable*/
	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, buf1, TRUE)) {
		dbdc_mode = (UCHAR) simple_strtol(tmpbuf, 0, 10);

		if (!dbdc_mode) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "DBDC_MODE is not enable! Not need to merge.\n");
			goto buf2_check;
		}

		return NDIS_STATUS_SUCCESS;
	}

buf2_check:

	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, buf2, TRUE)) {
		dbdc_mode = (UCHAR) simple_strtol(tmpbuf, 0, 10);

		if (!dbdc_mode) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "DBDC_MODE is not enable! Not need to merge.\n");
			return NDIS_STATUS_FAILURE;
		}
	}

	return NDIS_STATUS_SUCCESS;
#else
	UCHAR tmpbuf[25] = "";
	UCHAR dbdc_mode = 0;

	/*check dbdc mode is enable*/
	if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, buf1, TRUE)) {
		dbdc_mode = (UCHAR) simple_strtol(tmpbuf, 0, 10);

		if (!dbdc_mode) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "DBDC_MODE is not enable! Not need to merge.\n");
			return NDIS_STATUS_FAILURE;
		}
	}

	return NDIS_STATUS_SUCCESS;
#endif
}

/*
*  TXPOWER merge function for multiple profile mode
*/
static INT multi_profile_merge_txpwr(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
#ifdef SINGLE_SKU_V2
	/*merge SKUenable*/
	multi_profile_merge_separate("SKUenable", buf1, buf2, final);
#endif /* SINGLE_SKU_V2 */
	/*merge PERCENTAGEenable*/
	multi_profile_merge_separate("PERCENTAGEenable", buf1, buf2, final);
	/*merge BFBACKOFFenable*/
	multi_profile_merge_separate("BFBACKOFFenable", buf1, buf2, final);
	/*merge TxPower*/
	multi_profile_merge_separate("TxPower", buf1, buf2, final);

#ifdef TX_POWER_CONTROL_SUPPORT
	/*merge Tx Power Boost Table*/
	multi_profile_merge_separate("PowerUpCck", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpOfdm", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpHT20", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpHT40", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpVHT20", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpVHT40", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpVHT80", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpVHT160", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpRU26", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpRU52", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpRU106", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpRU242", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpRU484", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpRU996", buf1, buf2, final);
	multi_profile_merge_separate("PowerUpRU996X2", buf1, buf2, final);
#endif /* TX_POWER_CONTROL_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}

#ifndef MBSS_SUPPORT
#ifdef TXBF_SUPPORT
/*
*  TXBF merge function for multiple profile mode
*/
static INT multi_profile_merge_txbf(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/* merge ETxBfEnCond*/
	multi_profile_merge_separate("ETxBfEnCond", buf1, buf2, final);
	/* merge ITxBfEn */
	multi_profile_merge_separate("ITxBfEn", buf1, buf2, final);

	return NDIS_STATUS_SUCCESS;
}

#ifdef DOT11_HE_AX
static INT multi_profile_merge_mu(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_separate("MuOfdmaDlEnable", buf1, buf2, final);
	multi_profile_merge_separate("MuOfdmaUlEnable", buf1, buf2, final);
	multi_profile_merge_separate("MuMimoDlEnable", buf1, buf2, final);
	multi_profile_merge_separate("MuMimoUlEnable", buf1, buf2, final);
	multi_profile_merge_separate("Disable160RuMu", buf1, buf2, final);
	multi_profile_merge_separate("MaxRuOfdma", buf1, buf2, final);
	multi_profile_merge_separate("MaxDLMuMimo", buf1, buf2, final);
	multi_profile_merge_separate("MaxULMuMimo", buf1, buf2, final);

	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_ulmu_data(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_separate("HeOmiUlMuDataDisableRx", buf1, buf2, final);

	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_ersu(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_separate("HeErSuRxDisable", buf1, buf2, final);

	return NDIS_STATUS_SUCCESS;
}
#endif /* DOT11_HE_AX */
#endif /* TXBF_SUPPORT */
#endif /* n MBSS_SUPPORT */

#ifdef LINK_TEST_SUPPORT
/*
*  LinkTestSupport merge function for multiple profile mode
*/
static INT multi_profile_merge_linktest(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge LinkTestSupport*/
	multi_profile_merge_separate("LinkTestSupport", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif /* LINK_TEST_SUPPORT */

#ifdef MWDS
static INT multi_profile_merge_mwds(
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge ApMWDS*/
	multi_profile_merge_separate("ApMWDS", buf1, buf2, final);
	/*merge ApCliMWDS*/
	multi_profile_merge_separate("ApCliMWDS", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef WDS_SUPPORT
INT	multi_profile_wds_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name, INT ifidx)
{
	UCHAR *dev_name;
	struct mpf_data *data;
	INT ret;

	if (!ad->multi_pf_ctrl)
		return NDIS_STATUS_SUCCESS;

	data = (struct mpf_data *) ad->multi_pf_ctrl;

	if (!data->enable || !data->specific_dname)
		return NDIS_STATUS_SUCCESS;

	if (ifidx >= ad->WdsTab.wds_num[DBDC_BAND0])
		dev_name = get_dbdcdev_name_prefix(ad, INT_WDS);
	else
		dev_name = get_dev_name_prefix(ad, INT_WDS);

	ret = snprintf(final_name, IFNAMSIZ, "%s", dev_name);

	if (os_snprintf_error(IFNAMSIZ, ret)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "final_name snprintf error!!!\n");
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

static INT multi_profile_merge_wds(
	struct _RTMP_ADAPTER *ad,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT i;
	CHAR *KeyPerBand[] = {"WdsEnable", "WdsNum"};
	CHAR *KeyPerWds[] = {"WdsPhyMode", "WdsList", "WdsEncrypType", "WDS_VLANID", "Wds0Key", "Wds1Key", "Wds2Key", "Wds3Key", "Wds4Key", "Wds5Key", "Wds6Key", "Wds7Key"};
	INT ret_tmp;

	for(i = 0; i < sizeof(KeyPerBand)/sizeof(CHAR *); i++){
		multi_profile_merge_separate(KeyPerBand[i], buf1, buf2, final);
	}

	if(ad->WdsTab.wds_num[DBDC_BAND0]) {
		for(i = 0; i < sizeof(KeyPerWds)/sizeof(CHAR *); i++){
			multi_profile_merge_separate(KeyPerWds[i], buf1, buf2, final);
		}
	} else {
		for(i = 0; i < sizeof(KeyPerWds)/sizeof(CHAR *); i++){
			CHAR tmpbuf[256], value[256];

			if (!buf2)
				continue;

			if (RTMPGetKeyParameter(KeyPerWds[i], tmpbuf, TEMP_STR_SIZE, buf2, TRUE) != TRUE) {
				continue;
			}
			ret_tmp = snprintf(value, TEMP_STR_SIZE, ";%s", tmpbuf);
			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"value snprintf error!!!\n");
				return NDIS_STATUS_FAILURE;
			}
			RTMPSetKeyParameter(KeyPerWds[i], value, TEMP_STR_SIZE, final, TRUE);
		}
	}

	return NDIS_STATUS_SUCCESS;
}
#endif


#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
static INT multi_profile_merge_ft(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge FtMdId*/
	multi_profile_merge_increase(data, 1, "FtMdId", buf1, buf2, final);
	/*merge FtSupport */
	multi_profile_merge_perbss(data, "FtSupport", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge FtOnly */
	multi_profile_merge_perbss(data, "FtOnly", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge FtRic */
	multi_profile_merge_perbss(data, "FtRic", buf1, buf2, final, MPF_APPEND_BSS0);
	/*merge FtOtd */
	multi_profile_merge_perbss(data, "FtOtd", buf1, buf2, final, MPF_APPEND_BSS0);
	return NDIS_STATUS_SUCCESS;
}
#endif
#endif
#ifdef DOT11K_RRM_SUPPORT
static INT multi_profile_merge_rrm(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	/*merge FtSupport */
	multi_profile_merge_separate("RRMEnable", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef DSCP_PRI_SUPPORT
INT multi_profile_merge_dscp_pri(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT8 i = 0;
	CHAR *tok_str = NULL;
	CHAR *tmpbuf = NULL;
	INT ret = NDIS_STATUS_SUCCESS;
	INT ret_tmp;

	if (!buf1 || !buf2) {
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_alloc_mem_suspend(NULL, (UCHAR **)&tok_str, TEMP_STR_SIZE);
	os_alloc_mem_suspend(NULL, (UCHAR **)&tmpbuf, TEMP_STR_SIZE);

	if (!tok_str || !tmpbuf) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Allocate memory failed!");
		ret = NDIS_STATUS_FAILURE;
		goto end;
	}

	os_zero_mem(tok_str, TEMP_STR_SIZE);
	os_zero_mem(tmpbuf, TEMP_STR_SIZE);

	/*merge DscpPriMapEnable flag for both bands*/
	multi_profile_merge_separate("DscpPriMapEnable", buf1, buf2, final);

	for (i = 0; i < data->pf2_num; i++) {
		ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "DscpPriMapBss%d", i);

		if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "tok_str snprintf error!!!\n");
			goto end;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, buf2, TRUE)) {
			snprintf(tok_str, TEMP_STR_SIZE, "DscpPriMapBss%d", (data->pf1_num + i));
			RTMPSetKeyParameter(tok_str, tmpbuf, TEMP_STR_SIZE, final, TRUE);
		} else {
			ret_tmp = snprintf(tok_str, TEMP_STR_SIZE, "DscpPriMapBss%d", (data->pf1_num + i));

			if (os_snprintf_error(TEMP_STR_SIZE, ret_tmp)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "tok_str snprintf error!!!\n");
				goto end;
			}

			RTMPSetKeyParameter(tok_str, "", TEMP_STR_SIZE, final, TRUE);
		}
	}

end:
	if (tok_str)
		os_free_mem(tok_str);
	if (tmpbuf)
		os_free_mem(tmpbuf);
	return ret;
}
#endif /*DSCP_PRI_SUPPORT*/

#ifdef MGMT_TXPWR_CTRL
static INT multi_profile_merge_mgmt_pwr(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_separate("MgmtTxPwr", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef DATA_TXPWR_CTRL
static INT multi_profile_merge_data_pwr(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_separate("DataTxPwr", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef ANDLINK_FEATURE_SUPPORT
static INT multi_profile_merge_andlink_en(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_separate("andlink_en", buf1, buf2, final);
	multi_profile_merge_separate("andlink_ip_hostname_en", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}

#endif/*ANDLINK_FEATURE_SUPPORT*/

#ifdef ANTENNA_CONTROL_SUPPORT
static INT multi_profile_merge_ant_ctrl(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_separate("AntCtrl", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif /* ANTENNA_CONTROL_SUPPORT */
static INT multi_profile_merge_quick_channel_switch(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_perbss(data, "QuickChannelSwitch", buf1, buf2, final, MPF_APPEND_BSS0);
	return NDIS_STATUS_SUCCESS;
}

#ifdef CONFIG_CPE_SUPPORT
static INT multi_profile_merge_powerBackoff(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_separate("AmplifierPowerBackoff", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef CONFIG_3_WIRE_SUPPORT
static INT multi_profile_merge_3wireFunctionEnable(
	struct mpf_data *data,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	multi_profile_merge_separate("ThreeWireFunctionEnable", buf1, buf2, final);
	return NDIS_STATUS_SUCCESS;
}
#endif

/*
* set second profile and merge it.
*/
static INT multi_profile_merge(
	struct _RTMP_ADAPTER *ad,
	CHAR *buf1,
	CHAR *buf2,
	CHAR *final)
{
	INT retval = NDIS_STATUS_FAILURE;
	struct mpf_data *data = NULL;

	/*create mpf_data*/
	os_alloc_mem(ad, (UCHAR **)&data, sizeof(struct mpf_data));

	if (!data)
		return retval;

	ad->multi_pf_ctrl = data;
	/*first copy buf1 to final*/
	os_move_mem(final, buf1, MAX_INI_BUFFER_SIZE);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(ad) {
		/*merge BssidNum*/
		if (multi_profile_merge_bssidnum(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;

#ifdef MBSS_SUPPORT

		if (multi_profile_merge_mbss(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;

#endif /*MBSS_SUPPORT*/

#ifdef ICAP_FW_SUPPORT
	if (multi_profile_merge_is_icap_fw(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

#endif /*ICAP_FW_SUPPORT*/

		if (multi_profile_merge_edcca(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;
		if (multi_profile_merge_SCS(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;

#ifdef APCLI_SUPPORT

		if (multi_profile_merge_apcli(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;

#endif /*APCLI_SUPPORT*/
#ifdef BAND_STEERING
		if (multi_profile_merge_bandsteering(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;
#endif /* BAND_STEERING */
	}
#endif /*CONFIG_AP_SUPPORT*/

	if (multi_profile_merge_gi(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_protection(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_frag(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_mpdu_density(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_ht_mode(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_ba_related(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_bcn_dtim_period(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	if (multi_profile_merge_rate_config(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


#ifdef MCAST_RATE_SPECIFIC
	if (multi_profile_merge_mcast(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* MCAST_RATE_SPECIFIC */



#ifdef DEFAULT_5G_PROFILE

	if (multi_profile_merge_5g_only(data, buf2, buf1, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_2g_only(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	/* will remove global setting from 2G profile after UI 5G default is ready */
	if (multi_profile_merge_global_setting_only(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

#else

	if (multi_profile_merge_5g_only(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

#endif
#ifdef IGMP_SNOOP_SUPPORT

	if (multi_profile_merge_igmp(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

#endif /* IGMP_SNOOP_SUPPORT */
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		multi_profile_merge_separate("ApCliSyncAutoBandSupport", buf1, buf2, final);

		multi_profile_merge_separate("ApCliSyncAutoBWSupport", buf1, buf2, final);
#endif
	if (multi_profile_merge_txpwr(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

#ifndef MBSS_SUPPORT
#ifdef TXBF_SUPPORT
	if (multi_profile_merge_txbf(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* TXBF_SUPPORT */

#ifdef DOT11_HE_AX
	if (multi_profile_merge_mu(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_ulmu_data(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_ersu(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* DOT11_HE_AX */

#ifdef DYNAMIC_WMM_SUPPORT
	if multi_profile_merge_separate("DynWmmEnable", buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* DYNAMIC_WMM_SUPPORT */

#ifdef CONFIG_MAP_SUPPORT
	if multi_profile_merge_separate("SRMeshUlMode", buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* CONFIG_MAP_SUPPORT */

#ifdef WIFI_CSI_CN_INFO_SUPPORT
	if multi_profile_merge_separate("EnableCNInfo", buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* WIFI_CSI_CN_INFO_SUPPORT */

#endif

#ifdef LINK_TEST_SUPPORT
	if (multi_profile_merge_linktest(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* LINK_TEST_SUPPORT */

#ifdef MWDS
	if (multi_profile_merge_mwds(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif

#ifdef WDS_SUPPORT
	if (profile_wds_reg(ad, DBDC_BAND0, buf1) != NDIS_STATUS_SUCCESS)
		return retval;

	if (profile_wds_reg(ad, DBDC_BAND1, buf2) != NDIS_STATUS_SUCCESS)
		return retval;

	if (multi_profile_merge_wds(ad, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif	/* WDS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	if (multi_profile_merge_ft(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif
#endif

#ifdef MBO_SUPPORT
	if (multi_profile_merge_mbo(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif

#ifdef OCE_SUPPORT
	if (multi_profile_merge_oce(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif

#ifdef DOT11K_RRM_SUPPORT
	if (multi_profile_merge_rrm(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif

#ifdef DSCP_PRI_SUPPORT
	if (multi_profile_merge_dscp_pri(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /*DSCP_PRI_SUPPORT*/
#ifdef MGMT_TXPWR_CTRL
		if (multi_profile_merge_mgmt_pwr(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;
#endif
#ifdef DATA_TXPWR_CTRL
		if (multi_profile_merge_data_pwr(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
			return retval;
#endif
#ifdef ANDLINK_FEATURE_SUPPORT
	if (multi_profile_merge_andlink_en(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif/*ANDLINK_FEATURE_SUPPORT*/
#ifdef ANTENNA_CONTROL_SUPPORT
	if (multi_profile_merge_ant_ctrl(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif /* ANTENNA_CONTROL_SUPPORT */
	if (multi_profile_merge_quick_channel_switch(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#ifdef CONFIG_CPE_SUPPORT
	if (multi_profile_merge_powerBackoff(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif
#ifdef CONFIG_3_WIRE_SUPPORT
	if (multi_profile_merge_3wireFunctionEnable(data, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		return retval;
#endif
	data->enable = TRUE;
	/*adjust specific device name*/
	data->specific_dname = TRUE;
	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "multi-profile merge success, en:%d,pf1_num:%d,pf2_num:%d,total:%d\n",
				 data->enable,
				 data->pf1_num,
				 data->pf2_num,
				 data->total_num);
	return NDIS_STATUS_SUCCESS;
}

/*Global function body*/
INT multi_profile_check(struct _RTMP_ADAPTER *ad, CHAR *final)
{
	ULONG buf_size = MAX_INI_BUFFER_SIZE;
	CHAR *buf1 = NULL;
	CHAR *buf2 = NULL;
	INT retval = NDIS_STATUS_FAILURE;
	UCHAR *profile_path = NULL;
	/*open first profile file*/
	os_alloc_mem(ad, (UCHAR **)&buf1, buf_size);

	if (!buf1)
		goto end;

#ifdef DEFAULT_5G_PROFILE
	profile_path = multi_profile_fname_get(ad, MTB_EXT_PROFILE);
#else
	profile_path = get_dev_l2profile(ad);
#endif

	if (multi_profile_read(profile_path, buf1) != NDIS_STATUS_SUCCESS)
		goto end1;

	/*open second profile file*/
	os_alloc_mem(ad, (UCHAR **)&buf2, buf_size);

	if (!buf2)
		goto end1;

#ifdef DEFAULT_5G_PROFILE
	profile_path = get_dev_l2profile(ad);
#else
	profile_path = multi_profile_fname_get(ad, MTB_EXT_PROFILE);
#endif

	if (multi_profile_read(profile_path, buf2) != NDIS_STATUS_SUCCESS)
		goto end2;

	/*merge it*/
	if (multi_profile_dbdc_mode_sanity(buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		goto end2;

	if (multi_profile_merge(ad, buf1, buf2, final) != NDIS_STATUS_SUCCESS)
		goto end2;

	profile_path = multi_profile_fname_get(ad, MTB_MERGE_PROFILE);
	multi_profile_write(profile_path, final);
	retval = NDIS_STATUS_SUCCESS;
end2:
	os_free_mem(buf2);
end1:
	os_free_mem(buf1);
end:
	return retval;
}

/*
*
*/
INT	multi_profile_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name, UCHAR *ifidx)
{
	UCHAR *dev_name;
	struct mpf_data *data;
	INT ret;

	if (!ad->multi_pf_ctrl)
		return NDIS_STATUS_SUCCESS;

	data = (struct mpf_data *) ad->multi_pf_ctrl;

	if (!data->enable || !data->specific_dname)
		return NDIS_STATUS_SUCCESS;

	if (*ifidx >= data->pf1_num) {
		if (*ifidx == data->pf1_num)
			dev_name = get_dbdcdev_name_prefix(ad, INT_MAIN);
		else
			dev_name = get_dbdcdev_name_prefix(ad, INT_MBSSID);

		ret = snprintf(final_name, IFNAMSIZ, "%s", dev_name);

		if (os_snprintf_error(IFNAMSIZ, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "final_name snprintf error!!!\n");
		}

		*ifidx -= data->pf1_num;
	}

	return NDIS_STATUS_SUCCESS;
}

INT	multi_profile_apcli_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name, INT *ifidx)
{
	struct mpf_data *data;
	INT ret;

	if (!ad->multi_pf_ctrl)
		return NDIS_STATUS_SUCCESS;

	data = (struct mpf_data *) ad->multi_pf_ctrl;

	if (!data->enable || !data->specific_dname)
		return NDIS_STATUS_SUCCESS;

	if (*ifidx == 1) {
		/* apcli1 is 2.4G, name is apclix0*/
		ret = snprintf(final_name, IFNAMSIZ, "%s", get_dbdcdev_name_prefix(ad, INT_APCLI));

		if (os_snprintf_error(IFNAMSIZ, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "final_name snprintf error!!!\n");
		}

	}

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
VOID multi_profile_exit(struct _RTMP_ADAPTER *ad)
{
	if (ad->multi_pf_ctrl)
		os_free_mem(ad->multi_pf_ctrl);

	ad->multi_pf_ctrl = NULL;
}

/*
*
*/
UCHAR is_multi_profile_enable(struct _RTMP_ADAPTER *ad)
{
	struct mpf_data *data;

	if (!ad->multi_pf_ctrl)
		return FALSE;

	data = (struct mpf_data *) ad->multi_pf_ctrl;
	return data->enable;
}

/*
*
*/
UCHAR multi_profile_get_pf1_num(struct _RTMP_ADAPTER *ad)
{
	struct mpf_data *data;

	if (!ad->multi_pf_ctrl)
		return 0;

	data = (struct mpf_data *) ad->multi_pf_ctrl;
	return data->pf1_num;
}

/*
*
*/
UCHAR multi_profile_get_pf2_num(struct _RTMP_ADAPTER *ad)
{
	struct mpf_data *data;

	if (!ad->multi_pf_ctrl)
		return 0;

	data = (struct mpf_data *) ad->multi_pf_ctrl;
	return data->pf2_num;
}

#endif /*MULTI_PROFILE*/
