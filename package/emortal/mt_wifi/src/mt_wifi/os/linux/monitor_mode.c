/*
 */
#include "rt_os_util.h"

#ifdef CONFIG_STA_SUPPORT
/* 2-byte Frame control field */
typedef struct GNU_PACKED _DOT11_FC_FIELD {
#ifdef RT_BIG_ENDIAN
	UINT16 Order : 1;		/* Strict order expected */
	UINT16 Wep : 1;		/* Wep data */
	UINT16 MoreData : 1;	/* More data bit */
	UINT16 PwrMgmt : 1;	/* Power management bit */
	UINT16 Retry : 1;		/* Retry status bit */
	UINT16 MoreFrag : 1;	/* More fragment bit */
	UINT16 FrDs : 1;		/* From DS indication */
	UINT16 ToDs : 1;		/* To DS indication */
	UINT16 SubType : 4;	/* MSDU subtype */
	UINT16 Type : 2;		/* MSDU type */
	UINT16 Ver : 2;		/* Protocol version */
#else
	UINT16 Ver : 2;		/* Protocol version */
	UINT16 Type : 2;		/* MSDU type */
	UINT16 SubType : 4;	/* MSDU subtype */
	UINT16 ToDs : 1;		/* To DS indication */
	UINT16 FrDs : 1;		/* From DS indication */
	UINT16 MoreFrag : 1;	/* More fragment bit */
	UINT16 Retry : 1;		/* Retry status bit */
	UINT16 PwrMgmt : 1;	/* Power management bit */
	UINT16 MoreData : 1;	/* More data bit */
	UINT16 Wep : 1;		/* Wep data */
	UINT16 Order : 1;		/* Strict order expected */
#endif	/* !RT_BIG_ENDIAN */
} DOT11_FC_FIELD;

typedef struct GNU_PACKED _DOT11_HEADER {
	DOT11_FC_FIELD FC;
	UINT16          Duration;
	UCHAR           Addr1[6];
	UCHAR           Addr2[6];
	UCHAR		Addr3[6];
#ifdef RT_BIG_ENDIAN
	UINT16		Sequence:12;
	UINT16		Frag:4;
#else
	UINT16		Frag:4;
	UINT16		Sequence:12;
#endif /* !RT_BIG_ENDIAN */
	UCHAR		Octet[0];
} DOT11_HEADER;

INT32 ralinkrate[] = {
	2, 4, 11, 22,		/* CCK */
	12, 18, 24, 36, 48, 72, 96, 108,	/* OFDM */
	/* 20MHz, 800ns GI, MCS: 0 ~ 15 */
	13, 26, 39, 52, 78, 104, 117, 130, 26, 52, 78, 104, 156, 208, 234, 260,
	39, 78, 117, 156, 234, 312, 351, 390,	/* 20MHz, 800ns GI, MCS: 16 ~ 23 */
	/* 40MHz, 800ns GI, MCS: 0 ~ 15 */
	27, 54, 81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
	81, 162, 243, 324, 486, 648, 729, 810,	/* 40MHz, 800ns GI, MCS: 16 ~ 23 */
	/* 20MHz, 400ns GI, MCS: 0 ~ 15 */
	14, 29, 43, 57, 87, 115, 130, 144, 29, 59, 87, 115, 173, 230, 260, 288,
	43, 87, 130, 173, 260, 317, 390, 433,	/* 20MHz, 400ns GI, MCS: 16 ~ 23 */
	/* 40MHz, 400ns GI, MCS: 0 ~ 15 */
	30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
	90, 180, 270, 360, 540, 720, 810, 900
};	/* 40MHz, 400ns GI, MCS: 16 ~ 23 */

UINT32 RT_RateSize = sizeof(ralinkrate);

