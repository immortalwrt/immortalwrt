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

#ifdef WIFI_DIAG
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <asm/bitops.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include "rt_config.h"
#include "rtmp.h"
#include "rtmp_comm.h"
#include "dot11_base.h"


#if KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE
#define GET_PRIV_DATA_FROM_INODE(inode)		PDE_DATA(inode)
#define GET_PRIV_DATA_FROM_FILE(file)		PDE_DATA(file_inode(file))
#else
#define GET_PRIV_DATA_FROM_INODE(inode)		(PDE(inode)->data)
#define GET_PRIV_DATA_FROM_FILE(file)		(PDE(file->f_path.dentry->d_inode)->data)
#endif

#define DIAG_IN_TIME(now, pCtrl) \
	(RTMP_TIME_BEFORE(now, pCtrl->diag_enable_time + ((pCtrl->diag_duration) * OS_HZ)))

#define DIAG_PROC_PATH					"ctcwifi"
#define DIAG_VAR_PATH					"/var/ctcwifi"
#define DIAG_STATUS_FILE				"/var/ctcwifi/mt_wifi_diag"
#define DIAG_PROC_FILE_NUM				5
#define DIAG_VAR_FILE_NUM				2
#define DIAG_LOG_FILE_SIZE_MAX			(1024 * 1024) /* 1M bytes */
#define DIAG_PROCESS_NUM_MAX			(8)
#define DIAG_FRAME_SIZE_MAX				(2048)
#define DIAG_LOG_BUFF_SIZE				(1000)
#define DIAG_ASSOC_ERROR_BUFF_SIZE		(100)

typedef enum _ENUM_DIAG_BAND {
	DIAG_BAND_2G = 0,
	DIAG_BAND_5G,
	DIAG_BAND_MAX
} ENUM_DIAG_BAND;

typedef struct _DIAG_WIFI_PROCESS_ENTRY {
	UCHAR name[RTMP_OS_TASK_NAME_LEN];
	INT32 pid;
	UCHAR is_process;
} DIAG_WIFI_PROCESS_ENTRY;

typedef struct _DIAG_WIFI_PROCESS_INFO {
	UCHAR num;
	DIAG_WIFI_PROCESS_ENTRY entry[DIAG_PROCESS_NUM_MAX];
} DIAG_WIFI_PROCESS_INFO;

typedef struct _DIAG_LOG_ENTRY {
	struct rtc_time tm;
	BOOLEAN isTX;
	UCHAR ssid[MAX_LEN_OF_SSID+1];
	UCHAR ssid_len;
	UCHAR band;
	UCHAR frame_type[32];
	UCHAR sta_addr[6];
	UCHAR data[DIAG_FRAME_SIZE_MAX];
	UINT32 data_len;
} DIAG_LOG_ENTRY;

typedef struct _DIAG_FRAME_INFO {
	UCHAR isTX;
	UCHAR *ssid;
	UCHAR ssid_len;
	UCHAR band; /* 0:2G, 1:5G, get from pAd->LatchRfRegs.Channel */
	UCHAR *pData; /* include 80211 header, refer to pRxBlk (RX)/ pData (TX) */
	UINT32 dataLen; /* packet length */
} DIAG_FRAME_INFO;

typedef struct DIAG_ASSOC_ERROR_ENTRY {
	struct rtc_time tm;
	UCHAR StaAddr[6];
	UCHAR Ssid[32];
	ENUM_DIAG_CONN_ERROR_CODE errCode;
	UINT32 reason;
} DIAG_ASSOC_ERROR_ENTRY;

typedef struct _DIAG_CTRL {
	UINT8	init_flag;
	ENUM_DIAG_BAND	diag_band;
	UINT32	diag_duration;
	UINT32	diag_enable;
	ULONG	diag_enable_time;
	UINT32	busy_time;

	UINT32 diag_log_file_offset;
	DIAG_LOG_ENTRY diag_log_entry[DIAG_LOG_BUFF_SIZE];
	UINT32 diag_log_read_idx;
	UINT32 diag_log_write_idx;
	NDIS_SPIN_LOCK diag_log_lock;
	UCHAR diag_log_tmp_buf[2*DIAG_FRAME_SIZE_MAX + 128];

	UINT32 assoc_error_file_offset;
	DIAG_ASSOC_ERROR_ENTRY assoc_error_entry[DIAG_ASSOC_ERROR_BUFF_SIZE];
	UINT32 assoc_error_read_idx;
	UINT32 assoc_error_write_idx;
	NDIS_SPIN_LOCK assoc_error_lock;

	DIAG_WIFI_PROCESS_INFO process_info;
	PRTMP_ADAPTER pAd;
	UCHAR channel;
} DIAG_CTRL;


const PCHAR DIAG_CONN_ERR_INFO[DIAG_CONN_ERROR_MAX] = {
	"WiFi frames losing. reason code ",
	"Capability checking failed.\n",
	"Authentication failed. reason code ",
	"Rejected by blacklist.\n",
	"Rejected by STA limiation.\n",
	"De-authed. reason code ",
	"Association ignored by band steering.\n"
};

const PCHAR DIAG_PROC_FILE_LIST[DIAG_BAND_MAX][DIAG_PROC_FILE_NUM] = {
	{
		DIAG_PROC_PATH"/Stat2G",
		DIAG_PROC_PATH"/diag_duration_2G",
		DIAG_PROC_PATH"/diag_enable_2G",
		DIAG_PROC_PATH"/channel_occupancy_2G",
		DIAG_PROC_PATH"/stats_2G"
	},
	{
		DIAG_PROC_PATH"/Stat5G",
		DIAG_PROC_PATH"/diag_duration_5G",
		DIAG_PROC_PATH"/diag_enable_5G",
		DIAG_PROC_PATH"/channel_occupancy_5G",
		DIAG_PROC_PATH"/stats_5G"
	}
};

const PCHAR DIAG_VAR_FILE_LIST[DIAG_BAND_MAX][DIAG_VAR_FILE_NUM] = {
	{
		DIAG_VAR_PATH"/association_errors_2G",
		DIAG_VAR_PATH"/diag_log_2G",
	},
	{
		DIAG_VAR_PATH"/association_errors_5G",
		DIAG_VAR_PATH"/diag_log_5G"
	}
};

