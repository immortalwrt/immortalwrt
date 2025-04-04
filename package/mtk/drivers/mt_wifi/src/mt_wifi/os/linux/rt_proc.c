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
    rt_proc.c

    Abstract:
    Create and register proc file system for ralink device

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include "rt_config.h"

int wl_proc_init(void);
int wl_proc_exit(void);

#ifdef CONFIG_RALINK_RT2880
#define PROCREG_DIR             "rt2880"
#endif /* CONFIG_RALINK_RT2880 */

#ifdef CONFIG_RALINK_RT3052
#define PROCREG_DIR             "rt3052"
#endif /* CONFIG_RALINK_RT3052 */

#ifdef CONFIG_RALINK_RT2883
#define PROCREG_DIR             "rt2883"
#endif /* CONFIG_RALINK_RT2883 */

#ifdef CONFIG_RALINK_RT3883
#define PROCREG_DIR             "rt3883"
#endif /* CONFIG_RALINK_RT3883 */

#ifdef CONFIG_RALINK_RT5350
#define PROCREG_DIR             "rt5350"
#endif /* CONFIG_RALINK_RT5350 */

#ifndef PROCREG_DIR
#define PROCREG_DIR             "rt2880"
#endif /* PROCREG_DIR */

#ifdef CFG_SUPPORT_CSI
#define CSI_PROC_ROOT			"wlan"
#define PROC_CSI_DATA_NAME		"csi_data"
#define CSI_DATA_DIR_PATH		"/proc/net/wlan"

static ssize_t procCSIDataRead(struct file *filp,
	char __user *buf, size_t count, loff_t *f_pos);

static ssize_t procCSIDataPrepare(
	UINT_8 *buf,
	struct CSI_INFO_T *prCSIInfo,
	struct CSI_DATA_T *prCSIData);

static int procCSIDataOpen(struct inode *n, struct file *f);
static int procCSIDataRelease(struct inode *n, struct file *f);

static const struct file_operations csidata_ops = {
	.owner = THIS_MODULE,
	.read = procCSIDataRead,
	.open = procCSIDataOpen,
	.release = procCSIDataRelease,
};
#endif

#if defined(CONFIG_PROC_FS) && defined(VIDEO_TURBINE_SUPPORT)
extern struct proc_dir_entry *procRegDir;
extern BOOLEAN UpdateFromGlobal;
AP_VIDEO_STRUCT GLOBAL_AP_VIDEO_CONFIG;
/*struct proc_dir_entry *proc_ralink_platform, *proc_ralink_wl, *proc_ralink_wl_video; */
struct proc_dir_entry *proc_ralink_wl, *proc_ralink_wl_video;
static struct proc_dir_entry *entry_wl_video_Update, *entry_wl_video_Enable, *entry_wl_video_ClassifierEnable, *entry_wl_video_HighTxMode, *entry_wl_video_TxPwr, *entry_wl_video_VideoMCSEnable;
static struct proc_dir_entry *entry_wl_video_VideoMCS, *entry_wl_video_TxBASize, *entry_wl_video_TxLifeTimeMode, *entry_wl_video_TxLifeTime, *entry_wl_video_TxRetryLimit;


ssize_t video_Update_get(char *page, char **start, off_t off, int count,
						 int *eof, void *data_unused)
{
	sprintf(page, "%d\n", UpdateFromGlobal);
	*eof = 1;
	return strlen(page);
}

ssize_t video_Update_set(struct file *file, const char __user *buffer,
						 size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 10);

		UpdateFromGlobal = val;
	}

	return count;
}

ssize_t video_Enable_get(char *page, char **start, off_t off, int count,
						 int *eof, void *data_unused)
{
	sprintf(page, "%d\n", GLOBAL_AP_VIDEO_CONFIG.Enable);
	*eof = 1;
	return strlen(page);
}

ssize_t video_Enable_set(struct file *file, const char __user *buffer,
						 size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 10);

		GLOBAL_AP_VIDEO_CONFIG.Enable = val;
	}

	return count;
}

