/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2020-2021  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */


#include "rt_config.h"

#ifdef QOS_R1

#define MSCS_DESC_IE_LEN	12
#define DSCP_TBL_SIZE		64

#define is_valid_dscp(dscp) (dscp < 64)
#define is_valid_up(up) (up < 8)

UCHAR dft_mscs_desc_ie[] = {
	IE_WLAN_EXTENSION,
	MSCS_DESC_IE_LEN,
	IE_EXTENSION_ID_MSCS_DESC,
	SCS_REQ_TYPE_ADD,
	0, 0, 0, 0, 0, 0,	/*UP Control, Stream TimeOut*/
	1, 2, 0, 0		/*Subelements*/
};

#ifdef QOS_R2
UCHAR wfa_capa_ie[] = {
	IE_VENDOR_SPECIFIC,
	6,
	0x50, 0x6F, 0x9A,
	0x23,
	1, 0	/*capability attribute*/
};

static UCHAR WFA_OUI[3] = {0x50, 0x6F, 0x9A};

#endif
#ifdef DSCP_PRI_SUPPORT
UCHAR dft_dscp2up_tbl[DSCP_TBL_SIZE] = {
	0,	0,	0,	0,	0,	0,	0,	0, /* 0 ~ 7  */
	1,	1,	0,	1,	0,	1,	0,	1, /* 8 ~ 15 */
	2,	0,	3,	0,	3,	0,	3,	0, /*16 ~ 23 */
	4,	4,	4,	4,	4,	4,	4,	4, /*24 ~ 31 */
	4,	4,	4,	4,	4,	4,	4,	4, /*32 ~ 39 */
	5,	5,	5,	5,	6,	5,	6,	5, /*40 ~ 47 */
	7,	0,	0,	0,	0,	0,	0,	0, /*48 ~ 55 */
	7,	0,	0,	0,	0,	0,	0,	0  /*56 ~ 63 */
};

void QoS_Init_DSCP2UP_Mapping(PRTMP_ADAPTER pAd)
{
	UINT8 i;

	for (i = 0; i < MAX_BEACON_NUM; i++)
		NdisCopyMemory(pAd->ApCfg.MBSSID[i].dscp_pri_map, dft_dscp2up_tbl, DSCP_TBL_SIZE);
}
#endif /*DSCP_PRI_SUPPORT*/

void QoS_MapIE_Config(PRTMP_ADAPTER pAd, UINT8 bssidx, char *IE, UINT32 IELen)
{
	char *pos = NULL;
	UINT8 i = 0, dscp = 0, up = 0, l = 0, h = 0, count = 0;
	BSS_STRUCT *pmbss = &pAd->ApCfg.MBSSID[bssidx];

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "-->%s()\n", __func__);

	pmbss->QoSMapIsUP = 0;
	pmbss->DscpExceptionCount = 0;
	memset(pmbss->DscpRange, 0xFF, sizeof(pmbss->DscpRange));
	memset(pmbss->DscpException, 0xFF, sizeof(pmbss->DscpException));
	/*check ielen*/
	if (*(IE + 1) == 0 || IELen == 0)
		return;

	pos = (char *)(IE + 2);
	pmbss->QosMapSupport = 1;
	pmbss->DscpExceptionCount = (IELen - 16 - 2);
	count = pmbss->DscpExceptionCount/2;

	for (i = 0; i < count; i++) {
		dscp = *pos & 0xFF;
		up = (*(pos + 1) & 0xFF);
		if (is_valid_dscp(dscp) && is_valid_up(up)) {
			pmbss->QoSMapIsUP = 1;
			pmbss->DscpException[i] = dscp;
			pmbss->DscpException[i] |= (up << 8);
		}
		pos += 2;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"mbss->DscpException[%d]:0x%x\n", i, pmbss->DscpException[i]);
	}

	for (i = 0; i < 8; i++) {
		l = *pos & 0xFF;
		h = (*(pos + 1) & 0xFF);
		if (is_valid_dscp(l) && is_valid_dscp(h) && l < h) {
			pmbss->QoSMapIsUP = 1;
			pmbss->DscpRange[i] = l;
			pmbss->DscpRange[i] |= (h << 8);
		}
		pos += 2;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"mbss->DscpRange[%d]:0x%x\n", i, pmbss->DscpRange[i]);
	}
}

/**
 * Set_QoSR1Enable_Proc: Function to enable/disable QoS R1 feature
 *
 * This API is used to enabled/disable QOS R1 feature in WLAN driver.
 **/
INT Set_QoSR1Enable_Proc(PRTMP_ADAPTER pAd, char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);

	if (pAd->bQoSR1Enable == enable) {
		/* No need to do anything, current and previos values are same */
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s QoS R1 is already %s\n", __func__, enable?"enabled":"disabled");
		return TRUE;
	}

	if (!enable) {
		pAd->bQoSR1Enable = 0;
#ifdef DABS_QOS
		SendQoSCmd(pAd, QOS_CMD_PARAM_RESET, NULL);
#endif
	} else {
			pAd->bQoSR1Enable = 1;
#ifdef DABS_QOS
			SendQoSCmd(pAd, QOS_CMD_ENABLE_DLY_POLICY, NULL);
#endif
	}
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s: QoS R1 is %s\n", __func__, pAd->bQoSR1Enable?"enabled":"disabled");

	return TRUE;
}

#ifdef QOS_R2
INT Set_ScsEnable_Proc(PRTMP_ADAPTER pAd, char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);

	if (pAd->bScsEnable == enable) {
		/* No need to do anything, current and previos values are same */
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s QoS Scs is already %s\n", __func__, enable?"enabled":"disabled");
		return TRUE;
	}

	if (!enable)
		pAd->bScsEnable = 0;
	else
		pAd->bScsEnable = 1;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s: QoS SCS is %s\n", __func__, pAd->bScsEnable?"enabled":"disabled");

	return TRUE;
}

