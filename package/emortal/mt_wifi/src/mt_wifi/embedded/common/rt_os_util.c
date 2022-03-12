/****************************************************************************

    Module Name:
    rt_os_util.c

    Abstract:
	All functions provided from UTIL module are put here (OS independent).

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------

***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rtmp_osabl.h"
#include "oid.h"
#include "oid_struct.h"
#include "rtmp_iface.h"


extern INT32 getLegacyOFDMMCSIndex(UINT8 MCS);

UINT32 RalinkRate[256] = {
	2,  4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108, 109, 110, 111, 112,/* CCK and OFDM */
	13, 26,   39,  52,  78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260,
	39, 78,  117, 156, 234, 312, 351, 390, /* BW 20, 800ns GI, MCS 0~23 */
	27, 54,   81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
	81, 162, 243, 324, 486, 648, 729, 810, /* BW 40, 800ns GI, MCS 0~23 */
	14, 29,   43,  57,  87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288,
	43, 87,  130, 173, 260, 317, 390, 433, /* BW 20, 400ns GI, MCS 0~23 */
	30, 60,   90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
	90, 180, 270, 360, 540, 720, 810, 900, /* BW 40, 400ns GI, MCS 0~23 */
	13, 26,   39,  52,  78, 104, 117, 130, 156, /* 11ac: 20Mhz, 800ns GI, MCS: 0~8 */
	27, 54,   81, 108, 162, 216, 243, 270, 324, 360, /*11ac: 40Mhz, 800ns GI, MCS: 0~9 */
	59, 117, 176, 234, 351, 468, 527, 585, 702, 780, /*11ac: 80Mhz, 800ns GI, MCS: 0~9 */
	14, 29,   43,  57,  87, 115, 130, 144, 173, /* 11ac: 20Mhz, 400ns GI, MCS: 0~8 */
	30, 60,   90, 120, 180, 240, 270, 300, 360, 400, /*11ac: 40Mhz, 400ns GI, MCS: 0~9 */
	65, 130, 195, 260, 390, 520, 585, 650, 780, 867, /*11ac: 80Mhz, 400ns GI, MCS: 0~9 */
	59 * 2, 117 * 2, 176 * 2, 234 * 2, 351 * 2, 468 * 2, 527 * 2, 585 * 2, 702 * 2, 780 * 2, /*11ac: 160Mhz, 800ns GI, MCS: 0~9 */
	65 * 2, 130 * 2, 195 * 2, 260 * 2, 390 * 2, 520 * 2, 585 * 2, 650 * 2, 780 * 2, 867 * 2, /*11ac: 160Mhz, 400ns GI, MCS: 0~9 */
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37
}; /* 3*3 */

VOID RtmpDrvMaxRateGet(
	IN	VOID					*pReserved,
	IN	UINT8					MODE,
	IN	UINT8					ShortGI,
	IN	UINT8					BW,
	IN	UINT8					MCS,
	IN	UINT8					Antenna,
	OUT	UINT32					*pRate)
{
	int rate_index = 0;
#ifdef DOT11_VHT_AC

	if (MODE >= MODE_VHT) {
		if (BW == 0/*20Mhz*/) {
			rate_index = 112 +
						 ((UCHAR)ShortGI * 29) +
						 ((UCHAR)MCS);
		} else if (BW == 1/*40Mhz*/) {
			rate_index = 121 +
						 ((UCHAR)ShortGI * 29) +
						 ((UCHAR)MCS);
		} else if (BW == 2/*80Mhz*/) {
			rate_index = 131 +
						 ((UCHAR)ShortGI * 29) +
						 ((UCHAR)MCS);
		} else if (BW > 2/*160Mhz*/) {
			rate_index = (131 + 29 + 10) +
						 ((UCHAR)ShortGI * 10) +
						 ((UCHAR)MCS);
		}
	} else
#endif /* DOT11_VHT_AC */
#ifdef DOT11_N_SUPPORT
		if ((MODE >= MODE_HTMIX) && (MODE < MODE_VHT)) {
			/*           	rate_index = 16 + ((UCHAR)pHtPhyMode->field.BW *16) + ((UCHAR)pHtPhyMode->field.ShortGI *32) + ((UCHAR)pHtPhyMode->field.MCS);
			*/
			/* map back to 1SS MCS , multiply by antenna numbers later */
			if (MCS > 7)
				MCS %= 8;

			rate_index = 16 + ((UCHAR)BW * 24) + ((UCHAR)ShortGI * 48) + ((UCHAR)MCS);
		} else
#endif /* DOT11_N_SUPPORT */
			if (MODE == MODE_OFDM)
				rate_index = getLegacyOFDMMCSIndex(MCS) + 4;
			else
				rate_index = (UCHAR)(MCS);

	if (rate_index < 0)
		rate_index = 0;

	if (rate_index > 255)
		rate_index = 255;

	*pRate = RalinkRate[rate_index] * 500000;
#if defined(DOT11_VHT_AC) || defined(DOT11_N_SUPPORT)

	if (MODE >= MODE_HTMIX)
		*pRate *= Antenna;

#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
	if (MODE >= MODE_HE) {
		/*ax tx */
		ULONG DataRate = 0;

		get_rate_he(MCS, BW, Antenna, 0, &DataRate);
		if (ShortGI == 1)
			DataRate = (DataRate * 967) >> 10;
		if (ShortGI == 2)
			DataRate = (DataRate * 870) >> 10;
		*pRate = (UINT32)DataRate;
		*pRate = *pRate * 1000000;
	}
#endif /*DOT11_HE_AX*/

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s - MODE: %d shortGI: %d BW: %d MCS: %d Antenna num: %d  Rate = %u\n"
			 , __func__, MODE, ShortGI, BW, MCS, Antenna, *pRate));
}