const UCHAR MAC_ADDR_ZERO[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void diag_common_deauth_code_trans(USHORT reason,
	ENUM_DIAG_CONN_ERROR_CODE *Code)
{
	switch (reason) {
	case REASON_4_WAY_TIMEOUT:
	case REASON_GROUP_KEY_HS_TIMEOUT:
	case REASON_MIC_FAILURE:
	case REASON_STA_REQ_ASSOC_NOT_AUTH:
	case REASON_8021X_AUTH_FAIL:
	case REASON_IE_DIFFERENT:
	case REASON_MCIPHER_NOT_VALID:
	case REASON_UCIPHER_NOT_VALID:
	case REASON_AKMP_NOT_VALID:
	case REASON_UNSUPPORT_RSNE_VER:
	case REASON_CIPHER_SUITE_REJECTED:
		*Code = DIAG_CONN_AUTH_FAIL;
		break;
	case REASON_NO_LONGER_VALID:
		*Code = DIAG_CONN_FRAME_LOST;
		break;
	case REASON_INVALID_IE:
	case REASON_INVALID_RSNE_CAP:
		*Code = DIAG_CONN_CAP_ERROR;
		break;
	case REASON_DECLINED:
		*Code = DIAG_CONN_ACL_BLK;
		break;
	default:
		*Code = DIAG_CONN_DEAUTH;
		break;
	}
}

void diag_get_frame_info(
	PHEADER_802_11 pHeader, UCHAR isTx,
	UCHAR **pp_frame_type, UCHAR **pp_sta_addr)
{
	if (!pHeader || !pp_frame_type || !pp_sta_addr)
		return;

	*pp_frame_type = NULL;
	*pp_sta_addr = NULL;

	if (pHeader->FC.Type == FC_TYPE_MGMT) {

		if (isTx)
			*pp_sta_addr = pHeader->Addr1;
		else
			*pp_sta_addr = pHeader->Addr2;

		switch (pHeader->FC.SubType) {
		case SUBTYPE_BEACON:
			/* just record our beacon */
			if (isTx)
				*pp_frame_type = "Beacon";
			break;

		case SUBTYPE_PROBE_REQ:
			/* just record peer's probe request */
			if (!isTx)
				*pp_frame_type = "Probe Request";
			break;

		case SUBTYPE_PROBE_RSP:
			/* just record our probe response */
			if (isTx)
				*pp_frame_type = "Probe Response";
			break;

		case SUBTYPE_ATIM:
			*pp_frame_type = "ATIM";
			break;

		case SUBTYPE_DISASSOC:
			*pp_frame_type = "Disassociation";
			break;

		case SUBTYPE_DEAUTH:
			*pp_frame_type = "Deauthentication";
			break;

		case SUBTYPE_ASSOC_REQ:
			/* just record peer's Association request */
			if (!isTx)
				*pp_frame_type = "Association Request";
			break;

		case SUBTYPE_ASSOC_RSP:
			/* just record our Association response */
			if (isTx)
				*pp_frame_type = "Association Response";
			break;

		case SUBTYPE_REASSOC_REQ:
			/* just record peer's Reassociation request */
			if (!isTx)
				*pp_frame_type = "Reassociation Request";
			break;

		case SUBTYPE_REASSOC_RSP:
			/* just record our Reassociation response */
			if (isTx)
				*pp_frame_type = "Reassociation Response";
			break;

		case SUBTYPE_AUTH:
			*pp_frame_type = "Authentication";
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "not handled MGMT frme, subtype=%d\n", pHeader->FC.SubType);
			break;
		}
	} else if (pHeader->FC.Type == FC_TYPE_CNTL) {

		switch (pHeader->FC.SubType) {
		case SUBTYPE_RTS:
			*pp_frame_type = "RTS";
			if (isTx)
				*pp_sta_addr = pHeader->Addr1;
			else
				*pp_sta_addr = pHeader->Addr2;
			break;

		case SUBTYPE_CTS:
			*pp_frame_type = "CTS";
			if (isTx)
				*pp_sta_addr = pHeader->Addr1;
			break;

		case SUBTYPE_ACK:
			*pp_frame_type = "ACK";
			if (isTx)
				*pp_sta_addr = pHeader->Addr1;
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "not handled CNTL frme, subtype=%d\n", pHeader->FC.SubType);
			break;
		}
	}
}

void diag_frame_cache(DIAG_FRAME_INFO *info, DIAG_CTRL *pCtrl)
{
	DIAG_LOG_ENTRY *entry;
	UCHAR *p_frame_type = NULL;
	UCHAR *p_sta_addr = NULL;
	struct timex  txc;
	ULONG flags;
	ULONG now;

	NdisGetSystemUpTime(&now);

	if ((info == NULL) || (pCtrl == NULL))
		return;

	if (!DIAG_IN_TIME(now, pCtrl))
		return;

	diag_get_frame_info((PHEADER_802_11)info->pData, info->isTX, &p_frame_type, &p_sta_addr);
	if (!p_frame_type)
		return;

	RTMP_INT_LOCK(&pCtrl->diag_log_lock, flags);

	if (((pCtrl->diag_log_write_idx + 1) % DIAG_LOG_BUFF_SIZE)
		!= pCtrl->diag_log_read_idx) {

		entry = &pCtrl->diag_log_entry[pCtrl->diag_log_write_idx];
		memset(entry, 0, sizeof(*entry));

		do_gettimeofday(&(txc.time));
		txc.time.tv_sec -= sys_tz.tz_minuteswest * 60;
		rtc_time_to_tm(txc.time.tv_sec, &entry->tm);

		entry->isTX = info->isTX;

		if (info->ssid_len <= 32)
			entry->ssid_len = info->ssid_len;
		else
			entry->ssid_len = 32;
		memcpy(entry->ssid, info->ssid, entry->ssid_len);

		entry->band = info->band;

		if (strlen(p_frame_type) < 32)
			memcpy(entry->frame_type, p_frame_type, strlen(p_frame_type));
		else
			memcpy(entry->frame_type, p_frame_type, 32-1);

		if (p_sta_addr)
			memcpy(entry->sta_addr, p_sta_addr, 6);

		entry->data_len =
			(info->dataLen > DIAG_FRAME_SIZE_MAX) ? DIAG_FRAME_SIZE_MAX : info->dataLen;
		memcpy(entry->data, info->pData, entry->data_len);

		/* update write_idx */
		pCtrl->diag_log_write_idx =
			(pCtrl->diag_log_write_idx + 1) % DIAG_LOG_BUFF_SIZE;
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "diag_log_entry array full\n");
	}

	RTMP_INT_UNLOCK(&pCtrl->diag_log_lock, flags);
}

static int diag_statistic_proc_show(struct seq_file *m, void *v)
{
	DIAG_CTRL *pCtrl = (DIAG_CTRL *)m->private;
	PRTMP_ADAPTER pAd = NULL;
	ULONG txCount = 0;
	ULONG rxCount = 0;
	ULONG txFails = 0;
	UINT8 dbdc_idx = 0;
	struct _RTMP_CHIP_CAP *cap = NULL;

	if (pCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl is NULL\n");
		return -EFAULT;
	}
	pAd = pCtrl->pAd;
	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pAd is NULL\n");
		return -EFAULT;
	}
	cap = hc_get_chip_cap(pAd->hdev_ctrl);

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode == TRUE) {
		if (pCtrl->channel > 14)
			dbdc_idx = 1;
		else
			dbdc_idx = 0;
	} else
		dbdc_idx = 0;
#else
	dbdc_idx = 0;
#endif

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "dbdc_idx=%d\n", dbdc_idx);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (cap->fgRateAdaptFWOffload == TRUE) {
		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
		memset(&rTxStatResult, 0, sizeof(rTxStatResult));
		MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT, dbdc_idx, 0, &rTxStatResult);
		pAd->WlanCounters[dbdc_idx].TransmittedFragmentCount.u.LowPart += (rTxStatResult.u4TotalTxCount -
				rTxStatResult.u4TotalTxFailCount);
		pAd->WlanCounters[dbdc_idx].FailedCount.u.LowPart += rTxStatResult.u4TotalTxFailCount;
		pAd->WlanCounters[dbdc_idx].CurrentBwTxCount.u.LowPart += rTxStatResult.u4CurrBwTxCnt;
		pAd->WlanCounters[dbdc_idx].OtherBwTxCount.u.LowPart += rTxStatResult.u4OtherBwTxCnt;
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd))
		rxCount = (ULONG)pAd->WlanCounters[dbdc_idx].RxMpduCount.QuadPart;
	else
#endif
		rxCount = pAd->WlanCounters[dbdc_idx].ReceivedFragmentCount.QuadPart;
	txCount = pAd->WlanCounters[dbdc_idx].TransmittedFragmentCount.u.LowPart;
	txFails = pAd->WlanCounters[dbdc_idx].FailedCount.u.LowPart;

	seq_printf(m, "tx_packets:%ld\n", txCount);
	seq_printf(m, "rx_packets:%ld\n", rxCount);
	seq_printf(m, "tx_fails:%ld\n", txFails);

	return 0;
}

static int diag_statistic_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, diag_statistic_proc_show, GET_PRIV_DATA_FROM_INODE(inode));
}

static int diag_duration_proc_show(struct seq_file *m, void *v)
{
	DIAG_CTRL *pCtrl = (DIAG_CTRL *)m->private;

	if (pCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl is NULL\n");
		return -EFAULT;
	}

	seq_printf(m, "%d\n", pCtrl->diag_duration);
	return 0;
}

static int diag_duration_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, diag_duration_proc_show, GET_PRIV_DATA_FROM_INODE(inode));
}