INT Set_DSCPPolicyEnable_Proc(PRTMP_ADAPTER pAd, char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);
	UCHAR IfIdx = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	BSS_STRUCT *pmbss = NULL;


	/* only do this for AP MBSS, ignore other inf type */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		IfIdx = pObj->ioctl_if;
		pmbss = &pAd->ApCfg.MBSSID[IfIdx];
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s(), only support on bss interface.\n", __func__);
		return -1;
	}

	if (!pmbss) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s(), no mbss was found, please check command.\n", __func__);
		return -1;
	}

	if (pmbss->bDSCPPolicyEnable == enable) {
		/* No need to do anything, current and previos values are same */
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s DSCP Policy is already %s on %s\n",
			__func__, enable?"enabled":"disabled", pmbss->wdev.if_dev->name);
		return TRUE;
	}

	if (enable)
		pmbss->bDSCPPolicyEnable = 1;
	else
		pmbss->bDSCPPolicyEnable = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"%s: DSCP Policy is %s on %s\n", __func__,
		pmbss->bDSCPPolicyEnable ? "enabled" : "disabled", pmbss->wdev.if_dev->name);

	return TRUE;
}

INT Set_QoSMapCapa_Proc(PRTMP_ADAPTER pAd, char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);
	UCHAR IfIdx = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	BSS_STRUCT *pmbss = NULL;


	/* only do this for AP MBSS, ignore other inf type */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		IfIdx = pObj->ioctl_if;
		pmbss = &pAd->ApCfg.MBSSID[IfIdx];
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s(), only support on bss interface.\n", __func__);
		return -1;
	}

	if (!pmbss) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s(), no mbss was found, please check command.\n", __func__);
		return -1;
	}

	if (pmbss->QosMapSupport == enable) {
		/* No need to do anything, current and previos values are same */
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s QoS Map Capability is already %s on %s\n",
			__func__, enable?"enabled":"disabled", pmbss->wdev.if_dev->name);
		return TRUE;
	}

	if (enable)
		pmbss->QosMapSupport = 1;
	else
		pmbss->QosMapSupport = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"%s: QoS Map Capability is %s on %s\n",
		__func__, pmbss->QosMapSupport ? "enabled" : "disabled", pmbss->wdev.if_dev->name);

	return TRUE;
}

INT Set_QoSMgmtCapa_Proc(PRTMP_ADAPTER pAd, char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *pwdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

	if (!pwdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s(), pwdev is NULL.\n", __func__);
		return -1;
	}

	if (pwdev->bQoSMCapability == enable) {
		/* No need to do anything, current and previos values are same */
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s QoS Management Capability is already %s on %s\n",
			__func__, enable ? "enabled" : "disabled", pwdev->if_dev->name);
		return TRUE;
	}

	if (enable)
		pwdev->bQoSMCapability = 1;
	else
		pwdev->bQoSMCapability = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"%s: QoS Management Capability is %s on %s\n",
		__func__, pwdev->bQoSMCapability ? "enabled" : "disabled", pwdev->if_dev->name);

	return TRUE;
}

#endif

INT Show_QoS_MapIE_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i = 0, IfIdx = 0, count = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	BSS_STRUCT *pmbss = NULL;
	char *buf = NULL;
	UCHAR *tmp = NULL;
	ULONG bufsize = 1024;
	int ret;
	unsigned int tmp_buf_left = 0;

	/* only do this for AP MBSS, ignore other inf type */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		IfIdx = pObj->ioctl_if;
		pmbss = &pAd->ApCfg.MBSSID[IfIdx];
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s(), only support on bss interface.\n", __func__);
		return -1;
	}

	if (!pmbss) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s(), no mbss was found, please check command.\n", __func__);
		return -1;
	}

	os_alloc_mem(NULL, (UCHAR **)&buf, bufsize);
	if (buf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s():Alloc memory failed\n", __func__);
		return -1;
	}

	memset(buf, 0, bufsize);

	count = pmbss->DscpExceptionCount/2;
	if (pmbss->QoSMapIsUP) {
		tmp_buf_left = bufsize-strlen(buf);
		ret = snprintf(buf, tmp_buf_left, "[QoSR1]: DSCP Exception, Count:%d.\n", count);
		if (os_snprintf_error(tmp_buf_left, ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"buf snprintf error!\n");
			goto error;
		}
		tmp_buf_left = bufsize-strlen(buf);
		ret = snprintf(buf+strlen(buf), tmp_buf_left, "\t\t DSCP \t UP\n");
		if (os_snprintf_error(tmp_buf_left, ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"buf snprintf error!\n");
			goto error;
		}
		for (i = 0; i < count; i++) {
			tmp = (UCHAR *) &pmbss->DscpException[i];
			tmp_buf_left = bufsize-strlen(buf);
			ret = snprintf(buf+strlen(buf), tmp_buf_left,
				"\t\t %3d \t %2d\n", *tmp, *(tmp+1));
			if (os_snprintf_error(tmp_buf_left, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"buf snprintf error!\n");
				goto error;
			}
		}
		tmp_buf_left = bufsize-strlen(buf);
		ret = snprintf(buf+strlen(buf), tmp_buf_left, "[QoSR1]: DSCP Range.\n");
		if (os_snprintf_error(tmp_buf_left, ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"buf snprintf error!\n");
			goto error;
		}
		tmp_buf_left = bufsize-strlen(buf);
		ret = snprintf(buf+strlen(buf), tmp_buf_left, "\t\t UP \t DSCP_L \t DSCP_H\n");
		if (os_snprintf_error(tmp_buf_left, ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"buf snprintf error!\n");
			goto error;
		}
		for (i = 0; i < 8; i++) {
			tmp = (UCHAR *) &pmbss->DscpRange[i];
			tmp_buf_left = bufsize-strlen(buf);
			ret = snprintf(buf+strlen(buf), tmp_buf_left,
				"\t\t %2d \t %4d \t\t %4d\n", i, *tmp, *(tmp+1));
			if (os_snprintf_error(tmp_buf_left, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"buf snprintf error!\n");
				goto error;
			}
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Qosmap is not up.\n");
	}

#ifdef DSCP_PRI_SUPPORT
	if (pmbss->dscp_pri_map_enable) {
		tmp_buf_left = bufsize-strlen(buf);
		ret = snprintf(buf+strlen(buf), tmp_buf_left,
			"[QoSR1]: dscp to up mapping table.\n");
		if (os_snprintf_error(tmp_buf_left, ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"buf snprintf error!\n");
			goto error;
		}
		for (i = 0; i < DSCP_TBL_SIZE; i++) {
			if (i % 8 == 0) {
				tmp_buf_left = bufsize-strlen(buf);
				ret = snprintf(buf+strlen(buf), tmp_buf_left, "0x%04x : ", i);
				if (os_snprintf_error(tmp_buf_left, ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"buf snprintf error!\n");
					goto error;
				}
			}
			tmp_buf_left = bufsize-strlen(buf);
			ret = snprintf(buf+strlen(buf), tmp_buf_left, "%x ",
				((unsigned char)pmbss->dscp_pri_map[i]));
			if (os_snprintf_error(tmp_buf_left, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"buf snprintf error!\n");
				goto error;
			}
			if (i%8 == 7) {
				tmp_buf_left = bufsize-strlen(buf);
				ret = snprintf(buf+strlen(buf), tmp_buf_left, "\n");
				if (os_snprintf_error(tmp_buf_left, ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"buf snprintf error!\n");
				goto error;
				}
			}
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"dscp to priority mapping not enabled.\n");
	}
#endif
	MTWF_PRINT("%s\n", buf);
error:
	os_free_mem(buf);
	return TRUE;
}

