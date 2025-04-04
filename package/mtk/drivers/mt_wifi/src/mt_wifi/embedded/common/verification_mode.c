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

	Module Name:
	veri_ctl.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Carter Chen 02-April-2018    created

*/

#include "rt_config.h"

#define DOT_3_PACKET(_veri_type)	(_veri_type == VERI_PKT_DOT_3_TYPE)

#define VERI_CTRL_TYPE_PACKET(_veri_type, _veri_subtype) ((_veri_type == FC_TYPE_CNTL) && \
							 ((_veri_subtype == SUBTYPE_BLOCK_ACK_REQ) || \
							  (_veri_subtype == SUBTYPE_PS_POLL) || \
							  (_veri_subtype == SUBTYPE_RTS)))

static VOID show_set_pkt_head_format(VOID)
{
	MTWF_PRINT("Step 1:\n");
	MTWF_PRINT("iwpriv ra0 set veri_pkt_head='type'-'subtype'-'addr1'-'addr2'-'addr3'\n");
	MTWF_PRINT("\nDescription:\n");
	MTWF_PRINT("\tType: 0: MGMT, acceptable subtype: any subtype EXCEPT beacon of 802.11 mgmt pkt\n");
	MTWF_PRINT("\tType: 1: CTRL, acceptable subtype: 8:BAR, PS-Poll:10, RTS:11\n");
	MTWF_PRINT("\tType: 2: DATA, acceptable subtype: 0:data, 4:null data, 8: QoS data, 12: QoS Null\n");
	MTWF_PRINT("\tType: 3: 802.3 format pacekt\n");

	MTWF_PRINT("\t\t- addr1 & addr2 is for a1 and a3 of 802.11 data and mgmt pkt\n");
	MTWF_PRINT("\t\t- also could be da & sa of 802.3 packet, ra & ta of CTRL pkt\n");
	MTWF_PRINT("\t\t- addr3 is a must for 802.11 data and mgmt packet\n");
}

static VOID show_set_pkt_ctnt_format(VOID)
{
	MTWF_PRINT("Step 2:\n");
	MTWF_PRINT("iwpriv ra0 set veri_pkt_ctnt=len:'len'-ctnt:'ctnt'\n");
	MTWF_PRINT("\t- len: it will auto repeat the content to the length\n");
	MTWF_PRINT("\t\tif you don't assign length or it small than content, it will honor the len of ctnt\n");
	MTWF_PRINT("\t- ctnt: in hex format, such as, aabbcc\n");
}

static VOID show_set_pkt_ctrl_en_format(VOID)
{
	MTWF_PRINT("(Optional) Step 3: you can just type to enable some of them.\n");
	MTWF_PRINT("for example, iwpriv ra0 set veri_pkt_ctrl_en=du:1-na:1, to just control duration and no_ack.\n\n");
	MTWF_PRINT("\tiwpriv ra0 set veri_pkt_ctrl_en=du:'x'-na:'x'-tm:'x'");
	MTWF_PRINT("-sn:'x'-txs2m:'x'-txs2h:'x'-pm:'x'-life:'x'\n");
	MTWF_PRINT("\t\t- DU: 1:enable the duration is controlled by SW. need to assign dur in wifi hdr\n");
	MTWF_PRINT("\t\t- NA: 1:enable NO ACK packet in this packet\n");
	MTWF_PRINT("\t\t- TM: 1:enable timing measurement in this packet\n");
	MTWF_PRINT("\t\t- SN: 1:enable the seq is controlled by SW. need to assign seq in wifi hdr\n");
	MTWF_PRINT("\t\t- txs2m: 1:enable TXS2MCU in this packet. need to assign PacketID\n");
	MTWF_PRINT("\t\t- txs2h: 1:enable TXS2HOST in this packet. need to assign PacketID\n");
	MTWF_PRINT("\t\t- PM: 1:enable PM mode is controlled by SW. need to assign PM status in wifi hdr\n\n");
	MTWF_PRINT("\t\t- life: 1:enable pkt lifetime is filled by SW. need to assign lifetime\n\n");
}

static VOID show_set_pkt_ctrl_assign_format(VOID)
{
	MTWF_PRINT("(Optional) Step 4: you can only assign the value that you already want to controlled\n");
	MTWF_PRINT("\tfor example, iwpriv ra0 set veri_pkt_ctrl_en=sn:100, to set seq to 100 while you enabled seq\n");
	MTWF_PRINT("\tiwpriv ra0 set veri_pkt_ctrl_assign=du:'x'-sn:'x'-pm:'x'-pid:'x'-life:'x'\n");
	MTWF_PRINT("\t\t- DU: the duration in wifi_hdr\n");
	MTWF_PRINT("\t\t- SN: the SEQ in wifi_hdr, it cannot exceed 4095\n");
	MTWF_PRINT("\t\t- PM: the PM in wifi_hdr, it follows the WIFI spec\n");
	MTWF_PRINT("\t\t- PID: the PID assigned for TXS, it cannot exceed 255\n\n");
	MTWF_PRINT("\t\t- life: the lifetime assigned, it cannot exceed 255\n\n");
}

static VOID show_set_pkt_ctrl_send_format(VOID)
{
	MTWF_PRINT("Step 5:\n");
	MTWF_PRINT("\tiwpriv ra0 set send_veri_pkt=1\n");
}

static VOID show_cmd_format(VOID)
{
	show_set_pkt_head_format();
	show_set_pkt_ctnt_format();
	show_set_pkt_ctrl_en_format();
	show_set_pkt_ctrl_assign_format();
	show_set_pkt_ctrl_send_format();
}

INT set_dump_rx_debug(struct _RTMP_ADAPTER *ad, char *arg)
{
	ad->veri_ctrl.dump_rx_debug = os_str_tol(arg, 0, 10);
	MTWF_PRINT("set verification mode rx dump = %d\n", ad->veri_ctrl.dump_rx_debug);
	return TRUE;
}

INT set_skip_ageout(struct _RTMP_ADAPTER *ad, char *arg)
{
	ad->veri_ctrl.skip_ageout = os_str_tol(arg, 0, 10);
	MTWF_PRINT("set verification mode skip_ageout = %d\n", ad->veri_ctrl.skip_ageout);
	return TRUE;
}

