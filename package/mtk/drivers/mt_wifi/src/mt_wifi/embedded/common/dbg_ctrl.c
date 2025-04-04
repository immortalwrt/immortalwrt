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

*/

#include "rt_config.h"

#ifdef FW_LOG_DUMP

#ifndef IP_ASSEMBLY
typedef struct ip_v4_hdr {
#ifdef RT_BIG_ENDIAN
	UCHAR version:4, ihl:4;
#else
	UCHAR ihl:4, version:4;
#endif
	UCHAR tos;
	USHORT tot_len;
	USHORT identifier;
} IP_V4_HDR;
#endif

INT set_fw_log_dest_dir(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 index;
	CHAR last;
	UINT32 max_len = sizeof(pAd->fw_log_ctrl.fw_log_dest_dir) - 1;
	int ret;

	for (index = 0; index < max_len; index++)
		if (*(arg + index + 1) == '\0')
			break;
	last = *(arg + index);

	if (last == '/') {
		ret = snprintf(pAd->fw_log_ctrl.fw_log_dest_dir, max_len, "%sfw_log.bin", arg);
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"snprintf error!\n");
			return FALSE;
		}
	} else {
		ret = snprintf(pAd->fw_log_ctrl.fw_log_dest_dir, max_len, "%s/fw_log.bin", arg);
		if (os_snprintf_error(max_len, ret)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"snprintf error!\n");
			return FALSE;
		}
	}

	MTWF_PRINT("FW Binary log destination directory: %s\n", pAd->fw_log_ctrl.fw_log_dest_dir);

	return TRUE;
}


INT set_binary_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *dbg_module_str = NULL;
	UINT8 dbg_lvl;
	UINT32 dbg_module_idx;

	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	dbg_module_str = strsep(&arg, ":");
	dbg_module_idx = os_str_toul(dbg_module_str, 0, 10);

	if (arg == NULL || strlen(arg) == 0) {
		/* imply all modules */
		dbg_lvl = 0xff;
	} else {
		dbg_lvl = os_str_toul(arg, 0, 10);
	}

	if (dbg_module_idx >= BIN_DBG_LOG_NUM)
		return FALSE;

	pAd->fw_log_ctrl.debug_level_ctrl[dbg_module_idx] = dbg_lvl;

	MTWF_PRINT("%s: set debug_level_ctrl[%d] = 0x%x\n", __func__, dbg_module_idx, dbg_lvl);

	return TRUE;
}


UINT16 Checksum16(UINT8 *pData, int len)
{
	int sum = 0;

	while (len > 1) {
		sum += *((UINT16 *)pData);

		pData = pData + 2;

		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);
		len -= 2;
	}

	if (len)
		sum += *((UINT8 *)pData);

	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return ~sum;
}

void rtmp_read_fw_log_dump_parms_from_file(RTMP_ADAPTER *pAd, CHAR *tmpbuf, CHAR *buffer)
{
	UINT32 ip_addr;

	if (RTMPGetKeyParameter("fwlogserverip", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		if (rtinet_aton(tmpbuf, &ip_addr)) {
			pAd->fw_log_ctrl.fw_log_server_ip = ip_addr;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fw_log_server_ip=%s(%x)\n", tmpbuf, pAd->fw_log_ctrl.fw_log_server_ip);
		}
	}

	if (RTMPGetKeyParameter("fwlogservermac", tmpbuf, 25, buffer, TRUE)) {
		INT	i, mac_len;

		mac_len = strlen(tmpbuf);

		if (mac_len != 17) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid length (%d)\n", mac_len);
			return;
		}

		if (strcmp(tmpbuf, "00:00:00:00:00:00") == 0) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid mac setting\n");
			return;
		}

		for (i = 0; i < MAC_ADDR_LEN; i++) {
			AtoH(tmpbuf, &pAd->fw_log_ctrl.fw_log_server_mac[i], 1);
			tmpbuf = tmpbuf + 3;
		}
	}
}