INT QoS_send_action_frame(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct action_frm_data *frm)
{
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 Hdr;
	PUCHAR pOutBuffer = NULL;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--> %s()\n", __func__);

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is NULL!\n");
		return FALSE;
	}

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS)
		return NStatus;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t %s, DA:%pM, TA:%pM, BSSID:%pM\n",
		wdev->if_dev->name, frm->destination_addr, frm->transmitter_addr, frm->bssid);

	MgtMacHeaderInitExt(pAd, &Hdr, SUBTYPE_ACTION, 0, frm->destination_addr,
					frm->transmitter_addr,
					frm->bssid);

	Hdr.Sequence = frm->seq_no;
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &Hdr,
					  frm->frm_len, frm->frm,
					  END_OF_ARGS);

	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);

	return 0;
}

static INT wext_send_qos_action_frame(struct wifi_dev *wdev,
	const char *peer_mac_addr, UINT channel, const char *frm, ULONG frm_len)
{
	char *buf = NULL;
	struct wapp_event *event = NULL;
	struct wapp_qos_action_frame *req_data = NULL;
	UINT16 buflen = 0;

	if (!wdev->if_dev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s, wdev->if_dev is null\n", __func__);
		return 1;
	}

	buflen = 24 + frm_len; /*wapp event header+mscs_header len = 24*/
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	if (!buf) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s, alloc memory failed.\n", __func__);
		return 1;
	}
	NdisZeroMemory(buf, buflen);
	event = (struct wapp_event *)buf;
	event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	event->event_id = WAPP_QOS_ACTION_FRAME_EVENT;
	req_data = (struct wapp_qos_action_frame *)&(event->data.qos_frm);

	NdisCopyMemory(req_data->src, peer_mac_addr, 6);
	req_data->frm_len = frm_len;
	req_data->chan = channel;

	NdisCopyMemory(req_data->frm, frm, frm_len);
	MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s, to wapp, src mac:%02x:%02x:%02x:%02x:%02x:%02x, frm_len:%lu, buflen:%d\n",
		__func__, PRINT_MAC(req_data->src), frm_len, buflen);
	RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
		OID_WAPP_EVENT, NULL, (PUCHAR)buf, buflen);
	os_free_mem(buf);
	return 0;
}

static void wext_send_mscs_classifier_parameter(struct wifi_dev *wdev,
	UCHAR *classifier_parameter_report, ULONG len)
{
	char *buf = NULL;
	struct wapp_event *event = NULL;
	struct classifier_parameter *req_data = NULL;
	UINT16 buflen = 0;

	if (!wdev->if_dev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s, wdev->if_dev is null\n", __func__);
		return;
	}
	buflen = 6 + sizeof(struct classifier_parameter);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	if (!buf) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s, alloc memory failed.\n", __func__);
		return;
	}
	NdisZeroMemory(buf, buflen);
	event = (struct wapp_event *)buf;
	event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	event->event_id = WAPP_MSCS_CLASSIFIER_PARAM_EVENT;
	req_data = (struct classifier_parameter *)&(event->data.qos_frm);

	NdisCopyMemory(req_data, classifier_parameter_report, sizeof(*req_data));
	RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
		OID_WAPP_EVENT, NULL, (PUCHAR)buf, buflen);
	os_free_mem(buf);
}

UCHAR QoS_parse_mscs_descriptor_ies(struct wifi_dev *wdev,
	UCHAR *srcAddr, UCHAR *buf, UCHAR buflen)
{
	struct classifier_parameter cs_param;
	UCHAR *ptr = buf;
	UCHAR leftlen = buflen;
	UCHAR subielen = 0;

	if ((*ptr) == IE_EXTENSION_ID_MSCS_DESC) {
		ptr++;
		leftlen--;
	} else
		return 0;

	memset(&cs_param, 0, sizeof(cs_param));
	COPY_MAC_ADDR(cs_param.sta_mac, srcAddr);
	if ((*ptr) >= SCS_REQ_TYPE_UNKNOWN)
		return 0;

	if ((((*ptr) == SCS_REQ_TYPE_ADD || (*ptr) == SCS_REQ_TYPE_CHANGE) && buflen < 13) ||
		(((*ptr) == SCS_REQ_TYPE_REMOVE) && buflen < 7)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s, invalid mscs descriptor element.\n", __func__);
		return 0;
	}

	cs_param.requet_type = *ptr;
	cs_param.up_bitmap = *(ptr + 1);
	cs_param.up_limit = (*(ptr + 2)) & 0x07;
	cs_param.timeout = ((*((UINT32 *)(ptr + 3)))*1024)/1000000;
	ptr += 7;
	leftlen -= 7;
	if (cs_param.requet_type == SCS_REQ_TYPE_REMOVE) {
		wext_send_mscs_classifier_parameter(wdev, (UCHAR *)&cs_param, sizeof(cs_param));
		return 0;
	}
	while ((*ptr) == IE_WLAN_EXTENSION && (*(ptr + 2)) == IE_EXTENSION_ID_TCLAS_MASK) {
		subielen = *(ptr + 1);
		ptr += 2;
		leftlen -= 2;
		if (subielen > leftlen)
			break;
		if ((*(ptr+1)) == MSCS_CLASSIFIER_TYPE4) {
			cs_param.cs.header.cs_type = *(ptr + 1);
			cs_param.cs.header.cs_mask = *(ptr + 2);
			wext_send_mscs_classifier_parameter(wdev,
				(UCHAR *)&cs_param, sizeof(cs_param));
		} else
			MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s, not support mscs classifier type= %d.\n", __func__, (*ptr));
		ptr += subielen;
		leftlen -= subielen;
	}
	return 1;
}