static BOOLEAN construct_veri_pkt(struct _RTMP_ADAPTER *ad,
				  struct wifi_dev *wdev,
				  struct veri_ctrl *veri_ctrl,
				  UCHAR tx_hw_hdr_len,
				  UCHAR *buf)
{
	BOOLEAN ret = TRUE;

	switch (veri_ctrl->veri_pkt_type) {
	case FC_TYPE_MGMT:
	{
		ULONG frame_len = 0;

		MgtMacHeaderInit(ad,
				 (HEADER_802_11 *)(buf + tx_hw_hdr_len),
				 veri_ctrl->veri_pkt_subtype,
				 0,
				 veri_ctrl->addr1,
				 veri_ctrl->addr2,
				 veri_ctrl->addr3);

		MakeOutgoingFrame((UCHAR *)(buf + tx_hw_hdr_len + sizeof(HEADER_802_11)),
				  &frame_len,
				  veri_ctrl->veri_pkt_length - sizeof(HEADER_802_11),
				  veri_ctrl->veri_pkt_ctnt,
				  END_OF_ARGS);
	}
		break;
	case FC_TYPE_CNTL:
	{
		if (veri_ctrl->veri_pkt_subtype == SUBTYPE_BLOCK_ACK_REQ) {
			FRAME_BAR *bar_buf = (FRAME_BAR *)(buf + tx_hw_hdr_len);

			BarHeaderInit(ad,
				bar_buf,
				veri_ctrl->addr1,
				veri_ctrl->addr2);
			NdisCopyMemory((UCHAR *)&bar_buf->BarControl,
					veri_ctrl->veri_pkt_ctnt,
					veri_ctrl->veri_pkt_length - sizeof(PSPOLL_FRAME));
		} else if (veri_ctrl->veri_pkt_subtype == SUBTYPE_PS_POLL) {
			PSPOLL_FRAME *ps_poll_buf = (PSPOLL_FRAME *)(buf + tx_hw_hdr_len);
			USHORT aid = 0;

			NdisCopyMemory((UCHAR *)&aid,
					veri_ctrl->veri_pkt_ctnt,
					sizeof(USHORT));
			ComposePsPoll(ad, ps_poll_buf, aid, veri_ctrl->addr1, veri_ctrl->addr2);
			/* if the pkt is ps-poll,
			 * we need to mark the dur of pkt is controlled by sw,
			 * otherwise the field will be overwritten by hw to become hw duration.
			 */
			SET_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_DUR_CTRL_BY_SW);
		}
	}
		break;
	case FC_TYPE_DATA:
	{
		HEADER_802_11 wifi_hdr;
		HEADER_802_11 *buf_wifi_hdr = (HEADER_802_11 *)(buf + tx_hw_hdr_len);
		UCHAR wifi_hdr_length = sizeof(HEADER_802_11);

		NdisZeroMemory(&wifi_hdr, wifi_hdr_length);

		wifi_hdr.FC.Type = FC_TYPE_DATA;
		if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_NULL_DATA)) {
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_QOS_DATA))
				wifi_hdr.FC.SubType = SUBTYPE_QOS_NULL;
			else
				wifi_hdr.FC.SubType = SUBTYPE_DATA_NULL;
		} else {
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_QOS_DATA))
				wifi_hdr.FC.SubType = SUBTYPE_QDATA;
			else
				wifi_hdr.FC.SubType = SUBTYPE_DATA;
		}
		if (wdev->wdev_type == WDEV_TYPE_AP)
			wifi_hdr.FC.FrDs = 1;
		else if (wdev->wdev_type == WDEV_TYPE_STA)
			wifi_hdr.FC.ToDs = 1;

		COPY_MAC_ADDR(wifi_hdr.Addr1, veri_ctrl->addr1);
		COPY_MAC_ADDR(wifi_hdr.Addr2, veri_ctrl->addr2);
		COPY_MAC_ADDR(wifi_hdr.Addr3, veri_ctrl->addr3);

		if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_HTC_CTRL))
			wifi_hdr.FC.Order = 1;/*set HTC bit in frame control.*/

		NdisCopyMemory(buf_wifi_hdr,
				&wifi_hdr,
				wifi_hdr_length);

		if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_QOS_DATA)) {
			wifi_hdr_length += 2;
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_HTC_CTRL)) {
				NdisCopyMemory(((UCHAR *)buf_wifi_hdr) + wifi_hdr_length,
				(UCHAR *)&veri_ctrl->assign_ctrl.assigned_pkt_htc,
				sizeof(veri_ctrl->assign_ctrl.assigned_pkt_htc));
				wifi_hdr_length += 4;
			}
		}

		if (!(CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_NULL_DATA))) {
			NdisCopyMemory(((UCHAR *)buf_wifi_hdr) + wifi_hdr_length,
				veri_ctrl->veri_pkt_ctnt,
				veri_ctrl->veri_pkt_length - wifi_hdr_length);
		}
		hex_dump("802.11 data buf", (UCHAR *)buf_wifi_hdr, veri_ctrl->veri_pkt_length);
	}
		break;
	case VERI_PKT_DOT_3_TYPE:
	{
		UCHAR *eth_buf = buf;
		UINT32 offset = 0;

		NdisCopyMemory(eth_buf + offset,
				veri_ctrl->addr1,
				MAC_ADDR_LEN);
		offset += MAC_ADDR_LEN;
		NdisCopyMemory(eth_buf + offset,
				veri_ctrl->addr2,
				MAC_ADDR_LEN);
		offset += MAC_ADDR_LEN;
		NdisCopyMemory(eth_buf + offset,
				veri_ctrl->veri_pkt_ctnt,
				veri_ctrl->veri_pkt_length - offset);

		hex_dump("802.3 buf", eth_buf, veri_ctrl->veri_pkt_length);
	}
		break;
	default:
		/*shall not drop into here due to sanity check it before.*/
		MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" try to contruct wrong type:%d\n",
				veri_ctrl->veri_pkt_type);
		ret = FALSE;
	}

	return ret;
}

static BOOLEAN veri_pkt_ctnt_len_sanity_check(struct veri_ctrl *veri_ctrl, UCHAR tx_hw_hdr_len)
{
	BOOLEAN ret = TRUE;

	if ((veri_ctrl->veri_pkt_length + tx_hw_hdr_len) > MAX_LEN_OF_VERI_BUF) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" txd_size:%d pkt length:%d exceed:%d bytes\n",
				tx_hw_hdr_len,
				veri_ctrl->veri_pkt_length,
				MAX_LEN_OF_VERI_BUF);
		ret = FALSE;
	}

	return ret;
}

INT send_veri_pkt(struct _RTMP_ADAPTER *ad, char *arg)
{
	INT ret = NDIS_STATUS_SUCCESS;
	struct veri_ctrl *veri_ctrl = &ad->veri_ctrl;
	UCHAR *buf = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->TXWISize;
	PNDIS_PACKET pkt_skb = NULL;
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;

	POS_COOKIE pObj = (POS_COOKIE) ad->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(ad, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev == NULL) {
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	os_alloc_mem_suspend(ad, &buf, MAX_LEN_OF_VERI_BUF);
	if (!buf) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "can not allocate cmd buf\n");
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}
	os_zero_mem(buf, MAX_LEN_OF_VERI_BUF);

	if (CHECK_VERI_PKT_STATE(VERI_PKT_UPDATE_CTNT)) {
		MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" state error:%d\n", veri_ctrl->veri_pkt_state);
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	if (veri_pkt_ctnt_len_sanity_check(veri_ctrl, tx_hw_hdr_len) == FALSE) {
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	construct_veri_pkt(ad, wdev, veri_ctrl, tx_hw_hdr_len, buf);

	if (DOT_3_PACKET(veri_ctrl->veri_pkt_type))
		ret = RTMPAllocateNdisPacket(ad, &pkt_skb, NULL, 0, buf, veri_ctrl->veri_pkt_length);
	else
		ret = RTMPAllocateNdisPacket(ad, &pkt_skb, NULL, 0, buf, veri_ctrl->veri_pkt_length + tx_hw_hdr_len);

	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "AllocateNdisPacket fail\n");
		goto done;
	}

	RTMP_SET_PACKET_WDEV(pkt_skb, wdev->wdev_idx);

	RTMP_QueryPacketInfo(pkt_skb, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s:: allocate pkt, wdev_idx=%d, TotalPacketLength:%u, pkt_va:%p, VA:%p, Len:%u\n",
			  __func__, wdev->wdev_idx, PacketInfo.TotalPacketLength, pkt_skb, pSrcBufVA, SrcBufLen);

	if (DOT_3_PACKET(veri_ctrl->veri_pkt_type))
		wdev_tx_pkts((NDIS_HANDLE)ad, (PPNDIS_PACKET)&pkt_skb, 1, wdev);
	else {
		RTMP_SET_PACKET_TXTYPE(pkt_skb, TX_VERIFY_FRAME);
		RTMP_SET_PACKET_TYPE(pkt_skb, TX_MGMT);
		RTMP_SET_PACKET_QUEIDX(pkt_skb, 0);
		ret = send_mlme_pkt(ad, pkt_skb, wdev, 0, FALSE);
	}