ssize_t video_ClassifierEnable_get(char *page, char **start, off_t off, int count,
								   int *eof, void *data_unused)
{
	sprintf(page, "%d\n", GLOBAL_AP_VIDEO_CONFIG.ClassifierEnable);
	*eof = 1;
	return strlen(page);
}

ssize_t video_ClassifierEnable_set(struct file *file, const char __user *buffer,
								   size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 10);

		GLOBAL_AP_VIDEO_CONFIG.ClassifierEnable = val;
	}

	return count;
}

ssize_t video_HighTxMode_get(char *page, char **start, off_t off, int count,
							 int *eof, void *data_unused)
{
	sprintf(page, "%d\n", GLOBAL_AP_VIDEO_CONFIG.HighTxMode);
	*eof = 1;
	return strlen(page);
}

ssize_t video_HighTxMode_set(struct file *file, const char __user *buffer,
							 size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 10);

		GLOBAL_AP_VIDEO_CONFIG.HighTxMode = val;
	}

	return count;
}

ssize_t video_TxPwr_get(char *page, char **start, off_t off, int count,
						int *eof, void *data_unused)
{
	sprintf(page, "%d\n", GLOBAL_AP_VIDEO_CONFIG.TxPwr);
	*eof = 1;
	return strlen(page);
}

ssize_t video_TxPwr_set(struct file *file, const char __user *buffer,
						size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 10);

		GLOBAL_AP_VIDEO_CONFIG.TxPwr = val;
	}

	return count;
}

ssize_t video_VideoMCSEnable_get(char *page, char **start, off_t off, int count,
								 int *eof, void *data_unused)
{
	sprintf(page, "%d\n", GLOBAL_AP_VIDEO_CONFIG.VideoMCSEnable);
	*eof = 1;
	return strlen(page);
}

ssize_t video_VideoMCSEnable_set(struct file *file, const char __user *buffer,
								 size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 10);

		GLOBAL_AP_VIDEO_CONFIG.VideoMCSEnable = val;
	}

	return count;
}

ssize_t video_VideoMCS_get(char *page, char **start, off_t off, int count,
						   int *eof, void *data_unused)
{
	sprintf(page, "%d\n", GLOBAL_AP_VIDEO_CONFIG.VideoMCS);
	*eof = 1;
	return strlen(page);
}

ssize_t video_VideoMCS_set(struct file *file, const char __user *buffer,
						   size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 10);

		GLOBAL_AP_VIDEO_CONFIG.VideoMCS = val;
	}

	return count;
}

ssize_t video_TxBASize_get(char *page, char **start, off_t off, int count,
						   int *eof, void *data_unused)
{
	sprintf(page, "%d\n", GLOBAL_AP_VIDEO_CONFIG.TxBASize);
	*eof = 1;
	return strlen(page);
}

ssize_t video_TxBASize_set(struct file *file, const char __user *buffer,
						   size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 10);

		GLOBAL_AP_VIDEO_CONFIG.TxBASize = val;
	}

	return count;
}

ssize_t video_TxLifeTimeMode_get(char *page, char **start, off_t off, int count,
								 int *eof, void *data_unused)
{
	sprintf(page, "%d\n", GLOBAL_AP_VIDEO_CONFIG.TxLifeTimeMode);
	*eof = 1;
	return strlen(page);
}

ssize_t video_TxLifeTimeMode_set(struct file *file, const char __user *buffer,
								 size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 10);

		GLOBAL_AP_VIDEO_CONFIG.TxLifeTimeMode = val;
	}

	return count;
}

ssize_t video_TxLifeTime_get(char *page, char **start, off_t off, int count,
							 int *eof, void *data_unused)
{
	sprintf(page, "%d\n", GLOBAL_AP_VIDEO_CONFIG.TxLifeTime);
	*eof = 1;
	return strlen(page);
}

ssize_t video_TxLifeTime_set(struct file *file, const char __user *buffer,
							 size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 10);

		GLOBAL_AP_VIDEO_CONFIG.TxLifeTime = val;
	}

	return count;
}