char *rtstrchr(char *s, int c)
{
	for (; *s != (char) c; ++s)
		if (*s == '\0')
			return NULL;

	return (char *) s;
}


VOID RtmpMeshDown(
	IN VOID *pDrvCtrlBK,
	IN BOOLEAN WaitFlag,
	IN BOOLEAN(*RtmpMeshLinkCheck)(IN VOID *pAd))
{
}


#ifdef ETH_CONVERT_SUPPORT
USHORT RtmpOsNetPrivGet(
	IN PNET_DEV pDev)
{
	return RT_DEV_PRIV_FLAGS_GET(pDev);
}
#endif /* ETH_CONVERT_SUPPORT */


BOOLEAN RtmpOsCmdDisplayLenCheck(
	IN UINT32 LenSrc,
	IN UINT32 Offset)
{
	if (LenSrc > (IW_PRIV_SIZE_MASK - Offset))
		return FALSE;

	return TRUE;
}


#ifdef WPA_SUPPLICANT_SUPPORT
VOID WpaSendMicFailureToWpaSupplicant(
	IN PNET_DEV pNetDev,
	IN const PUCHAR src_addr,
	IN BOOLEAN bUnicast,
	IN INT key_id,
	IN const PUCHAR tsc)
{
#ifdef RT_CFG80211_SUPPORT
	CFG80211OS_MICFailReport(pNetDev, src_addr, bUnicast, key_id, tsc);
#else
	char custom[IW_CUSTOM_MAX] = {0};
	snprintf(custom, sizeof(custom), "MLME-MICHAELMICFAILURE.indication");

	if (bUnicast)
		sprintf(custom, "%s unicast", custom);

	RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM, -1, NULL, (PUCHAR)custom, strlen(custom));
#endif /* RT_CFG80211_SUPPORT */
	return;
}
#endif /* WPA_SUPPLICANT_SUPPORT */


#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
int wext_notify_event_assoc(
	IN PNET_DEV pNetDev,
	IN UCHAR * ReqVarIEs,
	IN UINT32 ReqVarIELen)
{
	char custom[IW_CUSTOM_MAX] = {0};
#if WIRELESS_EXT > 17

	if (ReqVarIELen <= IW_CUSTOM_MAX) {
		NdisMoveMemory(custom, ReqVarIEs, ReqVarIELen);
		RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_ASSOC_REQ_IE, -1, NULL,
								(UCHAR *)custom, ReqVarIELen);
	} else
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->StaCfg[0].ReqVarIELen > MAX_CUSTOM_LEN\n"));

#else
	int len;
	len = (ReqVarIELen * 2) + 17;

	if (len <= IW_CUSTOM_MAX) {
		UCHAR   idx;
		snprintf(custom, sizeof(custom), "ASSOCINFO(ReqIEs=");

		for (idx = 0; idx < ReqVarIELen; idx++)
			sprintf(custom, "%s%02x", custom, ReqVarIEs[idx]);

		RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM, -1, NULL, custom, len);
	} else
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("len(%d) > MAX_CUSTOM_LEN\n", len));