done:
	if (buf)
		os_free_mem(buf);
	return ret;
}

static BOOLEAN ctnt_length_sanity_check(char *arg, UINT32 ctnt_length, UINT32 padding_to_length)
{
	BOOLEAN ret = TRUE;

	if ((padding_to_length > MAX_LEN_OF_VERI_BUF) || (ctnt_length > MAX_LEN_OF_VERI_BUF))
		ret = FALSE;

	if ((ctnt_length * 2) != strlen(arg))
		ret = FALSE;

	if (ret == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ctnt format error: padding_to_length:%d, ctnt_length:%d, strlen(ctnt):%d\n",
				padding_to_length, ctnt_length, (int)strlen(arg));
	}

	return ret;
}

INT set_veri_pkt_ctnt(struct _RTMP_ADAPTER *ad, char *arg)
{
#define MAX_INPUT_CONTENT 100
	INT ret = NDIS_STATUS_SUCCESS;
	UINT32 padding_to_length = 0, ctnt_length = 0, do_pad_len = 0;
	UINT32 input_argument = 0;
	/*UCHAR ctnt_str[MAX_INPUT_CONTENT] = {0};*/
	UINT32 padding_loop = 0;
	/*UCHAR padded_ctnt[MAX_LEN_OF_VERI_BUF] = {0};*/
	UCHAR *ctnt_str;
	UCHAR *padded_ctnt;

	os_alloc_mem_suspend(ad, &ctnt_str, MAX_INPUT_CONTENT);
	os_alloc_mem_suspend(ad, &padded_ctnt, MAX_LEN_OF_VERI_BUF);
	if (!ctnt_str || !padded_ctnt) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "can not allocate cmd buf\n");
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}
	os_zero_mem(ctnt_str, MAX_INPUT_CONTENT);
	os_zero_mem(ctnt_str, MAX_LEN_OF_VERI_BUF);
	if (arg) {
		input_argument = sscanf(arg,
					"len:%d-ctnt:%100s",
					&padding_to_length, ctnt_str);
		if (input_argument != 2) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Invalid format, %s ignored!\n", arg);
			ret = NDIS_STATUS_FAILURE;
			goto done;
		}

		ctnt_length = strlen(ctnt_str) / 2;
		if (ctnt_length_sanity_check(ctnt_str, ctnt_length, padding_to_length) == FALSE) {
			show_set_pkt_ctnt_format();
			ret = NDIS_STATUS_FAILURE;
			goto done;
		}

		/* for safety,
		 * if the length of input content exceed the length we would like to padding,
		 * we honor the length of content.
		 */
		if (ctnt_length >= padding_to_length) {
			padding_to_length = ctnt_length;
			MTWF_PRINT("%s: honor ctnt_len\n", __func__);
		}

		do_pad_len = padding_to_length;

		while (do_pad_len) {
			AtoH(ctnt_str, padded_ctnt + (padding_loop * ctnt_length), ctnt_length);
			padding_loop++;
			if (do_pad_len >= ctnt_length)
				do_pad_len = do_pad_len - ctnt_length;
			else
				do_pad_len = ctnt_length;
		}

		if (prepare_veri_pkt_ctnt(ad, padded_ctnt, padding_to_length) == FALSE)
			ret = NDIS_STATUS_FAILURE;
	}
done:
	if (ctnt_str)
		os_free_mem(ctnt_str);
	if (padded_ctnt)
		os_free_mem(padded_ctnt);
#undef MAX_INPUT_CONTENT
	return ret;
}

static INT veri_pkt_head_sanity_check(INT pkt_type,
				      INT pkt_subtype,
				      UINT32 input_argument,
				      UCHAR *addr1_str,
				      UCHAR *addr2_str,
				      UCHAR *addr3_str)
{
	INT ret = NDIS_STATUS_SUCCESS;

	if (pkt_type > FC_TYPE_RSVED) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pkt_type error:%d\n", pkt_type);
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	if (addr1_str) {
		if ((strlen(addr1_str) != 17) && (strlen(addr1_str) != 0)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"len addr1_str is incorrect:%d\n", (int)strlen(addr1_str));
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}
	}

	if (addr2_str) {
		if ((strlen(addr2_str) != 17) && (strlen(addr2_str) != 0)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"len addr2_str is incorrect:%d\n", (int)strlen(addr2_str));
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}
	}

	if (addr3_str) {
		if ((strlen(addr3_str) != 17) && (strlen(addr3_str) != 0)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"len addr3_str is incorrect:%d\n", (int)strlen(addr3_str));
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}
	}

	if (pkt_type == FC_TYPE_CNTL) {
		if ((pkt_subtype != SUBTYPE_BLOCK_ACK_REQ) &&
		    (pkt_subtype != SUBTYPE_PS_POLL) &&
		    (pkt_subtype != SUBTYPE_RTS)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"ctrl type only support BAR/RTS/Ps-Poll pkt_subtype:%d\n", pkt_subtype);
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}

		if (input_argument > 4) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"the head of ctrl_type pkt need TA/RA only.\n");
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}
	} else if (pkt_type == FC_TYPE_MGMT) {
		if ((input_argument != 5) || (addr1_str == NULL) || (addr2_str == NULL) || (addr3_str == NULL)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"mgmt format error: input_argument:%d, addr1_str:%s, addr2_str:%s, addr3_str:%s\n",
				input_argument, addr1_str, addr2_str, addr3_str);
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}
	} else if (pkt_type == FC_TYPE_DATA) {
		if ((pkt_subtype != SUBTYPE_DATA) &&
		    (pkt_subtype != SUBTYPE_DATA_NULL) &&
		    (pkt_subtype != SUBTYPE_QDATA) &&
		    (pkt_subtype != SUBTYPE_QOS_NULL)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"verify 802.11 Data support DATA/NULL DATA/QoS DATA/QoS Null only\n");
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}

		if ((input_argument != 5) || (addr1_str == NULL) || (addr2_str == NULL) || (addr3_str == NULL)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"802.11 data format error: addr1_str:%s, addr2_str:%s, addr3_str:%s\n",
					addr1_str, addr2_str, addr3_str);
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}
	} else if (pkt_type == VERI_PKT_DOT_3_TYPE) {
		if ((input_argument >= 5) || (addr1_str == NULL) || (addr2_str == NULL)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"dot3 data format error: addr1_str:%s, addr2_str:%s\n",
					addr1_str, addr2_str);
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}
	}