INT32 set_fwlog_serverip(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	UINT32 ip_addr;
	UCHAR ip_buf[128];

	strncpy(ip_buf, arg, sizeof(ip_buf)-1);
	ip_buf[sizeof(ip_buf) - 1] = '\0';
	MTWF_PRINT("ip=[%s]\n", ip_buf);

	if (rtinet_aton(ip_buf, &ip_addr)) {
		ad->fw_log_ctrl.fw_log_server_ip = ip_addr;
		MTWF_PRINT("fw_log_server_ip=%s(%x)\n", arg, ad->fw_log_ctrl.fw_log_server_ip);
	}
	return TRUE;
}

INT32 set_fwlog_servermac(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	INT	i, mac_len;
	UCHAR mac_buf[20];

	strncpy(mac_buf, arg, sizeof(mac_buf) - 1);
	mac_buf[sizeof(mac_buf) - 1] = '\0';
	mac_len = strlen(arg);

	if (mac_len != 17) {
		MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid length (%d)\n", mac_len);
		return 0;
	}

	if (strcmp(arg, "00:00:00:00:00:00") == 0) {
		MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid mac setting\n");
		return 0;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++) {
		AtoH(arg, &ad->fw_log_ctrl.fw_log_server_mac[i], 1);
		arg = arg + 3;
	}
	MTWF_PRINT("mac=[%s]\n", mac_buf);
	return 0;
}

NTSTATUS fw_log_to_file(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	RTMP_OS_FD_EXT srcf;
	INT8 Ret;

	srcf = os_file_open(pAd->fw_log_ctrl.fw_log_dest_dir, O_WRONLY|O_CREAT|O_APPEND, 0);
	if (srcf.Status) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Open file \"%s\" failed!\n", pAd->fw_log_ctrl.fw_log_dest_dir);
		return NDIS_STATUS_FAILURE;
	}

	Ret = os_file_write(srcf, (INT8 *)CMDQelmt->buffer, (UINT32)CMDQelmt->bufferlength);
	if (Ret != 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Write file failed ! Ret=%d\n", Ret);
		return NDIS_STATUS_FAILURE;
	}

	Ret = os_file_close(srcf);

	if (Ret)
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "File Close Error ! Ret = %d\n", Ret);

	return NDIS_STATUS_SUCCESS;
}