void QoS_get_default_mscs_descriptor(UCHAR *tmpbuf, UCHAR *ielen)
{
	if (!tmpbuf || !ielen)
		return;

	*ielen = sizeof(dft_mscs_desc_ie);
	NdisCopyMemory(tmpbuf, dft_mscs_desc_ie, *ielen);
}

#ifdef QOS_R2
void QoS_Build_WFACapaIE(UCHAR *tmpbuf, UCHAR *ielen, UCHAR bCapa)
{
	if (!tmpbuf || !ielen)
		return;

	*ielen = sizeof(wfa_capa_ie);
	NdisCopyMemory(tmpbuf, wfa_capa_ie, *ielen);
	tmpbuf[*ielen-1] |= (1 & bCapa);
}
#endif

void QoS_PeerRobust_AV_Action(struct _RTMP_ADAPTER *pAd, struct _MLME_QUEUE_ELEM *Elem)
{
	UINT8 Robust_Action = Elem->Msg[LENGTH_802_11 + 1];
	UINT8 Elem_ID = Elem->Msg[LENGTH_802_11 + 3];
	UINT8 Elem_ID_Ext = Elem->Msg[LENGTH_802_11 + 5];
	UINT8 ie_len = 0;
	UINT8 tspec_ie = 0;
	PMSCS_DESC mscs_desc = NULL;
#ifdef MSCS_PROPRIETARY
	UINT8 reasoncode = REQUEST_DECLINED;
	UINT8 Dialog_Token = Elem->Msg[LENGTH_802_11 + 2];
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[Elem->Wcid];
#endif /*MSCS_PROPRIETARY*/

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:\n", __func__);
#ifdef MSCS_PROPRIETARY
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s:can't find pEntry\n", __func__);
		return;
	}
#endif
	switch (Robust_Action) {
#ifdef QOS_R2
	case ACT_SCS_REQ:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"receive scs req,Elem_ID =%d\n", Elem_ID);
		wext_send_qos_action_frame(Elem->wdev, &Elem->Msg[10],
			Elem->Channel, Elem->Msg, Elem->MsgLen);
		break;
	case ACT_SCS_RSP:
		if (Elem->wdev->wdev_type == WDEV_TYPE_STA)
			wext_send_qos_action_frame(Elem->wdev, &Elem->Msg[10],
			Elem->Channel, Elem->Msg, Elem->MsgLen);
		else
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
					"unexpected SCS Response Frame for AP!!!\n");
		break;
#endif
	case ACT_MSCS_REQ:
		mscs_desc = (PMSCS_DESC)(Elem->Msg + LENGTH_802_11 + 3);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"receive mscs req,Elem_ID =%d, Elem_ID_Ext=%d, mscs_desc->Length =%d!!\n",
			Elem_ID, Elem_ID_Ext, mscs_desc->Length);

		if  (mscs_desc->ElementID == IE_WLAN_EXTENSION &&
				mscs_desc->ElementID_Ext == IE_EXTENSION_ID_MSCS_DESC) {

			/*check whether mtk prorietary*/
			ie_len = Elem->MsgLen - LENGTH_802_11 - 3;
			if (ie_len > mscs_desc->Length + 2)
				tspec_ie = *((UINT8 *)mscs_desc + mscs_desc->Length + 2);

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"receive mscs req,tspec_ie=%d!!\n", tspec_ie);

			if (tspec_ie != IE_TSPEC) {
				wext_send_qos_action_frame(Elem->wdev, &Elem->Msg[10],
					Elem->Channel, Elem->Msg, Elem->MsgLen);
				break;
			}
#ifdef MSCS_PROPRIETARY
			else if (pEntry->dabs_cfg == FALSE)
				reasoncode = REQUEST_DECLINED;
			else if (pAd->mscs_req_reject == FALSE)
				QoS_mtk_prop_mscs_ies_parse(pAd, Elem->wdev,
				(UCHAR *)&(pEntry->Addr), &Elem->Msg[LENGTH_802_11 + 3],
				Elem->MsgLen - LENGTH_802_11 - 3, &reasoncode);
			else
				reasoncode = INSUFFICIENT_TCLAS_PROCESSING_RESOURCES;
			QoS_send_mscs_rsp(pAd, Elem->Wcid, Dialog_Token, reasoncode, NULL);
#endif /*MSCS_PROPRIETARY*/
		}
		break;

	case ACT_MSCS_RSP:
		if (Elem->wdev->wdev_type == WDEV_TYPE_STA)
			wext_send_qos_action_frame(Elem->wdev, &Elem->Msg[10],
				Elem->Channel, Elem->Msg, Elem->MsgLen);
		else
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_WARN,
				"unexpected MSCS Response Frame for AP!!!\n");
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
			"unexpected robust AV stream, Robust Action:%d!!!\n", Robust_Action);
		break;
	}
}
void Peer_DABS_Cfg_Timeout(struct _RTMP_ADAPTER *pAd, struct _MLME_QUEUE_ELEM *Elem)
{
	USHORT wcid = Elem->Priv;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
			"INVALID_UCAST_WCID, [wcid=%d]\n", wcid);
		return;
	}
	pEntry = &pAd->MacTab.Content[wcid];

	if (IS_ENTRY_NONE(pEntry)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ENTRY is NONE!!!\n");
		return;
	}
	if (!OS_TEST_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"DABSTimer is cancelled!!!\n");
		return;
	}
	pEntry->dabs_trans_id++;

		if (pEntry->dabs_trans_id <= DABS_RETRY_LIMIT) {
			Send_DABS_Announce(pAd, wcid);
			RTMPSetTimer(&pEntry->DABSRetryTimer, DABS_WAIT_TIME);
			OS_SET_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
		} else {
			pEntry->dabs_cfg = FALSE;
			OS_CLEAR_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
		}

}