err:
	return ret;
}

static VOID reset_veri_struct_key_para(struct _RTMP_ADAPTER *ad)
{
	struct veri_ctrl *veri_ctrl = &ad->veri_ctrl;

	UPDATE_VERI_PKT_STATE(VERI_PKT_PREPARE_INIT);
	veri_ctrl->veri_pkt_type = 0;
	veri_ctrl->veri_pkt_subtype = 0;
	veri_ctrl->veri_pkt_length = 0;
	veri_ctrl->veri_pkt_ctrl_map = 0;

	NdisZeroMemory(veri_ctrl->addr1, MAC_ADDR_LEN);
	NdisZeroMemory(veri_ctrl->addr2, MAC_ADDR_LEN);
	NdisZeroMemory(veri_ctrl->addr3, MAC_ADDR_LEN);
	NdisZeroMemory(veri_ctrl->veri_pkt_ctnt, MAX_LEN_OF_VERI_BUF);
}

INT prepare_veri_pkt_ctnt(struct _RTMP_ADAPTER *ad, UCHAR *padded_ctnt, UINT32 ctnt_length)
{
	INT ret = TRUE;
	struct veri_ctrl *veri_ctrl = &ad->veri_ctrl;

	if (CHECK_VERI_PKT_STATE(VERI_PKT_UPDATE_HEAD)) {
		MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" state error:%d update pkt head first\n", veri_ctrl->veri_pkt_state);
		ret = FALSE;
		return ret;
	}

	/*PS POLL can assign AID, but the pkt_len doesn't change.*/
	if (!((veri_ctrl->veri_pkt_type == FC_TYPE_CNTL) &&
		(veri_ctrl->veri_pkt_subtype == SUBTYPE_PS_POLL)))
		veri_ctrl->veri_pkt_length += ctnt_length;

	NdisZeroMemory(veri_ctrl->veri_pkt_ctnt, MAX_LEN_OF_VERI_BUF);
	NdisMoveMemory(veri_ctrl->veri_pkt_ctnt, padded_ctnt, ctnt_length);
	UPDATE_VERI_PKT_STATE(VERI_PKT_UPDATE_CTNT);

	return ret;
}

INT prepare_veri_pkt_ctrl_assign(struct _RTMP_ADAPTER *ad, struct veri_designated_ctrl *assign_ctrl_input)
{
	INT ret = TRUE;
	struct veri_ctrl *veri_ctrl = &ad->veri_ctrl;
	struct veri_designated_ctrl *veri_assign_ctrl = &veri_ctrl->assign_ctrl;

	NdisMoveMemory(veri_assign_ctrl, assign_ctrl_input, sizeof(struct veri_designated_ctrl));

	return ret;
}

INT prepare_veri_pkt_ctrl_en(struct _RTMP_ADAPTER *ad, UINT32 pkt_ctrl_map_input)
{
	INT ret = TRUE;
	struct veri_ctrl *veri_ctrl = &ad->veri_ctrl;

	/*keep the original ctrl_map, which may have content already due to set_pkt_head.*/
	veri_ctrl->veri_pkt_ctrl_map |= pkt_ctrl_map_input;
	return ret;
}

INT prepare_veri_pkt_head(struct _RTMP_ADAPTER *ad, struct veri_app_head_input *head_input)
{
	INT ret = TRUE;
	struct veri_ctrl *veri_ctrl = &ad->veri_ctrl;
	INT pkt_type = head_input->veri_pkt_type;
	INT pkt_subtype = head_input->veri_pkt_subtype;

	reset_veri_struct_key_para(ad);

	veri_ctrl->veri_pkt_type = pkt_type;
	veri_ctrl->veri_pkt_subtype = pkt_subtype;
	COPY_MAC_ADDR(veri_ctrl->addr1, head_input->addr1);
	COPY_MAC_ADDR(veri_ctrl->addr2, head_input->addr2);
	COPY_MAC_ADDR(veri_ctrl->addr3, head_input->addr3);

	/*the basic length of pkt.*/
	if ((pkt_type == FC_TYPE_MGMT) || (pkt_type == FC_TYPE_DATA))
		veri_ctrl->veri_pkt_length = sizeof(HEADER_802_11);
	else if (pkt_type == FC_TYPE_CNTL) {
		/*we only support PS_POLL/BAR at present. the base length of them are the same.*/
		veri_ctrl->veri_pkt_length = sizeof(HEADER_PS_POLL);
	} else if (pkt_type == VERI_PKT_DOT_3_TYPE) {
		SET_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_HDR_TRANS);
		veri_ctrl->veri_pkt_length = 12;/*2 mac addr length.*/
	}

	UPDATE_VERI_PKT_STATE(VERI_PKT_UPDATE_HEAD);

	if (pkt_type == FC_TYPE_DATA) {
		if ((pkt_subtype == SUBTYPE_QDATA) ||
			(pkt_subtype == SUBTYPE_QOS_NULL)) {
			SET_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_QOS_DATA);
			veri_ctrl->veri_pkt_length += 2;
		} else
			CLEAR_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_QOS_DATA);

		if ((pkt_subtype == SUBTYPE_DATA_NULL) ||
			(pkt_subtype == SUBTYPE_QOS_NULL)) {
			SET_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_NULL_DATA);
			/*Null Data frame no need to assign ctnt.*/
			UPDATE_VERI_PKT_STATE(VERI_PKT_UPDATE_CTNT);
		} else
			CLEAR_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_NULL_DATA);
	}
	return ret;
}

INT set_veri_pkt_head(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
#define MAC_INPUT_FORMAT_LEN 18
	INT ret = TRUE;

	UINT32 input_argument = 0;
	INT pkt_type = 0;
	INT pkt_subtype = 0;
	INT i;
	UCHAR addr1_str[MAC_INPUT_FORMAT_LEN] = {0};/*in format of 00:11:22:33:44:55 */
	UCHAR addr2_str[MAC_INPUT_FORMAT_LEN] = {0};/*in format of 00:11:22:33:44:55 */
	UCHAR addr3_str[MAC_INPUT_FORMAT_LEN] = {0};/*in format of 00:11:22:33:44:55 */
	RTMP_STRING *value;
	struct veri_app_head_input head_input, *phead_input = &head_input;

	NdisZeroMemory(phead_input, sizeof(struct veri_app_head_input));

	if (arg) {
		input_argument = sscanf(arg,
					"%d-%d-%17s-%17s-%17s",
					&pkt_type, &pkt_subtype, addr1_str, addr2_str, addr3_str);

		if (5 != input_argument) {
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "input argument format error\n");
			ret = FALSE;
			goto print_format;
		}

		addr1_str[MAC_INPUT_FORMAT_LEN - 1] = '\0';
		addr2_str[MAC_INPUT_FORMAT_LEN - 1] = '\0';
		addr3_str[MAC_INPUT_FORMAT_LEN - 1] = '\0';

		if (veri_pkt_head_sanity_check(pkt_type,
					       pkt_subtype,
					       input_argument,
					       addr1_str,
					       addr2_str,
					       addr3_str) == NDIS_STATUS_FAILURE) {
			ret = FALSE;
			goto print_format;
		}

		MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"pkt_type:%d, pkt_subtype:0x%x, addr1_str:%17s, addr2_str:%17s, addr3_str:%17s\n",
				pkt_type, pkt_subtype, addr1_str, addr2_str, addr3_str);

		phead_input->veri_pkt_type = pkt_type;
		phead_input->veri_pkt_subtype = pkt_subtype;

		for (i = 0, value = rstrtok(addr1_str, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				ret = FALSE;
				goto print_format;
			}
			AtoH(value, (UCHAR *)&phead_input->addr1[i++], 1);
		}

		for (i = 0, value = rstrtok(addr2_str, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				ret = FALSE;
				goto print_format;
			}
			AtoH(value, (UCHAR *)&phead_input->addr2[i++], 1);
		}

		for (i = 0, value = rstrtok(addr3_str, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				ret = FALSE;
				goto print_format;
			}
			AtoH(value, (UCHAR *)&phead_input->addr3[i++], 1);
		}

		prepare_veri_pkt_head(ad, phead_input);
	}