static ssize_t diag_duration_proc_write(struct file *file, const char __user *buffer,
	size_t count, loff_t *pos)
{
	DIAG_CTRL *pCtrl = NULL;
	unsigned int val = 0;
	UCHAR buf[10];

	if ((file == NULL) || (buffer == NULL))
		return -EFAULT;

	pCtrl = (DIAG_CTRL *)GET_PRIV_DATA_FROM_FILE(file);
	if (pCtrl == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl is NULL\n");
		return -EFAULT;
	}

	memset(buf, 0, sizeof(buf));
	if (copy_from_user(buf, buffer, min(sizeof(buf), count)))
		return -EFAULT;

	val = simple_strtol(buf, NULL, 10);
	if ((val <= 300) && (val >= 60)) {
		pCtrl->diag_duration = val;
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "set duraion %d, band=%d.\n", val, pCtrl->diag_band);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "rang error, valid rang is [60,300].\n");
		return -EFAULT;
	}
	return count;
}

static int diag_enable_proc_show(struct seq_file *m, void *v)
{
	DIAG_CTRL *pCtrl = (DIAG_CTRL *)m->private;

	if (pCtrl == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl is NULL\n");
		return -EFAULT;
	}

	seq_printf(m, "%d\n", pCtrl->diag_enable);
	return 0;
}

static int diag_enable_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, diag_enable_proc_show, GET_PRIV_DATA_FROM_INODE(inode));
}

static ssize_t diag_enable_proc_write(struct file *file, const char __user *buffer,
	size_t count, loff_t *pos)
{
	PRTMP_ADAPTER pAd = NULL;
	DIAG_CTRL *pCtrl = NULL;
	RTMP_OS_FD fd = NULL;
	RTMP_OS_FS_INFO osFSInfo;
	UCHAR buf[10];
	UINT val = 0;

	if ((file == NULL) || (buffer == NULL))
		return -EFAULT;

	pCtrl = (DIAG_CTRL *)GET_PRIV_DATA_FROM_FILE(file);
	if (pCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl is NULL\n");
		return -EFAULT;
	}

	pAd = pCtrl->pAd;
	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pAd is NULL\n");
		return -EFAULT;
	}

	memset(buf, 0, sizeof(buf));
	if (copy_from_user(buf, buffer, min(sizeof(buf), count)))
		return -EFAULT;

	val = simple_strtol(buf, NULL, 10);
	if (val > 1) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "rang error, valid rang is [0,1].\n");
		return -EFAULT;
	}

	/* stop diag_log handling */
	pCtrl->diag_enable = 0;
	OS_WAIT(500);

	if (val !=  0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Flush diag_log file\n");
		memset(pCtrl->diag_log_entry, 0, DIAG_LOG_BUFF_SIZE*sizeof(DIAG_LOG_ENTRY));
		pCtrl->diag_log_read_idx = 0;
		pCtrl->diag_log_write_idx = 0;

		RtmpOSFSInfoChange(&osFSInfo, TRUE);
		fd = RtmpOSFileOpen(DIAG_VAR_FILE_LIST[pCtrl->diag_band][1],
			O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
		if (!IS_FILE_OPEN_ERR(fd))
			RtmpOSFileClose(fd);
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
		pCtrl->diag_log_file_offset = 0;
		NdisGetSystemUpTime(&pCtrl->diag_enable_time);
		pCtrl->diag_enable = val;
	}

	return count;
}

static int diag_ch_occ_proc_show(struct seq_file *m, void *v)
{
	DIAG_CTRL *pCtrl = NULL;
	UINT32 ch_occ = 0;
	CHAR *band = NULL;
	UCHAR ch = 0;

	pCtrl = (DIAG_CTRL *)m->private;
	if (pCtrl == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl is NULL\n");
		return -EFAULT;
	}

	ch = pCtrl->channel;
	ch_occ = (100 * pCtrl->busy_time)/1000;
	seq_puts(m, "Channel\tBand\tOccupancy\n");
	if (ch < 14)
		band = "2G";
	else
		band = "5G";

	seq_printf(m, "%-7d\t%-4s\t%-9d\n", ch, band, ch_occ);
	return 0;
}

static int diag_ch_occ_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, diag_ch_occ_proc_show, GET_PRIV_DATA_FROM_INODE(inode));
}

static int diag_sta_proc_show(struct seq_file *m, void *v)
{
	PRTMP_ADAPTER pAd = NULL;
	DIAG_CTRL *pCtrl = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
#endif
	INT i = 0;
	UINT8 dbdc_idx = 0;
	UINT32 pecent;
	UINT32 deci;
	UINT32 tx_suc;
	UINT16 wtbl_max_num = 0;
	UCHAR apidx = 0;
	struct _RTMP_CHIP_CAP *cap = NULL;

	pCtrl = (DIAG_CTRL *)m->private;
	if (pCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl is NULL\n");
		return -EFAULT;
	}

	pAd = pCtrl->pAd;
	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pAd is NULL\n");
		return -EFAULT;
	}

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	wtbl_max_num = WTBL_MAX_NUM(pAd);
	dbdc_idx = HcGetBandByWdev(&pAd->ApCfg.MBSSID[apidx].wdev);

	seq_puts(m, "STA:             \tSNR  \tRSSI   \tTXSUCC\n");
	for (i = 0; i < wtbl_max_num; i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (pEntry->EntryType != ENTRY_CLIENT)
			continue;

		apidx = pEntry->func_tb_idx;
		if ((pCtrl->diag_band == DIAG_BAND_2G && WMODE_CAP_5G(&pAd->ApCfg.MBSSID[apidx].wdev.PhyMode)) ||
		(pCtrl->diag_band == DIAG_BAND_5G && WMODE_CAP_2G(pAd->ApCfg.MBSSID[apidx].wdev.PhyMode)))
			continue;

		seq_printf(m, MACSTR, (MAC2STR(pEntry->Addr)));
		seq_printf(m, "\t%d", pEntry->RssiSample.AvgSnr[0]);
		seq_printf(m, "\t%d", RTMPAvgRssi(pAd, &pEntry->RssiSample));

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (cap->fgRateAdaptFWOffload == TRUE) {
			memset(&rTxStatResult, 0, sizeof(EXT_EVENT_TX_STATISTIC_RESULT_T));
			MtCmdGetTxStatistic(pAd,
				GET_TX_STAT_TOTAL_TX_CNT, dbdc_idx,
				pEntry->wcid, &rTxStatResult);
			tx_suc = rTxStatResult.u4TotalTxCount - rTxStatResult.u4TotalTxFailCount;

			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "tx_succ=%d, tx_total=%d\n", tx_suc, rTxStatResult.u4TotalTxCount);

			if ((rTxStatResult.u4TotalTxCount > 0) && (tx_suc <= rTxStatResult.u4TotalTxCount)) {
				pecent = (UINT32)((100 * tx_suc)/rTxStatResult.u4TotalTxCount);
				deci = (UINT32)((100 * tx_suc)%rTxStatResult.u4TotalTxCount);
				deci = (UINT32)((100 * deci)/rTxStatResult.u4TotalTxCount);
			} else {
				pecent = 100;
				deci = 0;
			}
		}
#endif
		seq_printf(m, "\t%d.%02d\n", pecent, deci);
	}

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd))
		seq_printf(m, "RXCRCERR: %d\n", pAd->WlanCounters[dbdc_idx].RxFcsErrorCount.u.LowPart);
	else
#endif
		seq_printf(m, "RXCRCERR: %d\n", pAd->WlanCounters[dbdc_idx].FCSErrorCount.u.LowPart);
	return 0;
}

static int diag_sta_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, diag_sta_proc_show, GET_PRIV_DATA_FROM_INODE(inode));
}

static const struct file_operations diag_statistic_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= diag_statistic_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations diag_duration_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= diag_duration_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= diag_duration_proc_write,
};

static const struct file_operations diag_enable_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= diag_enable_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= diag_enable_proc_write,
};