ssize_t video_TxRetryLimit_get(char *page, char **start, off_t off, int count,
							   int *eof, void *data_unused)
{
	sprintf(page, "0x%x\n", GLOBAL_AP_VIDEO_CONFIG.TxRetryLimit);
	*eof = 1;
	return strlen(page);
}

ssize_t video_TxRetryLimit_set(struct file *file, const char __user *buffer,
							   size_t count, loff_t *ppos)
{
	char *buf;
	os_alloc_mem_suspend(NULL, (UCHAR **)&buf, count);

	if (buf) {
		unsigned long val;

		if (copy_from_user(buf, buffer, count))
			return -EFAULT;

		if (buf)
			val = os_str_toul(buf, NULL, 16);

		GLOBAL_AP_VIDEO_CONFIG.TxRetryLimit = val;
	}

	return count;
}

int wl_video_proc_init(void)
{
	GLOBAL_AP_VIDEO_CONFIG.Enable = FALSE;
	GLOBAL_AP_VIDEO_CONFIG.ClassifierEnable = FALSE;
	GLOBAL_AP_VIDEO_CONFIG.HighTxMode = FALSE;
	GLOBAL_AP_VIDEO_CONFIG.TxPwr = 0;
	GLOBAL_AP_VIDEO_CONFIG.VideoMCSEnable = FALSE;
	GLOBAL_AP_VIDEO_CONFIG.VideoMCS = 0;
	GLOBAL_AP_VIDEO_CONFIG.TxBASize = 0;
	GLOBAL_AP_VIDEO_CONFIG.TxLifeTimeMode = FALSE;
	GLOBAL_AP_VIDEO_CONFIG.TxLifeTime = 0;
	GLOBAL_AP_VIDEO_CONFIG.TxRetryLimit = 0;
	proc_ralink_wl = proc_mkdir("wl", procRegDir);

	if (proc_ralink_wl)
		proc_ralink_wl_video = proc_mkdir("VideoTurbine", proc_ralink_wl);

	if (proc_ralink_wl_video) {
		entry_wl_video_Update = create_proc_entry("UpdateFromGlobal", 0, proc_ralink_wl_video);

		if (entry_wl_video_Update) {
			entry_wl_video_Update->read_proc = (read_proc_t *)&video_Update_get;
			entry_wl_video_Update->write_proc = (write_proc_t *)&video_Update_set;
		}

		entry_wl_video_Enable = create_proc_entry("Enable", 0, proc_ralink_wl_video);

		if (entry_wl_video_Enable) {
			entry_wl_video_Enable->read_proc = (read_proc_t *)&video_Enable_get;
			entry_wl_video_Enable->write_proc = (write_proc_t *)&video_Enable_set;
		}

		entry_wl_video_ClassifierEnable = create_proc_entry("ClassifierEnable", 0, proc_ralink_wl_video);

		if (entry_wl_video_ClassifierEnable) {
			entry_wl_video_ClassifierEnable->read_proc = (read_proc_t *)&video_ClassifierEnable_get;
			entry_wl_video_ClassifierEnable->write_proc = (write_proc_t *)&video_ClassifierEnable_set;
		}

		entry_wl_video_HighTxMode = create_proc_entry("HighTxMode", 0, proc_ralink_wl_video);

		if (entry_wl_video_HighTxMode) {
			entry_wl_video_HighTxMode->read_proc = (read_proc_t *)&video_HighTxMode_get;
			entry_wl_video_HighTxMode->write_proc = (write_proc_t *)&video_HighTxMode_set;
		}

		entry_wl_video_TxPwr = create_proc_entry("TxPwr", 0, proc_ralink_wl_video);

		if (entry_wl_video_TxPwr) {
			entry_wl_video_TxPwr->read_proc = (read_proc_t *)&video_TxPwr_get;
			entry_wl_video_TxPwr->write_proc = (write_proc_t *)&video_TxPwr_set;
		}

		entry_wl_video_VideoMCSEnable = create_proc_entry("VideoMCSEnable", 0, proc_ralink_wl_video);

		if (entry_wl_video_VideoMCSEnable) {
			entry_wl_video_VideoMCSEnable->read_proc = (read_proc_t *)&video_VideoMCSEnable_get;
			entry_wl_video_VideoMCSEnable->write_proc = (write_proc_t *)&video_VideoMCSEnable_set;
		}

		entry_wl_video_VideoMCS = create_proc_entry("VideoMCS", 0, proc_ralink_wl_video);

		if (entry_wl_video_VideoMCS) {
			entry_wl_video_VideoMCS->read_proc = (read_proc_t *)&video_VideoMCS_get;
			entry_wl_video_VideoMCS->write_proc = (write_proc_t *)&video_VideoMCS_set;
		}

		entry_wl_video_TxBASize = create_proc_entry("TxBASize", 0, proc_ralink_wl_video);

		if (entry_wl_video_TxBASize) {
			entry_wl_video_TxBASize->read_proc = (read_proc_t *)&video_TxBASize_get;
			entry_wl_video_TxBASize->write_proc = (write_proc_t *)&video_TxBASize_set;
		}

		entry_wl_video_TxLifeTimeMode = create_proc_entry("TxLifeTimeMode", 0, proc_ralink_wl_video);

		if (entry_wl_video_TxLifeTimeMode) {
			entry_wl_video_TxLifeTimeMode->read_proc = (read_proc_t *)&video_TxLifeTimeMode_get;
			entry_wl_video_TxLifeTimeMode->write_proc = (write_proc_t *)&video_TxLifeTimeMode_set;
		}

		entry_wl_video_TxLifeTime = create_proc_entry("TxLifeTime", 0, proc_ralink_wl_video);

		if (entry_wl_video_TxLifeTime) {
			entry_wl_video_TxLifeTime->read_proc = (read_proc_t *)&video_TxLifeTime_get;
			entry_wl_video_TxLifeTime->write_proc = (write_proc_t *)&video_TxLifeTime_set;
		}

		entry_wl_video_TxRetryLimit = create_proc_entry("TxRetryLimit", 0, proc_ralink_wl_video);

		if (entry_wl_video_TxRetryLimit) {
			entry_wl_video_TxRetryLimit->read_proc = (read_proc_t *)&video_TxRetryLimit_get;
			entry_wl_video_TxRetryLimit->write_proc = (write_proc_t *)&video_TxRetryLimit_set;
		}
	}

	return 0;
}