print_format:
	show_cmd_format();
#undef MAC_INPUT_FORMAT_LEN
	return ret;
}

static BOOLEAN veri_fill_offload_tx_blk(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;

	pPacket = pTxBlk->pPacket;
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);

	pTxBlk->wmm_set = HcGetWmmIdx(ad, wdev);
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	return TRUE;
}

INT32 verify_pkt_tx(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _TX_BLK *tx_blk)
{
	PQUEUE_ENTRY q_entry;
	UCHAR *tmac_info, *frm_buf;
	UINT frm_len;
	PHEADER_802_11 pHeader_802_11 = NULL;
	UINT16 wcid = tx_blk->Wcid, tx_rate;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	HTTRANSMIT_SETTING *transmit, transmit_setting;
	MAC_TX_INFO mac_info;
	struct dev_rate_info *rate;
	struct veri_ctrl *veri_ctrl = &ad->veri_ctrl;
	struct veri_designated_ctrl *assign_ctrl = &veri_ctrl->assign_ctrl;
	struct wifi_dev_ops *wdev_ops;

	if (wdev == NULL)
		return NDIS_STATUS_FAILURE;

	wdev_ops = wdev->wdev_ops;
	q_entry = RemoveHeadQueue(&tx_blk->TxPacketList);
	tx_blk->pPacket = QUEUE_ENTRY_TO_PACKET(q_entry);
	veri_fill_offload_tx_blk(ad, wdev, tx_blk);
	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

	if (!DOT_3_PACKET(veri_ctrl->veri_pkt_type)) {
		pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);

		if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_SEQ_CTRL_BY_SW))
			pHeader_802_11->Sequence = assign_ctrl->assigned_seq;

		if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_PM_CTRL_BY_SW))
			pHeader_802_11->FC.PwrMgmt = assign_ctrl->assigned_pm;

		if (veri_ctrl->veri_pkt_subtype != SUBTYPE_PS_POLL) {
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_DUR_CTRL_BY_SW))
				pHeader_802_11->Duration = assign_ctrl->assigned_dur;
		}

		mac_info.Type = pHeader_802_11->FC.Type;
		mac_info.SubType = pHeader_802_11->FC.SubType;
	}

	rate = &wdev->rate;
	frm_buf = tx_blk->pSrcBufHeader;
	frm_len = tx_blk->SrcBufLen;
	tmac_info = tx_blk->pSrcBufHeader;

	wcid = RTMP_GET_PACKET_WCID(tx_blk->pPacket);

	tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
	/*transmit = &rate->MlmeTransmit;*/
	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.AMPDU = FALSE;
	mac_info.BM = IS_BM_MAC_ADDR(veri_ctrl->addr1);
	mac_info.BASize = 0;
	mac_info.WCID = wcid;
	mac_info.TID = 0;
	mac_info.wmm_set = HcGetWmmIdx(ad, wdev);
	mac_info.q_idx  = HcGetMgmtQueueIdx(ad, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
	mac_info.OmacIdx = wdev->OmacIdx;
	mac_info.Length = (tx_blk->SrcBufLen - tx_hw_hdr_len);

	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_NA))
		mac_info.Ack = FALSE;
	else
		mac_info.Ack = TRUE;

	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_TM))
		mac_info.IsTmr = TRUE;

	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_PM_CTRL_BY_SW))
		mac_info.PsmBySw = TRUE;

	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_DUR_CTRL_BY_SW))
		mac_info.sw_duration = TRUE;

	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_HTC_CTRL))
		mac_info.htc = TRUE;

	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_SEQ_CTRL_BY_SW)) {
		mac_info.NSeq = TRUE;
		mac_info.assigned_seq = assign_ctrl->assigned_seq;
	}

	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_TXS2M))
		mac_info.txs2m = TRUE;

	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_TXS2H))
		mac_info.txs2h = TRUE;

	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_LIFETIME_CTRL))
		mac_info.tx_lifetime = assign_ctrl->assigned_pkt_lifetime;

	if (veri_ctrl->veri_pkt_type == FC_TYPE_MGMT) {
		mac_info.hdr_len = sizeof(HEADER_802_11);
		if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_HTC_CTRL))
			mac_info.hdr_len = sizeof(HEADER_802_11) + 4;

	} else if (VERI_CTRL_TYPE_PACKET(veri_ctrl->veri_pkt_type, veri_ctrl->veri_pkt_subtype))
		mac_info.hdr_len = 16;
	else if (veri_ctrl->veri_pkt_type == FC_TYPE_DATA) {
		if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_QOS_DATA)) {
			mac_info.hdr_len = sizeof(HEADER_802_11) + 2;
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_HTC_CTRL))
				mac_info.hdr_len += 4;
		} else
			mac_info.hdr_len = sizeof(HEADER_802_11);
	}

	mac_info.PID = assign_ctrl->assigned_pid;
	mac_info.TxRate = tx_rate;
	mac_info.Txopmode = IFS_BACKOFF;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;
	mac_info.txpwr_offset = 0;

	if (!WMODE_CAP_2G(wdev->PhyMode))
		transmit_setting.field.MODE = MODE_OFDM;
	else
		transmit_setting.field.MODE = MODE_CCK;

	/* HTC pkt shall use the rate which is above HT.*/
	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_HTC_CTRL))
		transmit_setting.field.MODE = MODE_HTMIX;

	transmit_setting.field.BW = BW_20;
	transmit_setting.field.STBC = 0;
	transmit_setting.field.ShortGI = 0;
	transmit_setting.field.MCS = 0;
	transmit_setting.field.ldpc = 0;
	transmit = &transmit_setting;

	if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_HDR_TRANS)) {
		TX_BLK_SET_FLAG(tx_blk, fTX_HDR_TRANS);
		wdev_ops->ieee_802_3_data_tx(ad, wdev, tx_blk);
		return asic_hw_tx(ad, tx_blk);
	} else {
		return asic_mlme_hw_tx(ad, tmac_info, &mac_info, transmit, tx_blk);
	}
}