static const struct file_operations diag_ch_occ_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= diag_ch_occ_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations diag_sta_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= diag_sta_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#if defined(MT7663) || defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
void diag_get_snr(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR *pData)
{
	CHAR avgSnr;
	PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[wcid];


#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		struct rxd_grp_3 *rxd_grp3 = (struct rxd_grp_3 *)pData;

		if (rxd_grp3 && (wcid < MAX_LEN_OF_MAC_TABLE))
			avgSnr = ((rxd_grp3->rxd_17) & BITS(13, 18)) >> 13;
	}
#endif
	/* real SNR is read value -16.
		The real SNR may < 0, it's right, but customer may feel strange.
		so if real SNR < 0, treate it as 0. */
	avgSnr = (avgSnr >= 16) ? (avgSnr - 16) : 0;

	/* moving average */
	if (pEntry->RssiSample.AvgSnr[0])
		pEntry->RssiSample.AvgSnr[0] = (((pEntry->RssiSample.AvgSnr[0] * 7) + avgSnr) >> 3);
	else
		pEntry->RssiSample.AvgSnr[0] = avgSnr;

	MTWF_DBG(pAd, DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"DiagAvgSnr:%d\n", pEntry->RssiSample.AvgSnr[0]);
}
#endif

void diag_var_dir_mk(ENUM_DIAG_BAND diag_band)
{
	RTMP_OS_FS_INFO osFSInfo;
	int ret = 0;
	RTMP_OS_FD fd = NULL;
	int i;
	/* create var sub directory */
	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	ret = sys_access(DIAG_VAR_PATH, 0);
	if (ret != 0) {
		ret = sys_mkdir(DIAG_VAR_PATH, 0777);
		if (ret < 0)
			MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error create directory %s\n", DIAG_VAR_PATH);
	}
	for (i = 0; i < DIAG_VAR_FILE_NUM; i++) {
		fd = RtmpOSFileOpen(DIAG_VAR_FILE_LIST[diag_band][i], O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
		if (IS_FILE_OPEN_ERR(fd))
			MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error create %s\n", DIAG_VAR_FILE_LIST[diag_band][i]);
		else
			RtmpOSFileClose(fd);
	}

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
}

void diag_proc_dir_mk(ENUM_DIAG_BAND diag_band)
{
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_OS_FD fd = NULL;
	UCHAR buf[64];
	UCHAR buf_size = 64;
	UCHAR tmpbuf[64];
	UCHAR diag_2G = 0, diag_5G = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "diag_band:%d\n", diag_band);

	memset(buf, 0, 64);
	memset(tmpbuf, 0, 64);

	/*test the DIAG_PROC_PATH dir exist*/
	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	fd = RtmpOSFileOpen(DIAG_STATUS_FILE, O_CREAT | O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_FILE_OPEN_ERR(fd)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "open DIAG_STATUS_FILE for read failed\n");
	} else {
		if (RtmpOSFileRead(fd, buf, buf_size - 1) > 0) {
			if (RTMPGetKeyParameter("2G_WIFI_DIAG", tmpbuf, 64, buf, TRUE)) {
				diag_2G = (UCHAR)simple_strtol(tmpbuf, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "diag_2G=%d\n", diag_2G);
			}
			if (RTMPGetKeyParameter("5G_WIFI_DIAG", tmpbuf, 64, buf, TRUE)) {
				diag_5G = (UCHAR)simple_strtol(tmpbuf, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "diag_5G=%d\n", diag_5G);
			}
		}
		RtmpOSFileClose(fd);
	}

	if ((diag_2G == 0) && (diag_5G == 0)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" dir %s may not exist, create it\n", DIAG_PROC_PATH);
		proc_mkdir(DIAG_PROC_PATH, NULL);
	}

	fd = RtmpOSFileOpen(DIAG_STATUS_FILE, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_FILE_OPEN_ERR(fd)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "open DIAG_STATUS_FILE for write failed\n");
	} else {
		char *diag_stat_str = NULL;

		if (diag_band == DIAG_BAND_2G) {
			if (diag_5G == 0)
				diag_stat_str = "Default\n2G_WIFI_DIAG=1\n5G_WIFI_DIAG=0\n";
			else
				diag_stat_str = "Default\n2G_WIFI_DIAG=1\n5G_WIFI_DIAG=1\n";

		} else if (diag_band == DIAG_BAND_5G) {
			if (diag_2G == 0)
				diag_stat_str = "Default\n2G_WIFI_DIAG=0\n5G_WIFI_DIAG=1\n";
			else
				diag_stat_str = "Default\n2G_WIFI_DIAG=1\n5G_WIFI_DIAG=1\n";
		}
		RtmpOSFileWrite(fd, diag_stat_str, strlen(diag_stat_str));
		RtmpOSFileClose(fd);
	}

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
}


void diag_proc_dir_rm(ENUM_DIAG_BAND diag_band)
{
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_OS_FD fd = NULL;
	UCHAR buf[64];
	UCHAR buf_size = 64;
	UCHAR tmpbuf[64];
	UCHAR diag_2G = 0, diag_5G = 0;

	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "diag_band:%d\n", diag_band);

	memset(buf, 0, 64);
	memset(tmpbuf, 0, 64);

	/*test the DIAG_PROC_PATH dir exist*/
	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	fd = RtmpOSFileOpen(DIAG_STATUS_FILE, O_CREAT | O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_FILE_OPEN_ERR(fd)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "open DIAG_STATUS_FILE failed\n");
	} else {
		if (RtmpOSFileRead(fd, buf, buf_size - 1) > 0) {
			if (RTMPGetKeyParameter("2G_WIFI_DIAG", tmpbuf, 64, buf, TRUE)) {
				diag_2G = (UCHAR)simple_strtol(tmpbuf, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "diag_2G=%d\n", diag_2G);
			}
			if (RTMPGetKeyParameter("5G_WIFI_DIAG", tmpbuf, 64, buf, TRUE)) {
				diag_5G = (UCHAR)simple_strtol(tmpbuf, 0, 10);
				MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "diag_5G=%d\n", diag_5G);
			}
		}
		RtmpOSFileClose(fd);
	}

	if (((diag_band == DIAG_BAND_2G) && (diag_5G == 0)) ||
		((diag_band == DIAG_BAND_5G) && (diag_2G == 0))) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "remove proc/ctcwifi\n");
		remove_proc_entry(DIAG_PROC_PATH, NULL);
	}

	fd = RtmpOSFileOpen(DIAG_STATUS_FILE, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_FILE_OPEN_ERR(fd)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "open DIAG_STATUS_FILE for write failed\n");
	} else {
		char *diag_stat_str = NULL;

		if (diag_band == DIAG_BAND_2G) {
			if (diag_5G == 0)
				diag_stat_str = "Default\n2G_WIFI_DIAG=0\n5G_WIFI_DIAG=0\n";
			else
				diag_stat_str = "Default\n2G_WIFI_DIAG=0\n5G_WIFI_DIAG=1\n";

		} else if (diag_band == DIAG_BAND_5G) {
			if (diag_2G == 0)
				diag_stat_str = "Default\n2G_WIFI_DIAG=0\n5G_WIFI_DIAG=0\n";
			else
				diag_stat_str = "Default\n2G_WIFI_DIAG=1\n5G_WIFI_DIAG=0\n";
		}
		RtmpOSFileWrite(fd, diag_stat_str, strlen(diag_stat_str));
		RtmpOSFileClose(fd);
	}

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
}

void diag_proc_entry_init(ENUM_DIAG_BAND diag_band, DIAG_CTRL *pCtrl)
{
	const struct file_operations *DIAG_PROC_FOPS[5] = {
		&diag_statistic_proc_fops,
		&diag_duration_proc_fops,
		&diag_enable_proc_fops,
		&diag_ch_occ_proc_fops,
		&diag_sta_proc_fops
	};
	int i;

	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "diag_band:%d\n", diag_band);

	for (i = 0; i < DIAG_PROC_FILE_NUM; i++)
		proc_create_data(DIAG_PROC_FILE_LIST[diag_band][i], S_IRWXU | S_IRWXG | S_IRWXO, NULL,
				 DIAG_PROC_FOPS[i], pCtrl);
}