#if defined(MSCS_PROPRIETARY) || defined(QOS_R2)
void Peer_Vendor_Spec_Action(struct _RTMP_ADAPTER *pAd, struct _MLME_QUEUE_ELEM *Elem)
{
	UCHAR *pOUI = &Elem->Msg[LENGTH_802_11 + 1];
	FRAME_DABS_ACTION *pdabs_frame;
	UINT8 status = FALSE;
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[Elem->Wcid];
	UCHAR mtk_oui[MTK_OUI_LEN] = {0x00, 0x0C, 0xE7};
	EVENT_FAST_PATH_T event_fastpath;
	BOOLEAN cancelled;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s:\n", __func__);
	NdisZeroMemory(&event_fastpath, sizeof(EVENT_FAST_PATH_T));

	if ((memcmp(pOUI, WFA_OUI, 3) == 0) && (*(pOUI + 3) == QOS_ACT_FRM_OUI_TYPE)) {
		if (*(pOUI + 4) == DSCP_POLICY_REQ) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
				"DSCP Policy Request was received.\n");
		} else if (*(pOUI + 4) == DSCP_POLICY_RSP) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
				"DSCP Policy Response was received.\n");
			wext_send_qos_action_frame(Elem->wdev, &Elem->Msg[10], Elem->Channel,
				Elem->Msg, Elem->MsgLen);
		} else if (*(pOUI + 4) == DSCP_POLICY_QRY) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
				"DSCP Policy Query was received.\n");
			wext_send_qos_action_frame(Elem->wdev, &Elem->Msg[10], Elem->Channel,
				Elem->Msg, Elem->MsgLen);
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
				"unknown Vendor Specific Action Frame, Sub OUI Type:%d!!!\n",
				*(pOUI + 4));
		}
	} else {
		pdabs_frame = (FRAME_DABS_ACTION *)Elem->Msg;
		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s:can't find pEntry\n", __func__);
			return;
		}

		if (Elem->MsgLen != sizeof(FRAME_DABS_ACTION)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s:frame len check error\n", __func__);
			return;
		}
		if (!NdisEqualMemory((UCHAR *)pdabs_frame->Org_id,
			(UCHAR *)mtk_oui, MTK_OUI_LEN)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s:OUI check error\n", __func__);
			return;
		}

		if (pdabs_frame->trans_id != pEntry->dabs_trans_id) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s:trans_id check error\n", __func__);
			return;
		}

		switch (pdabs_frame->DABStype & 0x03) {
		case DABS_REQ:
			/*check bitmap*/
			OS_CLEAR_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
			RTMPCancelTimer(&pEntry->DABSRetryTimer, &cancelled);
			if ((pdabs_frame->keybitmap[0] & pAd->keybitmap[0]) ||
				(pdabs_frame->keybitmap[1] & pAd->keybitmap[1])	||
				(pdabs_frame->keybitmap[2] & pAd->keybitmap[2]) ||
				(pdabs_frame->keybitmap[3] & pAd->keybitmap[3])) {
				/*send to firmware*/
				if (FastPathCheckMIC(pAd, FAST_PATH_CMD_CAL_MIC, pEntry->wcid,
					pEntry->APRandNum, MIC_AP, pdabs_frame->u2MIC,
					pdabs_frame->keybitmap,
					&event_fastpath) != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"%s:fw check mic error\n", __func__);
					return;
				}
				if (RTMPEqualMemory(&pdabs_frame->u2MIC,
					&event_fastpath.u2Mic, 2)) {
					status = MIC_OK;
					pEntry->STARandNum = pdabs_frame->randnumber;
				} else {
					status = MIC_FAIL;
					pEntry->dabs_cfg = FALSE;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						"%s stamic= %x, ap cal stamic = %x\n", __func__,
						pdabs_frame->u2MIC, event_fastpath.u2Mic);
				}
			} else
				status = NO_MATCH_KEY;
			Send_DABS_Rsp(pAd, Elem->Wcid, status, &pdabs_frame->keybitmap[0]);
			if (status == MIC_OK) {
				RTMPSetTimer(&pEntry->DABSRetryTimer, DABS_WAIT_TIME);
				OS_SET_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
			}
			break;
		case DABS_CFM:
			if (pdabs_frame->status == SUCCESS) {
				pEntry->dabs_cfg = TRUE;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s dabs cfm success!!\n", __func__);
			} else
				pEntry->dabs_cfg = FALSE;
			OS_CLEAR_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
			RTMPCancelTimer(&pEntry->DABSRetryTimer, &cancelled);
			break;
		default:
			break;
		}
		return;
	}
}
#endif

#ifdef MSCS_PROPRIETARY
void RTMPDABSretry(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	UINT32 Len = 0;
	MAC_TABLE_ENTRY *pEntry;
	PRTMP_ADAPTER pAd;

	Len = sizeof(MAC_TABLE_ENTRY);

	if (FunctionContext) {
		pEntry = (PMAC_TABLE_ENTRY) FunctionContext;

		if (!OS_TEST_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag))
			return;
		pAd = pEntry->pAd;
		if (pAd)
			MlmeEnqueue(pAd, ACTION_STATE_MACHINE, MT2_MLME_DABS_CFG_TIMEOUT, 0, NULL, pEntry->wcid);
	}

}
BUILD_TIMER_FUNCTION(RTMPDABSretry);

INT Set_DABSkeybitmap_Proc(PRTMP_ADAPTER pAd, char *arg)
{
	ULONG keybitmap;
	UINT dabs_type;/*0 vendor 1 group*/
	RTMP_STRING *str;

	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	str = strsep(&arg, ":");

	keybitmap = os_str_tol(arg, 0, 16);
	dabs_type = os_str_tol(str, 0, 10);

	if (dabs_type > 1)
		return FALSE;
	memcpy((UCHAR *)&(pAd->keybitmap[dabs_type*2]), (UCHAR *)&keybitmap, 8);
	if (pAd->keybitmap[2] == 0 && pAd->keybitmap[3] == 0)
		pAd->keybitmap[2] = GENERAL_KEY;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s: set dabs %s keybitmap = %lx\n", __func__,
		dabs_type ? "group" : "vendor spec", *(ULONG *)(&pAd->keybitmap[dabs_type * 2]));

	return TRUE;
}