INT set_veri_pkt_ctrl_assign(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	INT ret = TRUE;
	struct veri_ctrl *veri_ctrl = &ad->veri_ctrl;

	RTMP_STRING *assign_dur_str = NULL;
	RTMP_STRING *assign_seq_str = NULL;
	RTMP_STRING *assign_pm_str = NULL;
	RTMP_STRING *assign_pid_str = NULL;
	RTMP_STRING *assign_lifetime_str = NULL;
	RTMP_STRING *assign_htc_str = NULL;
	CHAR *pch = NULL;
	UINT32 setting_value = 0;
	struct veri_designated_ctrl assign_ctrl_input;

	NdisZeroMemory(&assign_ctrl_input, sizeof(struct veri_designated_ctrl));
	if (arg) {
		assign_dur_str = strstr(arg, "du:");
		if (assign_dur_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg du:\n");
		else {
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_DUR_CTRL_BY_SW)) {
				pch = strchr(assign_dur_str, ':');
				if (pch == NULL)
					MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in htc string\n");
				else {
					setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);
					NdisCopyMemory(&assign_ctrl_input.assigned_dur,
						       &setting_value,
						       sizeof(assign_ctrl_input.assigned_dur));
				}
			} else
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"enable dur control in first\n");
		}

		assign_seq_str = strstr(arg, "sn:");
		if (assign_seq_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg sn:\n");
		else {
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_SEQ_CTRL_BY_SW)) {
				pch = strchr(assign_seq_str, ':');
				if (pch == NULL)
					MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in htc string\n");
				else {
					setting_value = (USHORT)os_str_tol(pch + 1, 0, 10);

					if (setting_value < 4096) /*sanity check*/
						NdisCopyMemory(&assign_ctrl_input.assigned_seq,
							       &setting_value,
							       sizeof(assign_ctrl_input.assigned_seq));
					else {
						MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"seq:%u over spec\n", setting_value);
						ret = FALSE;
						return ret;
					}
				}
			} else
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"enable sn control in first\n");
		}

		assign_pid_str = strstr(arg, "pid:");
		if (assign_pid_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg pid:\n");
		else {
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_TXS2H) ||
			    CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_TXS2M)) {
				pch = strchr(assign_pid_str, ':');
				if (pch == NULL)
					MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in htc string\n");
				else {
					setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);

					if (setting_value < 256) /*sanity check*/
						NdisCopyMemory(&assign_ctrl_input.assigned_pid,
							       &setting_value,
							       sizeof(assign_ctrl_input.assigned_pid));
					else {
						MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"seq:%u over spec\n", setting_value);
						ret = FALSE;
						return ret;
					}
				}
			} else
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"enable txs2m/txs2h control in first\n");
		}


		assign_pm_str = strstr(arg, "pm:");
		if (assign_pm_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg pm:\n");
		else {
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_PM_CTRL_BY_SW)) {
				pch = strchr(assign_pm_str, ':');
				if (pch == NULL)
					MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in htc string\n");
				else {
					setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);

					if (setting_value <= 1) /*sanity check*/
						NdisCopyMemory(&assign_ctrl_input.assigned_pm,
							       &setting_value,
							       sizeof(assign_ctrl_input.assigned_pm));
					else {
						MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"pm:%u over spec\n", setting_value);
						ret = FALSE;
						return ret;
					}
				}
			} else
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"enable pm control in first\n");
		}


		assign_lifetime_str = strstr(arg, "life:");
		if (assign_lifetime_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg life:\n");
		else {
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_LIFETIME_CTRL)) {
				pch = strchr(assign_lifetime_str, ':');
				if (pch == NULL)
					MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in htc string\n");
				else {
					setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);

					if (setting_value <= 255)/*sanity check*/
						NdisCopyMemory(&assign_ctrl_input.assigned_pkt_lifetime,
							       &setting_value,
							       sizeof(assign_ctrl_input.assigned_pkt_lifetime));
					else {
						MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"pkt_lifetime:%u over spec\n", setting_value);
						ret = FALSE;
						return ret;
					}
				}
			} else
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"enable assigned_pkt_lifetime control in first\n");
		}

		assign_htc_str = strstr(arg, "htc:");
		if (assign_htc_str == NULL)
					MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"can not find arg htc:\n");
		else {
			if (CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_HTC_CTRL)) {
				pch = strchr(assign_htc_str, ':');
				if (pch == NULL)
					MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in htc string\n");
				else {
					setting_value = (UINT32)os_str_tol(pch + 1, 0, 16);
					NdisCopyMemory(&assign_ctrl_input.assigned_pkt_htc,
							   &setting_value,
							   sizeof(assign_ctrl_input.assigned_pkt_htc));
				}
			} else
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"enable htc control in first\n");
		}

		prepare_veri_pkt_ctrl_assign(ad, &assign_ctrl_input);
	}

	return ret;
}

static VOID print_veri_ctrl_en_status(struct _RTMP_ADAPTER *ad)
{
	struct veri_ctrl *veri_ctrl = &ad->veri_ctrl;

	MTWF_PRINT("verify_pkt ctrl_en status:\n");
	MTWF_PRINT("\tdu:\tno_ack:\ttm:\tsn:\ttxs2m:\ttxs2h:\tpm:\thtc:\n");
	MTWF_PRINT("\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
			CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_DUR_CTRL_BY_SW) ? 1 : 0,
			CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_NA) ? 1 : 0,
			CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_TM) ? 1 : 0,
			CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_SEQ_CTRL_BY_SW) ? 1 : 0,
			CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_TXS2M) ? 1 : 0,
			CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_TXS2H) ? 1 : 0,
			CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_PM_CTRL_BY_SW) ? 1 : 0,
			CHECK_VERI_PKT_CTRL_IDX(veri_ctrl->veri_pkt_ctrl_map, VERI_HTC_CTRL) ? 1 : 0);
}