void diag_proc_entry_deinit(ENUM_DIAG_BAND diag_band)
{
	int i;

	for (i = 0; i < DIAG_PROC_FILE_NUM; i++)
		remove_proc_entry(DIAG_PROC_FILE_LIST[diag_band][i], 0);
}

void diag_ctrl_alloc(PRTMP_ADAPTER pAd)
{
	DIAG_CTRL *pCtrl = NULL;
	UCHAR index = 0;
	UCHAR dbdc_idx = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pAd:%p\n", pAd);

	for (dbdc_idx = 0; dbdc_idx < DBDC_BAND_NUM; dbdc_idx++) {
		os_alloc_mem(NULL, (UCHAR **)&pCtrl, sizeof(DIAG_CTRL));
		if (pCtrl == NULL) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"fail to allocate pDiagCtrl memory dbdc_idx=%d\n", dbdc_idx);
			return;
		}
		memset(pCtrl, 0, sizeof(DIAG_CTRL));

		for (index = 0; index < DIAG_PROCESS_NUM_MAX; index++)
			pCtrl->process_info.entry[index].pid = -1;

		pCtrl->diag_enable = 0;
		pCtrl->diag_duration = 60;

		pAd->pDiagCtrl[dbdc_idx] = pCtrl;
	}
}

void diag_ctrl_free(PRTMP_ADAPTER pAd)
{
	UCHAR dbdc_idx = 0;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pAd:%p\n", pAd);

	for (dbdc_idx = 0; dbdc_idx < DBDC_BAND_NUM; dbdc_idx++) {
		if (pAd->pDiagCtrl[dbdc_idx])
			os_free_mem(pAd->pDiagCtrl[dbdc_idx]);
		pAd->pDiagCtrl[dbdc_idx] = NULL;
	}
}

void diag_add_pid(OS_TASK *pTask)
{
	PRTMP_ADAPTER	pAd = NULL;
	DIAG_CTRL *pCtrl = NULL;
	UCHAR index = 0;

	if (!pTask) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTask is NULL\n");
		return;
	}

	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);
	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pAd is NULL\n");
		return;
	}

	/* for non-DBDC mode, pAd->pDiagCtrl has 1 array element, use pDiagCtrl[0] to record task info;
	*   for DBDC mode, pAd->pDiagCtrl has 2 array elements, but driver share the tasks, hence also
	*    use pDiagCtrl[0] to record task info.
	*/
	pCtrl = pAd->pDiagCtrl[0];
	if (!pCtrl) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl is NULL\n");
		return;
	}

	for (index = 0; index < DIAG_PROCESS_NUM_MAX; index++) {
		if (pCtrl->process_info.entry[index].pid == -1) {
			pCtrl->process_info.entry[index].is_process = 0;
#ifdef KTHREAD_SUPPORT
			pCtrl->process_info.entry[index].pid = current->pid;
#else
			pCtrl->process_info.entry[index].pid = pTask->taskPID;
#endif
			memcpy(pCtrl->process_info.entry[index].name, pTask->taskName,
				(strlen(pTask->taskName) > (RTMP_OS_TASK_NAME_LEN-1)) ?
				(RTMP_OS_TASK_NAME_LEN-1) : strlen(pTask->taskName));
			pCtrl->process_info.num++;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "add PID=%d, name=%s, total_num=%d\n", pCtrl->process_info.entry[index].pid,
				pCtrl->process_info.entry[index].name,
				pCtrl->process_info.num);
			break;
		}
	}
}

void diag_del_pid(OS_TASK *pTask)
{
	PRTMP_ADAPTER	pAd = NULL;
	DIAG_CTRL *pCtrl = NULL;
	UCHAR index = 0;

	if (!pTask) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTask is NULL\n");
		return;
	}

	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);
	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pAd is NULL\n");
		return;
	}

	/* for non-DBDC mode, pAd->pDiagCtrl has 1 array element, use pDiagCtrl[0] to record task info;
	*   for DBDC mode, pAd->pDiagCtrl has 2 array elements, but driver share the tasks, hence also
	*    use pDiagCtrl[0] to record task info.
	*/
	pCtrl = pAd->pDiagCtrl[0];
	if (!pCtrl) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl is NULL\n");
		return;
	}

	for (index = 0; index < DIAG_PROCESS_NUM_MAX; index++) {
#ifdef KTHREAD_SUPPORT
		if (pCtrl->process_info.entry[index].pid == current->pid)
#else
		if (pCtrl->process_info.entry[index].pid == pTask->taskPID)
#endif
		{
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "delete PID=%d, total_num=%d\n", pCtrl->process_info.entry[index].pid, pCtrl->process_info.num);
			pCtrl->process_info.entry[index].is_process = 0;
			pCtrl->process_info.entry[index].pid = -1;
			memset(pCtrl->process_info.entry[index].name,
				0, RTMP_OS_TASK_NAME_LEN);
			pCtrl->process_info.num--;
			break;
		}
	}
}

VOID diag_get_process_info(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	DIAG_WIFI_PROCESS_INFO process_info;
	UCHAR index = 0;

	/* for non-DBDC mode, pAd->pDiagCtrl has 1 array element, use pDiagCtrl[0] to record task info;
	*   for DBDC mode, pAd->pDiagCtrl has 2 array elements, but driver share the tasks, hence also
	*    use pDiagCtrl[0] to record task info.
	*/
	DIAG_CTRL *pCtrl = pAd->pDiagCtrl[0];

	if (!wrq)
		return;

	if ((!pCtrl) || (!pCtrl->init_flag))
		return;

	memset(&process_info, 0, sizeof(process_info));
	for (index = 0; index < DIAG_PROCESS_NUM_MAX; index++) {
		if (pCtrl->process_info.entry[index].pid != -1) {
			memcpy(&process_info.entry[process_info.num],
				&pCtrl->process_info.entry[index], sizeof(DIAG_WIFI_PROCESS_ENTRY));
			process_info.num++;
		}
	}

	wrq->u.data.length = sizeof(process_info);
	if (copy_to_user(wrq->u.data.pointer, &process_info, wrq->u.data.length))
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user() fail\n");
}

void diag_conn_error(PRTMP_ADAPTER pAd, UCHAR apidx, UCHAR *addr,
	ENUM_DIAG_CONN_ERROR_CODE Code, UINT32 Reason)
{
	struct timex  txc;
	DIAG_ASSOC_ERROR_ENTRY *entry;
	ULONG flags;
	ENUM_DIAG_CONN_ERROR_CODE DiagCode = 0;
	DIAG_CTRL *pCtrl = NULL;
	UCHAR dbdc_idx;

	if (!addr)
		return;

	if (!(VALID_MBSS(pAd, apidx) && (apidx < pAd->ApCfg.BssidNum)))
		return;

	dbdc_idx = HcGetBandByWdev(wdev);
	pCtrl = pAd->pDiagCtrl[dbdc_idx];

	if ((!pCtrl) || (!pCtrl->init_flag)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl not init\n");
		return;
	}

	if (Code == DIAG_CONN_DEAUTH_COM)
		diag_common_deauth_code_trans(Reason, &DiagCode);
	else if (Code < DIAG_CONN_ERROR_MAX)
		DiagCode = Code;

	RTMP_INT_LOCK(&pCtrl->assoc_error_lock, flags);
	if (((pCtrl->assoc_error_write_idx + 1) % DIAG_ASSOC_ERROR_BUFF_SIZE) !=
		pCtrl->assoc_error_read_idx) {

		entry = &pCtrl->assoc_error_entry[pCtrl->assoc_error_write_idx];
		memset(entry, 0, sizeof(*entry));

		do_gettimeofday(&(txc.time));
		txc.time.tv_sec -= sys_tz.tz_minuteswest * 60;
		rtc_time_to_tm(txc.time.tv_sec, &entry->tm);

		memcpy(entry->StaAddr, addr, 6);
		if (pAd->ApCfg.MBSSID[apidx].SsidLen > 0 && pAd->ApCfg.MBSSID[apidx].SsidLen <= 32)
			memcpy(entry->Ssid, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
		entry->errCode = DiagCode;
		entry->reason = Reason;

		/* update write_idx */
		pCtrl->assoc_error_write_idx =
			(pCtrl->assoc_error_write_idx + 1) % DIAG_ASSOC_ERROR_BUFF_SIZE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "assoc_error_entry array full\n");
	}

	RTMP_INT_UNLOCK(&pCtrl->assoc_error_lock, flags);
}