int wl_video_proc_exit(void)
{
	if (entry_wl_video_Enable)
		remove_proc_entry("Enable", proc_ralink_wl_video);

	if (entry_wl_video_ClassifierEnable)
		remove_proc_entry("ClassifierEnabl", proc_ralink_wl_video);

	if (entry_wl_video_HighTxMode)
		remove_proc_entry("HighTxMode", proc_ralink_wl_video);

	if (entry_wl_video_TxPwr)
		remove_proc_entry("TxPwr", proc_ralink_wl_video);

	if (entry_wl_video_VideoMCSEnable)
		remove_proc_entry("VideoMCSEnable", proc_ralink_wl_video);

	if (entry_wl_video_VideoMCS)
		remove_proc_entry("VideoMCS", proc_ralink_wl_video);

	if (entry_wl_video_TxBASize)
		remove_proc_entry("TxBASize", proc_ralink_wl_video);

	if (entry_wl_video_TxLifeTimeMode)
		remove_proc_entry("TxLifeTimeMode", proc_ralink_wl_video);

	if (entry_wl_video_TxLifeTime)
		remove_proc_entry("TxLifeTime", proc_ralink_wl_video);

	if (entry_wl_video_TxRetryLimit)
		remove_proc_entry("TxRetryLimit", proc_ralink_wl_video);

	if (proc_ralink_wl_video)
		remove_proc_entry("Video", proc_ralink_wl);

	return 0;
}

int wl_proc_init(void)
{
	if (procRegDir == NULL)
		procRegDir = proc_mkdir(PROCREG_DIR, NULL);

	if (procRegDir) {
#ifdef VIDEO_TURBINE_SUPPORT
		wl_video_proc_init();
#endif /* VIDEO_TURBINE_SUPPORT */
	}

	return 0;
}