INT set_veri_pkt_ctrl_en(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	INT ret = TRUE;

	RTMP_STRING *du_ctrl_str = NULL;
	RTMP_STRING *na_ctrl_str = NULL;
	RTMP_STRING *tm_ctrl_str = NULL;
	RTMP_STRING *sn_ctrl_str = NULL;
	RTMP_STRING *txs2m_ctrl_str = NULL;
	RTMP_STRING *txs2h_ctrl_str = NULL;
	RTMP_STRING *pm_ctrl_str = NULL;
	RTMP_STRING *lifetime_ctrl_str = NULL;
	RTMP_STRING *htc_ctrl_str = NULL;
	CHAR *pch = NULL;
	UCHAR setting_value = 0;
	UINT32 pkt_ctrl_map_input = 0;
	struct veri_ctrl *veri_ctrl = &ad->veri_ctrl;

	if (arg) {
		du_ctrl_str = strstr(arg, "du:");
		if (du_ctrl_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg du:\n");
		else {
			pch = strchr(du_ctrl_str, ':');
			if (pch == NULL)
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in du string\n");
			else {
				setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);
				if (setting_value)
					SET_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_DUR_CTRL_BY_SW);
				else
					CLEAR_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_DUR_CTRL_BY_SW);
			}
		}

		na_ctrl_str = strstr(arg, "na:");
		if (na_ctrl_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg na:\n");
		else {
			pch = strchr(na_ctrl_str, ':');
			if (pch == NULL)
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in na string\n");
			else {
				setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);
				if (setting_value)
					SET_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_NA);
				else
					CLEAR_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_NA);
			}
		}

		tm_ctrl_str = strstr(arg, "tm:");
		if (tm_ctrl_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg tm:\n");
		else {
			pch = strchr(tm_ctrl_str, ':');
			if (pch == NULL)
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in tm string\n");
			else {
				setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);
				if (setting_value)
					SET_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_TM);
				else
					CLEAR_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_TM);
			}
		}

		sn_ctrl_str = strstr(arg, "sn:");
		if (sn_ctrl_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg sn:\n");
		else {
			pch = strchr(sn_ctrl_str, ':');
			if (pch == NULL)
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in sn string\n");
			else {
				setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);
				if (setting_value)
					SET_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_SEQ_CTRL_BY_SW);
				else
					CLEAR_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_SEQ_CTRL_BY_SW);
			}
		}

		txs2m_ctrl_str = strstr(arg, "txs2m:");
		if (txs2m_ctrl_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg sn:\n");
		else {
			pch = strchr(txs2m_ctrl_str, ':');
			if (pch == NULL)
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in txs2m string\n");
			else {
				setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);
				if (setting_value)
					SET_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_TXS2M);
				else
					CLEAR_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_TXS2M);
			}
		}

		txs2h_ctrl_str = strstr(arg, "txs2h:");
		if (txs2h_ctrl_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg txs2h:\n");
		else {
			pch = strchr(txs2h_ctrl_str, ':');
			if (pch == NULL)
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in txs2h string\n");
			else {
				setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);
				if (setting_value)
					SET_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_TXS2H);
				else
					CLEAR_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_TXS2H);
			}
		}

		pm_ctrl_str = strstr(arg, "pm:");
		if (pm_ctrl_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg pm:\n");
		else {
			pch = strchr(pm_ctrl_str, ':');
			if (pch == NULL)
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in pm string\n");
			else {
				setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);
				if (setting_value)
					SET_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_PM_CTRL_BY_SW);
				else
					CLEAR_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_PM_CTRL_BY_SW);
			}
		}

		lifetime_ctrl_str = strstr(arg, "life:");
		if (lifetime_ctrl_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg pm:\n");
		else {
			pch = strchr(lifetime_ctrl_str, ':');
			if (pch == NULL)
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in lifetime string\n");
			else {
				setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);
				if (setting_value)
					SET_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_LIFETIME_CTRL);
				else
					CLEAR_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_LIFETIME_CTRL);
			}
		}

		htc_ctrl_str = strstr(arg, "htc:");
		if (htc_ctrl_str == NULL)
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"can not find arg htc:\n");
		else {
			pch = strchr(htc_ctrl_str, ':');
			if (pch == NULL)
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"can not find arg : in htc string\n");
			else {
				setting_value = (UCHAR)os_str_tol(pch + 1, 0, 10);
				if (setting_value) {
					SET_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_HTC_CTRL);
					veri_ctrl->veri_pkt_length += 4;/*if assign HTC, header extend 4 bytes.*/
				} else
					CLEAR_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_HTC_CTRL);
			}
		}

		prepare_veri_pkt_ctrl_en(ad, pkt_ctrl_map_input);
	}

	print_veri_ctrl_en_status(ad);

	return ret;
}

INT veri_mode_switch(struct _RTMP_ADAPTER *ad, char *arg)
{
	ad->veri_ctrl.verify_mode_on = os_str_tol(arg, 0, 10);
	MTWF_PRINT("set verify_mode_on = %d\n", ad->veri_ctrl.verify_mode_on);
	return TRUE;
}

UCHAR group19_generator_x[] = {
	0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47,
	0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40, 0xF2,
	0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 0xA0,
	0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96
};

UCHAR group19_generator_y[] = {
	0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B,
	0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 0x16,
	0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE,
	0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5
};