VOID fw_log_to_ethernet(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *fw_log,
	IN UINT32 log_len)
{
	UCHAR s_addr[MAC_ADDR_LEN];
	UINT32 source_ip = 0x00000000, dest_ip = 0xFFFFFFFF;
	UCHAR ETH_P_AIR_MONITOR[LENGTH_802_3_TYPE] = {0x08, 0x00};
	struct sk_buff *skb = NULL;
	UINT8 isPadding = 0;
	UINT8 *data, *header;
	UINT8 *ip_header, *ip_checksum;
	UINT8 *udp_header, *udp_checksum, *pseudo_header;
	UINT16 data_len, header_len;
	IP_V4_HDR *ipv4_hdr_ptr;
	UINT16 checksum;
	UINT8 i;
	struct wifi_dev *tmpWdev = NULL;

	if (pAd->fw_log_ctrl.fw_log_server_ip != 0xFFFFFFFF) {
		dest_ip = pAd->fw_log_ctrl.fw_log_server_ip;
		source_ip = (dest_ip & 0x00FFFFFF) | 0xFE000000;
	}

	header_len = LENGTH_802_3 + 20 + 8; /* 802.3 + IP + UDP */
	if ((log_len % 2) == 0)
		data_len = log_len;
	else {
		data_len = log_len + 1;
		isPadding = 1;
	}

	/* find up wdev interface for fw log capture */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tmpWdev = pAd->wdev_list[i];
		if (tmpWdev && tmpWdev->if_up_down_state)
			break;
	}
	if (i >= WDEV_NUM_MAX) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"no wdev if up, can't capture fw log!\n");
		return;
	}

	skb = dev_alloc_skb(log_len + header_len + 2);
	if (skb == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "failed to allocate sk_buff\n");
		return;
	}

	SET_OS_PKT_NETDEV(skb, tmpWdev->if_dev);

	OS_PKT_RESERVE(skb, header_len);

	/* Prepare payload*/
	data = OS_PKT_TAIL_BUF_EXTEND(skb, data_len);
	NdisCopyMemory(data, fw_log, log_len);
	if (isPadding)
		*(data + log_len) = 0;

	/* Prepare UDP header */
	header = OS_PKT_HEAD_BUF_EXTEND(skb, 8);
	udp_header = header;
	*(UINT16 *)header = htons(54321);           /* source port */
	header += sizeof(UINT16);
	*(UINT16 *)header = htons(55688);           /* destination port */
	header += sizeof(UINT16);
	*(UINT16 *)header = htons(data_len + 8);     /* Length */
	header += sizeof(UINT16);
	udp_checksum = header;
	*(UINT16 *)header = htons(0);               /* UDP Checksum */
	pseudo_header = udp_header - 12;
	header = pseudo_header;
	*(UINT32 *)header = source_ip;              /* Source IP */
	header += sizeof(UINT32);
	*(UINT32 *)header = dest_ip;                /* Destination IP */
	header += sizeof(UINT32);
	*(UINT16 *)header = htons(data_len + 8);    /* Length */
	header += sizeof(UINT16);
	*(UINT16 *)header = htons(17);              /* Length */
	checksum = Checksum16(pseudo_header, data_len + 8 + 12);
	*(UINT16 *)udp_checksum = checksum;

	/* Prepare IP header */
	header = OS_PKT_HEAD_BUF_EXTEND(skb, 20);
	ip_header = header;
	ipv4_hdr_ptr = (IP_V4_HDR *)header;
	ipv4_hdr_ptr->version = 4;
	ipv4_hdr_ptr->ihl = 5;
	ipv4_hdr_ptr->tos = 0;
	ipv4_hdr_ptr->tot_len = htons(data_len + 20 + 8);
	ipv4_hdr_ptr->identifier = 0;
	header += sizeof(IP_V4_HDR);
	*(UINT16 *)header = htons(0x4000);          /* Fragmentation flags and offset */
	header += sizeof(UINT16);
	*header = 7;                                /* Time to live */
	header++;
	*header = 17;                               /* Protocol UDP */
	header++;
	ip_checksum = header;
	*(UINT16 *)header = htons(0);               /* IP Checksum */
	header += sizeof(UINT16);
	*(UINT32 *)header = source_ip;              /* Source IP */
	header += sizeof(UINT32);
	*(UINT32 *)header = dest_ip;                /* Destination IP */
	checksum = Checksum16(ip_header, 20);
	*(UINT16 *)ip_checksum = checksum;

	/* Prepare 802.3 header */
	header = OS_PKT_HEAD_BUF_EXTEND(skb, LENGTH_802_3);
	/* Fake a Source Address for transmission */
#ifdef CONFIG_AP_SUPPORT
	COPY_MAC_ADDR(s_addr, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_addr);
#else
	COPY_MAC_ADDR(s_addr, pAd->StaCfg[0].wdev.if_addr);
#endif /* CONFIG_AP_SUPPORT */
	if (s_addr[1] == 0xff)
		s_addr[1] = 0;
	else
		s_addr[1]++;
	MAKE_802_3_HEADER(header, pAd->fw_log_ctrl.fw_log_server_mac, s_addr, ETH_P_AIR_MONITOR);

	/* Report to upper layer */
	RtmpOsPktProtocolAssign(skb);
	RtmpOsPktRcvHandle(skb, pAd->tr_ctl.napi);
}