void diag_conn_error_write(PRTMP_ADAPTER pAd)
{
	RTMP_OS_FD fd = NULL;
	RTMP_OS_FS_INFO osFSInfo;
	UCHAR buf[256];
	UINT32 buf_size = 256;
	DIAG_ASSOC_ERROR_ENTRY *log_entry;
	UINT32 write_size;
	DIAG_CTRL *pCtrl = NULL;
	UCHAR dbdc_idx = 0;

	for (dbdc_idx = 0; dbdc_idx < DBDC_BAND_NUM; dbdc_idx++) {
		pCtrl = pAd->pDiagCtrl[dbdc_idx];
		if ((!pCtrl) || (!pCtrl->init_flag)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "diag not init dbdc_idx=%d\n", dbdc_idx);
			continue;
		}

		memset(buf, 0, sizeof(buf));
		RtmpOSFSInfoChange(&osFSInfo, TRUE);
		fd = RtmpOSFileOpen(DIAG_VAR_FILE_LIST[pCtrl->diag_band][0], O_WRONLY, 0);
		if (IS_FILE_OPEN_ERR(fd)) {
			fd = RtmpOSFileOpen(DIAG_VAR_FILE_LIST[pCtrl->diag_band][0],
			O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
			if (IS_FILE_OPEN_ERR(fd)) {
				RtmpOSFSInfoChange(&osFSInfo, FALSE);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "open conn_error_file fail dbdc_idx=%d\n", dbdc_idx);
				continue;
			}
			pCtrl->assoc_error_file_offset = 0;
		}

		RtmpOSFileSeek(fd, pCtrl->assoc_error_file_offset);

		while (pCtrl->assoc_error_write_idx != pCtrl->assoc_error_read_idx) {
			log_entry = &pCtrl->assoc_error_entry[pCtrl->assoc_error_read_idx];
			snprintf(buf + strlen(buf),
				buf_size -  strlen(buf),
				"%04d-%02d-%02d %02d:%02d:%02d",
				log_entry->tm.tm_year + 1900,
				log_entry->tm.tm_mon + 1,
				log_entry->tm.tm_mday,
				log_entry->tm.tm_hour,
				log_entry->tm.tm_min,
				log_entry->tm.tm_sec);

			snprintf(buf + strlen(buf),
				buf_size -  strlen(buf),
				MACSTR" %s: %s",
				MAC2STR(log_entry->StaAddr),
				log_entry->Ssid,
				DIAG_CONN_ERR_INFO[log_entry->errCode]);

			if (log_entry->errCode == DIAG_CONN_DEAUTH ||
				log_entry->errCode == DIAG_CONN_FRAME_LOST ||
				log_entry->errCode == DIAG_CONN_AUTH_FAIL)
				snprintf(buf + strlen(buf),
					buf_size -  strlen(buf),
					"%d\n", log_entry->reason);

			write_size = strlen(buf);
			if ((pCtrl->assoc_error_file_offset + write_size) > DIAG_LOG_FILE_SIZE_MAX) {
				pCtrl->assoc_error_file_offset = 0; /* overwrite from file start */
				RtmpOSFileSeek(fd, 0);
			}
			RtmpOSFileWrite(fd, buf, write_size);
			pCtrl->assoc_error_file_offset += write_size;

			memset(buf, 0, buf_size);
			memset(log_entry, 0, sizeof(DIAG_ASSOC_ERROR_ENTRY));

			pCtrl->assoc_error_read_idx =
				(pCtrl->assoc_error_read_idx + 1) % DIAG_ASSOC_ERROR_BUFF_SIZE;
		}

		RtmpOSFileClose(fd);
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}
}

void diag_log_file_write(PRTMP_ADAPTER pAd)
{
	RTMP_OS_FD fd = NULL;
	RTMP_OS_FS_INFO osFSInfo;
	UCHAR *buf = NULL;
	UINT32 buf_size = 0;
	DIAG_LOG_ENTRY *log_entry;
	UINT32 write_size;
	UCHAR *pData = NULL;
	UINT32 dataLen = 0, index = 0;
	DIAG_CTRL *pCtrl = NULL;
	UCHAR dbdc_idx = 0;

	for (dbdc_idx = 0; dbdc_idx < DBDC_BAND_NUM; dbdc_idx++) {
		pCtrl = pAd->pDiagCtrl[dbdc_idx];
		if ((!pCtrl) || (!pCtrl->init_flag)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "diag not init dbdc_idx=%d\n", dbdc_idx);
			continue;
		}

		if (pCtrl->diag_enable == 0) {
			/*MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "diag_log not enable dbdc_idx=%d\n", dbdc_idx); */
			continue;
		}

		buf = pCtrl->diag_log_tmp_buf;
		buf_size = 2*DIAG_FRAME_SIZE_MAX + 128;
		memset(buf, 0, buf_size);

		RtmpOSFSInfoChange(&osFSInfo, TRUE);
		fd = RtmpOSFileOpen(DIAG_VAR_FILE_LIST[pCtrl->diag_band][1], O_WRONLY, 0);
		if (IS_FILE_OPEN_ERR(fd)) {
			fd = RtmpOSFileOpen(DIAG_VAR_FILE_LIST[pCtrl->diag_band][1],
			O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
			if (IS_FILE_OPEN_ERR(fd)) {
				RtmpOSFSInfoChange(&osFSInfo, FALSE);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "open diag_log_file fail dbdc_idx=%d\n", dbdc_idx);
				continue;
			}
			pCtrl->diag_log_file_offset = 0;
		}

		RtmpOSFileSeek(fd, pCtrl->diag_log_file_offset);

		while (pCtrl->diag_log_write_idx != pCtrl->diag_log_read_idx) {

			if (pCtrl->diag_enable == 0)
				break;

			log_entry = &pCtrl->diag_log_entry[pCtrl->diag_log_read_idx];

			/* pData include 80211 header, because CNTL frame just include FCS except the 80211 header */
			pData = log_entry->data;
			dataLen = log_entry->data_len;

			/* time */
			if (strlen(buf) < (buf_size - 1))
				snprintf(buf + strlen(buf),
				buf_size -  strlen(buf),
				"%04d-%02d-%02d %02d:%02d:%02d ",
				log_entry->tm.tm_year + 1900,
				log_entry->tm.tm_mon + 1,
				log_entry->tm.tm_mday,
				log_entry->tm.tm_hour,
				log_entry->tm.tm_min,
				log_entry->tm.tm_sec);

			/* SSID */
			if (strlen(buf) < (buf_size - 1))
				snprintf(buf + strlen(buf), buf_size -  strlen(buf), log_entry->ssid);

			/* sends/receives */
			if (strlen(buf) < (buf_size - 1))
				snprintf(buf + strlen(buf),
					buf_size -  strlen(buf), (log_entry->isTX) ?  " sends " : " receives ");

			/* frame type */
			if (strlen(buf) < (buf_size - 1))
				snprintf(buf + strlen(buf), buf_size -  strlen(buf), log_entry->frame_type);

			if (strlen(buf) < (buf_size - 1))
				snprintf(buf + strlen(buf), buf_size -  strlen(buf),
				(log_entry->isTX) ? " to " : " from ");

			/* STA MAC address */
			if (strlen(buf) < (buf_size - 1)) {
				if (!MAC_ADDR_EQUAL(log_entry->sta_addr, MAC_ADDR_ZERO))
					snprintf(buf + strlen(buf),
						buf_size -  strlen(buf),
						MACSTR,
						MAC2STR(log_entry->sta_addr));
				else
					snprintf(buf + strlen(buf),
						buf_size -  strlen(buf),
						"%s", "No-STA-Addr");
			}

			/* frame body */
			if (strlen(buf) < (buf_size - 1))
				snprintf(buf + strlen(buf), buf_size -  strlen(buf), "[");

			for (index = 0; index < dataLen; index++) {
				if (strlen(buf) < (buf_size - 1))
					snprintf(buf + strlen(buf), buf_size -  strlen(buf), "%02X", pData[index]);
			}

			if (strlen(buf) < (buf_size - 1))
				snprintf(buf + strlen(buf), buf_size -  strlen(buf), "]\n");

			write_size = strlen(buf);
			if ((pCtrl->diag_log_file_offset + write_size) > DIAG_LOG_FILE_SIZE_MAX) {
				pCtrl->diag_log_file_offset = 0; /* overwrite from file start */
				RtmpOSFileSeek(fd, 0);
			}
			RtmpOSFileWrite(fd, buf, write_size);
			pCtrl->diag_log_file_offset += write_size;

			memset(buf, 0, buf_size);
			memset(log_entry, 0, sizeof(DIAG_LOG_ENTRY));

			/* update read_idx */
			pCtrl->diag_log_read_idx =
				(pCtrl->diag_log_read_idx + 1) % DIAG_LOG_BUFF_SIZE;
		}

		RtmpOSFileClose(fd);
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}
}