#endif
	return 0;
}
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */


#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
VOID SendAssocIEsToWpaSupplicant(
	IN PNET_DEV pNetDev,
	IN UCHAR *ReqVarIEs,
	IN UINT32 ReqVarIELen)
{
	RTMP_STRING custom[IW_CUSTOM_MAX] = {0};

	if ((ReqVarIELen + 17) <= IW_CUSTOM_MAX) {
		snprintf(custom, sizeof(custom), "ASSOCINFO_ReqIEs=");
		NdisMoveMemory(custom + 17, ReqVarIEs, ReqVarIELen);
		RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM, RT_REQIE_EVENT_FLAG, NULL, (PUCHAR)custom, ReqVarIELen + 17);
		RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM, RT_ASSOCINFO_EVENT_FLAG, NULL, NULL, 0);
	} else
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->StaCfg[0].ReqVarIELen + 17 > MAX_CUSTOM_LEN\n"));

	return;
}
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */


INT32  RtPrivIoctlSetVal(VOID)
{
	return (INT32)RTPRIV_IOCTL_SET;
}

UINT32 MtRandom32(VOID)
{
#if (LINUX_VERSION_CODE  >= KERNEL_VERSION(3, 8, 0))
	return prandom_u32();
#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0) */
	return random32();
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0) */
}



/*Unify Utility APIs*/
RTMP_OS_FD_EXT os_file_open(
	CHAR * pPath,
	INT32  flag, /* CreateDisposition */
	INT32 file_mode)
{
	RTMP_OS_FD_EXT fd;
	os_zero_mem(&fd, sizeof(RTMP_OS_FD_EXT));
	fd.fsFd = RtmpOSFileOpen(pPath, flag, file_mode);
	fd.Status = IS_FILE_OPEN_ERR(fd.fsFd);

	if (!fd.Status) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
		fd.fsize = (ULONG)fd.fsFd->f_dentry->d_inode->i_size;
#else
		fd.fsize = (ULONG)fd.fsFd->f_path.dentry->d_inode->i_size;
#endif
		RtmpOSFSInfoChange(&fd.fsInfo, TRUE);
	}

	return fd;
}

INT os_file_close(
	RTMP_OS_FD_EXT osfd)
{
	INT ret;
	ret = RtmpOSFileClose(osfd.fsFd);

	if (!ret)
		RtmpOSFSInfoChange(&osfd.fsInfo, FALSE);

	return ret;
}


VOID os_file_seek(
	RTMP_OS_FD_EXT osfd,
	INT32 offset)
{
	RtmpOSFileSeek(osfd.fsFd, offset);
}

INT os_file_write(
	RTMP_OS_FD_EXT osfd,
	CHAR *pDataPtr,
	INT32 writeLen)
{
	return RtmpOSFileWrite(osfd.fsFd, pDataPtr, writeLen);
}

INT os_file_read(
	RTMP_OS_FD_EXT osfd,
	CHAR *pDataPtr,
	INT32 readLen)
{
	return RtmpOSFileRead(osfd.fsFd, pDataPtr, readLen);
}


VOID os_msec_delay(UINT msec)
{
	RtmpOsMsDelay(msec);
}

VOID os_usec_delay(UINT usec)
{
	RtmpusecDelay(usec);
}

#ifndef LINUX

static void os_kref_set(os_kref *kref, int num)
{
	RTMP_SPIN_LOCK_IRQ(&kref->lock);
	kref->refcount = num;
	RTMP_SPIN_UNLOCK_IRQ(&kref->lock);
}

void os_kref_init(os_kref *kref)
{
	NdisAllocateSpinLock(NULL, &kref->lock);
	os_kref_set(kref, 1);
}

void os_kref_get(os_kref *kref)
{
	RTMP_SPIN_LOCK_IRQ(&kref->lock);
	if (!kref->refcount)
		goto end;

	kref->refcount++;
end:
	RTMP_SPIN_UNLOCK_IRQ(&kref->lock);
}

int os_kref_put(os_kref *kref, void (*release)(os_kref *kref))
{
	int ret = 0;

	RTMP_SPIN_LOCK_IRQ(&kref->lock);
	if (!release)
		goto end;

	kref->refcount--;

	if (kref->refcount == 0) {
		release(kref);
		ret = 1;
	}
end:
	RTMP_SPIN_UNLOCK_IRQ(&kref->lock);
	return ret;
}
#endif