NTSTATUS
dbg_log_wrapper(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 ucPktType,
	IN UINT8 *pucData,
	IN UINT16 u2Length)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
	UINT8 *buffer = NULL;
	UINT16 msg_len = 0;
	UINT16 *serialID = &(pAd->fw_log_ctrl.fw_log_serialID_count);
	P_FW_BIN_LOG_HDR_T log_hdr;
	PICS_AGG_HEADER prIcsAggHeader;

	if ((pAd->fw_log_ctrl.wmcpu_log_type &
		(FW_LOG_2_HOST_CTRL_2_HOST_STORAGE | FW_LOG_2_HOST_CTRL_2_HOST_ETHNET)) == 0)
		return NDIS_STATUS_SUCCESS;

	switch (ucPktType) {
	case DBG_LOG_PKT_TYPE_ICS:
		prIcsAggHeader = (PICS_AGG_HEADER)GET_OS_PKT_DATAPTR(pucData);
		msg_len = prIcsAggHeader->rxByteCount + sizeof(FW_BIN_LOG_HDR_T);

		if (os_alloc_mem(pAd, (UCHAR **)&buffer, msg_len) != NDIS_STATUS_SUCCESS)
			return NDIS_STATUS_FAILURE;
		log_hdr = (P_FW_BIN_LOG_HDR_T)buffer;

		/* prepare ICS header */
		log_hdr->u4MagicNum = FW_BIN_LOG_MAGIC_NUM;
		log_hdr->u1Version = FW_BIN_LOG_VERSION;
		log_hdr->u1Rsv = FW_BIN_LOG_RSV;
		log_hdr->u2SerialID = (*serialID)++;
		if (chip_dbg->get_lpon_frcr)
			log_hdr->u4Timestamp = chip_dbg->get_lpon_frcr(pAd);
		else
			log_hdr->u4Timestamp = 0;
		log_hdr->u2MsgID = DBG_LOG_PKT_TYPE_ICS;
		log_hdr->u2Length = prIcsAggHeader->rxByteCount;

		/* prepare ICS frame */
		NdisCopyMemory(buffer + sizeof(FW_BIN_LOG_HDR_T), prIcsAggHeader, prIcsAggHeader->rxByteCount);
		break;

	case DBG_LOG_PKT_TYPE_TRIG_FRAME:
		if (pAd->fw_log_ctrl.debug_level_ctrl[BIN_DBG_LOG_TRIGGER_FRAME] == 0)
			return NDIS_STATUS_SUCCESS;

		msg_len = u2Length + sizeof(FW_BIN_LOG_HDR_T);
		if (os_alloc_mem(pAd, (UCHAR **)&buffer, msg_len) != NDIS_STATUS_SUCCESS)
			return NDIS_STATUS_FAILURE;
		log_hdr = (P_FW_BIN_LOG_HDR_T)buffer;

		log_hdr->u4MagicNum = FW_BIN_LOG_MAGIC_NUM;
		log_hdr->u1Version = FW_BIN_LOG_VERSION;
		log_hdr->u1Rsv = FW_BIN_LOG_RSV;
		log_hdr->u2SerialID = (*serialID)++;
		if (chip_dbg->get_lpon_frcr)
			log_hdr->u4Timestamp = chip_dbg->get_lpon_frcr(pAd);
		else
			log_hdr->u4Timestamp = 0;
		log_hdr->u2MsgID = DBG_LOG_PKT_TYPE_TRIG_FRAME;
		log_hdr->u2Length = u2Length;

		os_move_mem(buffer + sizeof(FW_BIN_LOG_HDR_T), pucData, u2Length);

		break;
	}

	if (msg_len) {
		if (pAd->fw_log_ctrl.wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_STORAGE)
			RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_TO_FILE, (VOID *)buffer, msg_len);
		if (pAd->fw_log_ctrl.wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_ETHNET)
			fw_log_to_ethernet(pAd, buffer, msg_len);

		os_free_mem(buffer);
	}

	return NDIS_STATUS_SUCCESS;
}



#endif /* FW_LOG_DUMP */

#ifdef DBG
#ifdef DBG_ENHANCE
static BOOLEAN mtwf_dbg_prtCatLvl = TRUE; /* Print debug category and level */
static BOOLEAN mtwf_dbg_prtIntfName = FALSE;/* Print interface name */
static BOOLEAN mtwf_dbg_prtThreadId = FALSE;/* Print current thread ID */
static BOOLEAN mtwf_dbg_prtFuncLine = TRUE;/* function name and line */

void mtwf_dbg_option(
	IN const BOOLEAN prtCatLvl,
	IN const BOOLEAN prtIntfName,
	IN const BOOLEAN prtThreadId,
	IN const BOOLEAN prtFuncLine)
{
	mtwf_dbg_prtCatLvl = prtCatLvl;
	mtwf_dbg_prtIntfName = prtIntfName;
	mtwf_dbg_prtThreadId = prtThreadId;
	mtwf_dbg_prtFuncLine = prtFuncLine;
}