/**/
/*Function Set_Dabs_Drop_Thre_Proc:set dabs drop threshold for reject mscs req*/
/**/
INT Set_Dabs_Drop_Thre_Proc(PRTMP_ADAPTER pAd, char *arg)
{
	UCHAR dabs_drop_thre = os_str_tol(arg, 0, 10);

	if (!IS_QOSR1_ENABLE(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s : please enable QOS_R1 first!!\n", __func__);
		return TRUE;
	}
	if (dabs_drop_thre > 100) {

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s : dabs_drop_thre is out of range!\n", __func__);
	}

	else
		pAd->dabs_drop_threashold = dabs_drop_thre;


	return TRUE;
}

void wext_send_vendor_spec_tclas_elment(struct wifi_dev *wdev,
	UCHAR *vend_spec_classifier_para_report, ULONG len)
{
	char *buf = NULL;
	struct wapp_event *event = NULL;
	struct wapp_vend_spec_classifier_para_report *req_data = NULL;
	UINT16 buflen = 0;

	if (!wdev->if_dev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s, wdev->if_dev is null\n", __func__);
		return;
	}
	MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s\n", __func__);
	buflen = 6 + sizeof(struct wapp_vend_spec_classifier_para_report);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	if (!buf) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s, alloc memory failed.\n", __func__);
		return;
	}
	NdisZeroMemory(buf, buflen);
	event = (struct wapp_event *)buf;
	event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	event->event_id = WAPP_VEND_SPEC_UP_TUPLE_EVENT;
	req_data = (struct wapp_vend_spec_classifier_para_report *)&(event->data.qos_frm);
	NdisCopyMemory(req_data, vend_spec_classifier_para_report, sizeof(*req_data));
	RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
		OID_WAPP_EVENT, NULL, (PUCHAR)buf, buflen);
	os_free_mem(buf);
}

void QoS_send_mscs_rsp(struct _RTMP_ADAPTER *pAd, UINT16 wcid, UINT8 Dialogtoken,
	UINT8 reasoncode, struct wapp_vend_spec_classifier_para_report *classifier_para)
{
	struct wifi_dev *wdev;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR *pOutBuffer = NULL;
	ULONG FrameLen;
	NDIS_STATUS status;
	ULONG ie_len = 0;
	UCHAR *mscs_propri_tclas = NULL;
	PTCLAS_ELEM ptclas_elem = NULL;
	PFRAME_CLASS_TYPE_4_ipv4 classtype_ipv4 = NULL;
	FRAME_MSCSRSP_ACTION mscs_rsp_frame;
	PMSCS_DESC mscs_desc = NULL;
	ULONG TmpLen = 0;

	NdisZeroMemory(&mscs_rsp_frame, sizeof(FRAME_MSCSRSP_ACTION));
	status = MlmeAllocateMemory(pAd, &pOutBuffer);
	pEntry = &pAd->MacTab.Content[wcid];


	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s:mscs rsp allocate memory failed\n", __func__);
		return;
	}
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s:entry not found\n", __func__);
		MlmeFreeMemory(pOutBuffer);
		return;
	}
	wdev = pEntry->wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s:wdev not found\n", __func__);
		MlmeFreeMemory(pOutBuffer);
		return;
	}
	ActHeaderInit(pAd, &(mscs_rsp_frame.Hdr), pEntry->Addr, wdev->if_addr, wdev->bssid);
	mscs_rsp_frame.Category = CATEGORY_RAVS;
	mscs_rsp_frame.RobustAction = ACT_MSCS_RSP;
	mscs_rsp_frame.DialogToken = Dialogtoken;
	mscs_rsp_frame.Status = reasoncode;

	MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(FRAME_MSCSRSP_ACTION),
		&mscs_rsp_frame, END_OF_ARGS);

	if (classifier_para) {
		ie_len = sizeof(MSCS_DESC) + sizeof(TCLAS_ELEM) + sizeof(FRAME_CLASS_TYPE_4_ipv4);
		os_alloc_mem(NULL, (UCHAR **)&mscs_propri_tclas, ie_len);
		if (!mscs_propri_tclas) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s, alloc memory failed.\n", __func__);
			MlmeFreeMemory(pOutBuffer);
			return;
		}
		mscs_desc = (struct _MSCS_DESC *)mscs_propri_tclas;
		mscs_desc->ElementID = IE_WLAN_EXTENSION;
		mscs_desc->Length = ie_len - 2;
		mscs_desc->ElementID_Ext = IE_EXTENSION_ID_MSCS_DESC;
		mscs_desc->ReqType = SCS_REQ_TYPE_REMOVE;
		mscs_desc->StreamTimeOut = STREAM_TIMEOUT;

		ptclas_elem = (PTCLAS_ELEM)((UCHAR *)mscs_propri_tclas + sizeof(MSCS_DESC));
		ptclas_elem->ElementID = IE_WLAN_EXTENSION;/*fill tclas ie*/
		ptclas_elem->Length =
			sizeof(TCLAS_ELEM) + sizeof(FRAME_CLASS_TYPE_4_ipv4) - 2; /*len*/
		ptclas_elem->UP_or_ElementID_Ext = IE_EXTENSION_ID_TCLAS_MASK;

		classtype_ipv4 =
			(PFRAME_CLASS_TYPE_4_ipv4)((UCHAR *)ptclas_elem + sizeof(TCLAS_ELEM));
		classtype_ipv4->classifierType = MSCS_CLASSIFIER_TYPE4;
		classtype_ipv4->Version = classifier_para->version;
		classtype_ipv4->srcipv4 = classifier_para->srcIp.ipv4;
		classtype_ipv4->destipv4 = classifier_para->destIp.ipv4;
		classtype_ipv4->srcPort = classifier_para->srcPort;
		classtype_ipv4->destPort = classifier_para->destPort;
		classtype_ipv4->protocol_or_nextHeader = classifier_para->protocol;
		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen, ie_len,
			mscs_propri_tclas, END_OF_ARGS);
		FrameLen += TmpLen;
		os_free_mem(mscs_propri_tclas);
	}
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"send mscs rsp - %s( %d )\n", __func__, mscs_rsp_frame.Status);
}