int wl_proc_exit(void)
{
#ifdef VIDEO_TURBINE_SUPPORT

	if (proc_ralink_wl_video) {
		wl_video_proc_exit();
		remove_proc_entry("Video", proc_ralink_wl);
	}

	if (proc_ralink_wl)
		remove_proc_entry("wl", procRegDir);

#endif /* VIDEO_TURBINE_SUPPORT */
	return 0;
}
#else
int wl_proc_init(void)
{
	return 0;
}

int wl_proc_exit(void)
{
	return 0;
}
#endif /* VIDEO_TURBINE_SUPPORT  && CONFIG_PROC_FS */

#ifdef CFG_SUPPORT_CSI
static struct proc_dir_entry *csi_proc_dir;
static INT8 csi_if_num;	/*csi proc interface counter*/
static int procCSIDataOpen(struct inode *n, struct file *f)
{
	struct CSI_INFO_T *prCSIInfo = NULL;
	RTMP_ADAPTER *pAd = NULL;
#if (KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE)
	pAd = (RTMP_ADAPTER *)(PDE_DATA(file_inode(f)));
#else
	pAd = (RTMP_ADAPTER *)(PDE(file_inode(f))->data);
#endif

	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"proc pointer get fail!!!\n");
		return -1;
	}

	prCSIInfo = &pAd->rCSIInfo;
	prCSIInfo->bIncomplete = FALSE;		/*no remain pkt bytes left*/

	return 0;
}

static int procCSIDataRelease(struct inode *n, struct file *f)
{
	struct CSI_INFO_T *prCSIInfo = NULL;
	RTMP_ADAPTER *pAd = NULL;
#if (KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE)
	pAd = (RTMP_ADAPTER *)(PDE_DATA(file_inode(f)));
#else
	pAd = (RTMP_ADAPTER *)(PDE(file_inode(f))->data);
#endif

	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"proc pointer get fail!!!\n");
		return -1;
	}

	prCSIInfo = &pAd->rCSIInfo;
	prCSIInfo->bIncomplete = FALSE;		/*no remain pkt bytes left*/

	return 0;
}
/*
*this function prepare a pkt for APP to use.
 *the format of pkt depends on some specific rules as follows.
*/
static ssize_t procCSIDataPrepare(
	UINT_8 *buf,
	struct CSI_INFO_T *prCSIInfo,
	struct CSI_DATA_T *prCSIData)
{

	INT_32 i4Pos = 0;
	UINT_8 *tmpBuf = buf;
	UINT_16 u2DataSize = prCSIData->u2DataCount * sizeof(INT_16);
	UINT_16 u2Rsvd1Size = prCSIData->ucRsvd1Cnt * sizeof(INT_32);
	enum ENUM_CSI_MODULATION_BW_TYPE_T eModulationType = 0;

	if (prCSIData->ucBw == 0)
		eModulationType = CSI_TYPE_OFDM_BW20;
	else if (prCSIData->ucBw == 1)
		eModulationType = CSI_TYPE_OFDM_BW40;
	else if (prCSIData->ucBw == 2)
		eModulationType = CSI_TYPE_OFDM_BW80;