void mtwf_dbg_prt(
	IN RTMP_ADAPTER	*pAd,
	IN const UINT32	dbgCat,
	IN const UINT32	dbgLvl,
	IN const INT8   *pFunc,
	IN const UINT32	line,
	IN const INT8   *pFmt,
	...)
{
	va_list args;
	INT8 strBuf[DBG_PRINT_BUF_SIZE];
	INT32 prefixLen = 0;
	INT32 avblBufLen = DBG_PRINT_BUF_SIZE;
	POS_COOKIE pObj = NULL;
	char *intf = NULL;
	char chip[5];
	UINT ifIndex, ifType;
	struct net_device *netDev = NULL;
	struct wifi_dev *wdev = NULL;
	int ret;

	if (pAd) {
		ret = snprintf(chip, sizeof(chip), "%04X", pAd->ChipID);
		if (os_snprintf_error(sizeof(chip), ret)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pObj = (POS_COOKIE)pAd->OS_Cookie;
	}

	if (pObj) {
		ifIndex = pObj->ioctl_if;
		ifType = pObj->ioctl_if_type;

		if (ifType == INT_MAIN || ifType == INT_MBSSID) {
			if (pAd->hdev_ctrl && VALID_MBSS(pAd, ifIndex))
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		} else if (ifType == INT_APCLI) {
			if (ifIndex < MAX_MULTI_STA)
				wdev = &pAd->StaCfg[ifIndex].wdev;
		} else if (ifType == INT_WDS) {
			if (ifIndex < MAX_WDS_ENTRY)
				wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
		} else {
		}

		if (wdev) {
			netDev = (struct net_device *) wdev->if_dev;
			if (netDev)
				intf = netDev->name;
		}
	}

	/**
	* Log message format:
	* <chip>@[C<categore>][L<level>][<thread name>][<interface>],[<function>][<line>]: <log>
	* For example: 7915@C13L1P13656apcli0,MacTableInsertEntry() 920: XXXX
	*/
	prefixLen = snprintf(strBuf, avblBufLen,
						"%s@", intf?chip:"WiFi");
	if (mtwf_dbg_prtCatLvl && ((avblBufLen - prefixLen) > 0))
		prefixLen += snprintf(strBuf + prefixLen, avblBufLen - prefixLen,
							"C%02dL%1d", dbgCat, dbgLvl);
	if (mtwf_dbg_prtThreadId && ((avblBufLen - prefixLen) > 0))
		prefixLen += snprintf(strBuf + prefixLen, avblBufLen - prefixLen,
							"%s", current->comm);
	if (mtwf_dbg_prtIntfName && intf && ((avblBufLen - prefixLen) > 0))
		prefixLen += snprintf(strBuf + prefixLen, avblBufLen - prefixLen,
							"%s", intf);
	if (mtwf_dbg_prtFuncLine && ((avblBufLen - prefixLen) > 0))
		prefixLen += snprintf(strBuf + prefixLen, avblBufLen - prefixLen,
							",%s() %d", pFunc, line);
	if ((avblBufLen - prefixLen) > 0)
		prefixLen += snprintf(strBuf + prefixLen, avblBufLen - prefixLen,
							": ");

	if ((avblBufLen - prefixLen) > 0) {
		va_start(args, pFmt);
		ret = vsnprintf(strBuf + prefixLen, avblBufLen - prefixLen, pFmt, args);
		if (os_snprintf_error(avblBufLen - prefixLen, ret)) {
			MTWF_PRINT("%s: vsnprintf error\n", __func__);
		}
		va_end(args);
	}

	switch (dbgLvl)	{
	case DBG_LVL_OFF:
		MTWF_PRINT_DBG_LVL_OFF("%s", strBuf);
		break;
	case DBG_LVL_ERROR:
		MTWF_PRINT_DBG_LVL_ERROR("%s", strBuf);
		break;
	case DBG_LVL_WARN:
		MTWF_PRINT_DBG_LVL_WARN("%s", strBuf);
		break;
	case DBG_LVL_NOTICE:
		MTWF_PRINT_DBG_LVL_NOTICE("%s", strBuf);
		break;
	case DBG_LVL_INFO:
		MTWF_PRINT_DBG_LVL_INFO("%s", strBuf);
		break;
	case DBG_LVL_DEBUG:
	default:
		MTWF_PRINT_DBG_LVL_DEBUG("%s", strBuf);
		break;
	}
}
#endif /* DBG_ENHANCE */
#endif /* DBG */