BOOLEAN QoS_mtk_prop_mscs_ies_parse(struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev, UCHAR *srcAddr, UCHAR *buf, ULONG len, UINT8 *reasoncode)
{
	PTSPEC_ELEM ptspec_elem = NULL;
	PFRAME_CLASS_TYPE_4_ipv4 pclasstpye4_ipv4 = NULL;
	PTCLAS_ELEM ptclas_elem = NULL;
	PMSCS_DESC pmscs_desc = NULL;
	UINT32 idx;
	struct wapp_vend_spec_classifier_para_report vend_spec_classifier_param;
#ifdef DABS_QOS
	struct qos_param_rec_add qos_param_rec_add;

	memset(&qos_param_rec_add, 0, sizeof(struct qos_param_rec_add));
#endif
	memset(&vend_spec_classifier_param, 0, sizeof(struct wapp_vend_spec_classifier_para_report));

	pmscs_desc = (PMSCS_DESC)buf;
	ptclas_elem = (PTCLAS_ELEM)(buf + sizeof(MSCS_DESC));
	pclasstpye4_ipv4 = (PFRAME_CLASS_TYPE_4_ipv4)((UCHAR *)ptclas_elem + sizeof(TCLAS_ELEM));
	ptspec_elem = (PTSPEC_ELEM)((UCHAR *)pclasstpye4_ipv4 + sizeof(FRAME_CLASS_TYPE_4_ipv4));

	if (!pmscs_desc || !ptclas_elem || !pclasstpye4_ipv4) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s, frame len error\n", __func__);
		return FALSE;
	}
	if ((ptclas_elem->ElementID != IE_WLAN_EXTENSION) ||
		(ptclas_elem->UP_or_ElementID_Ext) != IE_EXTENSION_ID_TCLAS_MASK) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s, tclas ie error\n", __func__);
		return FALSE;
	}
	if (ptspec_elem->ElmentID != IE_TSPEC) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s,  TSPEC ie error!!\n", __func__);
		return FALSE;
	}
	if (pclasstpye4_ipv4->classifierType != 4) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s, Classifier_type[%d] not match mtk proprietary define!!\n",
			__func__, pclasstpye4_ipv4->classifierType);
		return FALSE;
	}
		/*fill vend spec report to wapp*/
		COPY_MAC_ADDR(vend_spec_classifier_param.sta_mac, (UCHAR *)srcAddr);
		vend_spec_classifier_param.requet_type = pmscs_desc->ReqType;
		vend_spec_classifier_param.timeout =
			(pmscs_desc->StreamTimeOut) * 1024 / 1000000;/*TU to second*/

	if (pclasstpye4_ipv4->Version == 4) {
		vend_spec_classifier_param.version = pclasstpye4_ipv4->Version;
		vend_spec_classifier_param.srcIp.ipv4 = pclasstpye4_ipv4->srcipv4;
		vend_spec_classifier_param.destIp.ipv4 = pclasstpye4_ipv4->destipv4;
		vend_spec_classifier_param.srcPort = pclasstpye4_ipv4->srcPort;
		vend_spec_classifier_param.destPort = pclasstpye4_ipv4->destPort;
		vend_spec_classifier_param.protocol = pclasstpye4_ipv4->protocol_or_nextHeader;
		vend_spec_classifier_param.up = (ptspec_elem->TSInfo[1] & 0x38) >> 3;
#ifdef DABS_QOS
		COPY_MAC_ADDR(qos_param_rec_add.dest_mac, srcAddr);
		qos_param_rec_add.protocol = pclasstpye4_ipv4->protocol_or_nextHeader;
		qos_param_rec_add.ip_dest = pclasstpye4_ipv4->destipv4;
		qos_param_rec_add.ip_src = pclasstpye4_ipv4->srcipv4;
		qos_param_rec_add.dport = pclasstpye4_ipv4->destPort;
		qos_param_rec_add.sport = pclasstpye4_ipv4->srcPort;
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s, version not match mtk proprietary define!!\n", __func__);
		return FALSE;
	}

	/*parse tspec ie*/


#ifdef DABS_QOS
	switch (vend_spec_classifier_param.requet_type) {
	case SCS_REQ_TYPE_ADD:
		idx = ioctl_search_qos_param_tbl_idx_by_5_tuple(pAd,
			(VOID *)&qos_param_rec_add, TRUE);

		if (idx < MAX_QOS_PARAM_TBL) {

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"mscs req already in table,reject add!!\n");
			return FALSE;
		}
		idx = search_free_qos_param_tbl_idx(pAd);
		if (idx >= MAX_QOS_PARAM_TBL) {

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"dabs qos table if full ,insert fail\n");

			*reasoncode = INSUFFICIENT_TCLAS_PROCESSING_RESOURCES;
			return FALSE;
		}

		qos_param_rec_add.delay_req =
			(unsigned short)(ptspec_elem->DelayBound / 1000); /*microseconds to ms*/
		qos_param_rec_add.priority = vend_spec_classifier_param.up;
		qos_param_rec_add.force_ac = up_to_ac_mapping[qos_param_rec_add.priority];
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"qos_param_rec_add.delay_req=%u\n",
			qos_param_rec_add.delay_req);
		if (qos_param_rec_add.delay_req == 0) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"delay_bound = 0!\n");
			return FALSE;
		}

		if (vend_spec_classifier_param.up < 4 || vend_spec_classifier_param.up > 7) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"error,just accpet VO,VI delaybound requirement\n");
			return FALSE;
		}
		if (update_qos_param(pAd, idx, &qos_param_rec_add) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"update qos para Fail!!\n");
			OS_SPIN_LOCK_BH(&qos_param_table_lock);
			memset(&qos_param_table[idx], 0, sizeof(struct qos_param_rec));
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
			return FALSE;
		}
		break;
	case SCS_REQ_TYPE_REMOVE:
		idx = ioctl_search_qos_param_tbl_idx_by_5_tuple(pAd,
			(VOID *)&qos_param_rec_add, TRUE);

		if (idx >= MAX_QOS_PARAM_TBL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"can't find this entry, don't need remove again!!\n");
			*reasoncode = SUCCESS;
			return TRUE;
		} else if (delete_qos_param(pAd, idx) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"update qos para Fail!!\n");
			OS_SPIN_LOCK_BH(&qos_param_table_lock);
			memset(&qos_param_table[idx], 0, sizeof(struct qos_param_rec));
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
			return FALSE;
		}
		break;
	case SCS_REQ_TYPE_CHANGE:
		idx = ioctl_search_qos_param_tbl_idx_by_5_tuple(pAd,
			(VOID *)&qos_param_rec_add, TRUE);

		if (idx >= MAX_QOS_PARAM_TBL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"can't find this entry, don't can't change !!\n");
			return FALSE;
		}
		if (update_qos_param(pAd, idx, &qos_param_rec_add) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"update qos para Fail!!\n");
			OS_SPIN_LOCK_BH(&qos_param_table_lock);
			memset(&qos_param_table[idx], 0, sizeof(struct qos_param_rec));
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
			return FALSE;
		}
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s, unexpected request_type!!\n", __func__);
		return FALSE;
	}