void send_monitor_packets(IN PNET_DEV pNetDev,
						  IN PNDIS_PACKET pRxPacket,
						  IN UCHAR * pDot11Hdr,
						  IN UCHAR * pData,
						  IN USHORT DataSize,
						  IN UCHAR L2PAD,
						  IN UCHAR PHYMODE,
						  IN UCHAR BW,
						  IN UCHAR ShortGI,
						  IN UCHAR MCS,
						  IN UCHAR AMPDU,
						  IN UCHAR STBC,
						  IN UCHAR RSSI1,
						  IN UCHAR BssMonitorFlag11n,
						  IN UCHAR * pDevName,
						  IN UCHAR Channel,
						  IN UCHAR CentralChannel,
						  IN UINT32 MaxRssi)
{
	DOT11_HEADER *pHeader = (DOT11_HEADER *)pDot11Hdr;
	struct sk_buff *pOSPkt;
	wlan_ng_prism2_header *ph;
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	ETHEREAL_RADIO h;
	ETHEREAL_RADIO *ph_11n33;		/* for new 11n sniffer format */
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	int rate_index = 0;
	USHORT header_len = 0;
	UCHAR temp_header[40] = {0};

	MEM_DBG_PKT_FREE_INC(pRxPacket);
	pOSPkt = RTPKT_TO_OSPKT(pRxPacket);
	pOSPkt->dev = pNetDev;

	if (pHeader->FC.Type == 0x2 /* FC_TYPE_DATA */) {
		DataSize -= LENGTH_802_11;

		if ((pHeader->FC.ToDs == 1) && (pHeader->FC.FrDs == 1))
			header_len = LENGTH_802_11_WITH_ADDR4;
		else
			header_len = LENGTH_802_11;

		/* QOS */
		if (pHeader->FC.SubType & 0x08) {
			header_len += 2;
			/* Data skip QOS contorl field */
			DataSize -= 2;
		}

		/* Order bit: A-Ralink or HTC+ */
		if (pHeader->FC.Order) {
			header_len += 4;
			/* Data skip HTC contorl field */
			DataSize -= 4;
		}

		/* Copy Header */
		if (header_len <= 40)
			os_move_mem(temp_header, pData, header_len);

		/* skip HW padding */
		if (L2PAD)
			pData += (header_len + 2);
		else
			pData += header_len;
	}

	if (DataSize < pOSPkt->len)
		skb_trim(pOSPkt, DataSize);
	else
		skb_put(pOSPkt, (DataSize - pOSPkt->len));

	if ((pData - pOSPkt->data) > 0) {
		skb_put(pOSPkt, (pData - pOSPkt->data));
		skb_pull(pOSPkt, (pData - pOSPkt->data));
	}

	if (skb_headroom(pOSPkt) < (sizeof(wlan_ng_prism2_header) + header_len)) {
		if (pskb_expand_head(pOSPkt, (sizeof(wlan_ng_prism2_header) + header_len), 0, GFP_ATOMIC)) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s : Reallocate header size of sk_buff fail!\n",
					  __func__));
			goto err_free_sk_buff;
		}
	}

	if (header_len > 0)
		os_move_mem(skb_push(pOSPkt, header_len), temp_header,
					header_len);

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT

	if (BssMonitorFlag11n == 0)
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	{
		ph = (wlan_ng_prism2_header *) skb_push(pOSPkt,
												sizeof(wlan_ng_prism2_header));
		os_zero_mem(ph, sizeof(wlan_ng_prism2_header));
		ph->msgcode = DIDmsg_lnxind_wlansniffrm;
		ph->msglen = sizeof(wlan_ng_prism2_header);
		strcpy((RTMP_STRING *) ph->devname, (RTMP_STRING *) pDevName);
		ph->hosttime.did = DIDmsg_lnxind_wlansniffrm_hosttime;
		ph->hosttime.status = 0;
		ph->hosttime.len = 4;
		ph->hosttime.data = jiffies;
		ph->mactime.did = DIDmsg_lnxind_wlansniffrm_mactime;
		ph->mactime.status = 0;
		ph->mactime.len = 0;
		ph->mactime.data = 0;
		ph->istx.did = DIDmsg_lnxind_wlansniffrm_istx;
		ph->istx.status = 0;
		ph->istx.len = 0;
		ph->istx.data = 0;
		ph->channel.did = DIDmsg_lnxind_wlansniffrm_channel;
		ph->channel.status = 0;
		ph->channel.len = 4;
		ph->channel.data = (u_int32_t) Channel;
		ph->rssi.did = DIDmsg_lnxind_wlansniffrm_rssi;
		ph->rssi.status = 0;
		ph->rssi.len = 4;
		ph->rssi.data = MaxRssi;
		ph->signal.did = DIDmsg_lnxind_wlansniffrm_signal;
		ph->signal.status = 0;
		ph->signal.len = 4;
		ph->signal.data = 0;	/*rssi + noise; */
		ph->noise.did = DIDmsg_lnxind_wlansniffrm_noise;
		ph->noise.status = 0;
		ph->noise.len = 4;
		ph->noise.data = 0;
#ifdef DOT11_N_SUPPORT

		if (PHYMODE >= MODE_HTMIX)
			rate_index = 12 + ((UCHAR) BW * 24) + ((UCHAR) ShortGI * 48) + ((UCHAR) MCS);
		else
#endif /* DOT11_N_SUPPORT */
			if (PHYMODE == MODE_OFDM)
				rate_index = (UCHAR) (MCS) + 4;
			else
				rate_index = (UCHAR) (MCS);

		if (rate_index < 0)
			rate_index = 0;

		if (rate_index >= (sizeof(ralinkrate) / sizeof(ralinkrate[0])))
			rate_index = (sizeof(ralinkrate) / sizeof(ralinkrate[0])) - 1;

		ph->rate.did = DIDmsg_lnxind_wlansniffrm_rate;
		ph->rate.status = 0;
		ph->rate.len = 4;
		/* real rate = ralinkrate[rate_index] / 2 */
		ph->rate.data = ralinkrate[rate_index];
		ph->frmlen.did = DIDmsg_lnxind_wlansniffrm_frmlen;
		ph->frmlen.status = 0;
		ph->frmlen.len = 4;
		ph->frmlen.data = (u_int32_t) DataSize;
	}

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	else {
		ph_11n33 = &h;
		os_zero_mem((unsigned char *)ph_11n33,
					sizeof(ETHEREAL_RADIO));

		/*802.11n fields */
		if (MCS > 15)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_3x3;

		if (PHYMODE == MODE_HTGREENFIELD)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_GF;

		if (BW == 1)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_BW40;
		else if (Channel < CentralChannel)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_BW20U;
		else if (Channel > CentralChannel)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_BW20D;
		else {
			ph_11n33->Flag_80211n |=
				(WIRESHARK_11N_FLAG_BW20U |
				 WIRESHARK_11N_FLAG_BW20D);
		}

		if (ShortGI == 1)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_SGI;

		if (AMPDU)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_AMPDU;

		if (STBC)
			ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_STBC;

		ph_11n33->signal_level = (UCHAR) RSSI1;

		/* data_rate is the rate index in the wireshark rate table */
		if (PHYMODE >= MODE_HTMIX) {
			if (MCS == 32) {
				if (ShortGI)
					ph_11n33->data_rate = 16;
				else
					ph_11n33->data_rate = 4;
			} else if (MCS > 15)
				ph_11n33->data_rate =
					(16 * 4 + ((UCHAR) BW * 16) +
					 ((UCHAR) ShortGI * 32) + ((UCHAR) MCS));
			else
				ph_11n33->data_rate =
					16 + ((UCHAR) BW * 16) +
					((UCHAR) ShortGI * 32) + ((UCHAR) MCS);
		} else if (PHYMODE == MODE_OFDM)
			ph_11n33->data_rate = (UCHAR) (MCS) + 4;
		else
			ph_11n33->data_rate = (UCHAR) (MCS);

		/*channel field */
		ph_11n33->channel = (UCHAR) Channel;
		os_move_mem(skb_put(pOSPkt, sizeof(ETHEREAL_RADIO)),
					(UCHAR *) ph_11n33, sizeof(ETHEREAL_RADIO));
	}

#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	pOSPkt->pkt_type = PACKET_OTHERHOST;
	pOSPkt->protocol = eth_type_trans(pOSPkt, pOSPkt->dev);
	pOSPkt->ip_summed = CHECKSUM_NONE;
	netif_rx(pOSPkt);
	return;
err_free_sk_buff:
	RELEASE_NDIS_PACKET(NULL, pRxPacket, NDIS_STATUS_FAILURE);
}
#endif /* CONFIG_STA_SUPPORT */