void diag_miniport_mm_request(PRTMP_ADAPTER pAd, UCHAR *pData, UINT Length)
{
	PHEADER_802_11 pHdr = NULL;
	BSS_STRUCT *pMbssEntry = NULL;
	MAC_TABLE_ENTRY *pMacTblEntry = NULL;
	DIAG_FRAME_INFO diag_info;
	DIAG_CTRL *pCtrl = NULL;
	UCHAR dbdc_idx;
	ENUM_DIAG_BAND diag_band;

	if (!pAd || !pData || (Length == 0))
		return;

	memset(&diag_info, 0, sizeof(diag_info));
	pHdr = (PHEADER_802_11) pData;
	pMacTblEntry = MacTableLookup(pAd, pHdr->Addr1);

	if (IS_VALID_ENTRY(pMacTblEntry) && IS_ENTRY_CLIENT(pMacTblEntry) &&
		VALID_MBSS(pAd, pMacTblEntry->func_tb_idx) &&
		(pMacTblEntry->func_tb_idx < pAd->ApCfg.BssidNum))
		pMbssEntry = &pAd->ApCfg.MBSSID[pMacTblEntry->func_tb_idx];
	else {
		UCHAR apidx, first_up_idx = 0xFF;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
				(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev))) {
				if (RTMPEqualMemory(pAd->ApCfg.MBSSID[apidx].wdev.bssid,
					pHdr->Addr2, MAC_ADDR_LEN)) {
					pMbssEntry = &pAd->ApCfg.MBSSID[apidx];
					break;
				}
				if (first_up_idx == 0xFF)
					first_up_idx = apidx;
			}
		}
		if ((pMbssEntry == NULL) && (first_up_idx != 0xFF))
			pMbssEntry = &pAd->ApCfg.MBSSID[first_up_idx];
	}
	if (pMbssEntry) {
		if (WMODE_CAP_5G(pMbssEntry->wdev.PhyMode))
			diag_band = DIAG_BAND_5G;
		else
			diag_band = DIAG_BAND_2G;

		dbdc_idx = HcGetBandByWdev(&pMbssEntry->wdev);

		pCtrl = pAd->pDiagCtrl[dbdc_idx];
		if ((!pCtrl) || (!pCtrl->init_flag)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl not init, dbdc_idx=%d\n", dbdc_idx);
			return;
		}
		if (pCtrl->diag_enable == 0) {
			/*MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "diag_enable is 0, dbdc_idx=%d\n", dbdc_idx);*/
			return;
		}

		diag_info.ssid = pMbssEntry->Ssid;
		diag_info.ssid_len = pMbssEntry->SsidLen;
		diag_info.isTX = 1;
		diag_info.band = diag_band;
		diag_info.pData = pData;
		diag_info.dataLen = Length;
		diag_frame_cache(&diag_info, pCtrl);
	} else
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error, no UP ap interface!!!\n");
}

void diag_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	DIAG_FRAME_INFO diag_info;
	MAC_TABLE_ENTRY *pEntry = NULL;
	BSS_STRUCT *pMbss = NULL;
	FRAME_CONTROL *FC = NULL;
	DIAG_CTRL *pCtrl = NULL;
	UCHAR dbdc_idx;
	ENUM_DIAG_BAND diag_band;

	if (!pRxBlk || !pAd)
		return;

	/* frame type & SubType check */
	FC = (FRAME_CONTROL *)pRxBlk->FC;
	if ((FC->Type != FC_TYPE_MGMT) || (
		(FC->SubType != SUBTYPE_BEACON) &&
		(FC->SubType != SUBTYPE_PROBE_REQ) &&
		(FC->SubType != SUBTYPE_PROBE_RSP) &&
		(FC->SubType != SUBTYPE_ATIM) &&
		(FC->SubType != SUBTYPE_DISASSOC) &&
		(FC->SubType != SUBTYPE_DEAUTH) &&
		(FC->SubType != SUBTYPE_ASSOC_REQ) &&
		(FC->SubType != SUBTYPE_ASSOC_RSP) &&
		(FC->SubType != SUBTYPE_REASSOC_REQ) &&
		(FC->SubType != SUBTYPE_REASSOC_RSP) &&
		(FC->SubType != SUBTYPE_AUTH))) {
		return;
	}

	if (IS_WCID_VALID(pAd, pRxBlk->wcid))
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	else
		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

	if ((IS_VALID_ENTRY(pEntry)) && (IS_ENTRY_CLIENT(pEntry))
		&& VALID_MBSS(pAd, pEntry->func_tb_idx)
		&& (pEntry->func_tb_idx < pAd->ApCfg.BssidNum))
		pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
	else {
		UCHAR apidx, first_up_idx = 0xFF;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
				(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev))) {
				if (RTMPEqualMemory(pAd->ApCfg.MBSSID[apidx].wdev.bssid,
					pRxBlk->Addr1, MAC_ADDR_LEN)) {
					pMbss = &pAd->ApCfg.MBSSID[apidx];
					break;
				}
				if (first_up_idx == 0xFF)
					first_up_idx = apidx;
			}
		}
		if ((pMbss == NULL) && (first_up_idx != 0xFF))
			pMbss = &pAd->ApCfg.MBSSID[first_up_idx];
	}
	if (pMbss) {

		if (WMODE_CAP_5G(pMbss->wdev.PhyMode))
			diag_band = DIAG_BAND_5G;
		else
			diag_band = DIAG_BAND_2G;

		dbdc_idx = HcGetBandByWdev(&pMbss->wdev);

		pCtrl = pAd->pDiagCtrl[dbdc_idx];
		if ((!pCtrl) || (!pCtrl->init_flag)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl not init, dbdc_idx=%d\n", dbdc_idx);
			return;
		}
		if (pCtrl->diag_enable == 0) {
			/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "diag_enable is 0, dbdc_idx=%d\n", dbdc_idx); */
			return;
		}

		memset(&diag_info, 0, sizeof(diag_info));
		diag_info.isTX = 0;
		diag_info.band = diag_band;
		diag_info.pData = pRxBlk->pData;
		diag_info.dataLen = pRxBlk->DataSize;

		diag_info.ssid = pMbss->Ssid;
		diag_info.ssid_len = pMbss->SsidLen;
		diag_frame_cache(&diag_info, pCtrl);
	} else
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error, no UP ap interface!!!\n");
}