UCHAR group19_priv_pettern[20][32] = {
	{0x9D, 0x2E, 0x82, 0xFB, 0xC5, 0x12, 0x86, 0xB5,
	 0xE5, 0xB3, 0x5E, 0x1F, 0x5A, 0xC4, 0x9E, 0x63,
	 0x6B, 0xBD, 0x8B, 0x56, 0x34, 0x29, 0x6E, 0xEA,
	 0x68, 0x7B, 0x19, 0xBD, 0xD5, 0xE8, 0xFB, 0xDD},
	{0x01, 0x32, 0xAE, 0xF5, 0xF0, 0x6B, 0xF7, 0xCE,
	 0x41, 0xA3, 0x7E, 0x5D, 0x84, 0x84, 0xB1, 0x51,
	 0x09, 0x5B, 0x2E, 0xB8, 0x34, 0x07, 0x64, 0xE4,
	 0x54, 0x3F, 0x80, 0x1B, 0x92, 0xEF, 0xE3, 0xFA},
	{0x28, 0x0A, 0x89, 0xA1, 0xE7, 0x2E, 0xA3, 0xC4,
	 0x78, 0xD1, 0x03, 0xD6, 0x13, 0xA8, 0x01, 0x9C,
	 0x8B, 0x13, 0x11, 0xCA, 0x2E, 0x89, 0xA9, 0xA8,
	 0x4D, 0xE7, 0x8E, 0x5A, 0x7B, 0x17, 0x82, 0xA7},
	{0x80, 0xCA, 0xEA, 0xFF, 0xD1, 0xFD, 0x22, 0x33,
	 0xAC, 0x32, 0x30, 0xC8, 0xD1, 0xD5, 0x88, 0xA8,
	 0x9E, 0x51, 0x9E, 0x6D, 0x75, 0x91, 0x4C, 0xBE,
	 0xE8, 0x40, 0xF5, 0x3A, 0xA0, 0xE5, 0xB5, 0x34},
	{0x9D, 0xAA, 0xEB, 0x45, 0x82, 0x27, 0xE5, 0xA4,
	 0xCE, 0x7F, 0x46, 0x7C, 0x40, 0x37, 0xCE, 0x7F,
	 0x33, 0x1E, 0xD4, 0x94, 0x31, 0x78, 0x31, 0x68,
	 0x38, 0x13, 0xEE, 0x9B, 0xDD, 0x40, 0xFE, 0xFE},
	{0xD6, 0x99, 0xBB, 0x84, 0xE8, 0xB8, 0xFC, 0xDF,
	 0xD1, 0xAD, 0xE4, 0x3A, 0xB5, 0x44, 0x34, 0x2B,
	 0xE0, 0x57, 0xC0, 0xE0, 0xCD, 0x7E, 0x3B, 0x92,
	 0x8C, 0xED, 0x35, 0x35, 0x7C, 0xB9, 0xA1, 0xA1},
	{0x28, 0xB6, 0x00, 0x26, 0x52, 0x17, 0x81, 0xBE,
	 0xE8, 0xF1, 0x11, 0xF7, 0x89, 0x9A, 0x3B, 0x88,
	 0xFE, 0x53, 0xCD, 0x35, 0xCE, 0xDA, 0xF9, 0x4B,
	 0x18, 0xCA, 0x0E, 0xD9, 0x9C, 0x5D, 0xC1, 0x71},
	{0xCC, 0x32, 0x36, 0x72, 0x3D, 0xB8, 0x39, 0xA9,
	 0x16, 0x73, 0x65, 0xD1, 0x48, 0x08, 0xA1, 0x2D,
	 0xAD, 0xBE, 0xFB, 0x50, 0x43, 0xC8, 0x68, 0x9F,
	 0xA5, 0xFA, 0xF5, 0x96, 0xEC, 0x3C, 0x71, 0x15},
	{0xD4, 0x58, 0x91, 0x4C, 0x09, 0x8E, 0x21, 0xEA,
	 0x25, 0x7D, 0x03, 0x6A, 0xD4, 0x18, 0xC0, 0x6D,
	 0x2E, 0x7B, 0x8F, 0x75, 0x6F, 0x7F, 0xE0, 0x58,
	 0xD6, 0x69, 0xEA, 0xC9, 0x3B, 0xBA, 0xFA, 0x2D},
	{0x56, 0x67, 0xA6, 0x7C, 0x23, 0xD5, 0xBA, 0x70,
	 0xAB, 0xE1, 0x89, 0xE1, 0x1E, 0x12, 0xD9, 0xB0,
	 0x5F, 0x24, 0x6D, 0xFD, 0x56, 0x92, 0xB1, 0x4C,
	 0xEC, 0x6C, 0x22, 0x78, 0xF5, 0xD1, 0xF5, 0x29},
	{0xFC, 0x3C, 0x3C, 0x94, 0xB7, 0x95, 0xA6, 0x23,
	 0x0C, 0x0E, 0x52, 0xAE, 0x38, 0x52, 0x85, 0xA9,
	 0xE0, 0x13, 0x37, 0x60, 0x21, 0x79, 0x20, 0x09,
	 0x48, 0xFF, 0x72, 0x07, 0xB4, 0xDD, 0x9D, 0xA5},
	{0x4E, 0x5A, 0x8D, 0x5B, 0x03, 0x13, 0x9C, 0x4C,
	 0xC7, 0x5B, 0x48, 0x38, 0x28, 0x66, 0x90, 0xD7,
	 0x33, 0x96, 0x3F, 0xE1, 0x8B, 0xF0, 0xA7, 0xA4,
	 0x63, 0xA5, 0x5E, 0x6A, 0x78, 0x91, 0xC6, 0xDD},
	{0xB6, 0x94, 0xD3, 0x20, 0x13, 0x33, 0x18, 0xC5,
	 0x77, 0xEC, 0x6B, 0x3F, 0x26, 0x8A, 0x22, 0xB6,
	 0xB3, 0x29, 0x1B, 0xC1, 0xE5, 0x91, 0xEE, 0x69,
	 0x7B, 0x25, 0x92, 0x9A, 0x60, 0x36, 0xB5, 0xA0},
	{0x88, 0x29, 0x6C, 0x43, 0x89, 0x8B, 0xC0, 0x5B,
	 0xCF, 0x4D, 0x35, 0xD2, 0xD4, 0xE6, 0x23, 0xE7,
	 0xF4, 0x2A, 0x2B, 0xC9, 0x55, 0xD9, 0x9D, 0xCD,
	 0xFE, 0x85, 0x82, 0xB4, 0x2A, 0x63, 0xCA, 0x8D},
	{0x7B, 0x90, 0xBD, 0x79, 0x3C, 0x05, 0x16, 0x5A,
	 0x6B, 0x35, 0x79, 0x5B, 0x7B, 0x9A, 0x08, 0x9A,
	 0x16, 0x33, 0xFD, 0xC7, 0x17, 0x21, 0x7B, 0xA4,
	 0xD1, 0xC2, 0x03, 0x88, 0x1C, 0x60, 0x35, 0xF8},
	{0xE7, 0x0D, 0x3E, 0xD5, 0x77, 0xFD, 0x2F, 0x1F,
	 0x78, 0xFC, 0xF5, 0x03, 0x8D, 0xD1, 0xEB, 0xAB,
	 0xE0, 0x32, 0x76, 0x92, 0x26, 0x20, 0x43, 0xE1,
	 0xBC, 0x4A, 0x13, 0x9A, 0xF3, 0x6E, 0x3C, 0x6A},
	{0x31, 0xD6, 0x00, 0x21, 0x0F, 0x22, 0x8A, 0x23,
	 0xAD, 0x9B, 0x59, 0x97, 0x43, 0xF1, 0xA2, 0x42,
	 0xE4, 0x19, 0x3B, 0x08, 0x1B, 0x45, 0x7B, 0x9C,
	 0x84, 0xC3, 0x3A, 0x8D, 0xBB, 0x6A, 0x9A, 0x73},
	{0x03, 0xAA, 0xF5, 0xC1, 0xA3, 0x3A, 0x6C, 0xDB,
	 0xBC, 0xAA, 0xA8, 0x58, 0x06, 0x01, 0x78, 0x3D,
	 0x31, 0x07, 0x60, 0x92, 0x35, 0x88, 0xC2, 0x95,
	 0xD8, 0xC4, 0xA6, 0x01, 0xA2, 0xB5, 0x63, 0xEB},
	{0xB6, 0x6E, 0xE8, 0x5E, 0x74, 0x4C, 0x3E, 0xF8,
	 0x94, 0xEE, 0x11, 0xB9, 0x9E, 0xF3, 0x92, 0x55,
	 0xE4, 0xD5, 0xF3, 0xF5, 0xA4, 0x44, 0xDE, 0xBC,
	 0x71, 0x59, 0xD5, 0x23, 0x72, 0x4D, 0x3B, 0x7B},
	{0xC2, 0x22, 0x4D, 0xA5, 0x86, 0xDB, 0x17, 0x70,
	 0x15, 0xDD, 0x5B, 0x89, 0x6F, 0x8D, 0x04, 0x71,
	 0xF3, 0x83, 0x1B, 0x37, 0x95, 0x04, 0x07, 0x95,
	 0x33, 0x0A, 0x0A, 0x17, 0xEF, 0xE7, 0xA0, 0x54}
};

INT ecc_calculate_test(struct _RTMP_ADAPTER *ad, char *arg)
{
	UINT_32 scalar_pattern_offset = 0;
	UINT_32 oper = ECC_OP_CAL_GROUP_POINT;
	UINT_32 group = ECDH_GROUP_ID_256BIT;
	UINT_8 *scalar = NULL;
	UINT_8 *point_x = &group19_generator_x[0];
	UINT_8 *point_y = &group19_generator_y[0];

	scalar_pattern_offset = os_str_tol(arg, 0, 10);
	if (scalar_pattern_offset >= 20)
		scalar_pattern_offset = 0;

	scalar = &group19_priv_pettern[scalar_pattern_offset][0];

	asic_calculate_ecc(ad, oper, group, scalar, point_x, point_y);

	return TRUE;
}