#endif
	*reasoncode = SUCCESS;
	vend_spec_classifier_param.id = idx;
	wext_send_vendor_spec_tclas_elment(wdev, (UCHAR *)&vend_spec_classifier_param,
		sizeof(vend_spec_classifier_param));

	return TRUE;
}

#endif /*MSCS_PROPRIETARY*/

INT QoS_Config_for_RDUT_Test(struct _RTMP_ADAPTER *pAd, char *arg)
{
	UCHAR cmds = os_str_tol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#ifdef MSCS_PROPRIETARY
	struct wapp_vend_spec_classifier_para_report vs_param;
#endif

	UCHAR MSCS_ACT_FRM[] = {
		0xd0, 0x00, 0x24, 0x00,
		0x00, 0x90, 0x4C, 0x40, 0x0e, 0xaf, 0x00, 0x0c, 0x43, 0x2a, 0x07, 0x88,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0x50, 0x00,
		MT2_PEER_ROBUST_AV_CATE, ACT_MSCS_REQ, 0x06,
		IE_WLAN_EXTENSION,
		30, /*length*/
		IE_EXTENSION_ID_MSCS_DESC,
		SCS_REQ_TYPE_ADD,
		0xF0, 0x07, 0x60, 0xEA, 0, 0,	/*UP Control, Stream TimeOut*/
		IE_WLAN_EXTENSION,
		0x13, /*length*/
		IE_EXTENSION_ID_TCLAS_MASK,
		MSCS_CLASSIFIER_TYPE4, 0x5f,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0
	};
#ifdef QOS_R2
	UCHAR SCS_ACT_FRM1[] = {
		0xd0, 0x00, 0x24, 0x00,
		0x00, 0x90, 0x4C, 0x40, 0x0e, 0xaf, 0x00, 0x0c, 0x43, 0x2a, 0x07, 0x88,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0x50, 0x00,
		MT2_PEER_ROBUST_AV_CATE, ACT_SCS_REQ, 0x01,
		0xb9, 0x1a, 1, 0, 0xb8, 1, 5, 0x0e, 0x13,
		0xff, 0x04, 0x5f, 0x04, 0xc0, 0xa5, 0x64,
		0x68, 0xc0, 0xa5, 0x64, 0x92, 0x27, 0x74, 0x13, 0xec,
		0, 0x11, 0
	};
	UCHAR SCS_ACT_FRM2[] = {
		0xd0, 0x00, 0x24, 0x00, 0x00, 0x90, 0x4C, 0x40, 0x0e, 0xaf, 0x00, 0x0c, 0x43,
		0x2a, 0x07, 0x88, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x50, 0x00, 0x13, 0x00,
		0x01, 0xb9, 0x2b, 0x01, 0x00, 0xb8, 0x01, 0x04, 0x0e, 0x13, 0xff, 0x04,	0x47,
		0x04, 0xc0, 0xa5, 0x64, 0xfb, 0xc0, 0xa5, 0x64, 0x92, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x32, 0x00, 0x0e, 0x0c, 0xff, 0x0a, 0x00, 0x32, 0x00, 0x00, 0x2b, 0x67,
		0xff, 0xff, 0xff, 0xff, 0x2c, 0x01, 0x00, 0xb9, 0x2b, 0x02, 0x00, 0xb8, 0x01,
		0x06, 0x0e, 0x13, 0xff, 0x04, 0x47, 0x04, 0xc0,	0xa5, 0x64, 0xfb, 0xc0, 0xa5,
		0x64, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x0e, 0x0c, 0xff, 0x0a,
		0x00, 0x32, 0x00, 0x00, 0x2b, 0x68, 0xff, 0xff, 0xff, 0xff, 0x2c, 0x01, 0x00
	};
#endif
	if (!IS_QOSR1_ENABLE(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s : please enable QOS_R1 first!!\n", __func__);
		return FALSE;
	}
	NdisZeroMemory(&vs_param, sizeof(vs_param));

	if (cmds == 1) {
		wext_send_qos_action_frame(wdev, &MSCS_ACT_FRM[10], wdev->channel,
			MSCS_ACT_FRM, sizeof(MSCS_ACT_FRM));
#ifdef QOS_R2
	} else if (cmds == 2) {
		wext_send_qos_action_frame(wdev, &SCS_ACT_FRM1[10], wdev->channel,
			SCS_ACT_FRM1, sizeof(SCS_ACT_FRM1));
		wext_send_qos_action_frame(wdev, &SCS_ACT_FRM2[10], wdev->channel,
			SCS_ACT_FRM2, sizeof(SCS_ACT_FRM2));
#endif
#ifdef MSCS_PROPRIETARY
	} else if (cmds == 3) {
		vs_param.id = 0x00001;
		memset(vs_param.sta_mac, 0x55, MAC_ADDR_LEN);
		vs_param.requet_type = SCS_REQ_TYPE_ADD;
		vs_param.up = 5;
		vs_param.delay_bound = 100;
		vs_param.protocol = 88;
		vs_param.timeout = 50;
		vs_param.version = 4;

		wext_send_vendor_spec_tclas_elment(wdev, (UCHAR *)&vs_param, sizeof(vs_param));
#endif
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"valid parameter is:\n\t 1: Config MSCS,\n\t");
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"2: Config SCS,\n\t 3: Config VS MSCS, others:invalid\n");
	}

	return TRUE;
}

#endif/*QOS_R1*/