	/* magic number */
	put_unaligned(0xAA, (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(0xBB, (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(0xCC, (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(0xDD, (tmpBuf + i4Pos));
	i4Pos++;

	/*Just bypass total length feild here and update it in the end*/
	i4Pos += 2;

	put_unaligned(CSI_DATA_VER, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->FWVer, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_TYPE, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(eModulationType, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_TS, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(4, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->u4TimeStamp, (UINT_32 *) (tmpBuf + i4Pos));
	i4Pos += 4;

	put_unaligned(CSI_DATA_RSSI, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->cRssi, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_SNR, (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucSNR, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_DBW, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucDataBw, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_CH_IDX, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucPrimaryChIdx, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_TA, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(MAC_ADDR_LEN, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	os_move_mem((tmpBuf + i4Pos), prCSIData->aucTA, MAC_ADDR_LEN);
	i4Pos += MAC_ADDR_LEN;

	put_unaligned(CSI_DATA_EXTRA_INFO, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(4, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->u4ExtraInfo, (UINT_32 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_32);

	put_unaligned(CSI_DATA_I, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(u2DataSize, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	os_move_mem((tmpBuf + i4Pos), prCSIData->ac2IData, u2DataSize);
	i4Pos += u2DataSize;

	put_unaligned(CSI_DATA_Q, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(u2DataSize, (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	os_move_mem((tmpBuf + i4Pos), prCSIData->ac2QData, u2DataSize);
	i4Pos += u2DataSize;

	if (prCSIInfo->ucValue1[CSI_CONFIG_INFO] & CSI_INFO_RSVD1) {
		put_unaligned(CSI_DATA_RSVD1, (UINT_8 *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(u2Rsvd1Size, (UINT_16 *) (tmpBuf + i4Pos));
		i4Pos += 2;
		os_move_mem((tmpBuf + i4Pos), prCSIData->ai4Rsvd1, u2Rsvd1Size);
		i4Pos += u2Rsvd1Size;

		put_unaligned(CSI_DATA_RSVD2, (UINT_8 *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(u2Rsvd1Size, (UINT_16 *) (tmpBuf + i4Pos));
		i4Pos += 2;
		os_move_mem((tmpBuf + i4Pos), prCSIData->au4Rsvd2, u2Rsvd1Size);
		i4Pos += u2Rsvd1Size;

		put_unaligned(CSI_DATA_RSVD3, (UINT_8 *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(sizeof(INT_32), (INT_16 *) (tmpBuf + i4Pos));
		i4Pos += 2;
		put_unaligned(prCSIData->i4Rsvd3, (INT_32 *) (tmpBuf + i4Pos));
		i4Pos += sizeof(INT_32);
	}

	if (prCSIInfo->ucValue1[CSI_CONFIG_INFO] & CSI_INFO_RSVD2) {
		put_unaligned(CSI_DATA_RSVD4, (UINT_8 *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(sizeof(UINT_8), (INT_16 *) (tmpBuf + i4Pos));
		i4Pos += 2;
		put_unaligned(prCSIData->ucRsvd4, (UINT_8 *) (tmpBuf + i4Pos));
		i4Pos += sizeof(UINT_8);
	}

	put_unaligned(CSI_DATA_TX_IDX, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_16), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned((UINT_16)(((prCSIData->Tx_Rx_Idx)&0xffff0000) >> 16), (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_16);

	put_unaligned(CSI_DATA_RX_IDX, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_16), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned((UINT_16)((prCSIData->Tx_Rx_Idx)&0xffff), (UINT_16 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_16);

	put_unaligned(CSI_DATA_FRAME_MODE, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_8), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucRxMode, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_8);

	/* add antenna pattern*/
	put_unaligned(CSI_DATA_H_IDX, (UINT_8 *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(UINT_32), (INT_16 *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->Antenna_pattern, (UINT_32 *) (tmpBuf + i4Pos));
	i4Pos += sizeof(UINT_32);

	/*
	 * The lengths of magic number (4 byte) and total length (2 bytes)
	 * fields should not be counted in the total length value
	*/
	put_unaligned(i4Pos - 6, (UINT_16 *) (tmpBuf + 4));

	return i4Pos;
}

static ssize_t procCSIDataRead(struct file *filp,
	char __user *buf, size_t count, loff_t *f_pos)
{

	UINT_8 *temp = NULL;
	struct CSI_DATA_T *rTmpCSIData = NULL;
	UINT_32 u4CopySize = 0;
	UINT_32 u4StartIdx = 0;
	INT_32 i4Pos = 0;
	struct CSI_INFO_T *prCSIInfo = NULL;
	RTMP_ADAPTER *pAd = NULL;
#if (KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE)
	pAd = (RTMP_ADAPTER *)(PDE_DATA(file_inode(filp)));
#else
	pAd = (RTMP_ADAPTER *)(PDE(file_inode(filp))->data);
#endif

	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"proc pointer get fail!!!\n");
		return -1;
	}

	prCSIInfo = &pAd->rCSIInfo;
	temp = prCSIInfo->byte_stream;

	if (!temp) {
		MTWF_DBG(pAd, (DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"temp NULL pointer!!!\n");
		return -1;
	}
	/* alloc mem for CSIData */
	os_alloc_mem_suspend(NULL, (UCHAR **)&rTmpCSIData, sizeof(struct CSI_DATA_T));
	if (!rTmpCSIData) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"rTmpCSIData fail to alloc mem!\n");
		return -1;
	}

	if (prCSIInfo->bIncomplete == FALSE) {

		wait_event_interruptible(prCSIInfo->waitq,
			prCSIInfo->u4CSIBufferUsed != 0);
		/*
		*No older CSI data in buffer waiting for reading out, so prepare a new one
		*for reading.
		*/
		os_zero_mem(temp, Max_Stream_Bytes);
		if (wlanPopCSIData(pAd, rTmpCSIData)) {
			i4Pos = procCSIDataPrepare(temp,
				prCSIInfo, rTmpCSIData);
		}

		/*reading the CSI data from 0 byte*/
		u4StartIdx = 0;
		if (i4Pos > count) {
			u4CopySize = count;
			prCSIInfo->u4RemainingDataSize = i4Pos - count;
			prCSIInfo->u4CopiedDataSize = count;
			prCSIInfo->bIncomplete = TRUE;
		} else {
			u4CopySize = i4Pos;
		}
	} else {
		/*if there is one pkt left some bytes to read */
		/* Reading the remaining CSI data in the buffer*/
		u4StartIdx = prCSIInfo->u4CopiedDataSize;
		if (prCSIInfo->u4RemainingDataSize > count) {
			u4CopySize = count;
			prCSIInfo->u4RemainingDataSize -= count;
			prCSIInfo->u4CopiedDataSize += count;
		} else {
			u4CopySize = prCSIInfo->u4RemainingDataSize;
			prCSIInfo->bIncomplete = FALSE;
		}
	}

	if (copy_to_user(buf, &temp[u4StartIdx], u4CopySize)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy to user failed\n");
		if (rTmpCSIData)
			os_free_mem(rTmpCSIData);
		return -EFAULT;
	}

	*f_pos += u4CopySize;

	if (rTmpCSIData)
		os_free_mem(rTmpCSIData);

	return (ssize_t)u4CopySize;

}

int csi_proc_init(RTMP_ADAPTER *pAd)
{
	struct proc_dir_entry *prEntry;
	char csi_proc_name[64] = {0};

	if (!csi_proc_dir) {
		csi_proc_dir = proc_mkdir(CSI_PROC_ROOT, init_net.proc_net); /* proc /net /wlan */

		if (!csi_proc_dir) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "csi_proc_dir get error!\n");
			return -1;
		}
	}

	snprintf(csi_proc_name, sizeof(csi_proc_name), "%s_%d", PROC_CSI_DATA_NAME, get_dev_config_idx(pAd));
	/* proc /net /wlan /csi_data_0 */
	prEntry = proc_create_data(csi_proc_name, 0664, csi_proc_dir, &csidata_ops, pAd);

	if (!prEntry) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "prEntry is NULL!\n");
		return -1;
	}

	csi_if_num++;

	return 0;
}

int csi_proc_deinit(RTMP_ADAPTER *pAd)
{
	char Entry_name[64] = {0};

	if (!csi_proc_dir) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "csi_proc_dir is NULL!\n");
		return -1;
	}

	snprintf(Entry_name, sizeof(Entry_name), "%s_%d", PROC_CSI_DATA_NAME, get_dev_config_idx(pAd));
	/*delect  proc -net -wlan-csi_data*/
	remove_proc_entry(Entry_name, csi_proc_dir);

	csi_if_num--;

	if (csi_if_num == 0) {
		csi_proc_dir = NULL;
		remove_proc_entry(CSI_PROC_ROOT, init_net.proc_net);	/*delect dir: proc -net -wlan */
	} else if (csi_if_num < 0)
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"csi_if_num is error!(%d)\n", (INT32)csi_if_num);

	return 0;
}
#endif