void diag_dev_rx_cntl_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	DIAG_FRAME_INFO diag_info;
	MAC_TABLE_ENTRY *pEntry = NULL;
	BSS_STRUCT *pMbss = NULL;
	FRAME_CONTROL *FC = NULL;
	DIAG_CTRL *pCtrl = NULL;
	UCHAR dbdc_idx;
	ENUM_DIAG_BAND diag_band;


	if (!pAd || !pRxBlk)
		return;

	FC = (FRAME_CONTROL *)pRxBlk->FC;
	/*only BAR*/

	if (IS_WCID_VALID(pAd, pRxBlk->wcid))
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	else
		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

	if ((IS_VALID_ENTRY(pEntry)) && (IS_ENTRY_CLIENT(pEntry))
		&& VALID_MBSS(pAd, pEntry->func_tb_idx)
		&& (pEntry->func_tb_idx < pAd->ApCfg.BssidNum))
		pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
	else {
		UCHAR apidx, first_up_idx = 0xFF;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
				(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev))) {
				if (RTMPEqualMemory(pAd->ApCfg.MBSSID[apidx].wdev.bssid,
					pRxBlk->Addr1, MAC_ADDR_LEN)) {
					pMbss = &pAd->ApCfg.MBSSID[apidx];
					break;
				}
				if (first_up_idx == 0xFF)
					first_up_idx = apidx;
			}
		}
		if ((pMbss == NULL) && (first_up_idx != 0xFF))
			pMbss = &pAd->ApCfg.MBSSID[first_up_idx];
	}

	if (pMbss) {

		if (pMbss->wdev.channel > 14)
			diag_band = DIAG_BAND_5G;
		else
			diag_band = DIAG_BAND_2G;

#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode == TRUE)
			dbdc_idx = HcGetBandByWdev(&pMbss->wdev);
		else
			dbdc_idx = 0;
#else
		dbdc_idx = 0;
#endif
		pCtrl = pAd->pDiagCtrl[dbdc_idx];

	if ((!pCtrl) || (!pCtrl->init_flag)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl not init, dbdc_idx=%d\n", dbdc_idx);
		return;
	}
	if (pCtrl->diag_enable == 0) {
		/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "diag_enable is 0, dbdc_idx=%d\n", dbdc_idx); */
		return;
	}

	memset(&diag_info, 0, sizeof(diag_info));
	diag_info.isTX = 0;
	diag_info.band = diag_band;
	diag_info.pData = pRxBlk->pData;
	diag_info.dataLen = pRxBlk->DataSize;

	diag_info.ssid = pMbss->Ssid;
	diag_info.ssid_len = pMbss->SsidLen;
	diag_frame_cache(&diag_info, pCtrl);
	} else
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Error, no UP ap interface!!!\n");


}


void diag_bcn_tx(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, UCHAR *pBeaconFrame, ULONG FrameLen)
{
	DIAG_FRAME_INFO diag_info;
	DIAG_CTRL *pCtrl = NULL;
	UCHAR dbdc_idx;
	ENUM_DIAG_BAND diag_band;

	if (!pAd || !pMbss || !pBeaconFrame || (FrameLen == 0))
		return;

	if (WMODE_CAP_5G(pMbss->wdev.PhyMode))
		diag_band = DIAG_BAND_5G;
	else
		diag_band = DIAG_BAND_2G;

	dbdc_idx = HcGetBandByWdev(&pMbss->wdev);

	pCtrl = pAd->pDiagCtrl[dbdc_idx];
	if ((!pCtrl) || (!pCtrl->init_flag)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCtrl not init, dbdc_idx=%d\n", dbdc_idx);
		return;
	}
	if (pCtrl->diag_enable == 0) {
		/*MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "diag_enable is 0, dbdc_idx=%d\n", dbdc_idx);*/
		return;
	}

	memset(&diag_info, 0, sizeof(diag_info));
	diag_info.isTX = 1;
	diag_info.band = diag_band;
	diag_info.ssid = pMbss->Ssid;
	diag_info.ssid_len = pMbss->SsidLen;
	diag_info.pData = pBeaconFrame;
	diag_info.dataLen = FrameLen;
	diag_frame_cache(&diag_info, pCtrl);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!\n");
}

void diag_ap_mlme_one_sec_proc(PRTMP_ADAPTER pAd)
{
	DIAG_CTRL *pCtrl = NULL;
	UCHAR dbdc_idx;

	for (dbdc_idx = 0; dbdc_idx < DBDC_BAND_NUM; dbdc_idx++) {
		pCtrl = pAd->pDiagCtrl[dbdc_idx];
		if ((!pCtrl) || (!pCtrl->init_flag))
			continue;

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
			pCtrl->busy_time = 0;
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, "SCAN conflict with Diag channel Occupancy\n");
		} else {
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
			if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd))
				pCtrl->busy_time =
					((UINT32)pAd->WlanCounters[dbdc_idx].CcaNavTxTime.QuadPart) >> 10;
#else
			pCtrl->busy_time = (pAd->OneSecMibBucket.ChannelBusyTime[dbdc_idx]) >> 10;
#endif
		}

		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "update band:%d busy_time:%d ms by OneSecMibBucket\n", pCtrl->diag_band, pCtrl->busy_time);
	}

	/* For diag_log/association_error file write in MLME task */
	MlmeEnqueue(pAd, WIFI_DAIG_STATE_MACHINE, 0, 0, NULL, 0);
}

BOOLEAN diag_proc_init(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	DIAG_CTRL *pCtrl = NULL;
	UCHAR dbdc_idx = 0;
	ENUM_DIAG_BAND diag_band;
	UCHAR index;

	if (!pAd || !wdev) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "NULL parameters\n");
		return FALSE;
	}

	if (WMODE_CAP_5G(wdev->PhyMode))
		diag_band = DIAG_BAND_5G;
	else
		diag_band = DIAG_BAND_2G;

	dbdc_idx = HcGetBandByWdev(wdev);

	pCtrl = pAd->pDiagCtrl[dbdc_idx];
	if (!pCtrl) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pDiagCtrl not allocate dbdc_idx=%d\n", dbdc_idx);
		return FALSE;
	}
	if (pCtrl->init_flag == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pDiagCtrl already init dbdc_idx=%d\n", dbdc_idx);
		return FALSE;
	}

	memset(pCtrl, 0, sizeof(DIAG_CTRL));
	pCtrl->pAd = pAd;
	pCtrl->diag_band = diag_band;
	pCtrl->channel = wdev->channel;
	pCtrl->diag_duration = 60;
	for (index = 0; index < DIAG_PROCESS_NUM_MAX; index++)
		pCtrl->process_info.entry[index].pid = -1;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "diag for diag_band %d channel:%d\n", pCtrl->diag_band, wdev->channel);
	NdisAllocateSpinLock(pAd, &(pCtrl->diag_log_lock));
	NdisAllocateSpinLock(pAd, &(pCtrl->assoc_error_lock));
	diag_var_dir_mk(pCtrl->diag_band);
	diag_proc_dir_mk(pCtrl->diag_band);
	diag_proc_entry_init(pCtrl->diag_band, pCtrl);

	pCtrl->init_flag = TRUE;

	return TRUE;

}

BOOLEAN diag_proc_exit(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	DIAG_CTRL *pCtrl = NULL;
	UCHAR dbdc_idx = 0;

	if (!pAd || !wdev)
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pAd:%p\n", pAd);

	dbdc_idx = HcGetBandByWdev(wdev);

	pCtrl = pAd->pDiagCtrl[dbdc_idx];
	if (!pCtrl) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pDiagCtrl is NULL dbdc_idx=%d\n", dbdc_idx);
		return FALSE;
	}
	if (pCtrl->init_flag == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pDiagCtrl already deinit dbdc_idx=%d\n", dbdc_idx);
		return FALSE;
	}

	pCtrl->init_flag = FALSE;
	/* stop diag_log handling */
	pCtrl->diag_enable = 0;
	OS_WAIT(500);

	diag_proc_entry_deinit(pCtrl->diag_band);
	NdisFreeSpinLock(&pCtrl->diag_log_lock);
	NdisFreeSpinLock(&pCtrl->assoc_error_lock);

	diag_proc_dir_rm(pCtrl->diag_band);

	return TRUE;
}

#endif


