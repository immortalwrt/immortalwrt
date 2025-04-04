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
/***************************************************************************
 ***************************************************************************

*/

#include "rt_config.h"
#include "mac/mac_mt/fmac/mt_fmac.h"

#ifndef MT7915_MT7916_COEXIST_COMPATIBLE
#ifdef MT7986
#include "chip/mt7986_cr.h"
#endif
#ifdef MT7916
#include "chip/mt7916_cr.h"
#endif
#ifdef MT7981
#include "chip/mt7981_cr.h"
#endif
#endif

extern UCHAR tmi_rate_map_cck_lp[];
extern UCHAR tmi_rate_map_cck_lp_size;
extern UCHAR tmi_rate_map_cck_sp[];
extern UCHAR tmi_rate_map_cck_sp_size;
extern UCHAR tmi_rate_map_ofdm[];
extern UCHAR tmi_rate_map_ofdm_size;
extern char *pkt_ft_str[];
extern char *hdr_fmt_str[];
extern char *rmac_info_type_str[];
#if defined(MGMT_TXPWR_CTRL) || (defined(TPC_SUPPORT) && defined(TPC_MODE_CTRL))
extern BOOLEAN g_fgCommand;
#endif

#ifdef DATA_TXPWR_CTRL
extern UINT8 g_u1MinPwrlimitperRate[DBDC_BAND_NUM][DATA_TXPOWER_MAX_BW_NUM][DATA_TXPOWER_MAX_MCS_NUM];
#endif

VOID mtf_dump_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info)
{
	struct txd_l *txd = (struct txd_l *)tmac_info;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	hex_dump("txd raw data: ", (UCHAR *)tmac_info, cap->TXWISize);

	MTWF_PRINT("TMAC_TXD Fields:\n");
	MTWF_PRINT("\tTMAC_TXD_0:\n");


	/* DW0 */
	/* TX Byte Count [15:0]  */
	MTWF_PRINT("\t\tTxByteCnt = %d\n",
				((txd->txd_0 & TXD_TX_BYTE_COUNT_MASK) >> TXD_TX_BYTE_COUNT_SHIFT));

	/* PKT_FT: Packet Format [24:23] */
	MTWF_PRINT("\t\tpkt_ft = %d(%s)\n",
				((txd->txd_0 & TXD_PKT_FT_MASK) >> TXD_PKT_FT_SHIFT),
				pkt_ft_str[((txd->txd_0 & TXD_PKT_FT_MASK) >> TXD_PKT_FT_SHIFT)]);

	/* Q_IDX [31:25]  */
	MTWF_PRINT("\t\tQueID =0x%x\n",
				((txd->txd_0 & TXD_Q_IDX_MASK) >> TXD_Q_IDX_SHIFT));

	MTWF_PRINT("\tTMAC_TXD_1:\n");

	/* DW1 */
	/* WLAN Indec [9:0] */
	MTWF_PRINT("\t\tWlan Index = %d\n",
				((txd->txd_1 & TXD_WLAN_IDX_MASK) >> TXD_WLAN_IDX_SHIFT));

	/* VTA [10] */
	MTWF_PRINT("\t\tVTA = %d\n",
				((txd->txd_1 & TXD_VTA) ? 1 : 0));

	/* HF: Header Format [17:16] */
	MTWF_PRINT("\t\tHdrFmt = %d(%s)\n",
				((txd->txd_1 & TXD_HF_MASK) >> TXD_HF_SHIFT),
				 hdr_fmt_str[((txd->txd_1 & TXD_HF_MASK) >> TXD_HF_SHIFT)]);


	switch ((txd->txd_1 & TXD_HF_MASK) >> TXD_HF_SHIFT) {
	case TMI_HDR_FT_NON_80211:

		/* MRD [11], EOSP [12], RMVL [13], VLAN [14], ETYPE [15] */
		MTWF_PRINT("\t\t\tMRD = %d, EOSP = %d,\
				RMVL = %d, VLAN = %d, ETYP = %d\n",
				(txd->txd_1 & TXD_MRD) ? 1 : 0,
				(txd->txd_1 & TXD_EOSP) ? 1 : 0,
				(txd->txd_1 & TXD_RMVL) ? 1 : 0,
				(txd->txd_1 & TXD_VLAN) ? 1 : 0,
				(txd->txd_1 & TXD_ETYP) ? 1 : 0);
		break;
	case TMI_HDR_FT_NOR_80211:
		/* HEADER_LENGTH [15:11] */
		MTWF_PRINT("\t\t\tHeader Len = %d(WORD)\n",
				((txd->txd_1 & TXD_HDR_LEN_MASK) >> TXD_HDR_LEN_SHIFT));
		break;

	case TMI_HDR_FT_ENH_80211:
		/* EOSP [12], AMS [13]  */
		MTWF_PRINT("\t\t\tEOSP = %d, AMS = %d\n",
				(txd->txd_1 & TXD_EOSP) ? 1 : 0,
				(txd->txd_1 & TXD_AMS) ? 1 : 0);
		break;
	}

	/* Header Padding [19:18] */
	MTWF_PRINT("\t\tHdrPad = %d\n",
				((txd->txd_1 & TXD_HDR_PAD_MASK) >> TXD_HDR_PAD_SHIFT));

	/* TID [22:20] */
	MTWF_PRINT("\t\tTID = %d\n",
				((txd->txd_1 & TXD_TID_MASK) >> TXD_TID_SHIFT));

	/* UtxB/AMSDU_C/AMSDU [23] */
	MTWF_PRINT("\t\tamsdu = %d\n",
				((txd->txd_1 & TXD_AMSDU) ? 1 : 0));

	/* OM [29:24] */
	MTWF_PRINT("\t\town_mac = %d\n",
				((txd->txd_1 & TXD_OM_MASK) >> TXD_OM_SHIFT));

	/* TGID [30] */
	MTWF_PRINT("\t\tTGID = %d\n",
				((txd->txd_1 & TXD_TGID) ? 1 : 0));

	/* FT [31] */
	MTWF_PRINT("\t\tTxDFormatType = %d\n",
				(txd->txd_1 & TXD_FT) ? 1 : 0);

	MTWF_PRINT("\tTMAC_TXD_2:\n");

	/* DW2 */
	/* Subtype [3:0] */
	MTWF_PRINT("\t\tsub_type = %d\n",
				((txd->txd_2 & TXD_SUBTYPE_MASK) >> TXD_SUBTYPE_SHIFT));

	/* Type[5:4] */
	MTWF_PRINT("\t\tfrm_type = %d\n",
				((txd->txd_2 & TXD_TYPE_MASK) >> TXD_TYPE_SHIFT));

	/* NDP [6] */
	MTWF_PRINT("\t\tNDP = %d\n",
				((txd->txd_2 & TXD_NDP) ? 1 : 0));

	/* NDPA [7] */
	MTWF_PRINT("\t\tNDPA = %d\n",
				((txd->txd_2 & TXD_NDPA) ? 1 : 0));

	/* SD [8] */
	MTWF_PRINT("\t\tSounding = %d\n",
				((txd->txd_2 & TXD_SD) ? 1 : 0));

	/* RTS [9] */
	MTWF_PRINT("\t\tRTS = %d\n",
				((txd->txd_2 & TXD_RTS) ? 1 : 0));

	/* BM [10] */
	MTWF_PRINT("\t\tbc_mc_pkt = %d\n",
				((txd->txd_2 & TXD_BM) ? 1 : 0));

	/* B [11]  */
	MTWF_PRINT("\t\tBIP = %d\n",
				((txd->txd_2 & TXD_B) ? 1 : 0));

	/* DU [12] */
	MTWF_PRINT("\t\tDuration = %d\n",
				((txd->txd_2 & TXD_DU) ? 1 : 0));

	/* HE [13] */
	MTWF_PRINT("\t\tHE(HTC Exist) = %d\n",
				((txd->txd_2 & TXD_HE) ? 1 : 0));

	/* FRAG [15:14] */
	MTWF_PRINT("\t\tFRAG = %d\n",
				((txd->txd_2 & TXD_FRAG_MASK) >> TXD_FRAG_SHIFT));

	/* Remaining Life Time [23:16]*/
	MTWF_PRINT("\t\tReamingLife/MaxTx time = %d (unit: 64TU)\n",
				((txd->txd_2 & TXD_REMAIN_TIME_MASK) >> TXD_REMAIN_TIME_SHIFT));

	/* Power Offset [29:24] */
	MTWF_PRINT("\t\tpwr_offset = %d\n",
				((txd->txd_2 & TXD_PWR_OFFESET_MASK) >> TXD_PWR_OFFESET_SHIFT));

	/* FRM [30] */
	MTWF_PRINT("\t\tfix rate mode = %d\n",
				(txd->txd_2 & TXD_FRM) ? 1 : 0);

	/* FR[31] */
	MTWF_PRINT("\t\tfix rate = %d\n",
				(txd->txd_2 & TXD_FR) ? 1 : 0);

	MTWF_PRINT("\tTMAC_TXD_3:\n");

	/* DW3 */
	/* NA [0] */
	MTWF_PRINT("\t\tNoAck = %d\n",
				(txd->txd_3 & TXD_NA) ? 1 : 0);

	/* PF [1] */
	MTWF_PRINT("\t\tPF = %d\n",
				(txd->txd_3 & TXD_PF) ? 1 : 0);

	/* EMRD [2] */
	MTWF_PRINT("\t\tEMRD = %d\n",
				(txd->txd_3 & TXD_EMRD) ? 1 : 0);

	/* EEOSP [3] */
	MTWF_PRINT("\t\tEEOSP = %d\n",
				(txd->txd_3 & TXD_EEOSP) ? 1 : 0);

	/* DAS [4] */
	MTWF_PRINT("\t\tda_select = %d\n",
				(txd->txd_3 & TXD_DAS) ? 1 : 0);

	/* TM [5] */
	MTWF_PRINT("\t\ttm = %d\n",
				(txd->txd_3 & TXD_TM) ? 1 : 0);

	/* TX Count [10:6] */
	MTWF_PRINT("\t\ttx_cnt = %d\n",
				((txd->txd_3 & TXD_TX_CNT_MASK) >> TXD_TX_CNT_SHIFT));

	/* Remaining TX Count [15:11] */
	MTWF_PRINT("\t\tremain_tx_cnt = %d\n",
				((txd->txd_3 & TXD_REMAIN_TX_CNT_MASK) >> TXD_REMAIN_TX_CNT_SHIFT));

	/* SN [27:16] */
	MTWF_PRINT("\t\tsn = %d\n",
				((txd->txd_3 & TXD_SN_MASK) >> TXD_SN_SHIFT));

	/* BA_DIS [28] */
	MTWF_PRINT("\t\tba dis = %d\n",
				(txd->txd_3 & TXD_BA_DIS) ? 1 : 0);

	/* Power Management [29] */
	MTWF_PRINT("\t\tpwr_mgmt = 0x%x\n",
				(txd->txd_3 & TXD_PM) ? 1 : 0);

	/* PN_VLD [30] */
	MTWF_PRINT("\t\tpn_vld = %d\n",
				(txd->txd_3 & TXD_PN_VLD) ? 1 : 0);

	/* SN_VLD [31] */
	MTWF_PRINT("\t\tsn_vld = %d\n",
				(txd->txd_3 & TXD_SN_VLD) ? 1 : 0);


	/* DW4 */
	MTWF_PRINT("\tTMAC_TXD_4:\n");

	/* PN_LOW [31:0] */
	MTWF_PRINT("\t\tpn_low = 0x%x\n",
				((txd->txd_4 & TXD_PN1_MASK) >> TXD_PN1_SHIFT));


	/* DW5 */
	MTWF_PRINT("\tTMAC_TXD_5:\n");

	/* PID [7:0] */
	MTWF_PRINT("\t\tpid = %d\n",
				((txd->txd_5 & TXD_PID_MASK) >> TXD_PID_SHIFT));

	/* TXSFM [8] */
	MTWF_PRINT("\t\ttx_status_fmt = %d\n",
				(txd->txd_5 & TXD_TXSFM) ? 1 : 0);

	/* TXS2M [9] */
	MTWF_PRINT("\t\ttx_status_2_mcu = %d\n",
				(txd->txd_5 & TXD_TXS2M) ? 1 : 0);

	/* TXS2H [10] */
	MTWF_PRINT("\t\ttx_status_2_host = %d\n",
				(txd->txd_5 & TXD_TXS2H) ? 1 : 0);

	/* ADD_BA [14] */
	MTWF_PRINT("\t\tADD_BA = %d\n",
		(txd->txd_5 & TXD_ADD_BA) ? 1 : 0);
	/* MD [15] */
	MTWF_PRINT("\t\tMD = %d\n",
		(txd->txd_5 & TXD_MD) ? 1 : 0);

	/* PN_HIGH [31:16]  */
	MTWF_PRINT("\t\tpn_high = 0x%x\n",
				((txd->txd_5 & TXD_PN2_MASK) >> TXD_PN2_SHIFT));

	/* DW6 */
	MTWF_PRINT("\tTMAC_TXD_6:\n");

	if (txd->txd_2 & TXD_FR) {
		/* Fixed BandWidth mode [2:0] */
		MTWF_PRINT("\t\tbw = %d\n",
				((txd->txd_6 & TXD_BW_MASK) >> TXD_BW_SHIFT));

		/* DYN_BW [3] */
		MTWF_PRINT("\t\tdyn_bw = %d\n",
				(txd->txd_6 & TXD_DYN_BW) ? 1 : 0);

		/* ANT_ID [7:4] */
		MTWF_PRINT("\t\tant_id = %d\n",
				((txd->txd_6 & TXD_ANT_ID_MASK) >> TXD_ANT_ID_SHIFT));

		/* SPE_IDX_SEL [10] */
		MTWF_PRINT("\t\tspe_idx_sel = %d\n",
				(txd->txd_6 & TXD_SPE_IDX_SEL) ? 1 : 0);

		/* LDPC [11] */
		MTWF_PRINT("\t\tldpc = %d\n",
				(txd->txd_6 & TXD_LDPC) ? 1 : 0);
		/* HELTF Type[13:12] */
		MTWF_PRINT("\t\tHELTF Type = %d\n",
				((txd->txd_6 & TXD_HELTF_TYPE_MASK) >> TXD_HELTF_TYPE_SHIFT));

		/* GI Type [15:14] */
		MTWF_PRINT("\t\tGI = %d\n",
				((txd->txd_6 & TXD_GI_MASK) >> TXD_GI_SHIFT));


		/* Rate to be Fixed [29:16] */
		MTWF_PRINT("\t\ttx_rate = 0x%x\n",
				((txd->txd_6 & TXD_FR_RATE_MASK) >> TXD_FR_RATE_SHIFT));
	}

	/* TXEBF [30] */
	MTWF_PRINT("\t\ttxebf = %d\n",
				(txd->txd_6 & TXD_TXEBF)  ? 1 : 0);

	/* TXIBF [31] */
	MTWF_PRINT("\t\ttxibf = %d\n",
				(txd->txd_6 & TXD_TXIBF) ? 1 : 0);

	/* DW7 */
	MTWF_PRINT("\tTMAC_TXD_7:\n");

	if ((txd->txd_1 & TXD_VTA) == 0) {
		/* SW Tx Time [9:0] */
		MTWF_PRINT("\t\tsw_tx_time = %d\n",
					((txd->txd_7 & TXD_SW_TX_TIME_MASK) >> TXD_SW_TX_TIME_SHIFT));
	} else {
		/* TXD Arrival Time [9:0] */
		MTWF_PRINT("\t\tat = %d\n",
					((txd->txd_7 & TXD_TAT) >> TXD_TAT_SHIFT));
	}

	/* HW_AMSDU_CAP [10] */
	MTWF_PRINT("\t\thw amsdu cap = %d\n",
				(txd->txd_7 & TXD_HW_AMSDU_CAP) ? 1 : 0);

	/* SPE_IDX [15:11] */
	if (txd->txd_2 & TXD_FR) {
		MTWF_PRINT("\t\tspe_idx = 0x%x\n",
				((txd->txd_7 & TXD_SPE_IDX_MASK) >> TXD_SPE_IDX_SHIFT));
	}

	/* PSE_FID [27:16] */
	MTWF_PRINT("\t\tpse_fid = 0x%x\n",
				((txd->txd_7 & TXD_PSE_FID_MASK) >> TXD_PSE_FID_SHIFT));

	/* Subtype [19:16] */
	MTWF_PRINT("\t\tpp_sub_type=%d\n",
				((txd->txd_7 & TXD_PP_SUBTYPE_MASK) >> TXD_PP_SUBTYPE_SHIFT));

	/* Type [21:20] */
	MTWF_PRINT("\t\tpp_type=%d\n",
				((txd->txd_7 & TXD_PP_TYPE_MASK) >> TXD_PP_TYPE_SHIFT));

	/* CTXD_CNT [25:23] */
	MTWF_PRINT("\t\tctxd cnt=0x%x\n",
				((txd->txd_7 & TXD_CTXD_CNT_MASK) >> TXD_CTXD_CNT_SHIFT));

	/* CTXD [26] */
	MTWF_PRINT("\t\tctxd = %d\n",
				(txd->txd_7 & TXD_CTXD) ? 1 : 0);

	/* I [28]  */
	MTWF_PRINT("\t\ti = %d\n",
				(txd->txd_7 & TXD_IP_CHKSUM) ? 1 : 0);

	/* UT [29] */
	MTWF_PRINT("\t\tUT = %d\n",
				(txd->txd_7 & TXD_UT) ? 1 : 0);

	/* TXDLEN [31:30] */
	MTWF_PRINT("\t\t txd len= %d\n",
				((txd->txd_7 & TXD_TXD_LEN_MASK) >> TXD_TXD_LEN_SHIFT));
}

static INT mtf_dump_txp_info(RTMP_ADAPTER *pAd, UCHAR *txp_info)
{
	MAC_TX_PKT_T *txp = (MAC_TX_PKT_T *)txp_info;

	hex_dump("txp raw data: ", (UCHAR *)txp_info, sizeof(MAC_TX_PKT_T));

	MTWF_PRINT("TXP_Fields:\n");
	MTWF_PRINT("\tMSDU_ID0 = %d(VLD:%d)\n",
			 txp->au2MsduId[0] & TXD_MSDU_ID_MASK,
			 (txp->au2MsduId[0] & TXD_MSDU_ID_VLD) ? 1 : 0);
	MTWF_PRINT("\tMSDU_ID1 = %d(VLD:%d)\n",
			 txp->au2MsduId[1] & TXD_MSDU_ID_MASK,
			 (txp->au2MsduId[1] & TXD_MSDU_ID_VLD) ? 1 : 0);
	MTWF_PRINT("\tMSDU_ID2 = %d(VLD:%d)\n",
			 txp->au2MsduId[2] & TXD_MSDU_ID_MASK,
			 (txp->au2MsduId[2] & TXD_MSDU_ID_VLD) ? 1 : 0);
	MTWF_PRINT("\tMSDU_ID3 = %d(VLD:%d)\n",
			 txp->au2MsduId[3] & TXD_MSDU_ID_MASK,
			 (txp->au2MsduId[3] & TXD_MSDU_ID_VLD) ? 1 : 0);

	MTWF_PRINT("\tTXP_ADDR0[31:0] = 0x%x\n",
			 txp->arPtrLen[0].u4Ptr0);

	MTWF_PRINT("\tTXP_LEN0 = 0x%lx(ML:%d)\n",
			 txp->arPtrLen[0].u2Len0 & TXD_LEN_MASK_V2,
			 (txp->arPtrLen[0].u2Len0 & TXD_LEN_ML_V2) ? 1 : 0);

	MTWF_PRINT("\tTXP_LEN1 = 0x%lx(ML:%d)\n",
			 txp->arPtrLen[0].u2Len1 & TXD_LEN_MASK_V2,
			 (txp->arPtrLen[0].u2Len1 & TXD_LEN_ML_V2) ? 1 : 0);

	MTWF_PRINT("\tTXP_ADDR1[31:0] = 0x%x\n",
			 txp->arPtrLen[0].u4Ptr1);

	MTWF_PRINT("\tTXP_ADDR2[31:0] = 0x%x\n",
			 txp->arPtrLen[1].u4Ptr0);

	MTWF_PRINT("\tTXP_LEN2 = 0x%lx(ML:%d)\n",
			 txp->arPtrLen[1].u2Len0 & TXD_LEN_MASK_V2,
			 (txp->arPtrLen[1].u2Len0 & TXD_LEN_ML_V2) ? 1 : 0);

	MTWF_PRINT("\tTXP_LEN3 = 0x%lx(ML:%d)\n",
			 txp->arPtrLen[1].u2Len1 & TXD_LEN_MASK_V2,
			 (txp->arPtrLen[1].u2Len1 & TXD_LEN_ML_V2) ? 1 : 0);

	MTWF_PRINT("\tTXP_ADDR3[31:0] = 0x%x\n",
			 txp->arPtrLen[1].u4Ptr1);

	return 0;
}

VOID mtf_dump_rxinfo(RTMP_ADAPTER *pAd, UCHAR *rxinfo)
{
	RXINFO_STRUC *pRxInfo = (RXINFO_STRUC *) rxinfo;
	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(RXINFO_STRUC));
	MTWF_PRINT("RxInfo Fields:\n");
	MTWF_PRINT("\tBA=%d\n", pRxInfo->BA);
	MTWF_PRINT("\tDATA=%d\n", pRxInfo->DATA);
	MTWF_PRINT("\tNULLDATA=%d\n", pRxInfo->NULLDATA);
	MTWF_PRINT("\tFRAG=%d\n", pRxInfo->FRAG);
	MTWF_PRINT("\tU2M=%d\n", pRxInfo->U2M);
	MTWF_PRINT("\tMcast=%d\n", pRxInfo->Mcast);
	MTWF_PRINT("\tBcast=%d\n", pRxInfo->Bcast);
	MTWF_PRINT("\tMyBss=%d\n", pRxInfo->MyBss);
	MTWF_PRINT("\tCrc=%d\n", pRxInfo->Crc);
	MTWF_PRINT("\tCipherErr=%d\n", pRxInfo->CipherErr);
	MTWF_PRINT("\tAMSDU=%d\n", pRxInfo->AMSDU);
	MTWF_PRINT("\tHTC=%d\n", pRxInfo->HTC);
	MTWF_PRINT("\tRSSI=%d\n", pRxInfo->RSSI);
	MTWF_PRINT("\tL2PAD=%d\n", pRxInfo->L2PAD);
	MTWF_PRINT("\tAMPDU=%d\n", pRxInfo->AMPDU);
	MTWF_PRINT("\tDecrypted=%d\n", pRxInfo->Decrypted);
	MTWF_PRINT("\tBssIdx3=%d\n", pRxInfo->BssIdx3);
	MTWF_PRINT("\twapi_kidx=%d\n", pRxInfo->wapi_kidx);
	MTWF_PRINT("\tpn_len=%d\n", pRxInfo->pn_len);
	MTWF_PRINT("\tsw_fc_type0=%d\n", pRxInfo->sw_fc_type0);
	MTWF_PRINT("\tsw_fc_type1=%d\n", pRxInfo->sw_fc_type1);
}

/*static*/ VOID mtf_dump_rmac_info_normal(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	struct rxd_grp_0 *rxd_grp0 = (struct rxd_grp_0 *)(rmac_info);

	hex_dump_always("rxd raw data: ", (UCHAR *)rxd_grp0, sizeof(*rxd_grp0));
	MTWF_PRINT("RxData_BASE:\n");

	/* DW0 */
	MTWF_PRINT("\tPktType = %d(%s), ",
			((rxd_grp0->rxd_0 & RXD_PKT_TYPE_MASK) >> RXD_PKT_TYPE_SHIFT),
			rxd_pkt_type_str((rxd_grp0->rxd_0 & RXD_PKT_TYPE_MASK) >> RXD_PKT_TYPE_SHIFT));

	MTWF_PRINT("RxByteCnt = %d\n",
			((rxd_grp0->rxd_0 & RXD_RX_BYTE_COUNT_MASK) >> RXD_RX_BYTE_COUNT_SHIFT));
	MTWF_PRINT("\tEtherTypeOffset = %d(WORD), ",
			((rxd_grp0->rxd_0 & RXD_ETH_TYPE_OFFSET_MASK) >> RXD_ETH_TYPE_OFFSET_SHIFT));
	MTWF_PRINT("IP/UT = %d/%d, ",
			(rxd_grp0->rxd_0 & RXD_IP) ? 1 : 0,
			(rxd_grp0->rxd_0 & RXD_UT) ? 1 : 0);

	/* DW1 */
	MTWF_PRINT("SEC_DONE = %d\n",
			(rxd_grp0->rxd_1 & RXD_SEC_DONE) ? 1 : 0);
	MTWF_PRINT("\tWlanIndex = %d, ",
			((rxd_grp0->rxd_1 & RXD_WLAN_IDX_MASK) >> RXD_WLAN_IDX_SHIFT));
	MTWF_PRINT("GroupValid = 0x%x, ",
			((rxd_grp0->rxd_1 & RXD_GROUP_VLD_MASK) >> RXD_GROUP_VLD_SHIFT));
	MTWF_PRINT("BN = %d, ",
				(rxd_grp0->rxd_1 & RXD_BN) ? 1 : 0);
	MTWF_PRINT("SPP_EN = %d\n",
			(rxd_grp0->rxd_1 & RXD_SPP_EN) ? 1 : 0);
	MTWF_PRINT("\tSEC Mode = %d, ",
			((rxd_grp0->rxd_1 & RXD_SEC_MODE_MASK) >> RXD_SEC_MODE_SHIFT));
	MTWF_PRINT("KeyID = %d, ",
			((rxd_grp0->rxd_1 & RXD_KID_MASK) >> RXD_KID_SHIFT));
	MTWF_PRINT("CM = %d, ",
			(rxd_grp0->rxd_1 & RXD_CM) ? 1 : 0);
	MTWF_PRINT("CLM = %d,",
			(rxd_grp0->rxd_1 & RXD_CLM) ? 1 : 0);
	MTWF_PRINT("ADD_OM = %d\n",
			(rxd_grp0->rxd_1 & RXD_ADD_OM) ? 1 : 0);
	MTWF_PRINT("\tICV Err(I) = %d, ",
			(rxd_grp0->rxd_1 & RXD_ICV_ERR) ? 1 : 0);
	MTWF_PRINT("TKIP MIC Err(T) = %d, ",
			(rxd_grp0->rxd_1 & RXD_TKIPMIC_ERR) ? 1 : 0);
	MTWF_PRINT("FCE Error(FC) = %d\n",
			(rxd_grp0->rxd_1 & RXD_FCS_ERR) ? 1 : 0);

	/* DW2 */
	MTWF_PRINT("\tBSSID = %d, ",
			((rxd_grp0->rxd_2 & RXD_BSSID_MASK) >> RXD_BSSID_SHIFT));
	MTWF_PRINT("BF_CQI = %d, ",
			(rxd_grp0->rxd_2 & RXD_BF_CQI) ? 1 : 0);
	MTWF_PRINT("HdearLength(MAC) = %d\n",
			((rxd_grp0->rxd_2 & RXD_MAC_HDR_LEN_MASK) >> RXD_MAC_HDR_LEN_SHIFT));
	MTWF_PRINT("\tHeaerTrans(H) = %d, ",
			(rxd_grp0->rxd_2 & RXD_H) ? 1 : 0);
	MTWF_PRINT("HeaerOffset(HO) = %d, ",
			((rxd_grp0->rxd_2 & RXD_HO_MASK) >> RXD_HO_SHIFT));
	MTWF_PRINT("TID = %d\n",
			((rxd_grp0->rxd_2 & RXD_TID_MASK) >> RXD_TID_SHIFT));
	MTWF_PRINT("\tMU_BAR = %d, ",
			(rxd_grp0->rxd_2 & RXD_MU_BAR) ? 1 : 0);
	MTWF_PRINT("SWBIT = %d, ",
			(rxd_grp0->rxd_2 & RXD_SWBIT) ? 1 : 0);
	MTWF_PRINT("DeAMSDU Fail(DAF) = %d\n",
			(rxd_grp0->rxd_2 & RXD_DAF) ? 1 : 0);
	MTWF_PRINT("\tExceedMax Rx Length(EL) = %d, ",
			(rxd_grp0->rxd_2 & RXD_EL) ? 1 : 0);
	MTWF_PRINT("HdrTransFail(HTF) = %d\n",
			(rxd_grp0->rxd_2 & RXD_HTF) ? 1 : 0);
	MTWF_PRINT("\tInterested Frame(INTF) = %d, ",
			(rxd_grp0->rxd_2 & RXD_INTF) ? 1 : 0);
	MTWF_PRINT("Fragment Frame(FRAG) = %d\n",
			(rxd_grp0->rxd_2 & RXD_FRAG) ? 1 : 0);
	MTWF_PRINT("\tNull Frame(NULL) = %d, ",
			(rxd_grp0->rxd_2 & RXD_NULL) ? 1 : 0);
	MTWF_PRINT("Non Data Frame(NDATA) = %d\n",
			(rxd_grp0->rxd_2 & RXD_NDATA) ? 1 : 0);
	MTWF_PRINT("\tNon-AMPDU Subframe(NASF) = %d, ",
			(rxd_grp0->rxd_2 & RXD_NAMP) ? 1 : 0);

	MTWF_PRINT("BF_RPT = %d\n",
			(rxd_grp0->rxd_2 & RXD_BF_RPT) ? 1 : 0);

	/* DW3 */
	MTWF_PRINT("\tRX Vector Sequence No = %d, ",
			((rxd_grp0->rxd_3 & RXD_RXV_SN_MASK) >> RXD_RXV_SN_SHIFT));
	MTWF_PRINT("Channel Frequency = %d\n",
			((rxd_grp0->rxd_3 & RXD_CF_MASK) >> RXD_CF_SHIFT));
	MTWF_PRINT("\tHTC/UC2ME/MC/BC = %d/%d/%d/%d\n",
			(rxd_grp0->rxd_3 & RXD_HTC) ? 1 : 0,
			(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x1) ? 1 : 0,
			(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x2) ? 1 : 0,
			(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x3) ? 1 : 0);
	MTWF_PRINT("\tTCL = %d, ",
			(rxd_grp0->rxd_3 & RXD_TCL) ? 1 : 0);
	MTWF_PRINT("BBM = %d, ",
			(rxd_grp0->rxd_3 & RXD_BBM) ? 1 : 0);
	MTWF_PRINT("BU = %d, ",
			(rxd_grp0->rxd_3 & RXD_BU) ? 1 : 0);
	MTWF_PRINT("AMS = %d, ",
			(rxd_grp0->rxd_3 & RXD_AMS) ? 1 : 0);
	MTWF_PRINT("MESH = %d\n",
			(rxd_grp0->rxd_3 & RXD_MESH) ? 1 : 0);
	MTWF_PRINT("\tMHCP = %d, ",
			(rxd_grp0->rxd_3 & RXD_MHCP) ? 1 : 0);
	MTWF_PRINT("NO_INFO_WB = %d,",
			(rxd_grp0->rxd_3 & RXD_NO_INFO_WB) ? 1 : 0);
	MTWF_PRINT("DIS_RHTR = %d, ",
			(rxd_grp0->rxd_3 & RXD_DIS_RHTR) ? 1 : 0);
	MTWF_PRINT("PSS = %d\n",
			(rxd_grp0->rxd_3 & RXD_PSS) ? 1 : 0);
	MTWF_PRINT("\tMORE = %d, ",
			(rxd_grp0->rxd_3 & RXD_MORE) ? 1 : 0);
	MTWF_PRINT("UWAT = %d, ",
			(rxd_grp0->rxd_3 & RXD_UWAT) ? 1 : 0);
	MTWF_PRINT("RX_DROP = %d, ",
			(rxd_grp0->rxd_3 & RXD_RX_DROP) ? 1 : 0);
	MTWF_PRINT("VLAN2ETH = %d\n",
			(rxd_grp0->rxd_3 & RXD_VLAN2ETH) ? 1 : 0);

	/* DW4 */
	MTWF_PRINT("\tPF = %d, ",
			((rxd_grp0->rxd_4 & RXD_PF_MASK) >> RXD_PF_SHIFT));
	MTWF_PRINT("DP = %d, ",
			(rxd_grp0->rxd_4 & RXD_DP) ? 1 : 0);
	MTWF_PRINT("CLS = %d, ",
			(rxd_grp0->rxd_4 & RXD_CLS) ? 1 : 0);
	MTWF_PRINT("OFLD = %d, ",
			((rxd_grp0->rxd_4 & RXD_OFLD_MASK) >> RXD_OFLD_SHIFT));
	MTWF_PRINT("MGC = %d\n",
			(rxd_grp0->rxd_4 & RXD_MGC) ? 1 : 0);
	MTWF_PRINT("\tWOL = %d, ",
			((rxd_grp0->rxd_4 & RXD_WOL_MASK) >> RXD_WOL_SHIFT));
	MTWF_PRINT("CLS_BITMAP = %d, ",
			((rxd_grp0->rxd_4 & RXD_CLS_BITMAP_MASK) >> RXD_CLS_BITMAP_SHIFT));
	MTWF_PRINT("PF_MODE = %d, ",
			(rxd_grp0->rxd_4 & RXD_PF_MODE) ? 1 : 0);
	MTWF_PRINT("PF_STS = %d\n",
			((rxd_grp0->rxd_4 & RXD_PF_STS_MASK) >> RXD_PF_STS_SHIFT));
}

VOID mtf_dump_rmac_info_for_ICVERR(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	struct rxd_grp_0 *rxd_grp0 = (struct rxd_grp_0 *)(rmac_info);
	int LogDbgLvl = DBG_LVL_DEBUG;

	if (!IS_HIF_TYPE(pAd, HIF_MT))
		return;


	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, "\tHTC/UC2ME/MC/BC = %d/%d/%d/%d\n",
			(rxd_grp0->rxd_3 & RXD_HTC) ? 1 : 0,
			(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x1) ? 1 : 0,
			(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x2) ? 1 : 0,
			(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x3) ? 1 : 0);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", WlanIndex=%d",
		((rxd_grp0->rxd_1 & RXD_WLAN_IDX_MASK) >> RXD_WLAN_IDX_SHIFT));
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", SEC Mode=%d\n",
		((rxd_grp0->rxd_1 & RXD_SEC_MODE_MASK) >> RXD_SEC_MODE_SHIFT));
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, "\tFCE Error(FC)=%d", (rxd_grp0->rxd_1 & RXD_FCS_ERR) ? 1 : 0);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", CM=%d", (rxd_grp0->rxd_1 & RXD_CM) ? 1 : 0);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", CLM=%d", (rxd_grp0->rxd_1 & RXD_CLM) ? 1 : 0);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", I=%d", (rxd_grp0->rxd_1 & RXD_ICV_ERR) ? 1 : 0);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", T=%d", (rxd_grp0->rxd_1 & RXD_TKIPMIC_ERR) ? 1 : 0);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, "\tFragment Frame(FRAG)=%d\n", (rxd_grp0->rxd_2 & RXD_FRAG) ? 1 : 0);
}

static VOID dump_rmac_info_txs(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* TXS_FRM_STRUC *txs_frm = (TXS_FRM_STRUC *)rmac_info; */
}


static VOID dump_rmac_info_rxv(RTMP_ADAPTER *pAd, UCHAR *Data)
{
	RXV_DWORD0 *DW0 = NULL;
	RXV_DWORD1 *DW1 = NULL;

	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
	RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = NULL;
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = NULL;
	RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = NULL;

	RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
	RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
	RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;

	DW0 = (RXV_DWORD0 *)Data;
	MTWF_PRINT(", TA_0_31=%d\n", DW0->TA_0_31);

	DW1 = (RXV_DWORD1 *)(Data + 4);
	MTWF_PRINT(", TA_32_47=%d", DW1->TA_32_47);
	MTWF_PRINT(", RxvSn=%d", DW1->RxvSn);
	MTWF_PRINT(", TR=%d\n", DW1->TR);


	RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
	MTWF_PRINT(", TxRate=%d", RXV1_1ST_CYCLE->TxRate);
	MTWF_PRINT(", HtStbc=%d", RXV1_1ST_CYCLE->HtStbc);
	MTWF_PRINT(", HtAdCode=%d", RXV1_1ST_CYCLE->HtAdCode);
	MTWF_PRINT(", HtExtltf=%d", RXV1_1ST_CYCLE->HtExtltf);
	MTWF_PRINT(", TxMode=%d", RXV1_1ST_CYCLE->TxMode);
	MTWF_PRINT(", FrMode=%d\n", RXV1_1ST_CYCLE->FrMode);
	MTWF_PRINT(", VHTA1_B22=%d", RXV1_1ST_CYCLE->VHTA1_B22);
	MTWF_PRINT(", HtAggregation=%d", RXV1_1ST_CYCLE->HtAggregation);
	MTWF_PRINT(", HtShortGi=%d", RXV1_1ST_CYCLE->HtShortGi);
	MTWF_PRINT(", HtSmooth=%d", RXV1_1ST_CYCLE->HtSmooth);
	MTWF_PRINT(", HtNoSound=%d", RXV1_1ST_CYCLE->HtNoSound);
	MTWF_PRINT(", NumRx=%d\n", RXV1_1ST_CYCLE->NumRx);
	MTWF_PRINT(", LdpcExtraOFDMSymbol=%d", RXV1_1ST_CYCLE->LdpcExtraOFDMSymbol);
	MTWF_PRINT(", SuVhtMcs_MuCoding=%d", RXV1_1ST_CYCLE->SuVhtMcs_MuCoding);
	MTWF_PRINT(", Beamormed=%d", RXV1_1ST_CYCLE->Beamormed);

	MTWF_PRINT(", ACID_DET_LOWER=%d", RXV1_1ST_CYCLE->ACID_DET_LOWER);
	MTWF_PRINT(", ACID_DET_UPPER=%d\n", RXV1_1ST_CYCLE->ACID_DET_UPPER);

	RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);
	MTWF_PRINT(", Length=%d", RXV1_2ND_CYCLE->Length);
	MTWF_PRINT(", GroupId=%d", RXV1_2ND_CYCLE->GroupId);
	MTWF_PRINT(", NstsField=%d", RXV1_2ND_CYCLE->NstsField);
	MTWF_PRINT(", RxValidIndicator=%d", RXV1_2ND_CYCLE->RxValidIndicator);
	MTWF_PRINT(", SelAnt=%d\n", RXV1_2ND_CYCLE->SelAnt);

	RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(Data + 16);
	MTWF_PRINT(", VHTA1_B21_B10=%d", RXV1_3TH_CYCLE->VHTA1_B21_B10);
	MTWF_PRINT(", POPEverTrig=%d", RXV1_3TH_CYCLE->POPEverTrig);
	MTWF_PRINT(", FgacCalLnaRx=%d", RXV1_3TH_CYCLE->FgacCalLnaRx);

	MTWF_PRINT(", IBRssiRx=%d", RXV1_3TH_CYCLE->IBRssiRx);
	MTWF_PRINT(", WBRssiRx=%d\n", RXV1_3TH_CYCLE->WBRssiRx);

	RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 20);
	MTWF_PRINT(", RCPI0=%d", RXV1_4TH_CYCLE->RCPI0);
	MTWF_PRINT(", RCPI1=%d", RXV1_4TH_CYCLE->RCPI1);
	MTWF_PRINT(", RCPI2=%d", RXV1_4TH_CYCLE->RCPI2);
	MTWF_PRINT(", RCPI3=%d\n", RXV1_4TH_CYCLE->RCPI3);

	RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(Data + 24);
	MTWF_PRINT(", FagcLnaGainx=%d", RXV1_5TH_CYCLE->FagcLnaGainx);
	MTWF_PRINT(", FagcLpfGainx=%d", RXV1_5TH_CYCLE->FagcLpfGainx);
	MTWF_PRINT(", MISC1=%d\n", RXV1_5TH_CYCLE->MISC1);

	RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(Data + 28);
	MTWF_PRINT(", Nf0=%d", RXV1_6TH_CYCLE->Nf0);
	MTWF_PRINT(", Nf1=%d", RXV1_6TH_CYCLE->Nf1);
	MTWF_PRINT(", Nf2=%d", RXV1_6TH_CYCLE->Nf2);
	MTWF_PRINT(", Nf3=%d\n", RXV1_6TH_CYCLE->Nf3);

	RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
	MTWF_PRINT(", PrimItfrEnv=%d", RXV2_1ST_CYCLE->PrimItfrEnv);
	MTWF_PRINT(", SecItfrEnv=%d", RXV2_1ST_CYCLE->SecItfrEnv);
	MTWF_PRINT(", Sec40ItfrEnv=%d", RXV2_1ST_CYCLE->Sec40ItfrEnv);
	MTWF_PRINT(", Sec80ItfrEnv=%d", RXV2_1ST_CYCLE->Sec80ItfrEnv);
	MTWF_PRINT(", RxLQ=%d", RXV2_1ST_CYCLE->RxLQ);
	MTWF_PRINT(", BtEnv=%d", RXV2_1ST_CYCLE->BtEnv);
	MTWF_PRINT(", ScrambleSeed=%d\n", RXV2_1ST_CYCLE->ScrambleSeed);
	MTWF_PRINT(", LqDataBit=%d", RXV2_1ST_CYCLE->LqDataBit);

	MTWF_PRINT(", RxCERmsd=%d", RXV2_1ST_CYCLE->RxCERmsd);
	MTWF_PRINT(", FCSErr=%d\n", RXV2_1ST_CYCLE->FCSErr);

	RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
	MTWF_PRINT(", PostTMD=%d", RXV2_2ND_CYCLE->PostTMD);
	MTWF_PRINT(", RxBW=%d", RXV2_2ND_CYCLE->RxBW);
	MTWF_PRINT(", PostBWDSecCh=%d", RXV2_2ND_CYCLE->PostBWDSecCh);
	MTWF_PRINT(", PostDewSecCh=%d", RXV2_2ND_CYCLE->PostDewSecCh);
	MTWF_PRINT(", HtSTFDet=%d", RXV2_2ND_CYCLE->HtSTFDet);
	MTWF_PRINT(", CagcSTFDet=%d", RXV2_2ND_CYCLE->CagcSTFDet);
	MTWF_PRINT(", IBRssi0=%d", RXV2_2ND_CYCLE->IBRssi0);
	MTWF_PRINT(", WBRssi0=%d", RXV2_2ND_CYCLE->WBRssi0);

	RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);
}


static VOID dump_rmac_info_rfb(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* RXD_BASE_STRUCT *rfb_frm = (RXD_BASE_STRUCT *)rmac_info; */
}


static VOID dump_rmac_info_tmr(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* TMR_FRM_STRUC *rxd_base = (TMR_FRM_STRUC *)rmac_info; */
}


VOID mtf_dump_rmac_info(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);
	MTWF_PRINT("RMAC_RXD Header Format :%s\n",
			 rxd_pkt_type_str(pkt_type));

	switch (pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_TXS:
		dump_rmac_info_txs(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXRXV:
		dump_rmac_info_rxv(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_NORMAL:
		/* Just for Debug, Enable it will hurt performance */
		if (DebugLevel >= DBG_LVL_INFO)
			mtf_dump_rmac_info_normal(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
		dump_rmac_info_rfb(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_TMR:
		dump_rmac_info_tmr(pAd, rmac_info);
		break;

	default:
		break;
	}
}


VOID mtf_dump_txs(struct _RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
	TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
	TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
	TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
	/* TXS_D_4 *TxSD4 = &txs_entry->TxSD4; */
	/* TXS_D_5 *TxSD5 = &txs_entry->TxSD5; */
	/* TXS_D_6 *TxSD6 = &txs_entry->TxSD6; */

	if (Format == TXS_FORMAT0) {
		MTWF_PRINT("\tType=TimeStamp/FrontTime Mode\n");
		MTWF_PRINT("\t\tTXSFM=%d, TXS2M/H=%d/%d, FixRate=%d, TxRate/BW=0x%x/%d\n",
				  TxSD0->TxSFmt, TxSD0->TxS2M, TxSD0->TxS2H,
				  TxSD0->TxS_FR, TxSD0->TxRate, TxSD1->TxS_TxBW);
		MTWF_PRINT("\t\tME/RE/LE/BE/TxOPLimitErr/BA-Fail=%d/%d/%d/%d/%d/%d, PS=%d, Pid=%d\n",
				  TxSD0->ME, TxSD0->RE, TxSD0->LE, TxSD0->BE, TxSD0->TxOp,
				  TxSD0->BAFail, TxSD0->PSBit, TxSD0->TxS_PId);
		MTWF_PRINT("\t\tTid=%d, AntId=%d, ETxBF/ITxBf=%d/%d\n",
				  TxSD1->TxS_Tid, TxSD1->TxS_AntId,
				  TxSD1->TxS_ETxBF, TxSD1->TxS_ITxBF);
		MTWF_PRINT("\t\tTxPwrdBm=0x%x, FinalMPDU=0x%x, AMPDU=0x%x\n",
				  TxSD1->TxPwrdBm, TxSD1->TxS_FianlMPDU, TxSD1->TxS_AMPDU);
		MTWF_PRINT("\t\tTxDelay=0x%x, RxVSeqNum=0x%x, Wlan Idx=0x%x\n",
				  TxSD2->TxS_TxDelay, TxSD2->TxS_RxVSN, TxSD2->TxS_WlanIdx);
		MTWF_PRINT("\t\tSN=0x%x, MPDU TxCnt=%d, LastTxRateIdx=%d\n",
				  TxSD3->type_0.TxS_SN, TxSD3->type_0.TxS_MpduTxCnt,
				  TxSD3->type_0.TxS_LastTxRateIdx);
	} else if (Format == TXS_FORMAT1) {
		MTWF_PRINT("\tType=Noisy/RCPI Mode\n");
	} else {
		MTWF_PRINT("%s: Unknown TxSFormat(%d)\n", __func__, Format);
	}
}

INT mtf_dump_dmac_amsdu_info(RTMP_ADAPTER *pAd)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#define MSDU_CNT_CR_NUMBER 8
	UINT32 msdu_cnt[MSDU_CNT_CR_NUMBER] = {0};
	UINT idx = 0;
	PMAC_TABLE_ENTRY pEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;

	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_1_MSDU_CNT, &msdu_cnt[0]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_2_MSDU_CNT, &msdu_cnt[1]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_3_MSDU_CNT, &msdu_cnt[2]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_4_MSDU_CNT, &msdu_cnt[3]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_5_MSDU_CNT, &msdu_cnt[4]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_6_MSDU_CNT, &msdu_cnt[5]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_7_MSDU_CNT, &msdu_cnt[6]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_8_MSDU_CNT, &msdu_cnt[7]);

	MTWF_PRINT("=== HW_AMSDU INFO.===\n");
	for (idx = 0; idx < MSDU_CNT_CR_NUMBER; idx++) {
		MTWF_PRINT("PACK_%d_MSDU_CNT=%d\n", (idx+1), msdu_cnt[idx]);
	}
	for (idx = 0; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++) {
		pEntry = &pAd->MacTab.Content[idx];
		tr_entry = &tr_ctl->tr_entry[idx];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if ((pEntry->Sst == SST_ASSOC) &&
			(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) &&
			(pEntry->tx_amsdu_bitmap != 0)) {
			MTWF_PRINT("Wcid%03d: %02x", idx, pEntry->tx_amsdu_bitmap);
		}
	}
	return TRUE;
}

VOID mtf_update_mib_bucket(RTMP_ADAPTER *pAd)
{
	/*update chip specific mib stats*/
	chip_update_mib_bucket(pAd);

}

#ifdef OFFCHANNEL_ZERO_LOSS
VOID mtf_read_channel_stat_registers(RTMP_ADAPTER *pAd, UINT8 BandIdx, void *ChStat)
{
	/*update chip specific mib stats*/
	chip_read_channel_stat_registers(pAd, BandIdx, ChStat);

}
#endif

/*
	========================================================================

	Routine Description:
		Read statistical counters from hardware registers and record them
		in software variables for later on query

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
#ifdef MT7915_MT7916_COEXIST_COMPATIBLE
static VOID NICUpdateAmpduRawCounters(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
}
#else
static VOID NICUpdateAmpduRawCounters(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	/* for PER debug */
	UINT32 AmpduTxCount = 0;
	UINT32 AmpduTxSuccessCount = 0;
	COUNTER_802_11 *wlanCounter;
	UINT32 mac_val = 0;
	UINT32 ampdu_range_cnt[4] = {0};
	UINT32 Offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * BandIdx;
	COUNTER_802_3 *dot3Counters;
	UINT32 rx_fcs_error_cnt = 0;
	UINT32 rx_fifo_full_cnt = 0;
	UINT32 rx_mdrdy_cnt = 0;
	UINT32 old_val;

	dot3Counters = &pAd->Counters8023;
	wlanCounter = &pAd->WlanCounters[BandIdx];

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR14_ADDR + Offset, &AmpduTxCount);
		AmpduTxCount &= BN0_WF_MIB_TOP_M0SDR14_AMPDU_MPDU_COUNT_MASK;

		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0SDR15_ADDR + Offset, &AmpduTxSuccessCount);
		AmpduTxSuccessCount &= BN0_WF_MIB_TOP_M0SDR15_AMPDU_ACKED_COUNT_MASK;

		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR2_ADDR + Offset, &mac_val);
		ampdu_range_cnt[0] = mac_val & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE0_CNT_MASK;
		ampdu_range_cnt[1] = (mac_val & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_SHFT;

		MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_MIB_TOP_M0DR3_ADDR + Offset, &mac_val);
		ampdu_range_cnt[2] = mac_val & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE2_CNT_MASK;
		ampdu_range_cnt[3] = (mac_val & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_SHFT;
	}
#endif

#ifdef STATS_COUNT_SUPPORT
	wlanCounter->AmpduSuccessCount.u.LowPart += AmpduTxSuccessCount;
	wlanCounter->AmpduFailCount.u.LowPart += (AmpduTxCount - AmpduTxSuccessCount);
#endif
	wlanCounter->TxAggRange1Count.u.LowPart += ampdu_range_cnt[0];
	wlanCounter->TxAggRange2Count.u.LowPart += ampdu_range_cnt[1];
	wlanCounter->TxAggRange3Count.u.LowPart += ampdu_range_cnt[2];
	wlanCounter->TxAggRange4Count.u.LowPart += ampdu_range_cnt[3];

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		MT_PARTIAL_MIB_INFO_CNT_CTRL_T PartialMibInfoCtrl;

		memset(&PartialMibInfoCtrl, 0, sizeof(MT_PARTIAL_MIB_INFO_CNT_CTRL_T));
		Offset = 0x10000 * BandIdx;

		if (pAd->mcli_ctl[BandIdx].debug_on & MCLI_DEBUG_RMAC_DROP) {
			UINT32 val = 0;

			MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_RX_STS3_ADDR + Offset, &val);
			pAd->mcli_ctl[BandIdx].RxFIFONotEnoughCnt += ((val & (1 << 26)) > 0) ? 1 : 0;
			pAd->mcli_ctl[BandIdx].RxPPDUDropCnt += ((val & (1 << 24)) > 0) ? 1 : 0;
			val = 0xdf000000;
			MAC_IO_WRITE32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_DBGCTRL_ADDR + Offset, val);
			val = 0x1f000000;
			MAC_IO_WRITE32(pAd->hdev_ctrl, BN0_WF_RMAC_TOP_DBGCTRL_ADDR + Offset, val);
		}

		if (pAd->partial_mib_show_en == 1) {
			MtCmdGetPartialMibInfoCnt(pAd, BandIdx, &PartialMibInfoCtrl);

			wlanCounter->RxFcsErrorCount.u.LowPart +=
				PartialMibInfoCtrl.rMibInfoParam.u4RxFcsErrCnt;
			wlanCounter->RxFifoFullCount.u.LowPart +=
				PartialMibInfoCtrl.rMibInfoParam.u4RxFifoOverflowCnt;
			wlanCounter->RxMpduCount.QuadPart +=
				PartialMibInfoCtrl.rMibInfoParam.u4RxMpduCnt;
			wlanCounter->ChannelIdleCount.QuadPart +=
				PartialMibInfoCtrl.rMibInfoParam.u4RxChannelIdleCnt;
			wlanCounter->CcaNavTxTime.QuadPart =
				PartialMibInfoCtrl.rMibInfoParam.u4CcaNavTxTimeCnt;
			wlanCounter->RxMdrdyCount.QuadPart +=
				PartialMibInfoCtrl.rMibInfoParam.u4MdrdyCnt;
			wlanCounter->SCcaTime.QuadPart =
				PartialMibInfoCtrl.rMibInfoParam.u4SCcaCnt;
			wlanCounter->PEdTime.QuadPart =
				PartialMibInfoCtrl.rMibInfoParam.u4PEdCnt;
			wlanCounter->RxTotByteCount.QuadPart +=
				PartialMibInfoCtrl.rMibInfoParam.u4RxTotalByteCnt;
		} else if (pAd->RxDebug[BandIdx] != TRUE) {
			UINT32 rx_mpdu_cnt = 0;
			UINT32 rx_total_byte_cnt = 0;
			UINT32 cca_nav_tx_time_cnt = 0;

			MAC_IO_READ32(pAd->hdev_ctrl,
			BN0_WF_MIB_TOP_M0SDR5_RX_MPDU_COUNT_ADDR + Offset,
			&rx_mpdu_cnt);
			wlanCounter->RxMpduCount.QuadPart += rx_mpdu_cnt;

			MAC_IO_READ32(pAd->hdev_ctrl,
				BN0_WF_MIB_TOP_M0SDR23_RX_TOTBYTE_COUNT_ADDR + Offset,
				&rx_total_byte_cnt);
			wlanCounter->RxTotByteCount.QuadPart += rx_total_byte_cnt;

			MAC_IO_READ32(pAd->hdev_ctrl,
				BN0_WF_MIB_TOP_M0SDR3_RX_FCS_ERROR_COUNT_ADDR + Offset,
				&rx_fcs_error_cnt);
			rx_fcs_error_cnt &= BN0_WF_MIB_TOP_M0SDR3_RX_FCS_ERROR_COUNT_MASK;
			rx_fcs_error_cnt >>= BN0_WF_MIB_TOP_M0SDR3_RX_FCS_ERROR_COUNT_SHFT;
			wlanCounter->RxFcsErrorCount.u.LowPart += rx_fcs_error_cnt;

			MAC_IO_READ32(pAd->hdev_ctrl,
				BN0_WF_MIB_TOP_M0SDR4_RX_FIFO_FULL_COUNT_ADDR + Offset,
				&rx_fifo_full_cnt);
			rx_fifo_full_cnt &= BN0_WF_MIB_TOP_M0SDR4_RX_FIFO_FULL_COUNT_MASK;
			wlanCounter->RxFifoFullCount.u.LowPart += rx_fifo_full_cnt;

			MAC_IO_READ32(pAd->hdev_ctrl,
				BN0_WF_MIB_TOP_M0SDR10_RX_MDRDY_COUNT_ADDR + Offset,
				&rx_mdrdy_cnt);
			rx_mdrdy_cnt &= BN0_WF_MIB_TOP_M0SDR10_RX_MDRDY_COUNT_MASK;

			MAC_IO_READ32(pAd->hdev_ctrl,
				BN0_WF_MIB_TOP_M0SDR9_CCA_NAV_TX_TIME_ADDR + Offset,
				&cca_nav_tx_time_cnt);
			cca_nav_tx_time_cnt &= BN0_WF_MIB_TOP_M0SDR9_CCA_NAV_TX_TIME_MASK;
			wlanCounter->CcaNavTxTime.QuadPart = cca_nav_tx_time_cnt;
		}
	} else
#endif
	{
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR3 + Offset,
			&rx_fcs_error_cnt);
		rx_fcs_error_cnt &= 0xffff;
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR4 + Offset,
			&rx_fifo_full_cnt);

		if (pAd->parse_rxv_stat_enable)
			MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR10 + Offset,
			&rx_mdrdy_cnt);
	}

	if (pAd->parse_rxv_stat_enable) {
		/* Used for rx statistic */
		if (BandIdx == 0) {
			pAd->AccuOneSecRxBand0FcsErrCnt += rx_fcs_error_cnt;
			pAd->AccuOneSecRxBand0MdrdyCnt += rx_mdrdy_cnt;
		} else {
			pAd->AccuOneSecRxBand1FcsErrCnt += rx_fcs_error_cnt;
			pAd->AccuOneSecRxBand1MdrdyCnt += rx_mdrdy_cnt;
		}
	}

#ifdef STATS_COUNT_SUPPORT
	old_val = wlanCounter->FCSErrorCount.u.LowPart;
	wlanCounter->FCSErrorCount.u.LowPart += rx_fcs_error_cnt;

	if (wlanCounter->FCSErrorCount.u.LowPart < old_val)
		wlanCounter->FCSErrorCount.u.HighPart++;
#endif

	dot3Counters->RxNoBuffer += rx_fifo_full_cnt;
}
#endif

VOID mtf_update_raw_counters(RTMP_ADAPTER *pAd)
{
	UINT32 i;
#ifdef CONFIG_QA
	COUNTER_802_11 *wlanCounter;
	wlanCounter = &pAd->WlanCounters[0];
#endif

#ifdef ERR_RECOVERY
	if (IsStopingPdma(&pAd->ErrRecoveryCtl))
		return;
#endif /* ERR_RECOVERY */

	for (i = 0; i < DBDC_BAND_NUM; i++)
		NICUpdateAmpduRawCounters(pAd, i);

#ifdef CONFIG_QA
	if (pAd->ATECtrl.bQAEnabled == TRUE) {
		/* Modify Rx stat structure */
		/* pAd->ATECtrl.rx_stat.RxMacFCSErrCount += fcs_err_cnt; */
		MT_ATEUpdateRxStatistic(pAd, 3, wlanCounter);
	}
#endif
	return;
}


/*
	========================================================================

	Routine Description:
		Clean all Tx/Rx statistic raw counters from hardware registers

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	========================================================================
*/

#define TMI_TX_RATE_CCK_VAL(_mcs) \
	((TMI_TX_RATE_MODE_CCK << TXD_TX_RATE_BIT_MODE) | (_mcs))

#define TMI_TX_RATE_OFDM_VAL(_mcs) \
	((TMI_TX_RATE_MODE_OFDM << TXD_TX_RATE_BIT_MODE) | (_mcs))

#define TMI_TX_RATE_HT_VAL(_mode, _mcs, _stbc) \
	(((_stbc) << TXD_TX_RATE_BIT_STBC) |\
	 ((_mode) << TXD_TX_RATE_BIT_MODE) | \
	 (_mcs))

#define TMI_TX_RATE_VHT_VAL(_nss, _mcs, _stbc) \
	(((_stbc) << TXD_TX_RATE_BIT_STBC) |\
	 (((_nss - 1) & (TXD_TX_RATE_MASK_NSS)) << TXD_TX_RATE_BIT_NSS) | \
	 (TMI_TX_RATE_MODE_VHT << TXD_TX_RATE_BIT_MODE) | \
	 (_mcs))

#define TMI_TX_RATE_HE_VAL(_mode, _nss, _mcs, _stbc) \
	(((_stbc) << TXD_TX_RATE_BIT_STBC) |\
	 (((_nss - 1) & (TXD_TX_RATE_MASK_NSS)) << TXD_TX_RATE_BIT_NSS) | \
	 (_mode << TXD_TX_RATE_BIT_MODE) | \
	 (_mcs))


UCHAR mtf_get_nsts_by_mcs(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss)
{
	UINT8 nsts = 1;

	switch (phy_mode) {
	case MODE_HE_SU:
	case MODE_HE_EXT_SU:
	case MODE_VHT:
	case MODE_HE_TRIG:
	case MODE_HE_MU:
		if (stbc && (vht_nss == 1))
			nsts++;
		else
			nsts = vht_nss;

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		if (mcs != 32) {
			nsts += (mcs >> 3);

			if (stbc && (nsts == 1))
				nsts++;
		}

		break;

	case MODE_CCK:
	case MODE_OFDM:
	default:
		break;
	}

	return nsts;
}

static const  UCHAR fmac_wmm_aci_2_hw_ac_que[4][4] = {
	{
		TxQ_IDX_AC1, /* 0: QID_AC_BK */
		TxQ_IDX_AC0, /* 1: QID_AC_BE */
		TxQ_IDX_AC2, /* 2: QID_AC_VI */
		TxQ_IDX_AC3, /* 3: QID_AC_VO */
	},
	{
		TxQ_IDX_AC11,
		TxQ_IDX_AC10,
		TxQ_IDX_AC12,
		TxQ_IDX_AC13,
	},
	{
		TxQ_IDX_AC21,
		TxQ_IDX_AC20,
		TxQ_IDX_AC22,
		TxQ_IDX_AC23,
	},
	{
		TxQ_IDX_AC31,
		TxQ_IDX_AC30,
		TxQ_IDX_AC32,
		TxQ_IDX_AC33,
	}
};

static const  UCHAR fmac_wmm_swq_2_hw_ac_que[4][4] = {
	{
		TxQ_IDX_AC0, /* 0: QID_AC_BK */
		TxQ_IDX_AC1, /* 1: QID_AC_BE */
		TxQ_IDX_AC2, /* 2: QID_AC_VI */
		TxQ_IDX_AC3, /* 3: QID_AC_VO */
	},
	{
		TxQ_IDX_AC10,
		TxQ_IDX_AC11,
		TxQ_IDX_AC12,
		TxQ_IDX_AC13,
	},
	{
		TxQ_IDX_AC20,
		TxQ_IDX_AC21,
		TxQ_IDX_AC22,
		TxQ_IDX_AC23,
	},
	{
		TxQ_IDX_AC30,
		TxQ_IDX_AC31,
		TxQ_IDX_AC32,
		TxQ_IDX_AC33,
	}
};

UINT32 mtf_get_hwq_from_ac(UINT8 wmm_idx, UINT8 wmm_ac)
{
	if (wmm_idx >= 4 || wmm_ac > QID_AC_VO)
		return TxQ_IDX_AC0;
	return fmac_wmm_aci_2_hw_ac_que[wmm_idx][wmm_ac];
}

VOID mtf_write_tmac_info_fixed_rate(
	RTMP_ADAPTER *pAd,
	UCHAR *tmac_info,
	MAC_TX_INFO *info,
	HTTRANSMIT_SETTING *transmit)
{
	MAC_TABLE_ENTRY *mac_entry = NULL;
	CHAR stbc, bw, mcs, nss = 1, sgi, phy_mode, ldpc = 0, preamble = LONG_PREAMBLE;
	UCHAR q_idx = info->q_idx;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = NULL;
	struct txd_l *txd = (struct txd_l *)tmac_info;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	NdisZeroMemory(txd, sizeof(*txd));

	if (VALID_UCAST_ENTRY_WCID(pAd, info->WCID))
		mac_entry = &pAd->MacTab.Content[info->WCID];

	/*  DW0 */

	/* TX Byte Count [15:0] */
	txd->txd_0 |= ((sizeof(struct txd_l) + info->Length)
				<< TXD_TX_BYTE_COUNT_SHIFT);

	/* PKT_FT [24:23] */
	if (info->IsOffloadPkt == TRUE)
		txd->txd_0 |= (FT_MCU_FW << TXD_PKT_FT_SHIFT);
	else
		txd->txd_0 |= (FT_HIF_CTD << TXD_PKT_FT_SHIFT);

	/* Q_IDX [31:25] */
	if (q_idx < 4) {
		txd->txd_0 |= (fmac_wmm_swq_2_hw_ac_que[info->wmm_set][q_idx] << TXD_Q_IDX_SHIFT);
	} else {
		/* Falcon remove ALTX1/BMC1/BCN1/PSMP1 and add TGID for band indication */
		if ((q_idx >= TxQ_IDX_ALTX1) && (q_idx <= TxQ_IDX_PSMP1)) {
#ifdef MT7915_E1_WORKAROUND
			/* HWITS00004727 - [E1 SW Workaroud] Band1 ALTX/BMC/PSMP_Q not apply TGID setting */
			if (MTK_REV_ET(pAd, MT7915, MT7915E1) && (q_idx != TxQ_IDX_BCN1)) {
				txd->txd_0 |= (q_idx << TXD_Q_IDX_SHIFT);
			} else
#endif
				txd->txd_0 |= ((q_idx - (TxQ_IDX_ALTX1 - TxQ_IDX_ALTX0)) << TXD_Q_IDX_SHIFT);

			txd->txd_1 |= TXD_TGID;
		} else {
			txd->txd_0 |= (q_idx << TXD_Q_IDX_SHIFT);
		}
	}

	/* DW1 */

	/* WLAN Index [9:0] */
	txd->txd_1 |= (info->WCID << TXD_WLAN_IDX_SHIFT);

	/*VTA [10]*/
#ifdef EAP_STATS_SUPPORT
	txd->txd_1 |= TXD_VTA;
#endif

	/* HEADER_LENGTH [15:11] (HF=2'b10) */
	/* MRD [11] (HF=2'b00) */
	/* EOSP [12] (HF=2'b00) */
	/* RMVL [13] (HF=2'b00) */
	/* VLAN [14] (HF=2'b00) */
	/* ETYP [15] (HF=2'b00) */
	/* HF [17:16] */
	/* Header Padding [19:18] */
	txd->txd_1 |= ((info->hdr_len >> 1) << TXD_HDR_LEN_SHIFT);

	txd->txd_1 |= (HF_802_11_FRAME << TXD_HF_SHIFT);

	if (info->hdr_pad) {
		txd->txd_1 |= (((TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1)
					<< TXD_HDR_PAD_SHIFT);
	}

	/* TID [22:20]  */
	txd->txd_1 |= (info->TID << TXD_TID_SHIFT);

	/* OM [29:24] */
	if (mac_entry && IS_ENTRY_REPEATER(mac_entry)) {
		tr_entry = &tr_ctl->tr_entry[mac_entry->wcid];
		txd->txd_1 |= (tr_entry->OmacIdx << TXD_OM_SHIFT);
	} else if (mac_entry && !IS_ENTRY_NONE(mac_entry) && !IS_ENTRY_MCAST(mac_entry)) {
		txd->txd_1 |= (mac_entry->wdev->OmacIdx << TXD_OM_SHIFT);
	} else {
		txd->txd_1 |= (info->OmacIdx << TXD_OM_SHIFT);
	}

	/* FT [31] */
	txd->txd_1 |= TXD_FT;

	/* DW2 */
	/* Subtype [3:0] */
	/* Type [5:4] */
	if (info->IsOffloadPkt == TRUE) {
		txd->txd_2 |= ((info->SubType & TXD_SUBTYPE_MASK) << TXD_SUBTYPE_SHIFT);
		txd->txd_2 |= ((info->Type & TXD_TYPE_MASK) << TXD_TYPE_SHIFT);
	}

	/* BM [10]  */
	if (info->BM)
		txd->txd_2 |= TXD_BM;

	/* B [11] */
	if (info->prot == 2) {
#ifdef CONFIG_AP_SUPPORT
		if (mac_entry) {
			struct wifi_dev *wdev = mac_entry->wdev;
			GET_GroupKey_WCID(wdev, info->WCID);
		}
		txd->txd_1 |= (info->WCID << TXD_WLAN_IDX_SHIFT);
#endif

		txd->txd_2 |= TXD_B;
	}

	if (info->sw_duration)
		txd->txd_2 |= TXD_DU;

	/* H/W won't add HTC for mgmt/ctrl frame */
	if (((info->Type == FC_TYPE_MGMT) || ((info->Type == FC_TYPE_CNTL))) &&
		cap->mgmt_ctrl_frm_hw_htc_disable)
		info->htc = TRUE;

	if (info->htc)
		txd->txd_2 |= TXD_HE;

	/* life time [23:16]*/
	/* convert timing unit from 32TU to 64TU */
	txd->txd_2 |= (((info->tx_lifetime >> 1) << TXD_REMAIN_TIME_SHIFT) & TXD_REMAIN_TIME_MASK);

	/* FR [31] */
	if (!info->IsAutoRate)
		txd->txd_2 |= TXD_FR;

	txd->txd_2 &= ~TXD_PWR_OFFESET_MASK;
	txd->txd_2 |= (((info->txpwr_offset) & 0x3f) << TXD_PWR_OFFESET_SHIFT);

	/* DW3 */
	/* NA [0] */
	/* Remaining TX Count [15:11] */

	if (info->Ack) {
		txd->txd_3 |= (MT_TX_LONG_RETRY << TXD_REMAIN_TX_CNT_SHIFT);
	} else {
		txd->txd_3 |= TXD_NA;
		txd->txd_3 |= (MT_TX_RETRY_UNLIMIT << TXD_REMAIN_TX_CNT_SHIFT);
	}


	/* PF [1] */
	if (info->prot == 1)
		txd->txd_3 |= TXD_PF;


	/* TM [5] */
	if (cap->TmrEnable == 1) {
		if (info->IsTmr)
			txd->txd_3 |= TXD_TM;
	}

	/* Power Management [29] */
	if (info->PsmBySw)
		txd->txd_3 |= TXD_PM;

	if (info->NSeq) {
		txd->txd_3 |= TXD_SN_VLD;
		txd->txd_3 |= ((info->assigned_seq & 0xfff) << TXD_SN_SHIFT);
	}

#ifdef AUTOMATION

			if (is_frame_test(pAd, 0) == 2) {
				if (info->Type == FC_TYPE_MGMT && info->SubType == SUBTYPE_BEACON)
				;
				else {
					txd->txd_5 &= (~TXD_PID_MASK);
					txd->txd_5 |= (pAd->auto_dvt->txs.pid << TXD_PID_SHIFT);

					txd->txd_5 &= (~TXD_TXSFM);
					txd->txd_5 |= (pAd->auto_dvt->txs.format << TXD_TXSFM_SHIFT);

					txd->txd_5 &= (~TXD_TXS2M);
					txd->txd_5 |= TXD_TXS2H;

					send_add_txs_queue(pAd->auto_dvt->txs.pid, info->WCID);
				}
			}
#endif /* AUTOMATION */


	/* DW5 */
	if (info->txs2m)
		txd->txd_5 |= TXD_TXS2M;
#ifndef AUTOMATION
	if (info->txs2h)
		txd->txd_5 |= TXD_TXS2H;
	txd->txd_5 |= (info->PID << TXD_PID_SHIFT);

	if (info->addba)
		txd->txd_5 |= TXD_ADD_BA;
	/* LDPC [11] */
#endif
	/* LDPC [1:1] */
	/* GI [15:14] */
	/* DW6 */
	/* BW [2:0]  */
	/* Rate to be Fixed [29:16] */

	if (!info->IsAutoRate
#ifdef MGMT_TXPWR_CTRL
		/* ensure correct txd rate when set mgmt_frame_power in mt7915 */
		|| (IS_MT7915(pAd) && info->Type == FC_TYPE_MGMT)
#endif
		) {
		ldpc = transmit->field.ldpc;

		if (ldpc)
			txd->txd_6 |= TXD_LDPC;
		sgi = transmit->field.ShortGI;
		txd->txd_6 |= ((sgi << TXD_GI_SHIFT) & TXD_GI_MASK);

		if (transmit->field.MODE == MODE_HE) {
			/* use txd reserved mode for HE */
			phy_mode = MODE_HE_SU;
			txd->txd_6 |= ((0x1 << TXD_GI_SHIFT) & TXD_GI_MASK);					/* 01: 2x HT_LTF + 0.8us GI */
			txd->txd_6 |= ((0x1 << TXD_HELTF_TYPE_SHIFT) & TXD_HELTF_TYPE_MASK);	/* 01: 2x HELTF, 6.4us */
		} else {
			phy_mode = transmit->field.MODE;
			txd->txd_6 |= ((0x0 << TXD_GI_SHIFT) & TXD_GI_MASK);
			txd->txd_6 |= ((0x0 << TXD_HELTF_TYPE_SHIFT) & TXD_HELTF_TYPE_MASK);
		}

		bw = (phy_mode <= MODE_CCK) ? (BW_20) : (transmit->field.BW);

		txd->txd_6 |= (((1 << 2) | bw) << TXD_BW_SHIFT);

		mcs = transmit->field.MCS;
		stbc = transmit->field.STBC;

		if (phy_mode == MODE_CCK)
			preamble = info->Preamble;

		nss = 1;

		txd->txd_6 |= (mtf_tx_rate_to_tmi_rate(phy_mode, mcs, 1, stbc, preamble) << TXD_FR_RATE_SHIFT) ;
	}

	/* DW7 */
	/* SPE_IDX [15:11] */
	/* Subtype [19:16] */
	/* Type [21:20] */
	txd->txd_7 |= (info->AntPri << TXD_SPE_IDX_SHIFT);
	txd->txd_7 |= (info->SubType  << TXD_PP_SUBTYPE_SHIFT);
	txd->txd_7 |= (info->Type << TXD_PP_TYPE_SHIFT);

}

VOID mtf_write_tmac_info(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_MCU_OFFLOAD))
		mtf_write_tmac_info_by_wa(pAd, buf, pTxBlk);
	else
		mtf_write_tmac_info_by_host(pAd, buf, pTxBlk);
}

UINT16 mtf_tx_rate_to_tmi_rate(UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble)
{
	UINT16 tmi_rate = 0, mcs_id = 0;

	stbc = (stbc == TRUE) ? 1 : 0;

	switch (mode) {
	case MODE_CCK:
		if (preamble) {
			if (mcs < tmi_rate_map_cck_lp_size)
				mcs_id = tmi_rate_map_cck_lp[mcs];
		} else {
			if (mcs < tmi_rate_map_cck_sp_size)
				mcs_id = tmi_rate_map_cck_sp[mcs];
		}

		tmi_rate = (TMI_TX_RATE_MODE_CCK << TXD_TX_RATE_BIT_MODE) | (mcs_id);
		break;

	case MODE_OFDM:
		if (mcs < tmi_rate_map_ofdm_size) {
			mcs_id = tmi_rate_map_ofdm[mcs];
			tmi_rate = (TMI_TX_RATE_MODE_OFDM << TXD_TX_RATE_BIT_MODE) | (mcs_id);
		}

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		tmi_rate = ((USHORT)(stbc << TXD_TX_RATE_BIT_STBC)) |
				   (((nss - 1) & TXD_TX_RATE_MASK_NSS) << TXD_TX_RATE_BIT_NSS) |
				   ((USHORT)(mode << TXD_TX_RATE_BIT_MODE)) |
				   ((USHORT)(mcs));
		break;

	case MODE_VHT:
		tmi_rate = TMI_TX_RATE_VHT_VAL(nss, mcs, stbc);
		break;

	case MODE_HE:
	case MODE_HE_SU:
	case MODE_HE_EXT_SU:
	case MODE_HE_TRIG:
	case MODE_HE_MU:
		tmi_rate = TMI_TX_RATE_HE_VAL(mode, nss, mcs, stbc);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid mode(mode=%d)\n",
				 mode);
		break;
	}

	return tmi_rate;
}

VOID mtf_write_tmac_info_by_host(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *tx_blk)
{
	struct txd_l *txd = (struct txd_l *)buf;
	MAC_TABLE_ENTRY *mac_entry = tx_blk->pMacEntry;
	struct wifi_dev *wdev  = tx_blk->wdev;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	HTTRANSMIT_SETTING *transmit = tx_blk->pTransmit;
	struct phy_params *phy_info = NULL;
	UCHAR stbc = 0, bw = 0, tx_bw = BW_20, mcs = 0, nss = 0, sgi = 0, ltf_type = 0, phy_mode = 0, preamble = 1, ldpc = 0;
	UCHAR tx_ibf = 0, tx_ebf = 0;
	STA_TR_ENTRY *tr_entry = NULL;
#ifdef PER_PKT_CTRL_FOR_CTMR
	TRANSMIT_SETTING_HE *transmit_he;
#endif
	PEDCA_PARM pEdca = HcGetEdca(pAd, tx_blk->wdev);

	if (!transmit && tx_blk->pMacEntry)
		phy_info = &tx_blk->pMacEntry->phy_param;

	NdisZeroMemory(txd, sizeof(*txd));

	/*  DW0 */

	/* TX Byte Count [15:0] */
	txd->txd_0 |= (tx_blk->tx_bytes_len << TXD_TX_BYTE_COUNT_SHIFT);

	/* PKT_FT [24:23] */
	txd->txd_0 |= (FT_HIF_CTD << TXD_PKT_FT_SHIFT);

	/* Q_IDX [31:25] */
	if (tx_blk->QueIdx < 4) {
		txd->txd_0 |= (fmac_wmm_swq_2_hw_ac_que[tx_blk->wmm_set][tx_blk->QueIdx] << TXD_Q_IDX_SHIFT);
		if (HcGetBandByWdev(wdev))
			txd->txd_1 |= TXD_TGID;
	} else {
		/* Falcon remove ALTX1/BMC1/BCN1/PSMP1 and add TGID for band indication */
		if ((tx_blk->QueIdx >= TxQ_IDX_ALTX1) && (tx_blk->QueIdx <= TxQ_IDX_PSMP1)) {
#ifdef MT7915_E1_WORKAROUND
			/* HWITS00004727 - [E1 SW Workaroud] Band1 ALTX/BMC/PSMP_Q not apply TGID setting */
			if (MTK_REV_ET(pAd, MT7915, MT7915E1) && (tx_blk->QueIdx != TxQ_IDX_BCN1)) {
				txd->txd_0 |= (tx_blk->QueIdx << TXD_Q_IDX_SHIFT);
			} else
#endif
				txd->txd_0 |= ((tx_blk->QueIdx - (TxQ_IDX_ALTX1 - TxQ_IDX_ALTX0)) << TXD_Q_IDX_SHIFT);

			txd->txd_1 |= TXD_TGID;
		} else {
			txd->txd_0 |= (tx_blk->QueIdx << TXD_Q_IDX_SHIFT);
		}
	}

	/* DW1 */

	/* WLAN Index [9:0] */
	txd->txd_1 |= (tx_blk->Wcid << TXD_WLAN_IDX_SHIFT);

    /* VTA [10] */
#ifdef IGMP_SNOOPING_NON_OFFLOAD
		if (wdev->IgmpSnoopEnable && TX_BLK_TEST_FLAG(tx_blk, fTX_MCAST_CLONE)) {
				txd->txd_1 |= TXD_VTA;
		}
#endif
#ifdef EAP_STATS_SUPPORT
	txd->txd_1 |= TXD_VTA;
#endif


	/* HEADER_LENGTH [15:11] (HF=2'b10) */
	/* MRD [11] (HF=2'b00) */
	/* EOSP [12] (HF=2'b00) */
	/* RMVL [13] (HF=2'b00) */
	/* VLAN [14] (HF=2'b00) */
	/* ETYP [15] (HF=2'b00) */
	/* HF [17:16] */
	/* Header Padding [19:18] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS)) {

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bMoreData))
			txd->txd_1 |= TXD_MRD;

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bWMM_UAPSD_EOSP))
			txd->txd_1 |= TXD_EOSP;

		txd->txd_1 |= TXD_RMVL;

		if (RTMP_GET_PACKET_VLAN(tx_blk->pPacket))
			txd->txd_1 |= TXD_VLAN;

		if (RTMP_GET_PACKET_PROTOCOL(tx_blk->pPacket) > 1500)
			txd->txd_1 |= TXD_ETYP;

#if defined(IGMP_SNOOPING_NON_OFFLOAD) && defined(VLAN_SUPPORT)
		if ((mac_entry && mac_entry->wdev && mac_entry->wdev->bVLAN_Tag) &&
			(wdev->IgmpSnoopEnable && TX_BLK_TEST_FLAG(tx_blk, fTX_MCAST_CLONE))) {
			txd->txd_1 &= ~TXD_RMVL;
			/* For IOT, identify the vlan packet as a non-vlan pkt. Otherwise, the pkt will have 2 LLC headers */
			if (RTMP_GET_PACKET_PROTOCOL(tx_blk->pPacket) > 1500)
				txd->txd_1 &= ~TXD_VLAN;
			else
				txd->txd_1 |= TXD_VLAN;
		}
#endif

		txd->txd_1 |= (HF_802_3_FRAME << TXD_HF_SHIFT);

		if (tx_blk->HdrPadLen) {
			txd->txd_1 |= (((TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1)
						<< TXD_HDR_PAD_SHIFT);
		}
	} else {
		txd->txd_1 |= ((tx_blk->wifi_hdr_len >> 1) << TXD_HDR_LEN_SHIFT);

		txd->txd_1 |= (HF_802_11_FRAME << TXD_HF_SHIFT);

		if (tx_blk->HdrPadLen) {
			txd->txd_1 |= (((TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1)
						<< TXD_HDR_PAD_SHIFT);

		}

	}

	/* TID [22:20]  */
	txd->txd_1 |= (tx_blk->UserPriority << TXD_TID_SHIFT);

	/* AMSDU [23] */
	if (tx_blk->TxFrameType == TX_AMSDU_FRAME)
		txd->txd_1 |= TXD_AMSDU;

	/* OM [29:24] */
	if (mac_entry && IS_ENTRY_REPEATER(mac_entry)) {
		if (tx_blk->tr_entry) {
			tr_entry = tx_blk->tr_entry;
			txd->txd_1 |= (tr_entry->OmacIdx << TXD_OM_SHIFT);
		}
	} else {
		txd->txd_1 |= (wdev->OmacIdx << TXD_OM_SHIFT);
	}

	/* FT [31] */
	txd->txd_1 |= TXD_FT;

	/* BM [10]  */
	if (tx_blk->TxFrameType == TX_MCAST_FRAME)
		txd->txd_2 |= TXD_BM;

	/* FRAG [15:14] */
	txd->txd_2 |= (tx_blk->FragIdx << TXD_FRAG_SHIFT);

	/* FR [31] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceRate))
		txd->txd_2 |= TXD_FR;

	/* DW3 */
	/* NA [0] */
	/* Remaining TX Count [15:11] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAckRequired)) {
		txd->txd_3 |= (MT_TX_LONG_RETRY << TXD_REMAIN_TX_CNT_SHIFT);
	} else {
		txd->txd_3 |= TXD_NA;
		txd->txd_3 |= (MT_TX_RETRY_UNLIMIT << TXD_REMAIN_TX_CNT_SHIFT);
	}
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bNoRetry)) {
		txd->txd_3 &= ~TXD_REMAIN_TX_CNT_MASK;
		txd->txd_3 |= 0x1 << TXD_REMAIN_TX_CNT_SHIFT;
	}
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bRetryUnlimit)) {
		txd->txd_3 &= ~TXD_REMAIN_TX_CNT_MASK;
		txd->txd_3 |= MT_TX_RETRY_UNLIMIT << TXD_REMAIN_TX_CNT_SHIFT;
	}

	/* PF [1] */
	if (!IS_CIPHER_NONE(tx_blk->CipherAlg))
		txd->txd_3 |= TXD_PF;

	/* DAS [4] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS)) {
		if (TX_BLK_TEST_FLAG(tx_blk, fTX_MCAST_CLONE))
			txd->txd_3 |= TXD_DAS;
	}

	/* TM [5] */
	if (cap->TmrEnable == 1) {
		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAckRequired))
			txd->txd_3 |= TXD_TM;
	}

#ifdef SW_CONNECT_SUPPORT
	/* SN [27:16] */
	if (tx_blk->tr_entry) {
			if (tx_blk->tr_entry->bSw) {
				if (tx_blk->wifi_hdr) {
					HEADER_802_11 *wifi_hdr = (HEADER_802_11 *)tx_blk->wifi_hdr;
					txd->txd_3 |= TXD_SN_VLD;
					txd->txd_3 |= ((wifi_hdr->Sequence & 0xfff) << TXD_SN_SHIFT);
				}
			}
	}
#endif /* SW_CONNECT_SUPPORT */

	/* Power Management [29] */
#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
	txd->txd_3 &= ~TXD_PM;
#else
	txd->txd_3 |= TXD_PM;
#endif

#ifdef IGMP_SNOOPING_NON_OFFLOAD
	if (wdev->IgmpSnoopEnable && TX_BLK_TEST_FLAG(tx_blk, fTX_MCAST_CLONE)) {
		txd->txd_3 &= ~TXD_PM;
	}
#endif

#ifdef AUTOMATION
		if (is_frame_test(pAd, 0) == 1) {
			txd->txd_5 &= (~TXD_PID_MASK);
			txd->txd_5 |= (pAd->auto_dvt->txs.pid << TXD_PID_SHIFT);

			txd->txd_5 &= (~TXD_TXSFM);
			txd->txd_5 |= (pAd->auto_dvt->txs.format << TXD_TXSFM_SHIFT);

			txd->txd_5 &= (~TXD_TXS2M);
			txd->txd_5 |= TXD_TXS2H;

			send_add_txs_queue(pAd->auto_dvt->txs.pid, tx_blk->Wcid);
		}

#endif /* AUTOMATION */

	/* DW5 */
	if (0)
		txd->txd_5 |= TXD_TXS2H;
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAteTxsRequired)) {
		txd->txd_5 &= ~TXD_TXS2M;
		txd->txd_5 |= TXD_TXS2H;
		txd->txd_5 |= TXD_TXSFM;
	}

#ifdef IGMP_SNOOPING_NON_OFFLOAD
	if (wdev->IgmpSnoopEnable && TX_BLK_TEST_FLAG(tx_blk, fTX_MCAST_CLONE)) {
		txd->txd_5 |= (PID_DATA_AMPDU << TXD_PID_SHIFT) & TXD_PID_SHIFT;
		txd->txd_5 &= ~TXD_TXSFM;
		txd->txd_5 &= ~TXD_TXS2M;
		txd->txd_5 &= ~TXD_TXS2H;
	}
#endif


	/* LDPC [11] */
	/* GI [15:14] */

	/* DW6 */
	/* BW [2:0]  */
	/* Rate to be Fixed [29:16] */

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_ForceRate)) {
		if (transmit) {
			/* backward compatible */
			ldpc = transmit->field.ldpc;
			sgi = transmit->field.ShortGI;
			stbc = transmit->field.STBC;
			mcs = transmit->field.MCS;
			phy_mode = transmit->field.MODE;
			bw = transmit->field.BW;
		} else {
			ldpc = phy_info->ldpc;
			sgi = phy_info->gi_type;
			ltf_type = phy_info->ltf_type;
			stbc = phy_info->stbc;
			mcs = phy_info->rate;
			phy_mode = phy_info->phy_mode;
			bw = phy_info->bw;
			tx_ibf = phy_info->tx_ibf;
			tx_ebf = phy_info->tx_ebf;
		}

		if (ldpc)
			txd->txd_6 |= TXD_LDPC;
		txd->txd_6 |= ((sgi << TXD_GI_SHIFT) & TXD_GI_MASK);
		txd->txd_6 |= ((ltf_type << TXD_HELTF_TYPE_SHIFT) & TXD_HELTF_TYPE_MASK);

		tx_bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (bw);

		txd->txd_6 |= (((1 << 2) | tx_bw) << TXD_BW_SHIFT);

		if (tx_ibf)
			txd->txd_6 |= TXD_TXIBF;

		if (tx_ebf)
			txd->txd_6 |= TXD_TXEBF;

		if (phy_info) {
			nss = mtf_get_nsts_by_mcs(phy_mode, mcs, stbc, (phy_mode >= MODE_VHT) ?
					((phy_info->vht_nss) ? phy_info->vht_nss : 1) : 0);
		} else {
			nss = mtf_get_nsts_by_mcs(phy_mode, mcs, stbc, (phy_mode >= MODE_VHT) ?
					((mcs & (0x3 << 4)) >> 4) + 1 : 0);

#ifdef SW_CONNECT_SUPPORT
			/* Host TxD based fixed rate :  ref TMI_TX_RATE_VHT_VAL() & TMI_TX_RATE_HE_VAL() MCS need to be translate from Set_Fixed_Rate_Proc() */
			if (transmit && (phy_mode >= MODE_VHT))
				 mcs = (mcs & 0xf);
#endif /* SW_CONNECT_SUPPORT */

		}

		if (phy_mode == MODE_CCK) {
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
				preamble = SHORT_PREAMBLE;
			else
				preamble = LONG_PREAMBLE;

		}

		if (phy_info) {
			mcs |= (phy_info->dcm) ? (0x1 << TXD_TX_RATE_BIT_DCM) : 0x0;
			mcs |= (phy_info->su_ext_tone) ? (0x1 << TXD_TX_RATE_BIT_SUEXTTONE) : 0x0;
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
			if (phy_info->spe_idx)
				txd->txd_6 &= ~TXD_SPE_IDX_SEL;	/* 0 means refer spe_idx to TXD ; 1: means refer spe_idx to WTBL */
#endif
		}

		txd->txd_6 |= (mtf_tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble) << TXD_FR_RATE_SHIFT);
	}

	/* DW7 */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bAteAgg)) {
		txd->txd_7 |= (SUBTYPE_QDATA << TXD_PP_SUBTYPE_SHIFT) & TXD_PP_SUBTYPE_MASK;
		txd->txd_7 |= (FC_TYPE_DATA << TXD_PP_TYPE_SHIFT) & TXD_PP_TYPE_MASK;
	}

#ifdef IGMP_SNOOPING_NON_OFFLOAD
	if (wdev->IgmpSnoopEnable && TX_BLK_TEST_FLAG(tx_blk, fTX_MCAST_CLONE)) {
		txd->txd_7 |= (SUBTYPE_DATA << TXD_PP_SUBTYPE_SHIFT) & TXD_PP_SUBTYPE_MASK;
		txd->txd_7 |= (FC_TYPE_DATA << TXD_PP_TYPE_SHIFT) & TXD_PP_TYPE_MASK;
	}
#endif


	/* HW_AMSDU_CAP [10] */
	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HW_AMSDU)) {
		txd->txd_7 |= TXD_HW_AMSDU_CAP;
	}
	if (phy_info) {
		txd->txd_7 |= (phy_info->spe_idx << TXD_SPE_IDX_SHIFT) & TXD_SPE_IDX_MASK;
	}

	if (tx_blk->TotalFrameNum > 4)
		txd->txd_7 |= (TXDLEN_2_PAGE << TXD_TXD_LEN_SHIFT);

	txd->txd_7 &= ~TXD_PP_SUBTYPE_MASK;
	txd->txd_7 |= (pEdca->bValid ? SUBTYPE_QDATA : SUBTYPE_DATA) << TXD_PP_SUBTYPE_SHIFT;

	txd->txd_7 &= ~TXD_PP_TYPE_MASK;
	txd->txd_7 |= FC_TYPE_DATA << TXD_PP_TYPE_SHIFT;

#ifdef PER_PKT_CTRL_FOR_CTMR
	if (tx_blk->ApplyTid) {
		UCHAR q_idx = QID_AC_BE;

		if (tx_blk->TidByHost> 7)
			tx_blk->TidByHost= 7;
		q_idx = WMM_UP2AC_MAP[tx_blk->TidByHost];
		txd->txd_0 &= ~TXD_Q_IDX_MASK;
		txd->txd_0 |= (fmac_wmm_swq_2_hw_ac_que[tx_blk->wmm_set][q_idx] << TXD_Q_IDX_SHIFT);

		txd->txd_1 &= ~TXD_TID_MASK;
		txd->txd_1 |= (tx_blk->TidByHost << TXD_TID_SHIFT);
	}

	if (tx_blk->ApplyRetryLimit) {
		if (tx_blk->RetryLimitByHost > MT_TX_RETRY_UNLIMIT)
			tx_blk->RetryLimitByHost = MT_TX_RETRY_UNLIMIT;

		txd->txd_3 &= ~TXD_REMAIN_TX_CNT_MASK;
		txd->txd_3 |= (tx_blk->RetryLimitByHost << TXD_REMAIN_TX_CNT_SHIFT);

		txd->txd_2 &= ~TXD_REMAIN_TIME_MASK;
		txd->txd_2 |= (tx_blk->MaxTxTimeByHost << TXD_REMAIN_TIME_SHIFT);
	}

	if (tx_blk->ApplyFixRate) {
		UINT8 gi_ltf;

		txd->txd_2 |= TXD_FR;

		transmit_he = &tx_blk->TransmitHeByHost;
		ldpc = transmit_he->field.ldpc;
		if (ldpc)
			txd->txd_6 |= TXD_LDPC;

		gi_ltf = transmit_he->field.GILTF;
		txd->txd_6 |= ((gi_ltf << TXD_GI_SHIFT) & TXD_GI_MASK);
		switch (sgi) {
		case 0: /*1x HELTF + 0.8usGI*/
			txd->txd_6 |= ((0 << TXD_HELTF_TYPE_SHIFT) & TXD_HELTF_TYPE_MASK);
			break;
		case 1: /*2x HELTF + 0.8usGI*/
		case 2: /*2x HELTF + 1.6usGI*/
			txd->txd_6 |= ((1 << TXD_HELTF_TYPE_SHIFT) & TXD_HELTF_TYPE_MASK);
			break;
		case 3: /*4x HELTF + 3.2usGI*/
			txd->txd_6 |= ((2 << TXD_HELTF_TYPE_SHIFT) & TXD_HELTF_TYPE_MASK);
			break;
		}

		phy_mode = transmit_he->field.MODE;
		bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (transmit_he->field.BW);

		txd->txd_6 |= (((1 << 2) | bw) << TXD_BW_SHIFT);

		mcs = transmit_he->field.MCS;
		stbc = transmit_he->field.STBC;

		if (phy_mode == MODE_CCK)
			preamble = LONG_PREAMBLE;

		nss = transmit_he->field.nss + 1;

		txd->txd_6 |= (mtf_tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble) << TXD_FR_RATE_SHIFT) ;

		if (phy_mode <= MODE_OFDM)
			txd->txd_7 &= (~TXD_HW_AMSDU_CAP);
	}

	if (tx_blk->ApplyBaPowerTxs) {
		txd->txd_3 &= ~TXD_BA_DIS;
		txd->txd_3 |= (tx_blk->BaDisableByHost << TXD_BA_DIS_SHIFT);

		txd->txd_2 &= ~TXD_PWR_OFFESET_MASK;
		txd->txd_2 |= (tx_blk->PowerOffsetByHost << TXD_PWR_OFFESET_SHIFT);

		txd->txd_5 &= ~TXD_TXS2H;
		txd->txd_5 |= (tx_blk->TxS2HostByHost << TXD_TXS2H_SHIFT);
		txd->txd_5 &= ~TXD_PID_MASK;
		txd->txd_5 |= (tx_blk->PidByHost << TXD_PID_SHIFT);
	}
#endif

#ifdef RT_BIG_ENDIAN
	MTMacInfoEndianChange(pAd, buf, TYPE_TMACINFO, sizeof(*txd));
#endif
}

VOID mtf_write_tmac_info_by_wa(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *tx_blk)
{
	struct txd_l *txd = (struct txd_l *)buf;

	NdisZeroMemory(txd, sizeof(*txd));

	/* DW0 */
	txd->txd_0 |= (((sizeof(*txd) + tx_blk->MpduHeaderLen + tx_blk->HdrPadLen + tx_blk->SrcBufLen) & TXD_TX_BYTE_COUNT_MASK) << TXD_TX_BYTE_COUNT_SHIFT);
	txd->txd_0 |= (FT_HIF_CTD << TXD_PKT_FT_SHIFT);

	/* DW1 */
	txd->txd_1 |= (HF_802_3_FRAME << TXD_HF_SHIFT);
	txd->txd_1 |= TXD_FT;

	if (tx_blk->HdrPadLen) {
		txd->txd_1 |= (((TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1)
					<< TXD_HDR_PAD_SHIFT);
	}
#ifdef VLAN_SUPPORT
	if ((pAd->tr_ctl.vlan2ethctrl == TRUE)
		&& tx_blk->is_vlan) {
		txd->txd_1 |= TXD_VLAN;
	}
#endif
#ifdef CONFIG_VLAN_GTK_SUPPORT
	if (RTMP_GET_PACKET_VLANGTK(tx_blk->pPacket) > 0) {
		txd->txd_1 &= ~TXD_WLAN_IDX_MASK;
		txd->txd_1 |= tx_blk->Wcid << TXD_WLAN_IDX_SHIFT;
	}
#endif
	/*VTA [10]*/
#ifdef EAP_STATS_SUPPORT
	txd->txd_1 |= TXD_VTA;
#endif

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	/* check queue status */
	if (pAd->vow_dvt_en) {
		if (tx_blk->TxFrameType == TX_LEGACY_FRAME) {
			if ((!RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) &&
			    (!RTMP_GET_PACKET_HIGH_PRIO(tx_blk->pPacket))) {
				tx_blk->wmm_set = pAd->vow_sta_wmm[tx_blk->Wcid];
				tx_blk->QueIdx = pAd->vow_sta_ac[tx_blk->Wcid];
				txd->txd_1 &= ~TXD_WLAN_IDX_MASK;
				txd->txd_1 |= tx_blk->Wcid << TXD_WLAN_IDX_SHIFT;
			}
		}
	}
#endif

#ifdef RT_BIG_ENDIAN
	MTMacInfoEndianChange(pAd, buf, TYPE_TMACINFO, sizeof(*txd));
#endif
}

#ifdef RANDOM_PKT_GEN
VOID random_write_qidx(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_l->TxD0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (RandomTxCtrl != 0 && txd_0->q_idx < (cap->qos.WmmHwNum * 4))
		txd_0->q_idx = pTxBlk->lmac_qidx;
}
#endif

INT32 mtf_write_txp_info_by_host(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *tx_blk)
{
	MAC_TX_PKT_T *txp;
	TXD_PTR_LEN_T *txp_ptr_len;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	UINT16 token;
	struct wifi_dev *wdev = tx_blk->wdev;
	UINT8 band_idx = HcGetBandByWdev(wdev);
	struct token_tx_pkt_queue *que = token_tx_get_queue_by_band(cb, band_idx);

	if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
		(tx_blk->amsdu_state == TX_AMSDU_ID_FIRST)) {

		if (tx_blk->TotalFrameNum <= 4) {
			NdisZeroMemory(buf, sizeof(MAC_TX_PKT_T));
			tx_blk->txp_len = sizeof(MAC_TX_PKT_T);
		} else {
			/* need one more page to store payload address and length */
			NdisZeroMemory(buf, sizeof(MAC_TX_PKT_T) + TXDLEN_PAGE_SIZE);
			tx_blk->txp_len = sizeof(MAC_TX_PKT_T) + TXDLEN_PAGE_SIZE;
		}
	}

	if (tx_blk->frame_idx < 4) {
		txp = (MAC_TX_PKT_T *)buf;
		txp_ptr_len = &txp->arPtrLen[tx_blk->frame_idx / 2];
	} else {
		txp = (MAC_TX_PKT_T *)(buf + sizeof(MAC_TX_PKT_T));
		txp_ptr_len = &txp->arPtrLen[(tx_blk->frame_idx - 4) / 2];
	}

	RTMP_SET_BAND_IDX(tx_blk->pPacket, band_idx);

	if ((tx_blk->frame_idx & 0x1) == 0x0) {
		txp_ptr_len->u4Ptr0 = cpu2le32(PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE));

		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = token_tx_enq(pAd, que, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
							txp_ptr_len->u4Ptr0, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = token_tx_enq(pAd, que, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
							txp_ptr_len->u4Ptr0, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len0 = cpu2le16((tx_blk->SrcBufLen & TXD_LEN_MASK_V2) | TXD_LEN_ML_V2);
	} else {
		txp_ptr_len->u4Ptr1 = cpu2le32(PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE));

		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = token_tx_enq(pAd, que, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
							txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = token_tx_enq(pAd, que, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
							txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len1 = cpu2le16((tx_blk->SrcBufLen & TXD_LEN_MASK_V2) | TXD_LEN_ML_V2);
	}

	if (tx_blk->frame_idx < 4)
		txp->au2MsduId[tx_blk->frame_idx] = cpu2le16(token | TXD_MSDU_ID_VLD);
	else
		txp->au2MsduId[tx_blk->frame_idx - 4] = cpu2le16(token | TXD_MSDU_ID_VLD);

/* TODO: Fix for preflight pass */
	if (0)
		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			mtf_dump_txp_info(pAd, buf);

	return NDIS_STATUS_SUCCESS;
}

INT32 mtf_write_txp_info_by_wa(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	CR4_TXP_MSDU_INFO *cr4_txp_msdu_info = (CR4_TXP_MSDU_INFO *)buf;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	UINT16 token;
	struct wifi_dev *wdev = pTxBlk->wdev;
	UINT8 band_idx = HcGetBandByWdev(wdev);
	struct token_tx_pkt_queue *que = token_tx_get_queue_by_band(cb, band_idx);
	UCHAR BssInfoIdx = 0;
	UINT32 dma_addr = 0, index = 0, pl_cnt = 0, last_pl_len;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	/* check queue status */
	if (pAd->vow_dvt_en) {
		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME) {
			if ((!RTMP_GET_PACKET_MGMT_PKT(pTxBlk->pPacket)) &&
				(!RTMP_GET_PACKET_HIGH_PRIO(pTxBlk->pPacket))) {
				if (vow_is_queue_full(pAd, pTxBlk->Wcid, pTxBlk->QueIdx)) {
					pAd->vow_need_drop_cnt[pTxBlk->Wcid]++;
					return NDIS_STATUS_SUCCESS;
				}
			}
		}
	}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */

	NdisZeroMemory(cr4_txp_msdu_info, sizeof(CR4_TXP_MSDU_INFO));
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
	/* Inform CR4 to bypass ProxyARP check on this packet */
	if (RTMP_IS_PACKET_DIRECT_TX(pTxBlk->pPacket))
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_HSR2_TX;

#endif /* HOTSPOT_SUPPORT_R2 */

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_CT_WithTxD))
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_APPLY_TXD;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame))
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_NONE_CIPHER_FRAME;

#ifdef RT_CFG80211_SUPPORT
	if (RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket)) {
		if (!(pTxBlk->pMacEntry->bLastRTSFailed))
			cr4_txp_msdu_info->type_and_flags |= CT_INFO_RTS_ENABLE;
	}
#endif

	dma_addr = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	pl_cnt = pTxBlk->SrcBufLen / MAX_TXP_LEN;
	if ((pTxBlk->SrcBufLen % MAX_TXP_LEN) != 0)
		pl_cnt++;

	last_pl_len = (pTxBlk->SrcBufLen % MAX_TXP_LEN != 0)
			? pTxBlk->SrcBufLen % MAX_TXP_LEN : MAX_TXP_LEN;

	for (index = 0; index < pl_cnt; index++) {
		cr4_txp_msdu_info->buf_ptr[index] = cpu2le32(dma_addr + MAX_TXP_LEN*index);

		if (index == (pl_cnt - 1))
			cr4_txp_msdu_info->buf_len[index] = cpu2le16((last_pl_len & TXD_LEN_MASK_V2));
		else
			cr4_txp_msdu_info->buf_len[index] = cpu2le16(MAX_TXP_LEN & TXD_LEN_MASK_V2);
	}

	RTMP_SET_BAND_IDX(pTxBlk->pPacket, band_idx);

	if (RTMP_GET_PACKET_MGMT_PKT(pTxBlk->pPacket)) {
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_MGN_FRAME;
		token = token_tx_enq(pAd, que, pTxBlk->pPacket, TOKEN_TX_MGT, pTxBlk->Wcid,
					cr4_txp_msdu_info->buf_ptr[0], GET_OS_PKT_LEN(pTxBlk->pPacket));
	} else {
		token = token_tx_enq(pAd, que, pTxBlk->pPacket, TOKEN_TX_DATA, pTxBlk->Wcid,
					cr4_txp_msdu_info->buf_ptr[0], GET_OS_PKT_LEN(pTxBlk->pPacket));
	}

	MEM_DBG_PKT_RECORD(pTxBlk->pPacket, 1<<5);
	MEM_DBG_PKT_RECORD(pTxBlk->pPacket, token<<18);

#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	if (pTxBlk->pMacEntry && IS_ENTRY_PEER_AP(pTxBlk->pMacEntry)) {
#ifdef WARP_512_SUPPORT
		if (pAd->Warp512Support)
			WCID_SET_H_L(cr4_txp_msdu_info->reserved, cr4_txp_msdu_info->rept_wds_wcid, pTxBlk->pMacEntry->wcid  + WCID_SHIFT);
		else
#endif /* WARP_512_SUPPORT */
			WCID_SET_H_L(cr4_txp_msdu_info->reserved, cr4_txp_msdu_info->rept_wds_wcid, pTxBlk->pMacEntry->wcid);
	}
	else
#endif /* APCLI_SUPPORT || CONFIG_STA_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT

	if (pTxBlk->pMacEntry && IS_ENTRY_REPEATER(pTxBlk->pMacEntry))
		WCID_SET_H_L(cr4_txp_msdu_info->reserved, cr4_txp_msdu_info->rept_wds_wcid, pTxBlk->pMacEntry->wcid);
	else
		/*TODO: WDS case.*/
#endif
#if  (defined(MWDS)) || (defined(A4_CONN))
/*	if (pTxBlk->pMacEntry && (IS_ENTRY_MWDS(pTxBlk->pMacEntry) || IS_ENTRY_A4(pTxBlk->pMacEntry)))*/
	if (pTxBlk->pMacEntry && IS_ENTRY_A4(pTxBlk->pMacEntry)) {
#ifdef WARP_512_SUPPORT
		if (pAd->Warp512Support)
			WCID_SET_H_L(cr4_txp_msdu_info->reserved, cr4_txp_msdu_info->rept_wds_wcid, (pTxBlk->pMacEntry->wcid + WCID_SHIFT));
		else
#endif /* WARP_512_SUPPORT */
			WCID_SET_H_L(cr4_txp_msdu_info->reserved, cr4_txp_msdu_info->rept_wds_wcid, pTxBlk->pMacEntry->wcid);
	}
	else
#endif /* MWDS */
#ifdef MBSS_AS_WDS_AP_SUPPORT
#ifdef CLIENT_WDS
	if (pTxBlk->pMacEntry && IS_ENTRY_CLIWDS(pTxBlk->pMacEntry)) {
		WCID_SET_H_L(cr4_txp_msdu_info->reserved, cr4_txp_msdu_info->rept_wds_wcid, pTxBlk->pMacEntry->wcid);
	} else
#endif
#endif
#ifdef WDS_SUPPORT
	if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry)) {
		WCID_SET_H_L(cr4_txp_msdu_info->reserved, cr4_txp_msdu_info->rept_wds_wcid, pTxBlk->pMacEntry->wcid);
	} else
#endif	/* WDS_SUPPORT */
	{
		WCID_SET_H_L(cr4_txp_msdu_info->reserved, cr4_txp_msdu_info->rept_wds_wcid, WCID_NO_MATCHED(pAd));
	}

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (pAd->vow_dvt_en) {
		if (pAd->vow_sta_ack[pTxBlk->Wcid]) {
			cr4_txp_msdu_info->type_and_flags |= CT_INFO_PTK_NO_ACK;
			WCID_SET_H_L(cr4_txp_msdu_info->reserved, cr4_txp_msdu_info->rept_wds_wcid, pTxBlk->Wcid);
		}
	}
#endif

	BssInfoIdx = wdev->bss_info_argument.ucBssIndex;

	cr4_txp_msdu_info->type_and_flags = cpu2le16(cr4_txp_msdu_info->type_and_flags);
	cr4_txp_msdu_info->msdu_token = cpu2le16(token);
	cr4_txp_msdu_info->bss_index = BssInfoIdx;
	cr4_txp_msdu_info->buf_num = pl_cnt;	/* os get scatter. */

#ifdef DSCP_PRI_SUPPORT
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		/*handle DSCP-UP Mapping in 7915 CR4*/
		/*cr4_txp_msdu_info->reserved is used as wcid > 256 in 7915*/
	} else
#endif
	{
		if ((pAd->ApCfg.DscpPriMapSupport) && (pTxBlk->DscpMappedPri != -1)) {
			cr4_txp_msdu_info->reserved = pTxBlk->DscpMappedPri;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cr4->reserved:%d\n",
									cr4_txp_msdu_info->reserved);
		}
	}
#endif /*DSCP_PRI_SUPPORT*/
	pTxBlk->txp_len = sizeof(CR4_TXP_MSDU_INFO);

	if (cr4_txp_msdu_info->type_and_flags == 0) {
#ifdef WFDMA_WED_COMPATIBLE
		if (pAd->CommonCfg.whnat_en)
			pTxBlk->dbdc_band = band_idx;
		else
#endif
			pTxBlk->dbdc_band = pTxBlk->resource_idx;
		WLAN_HOOK_CALL(WLAN_HOOK_TX, pAd, pTxBlk);
	}

	cr4_txp_msdu_info->type_and_flags |= cpu2le16(CT_INFO_PKT_FR_HOST);

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if ((pTxBlk->Wcid != 0) && (pAd->vow_dvt_en)) {
		if ((!RTMP_GET_PACKET_MGMT_PKT(pTxBlk->pPacket)) &&
		    (!RTMP_GET_PACKET_HIGH_PRIO(pTxBlk->pPacket))) {
			pAd->vow_queue_map[token][0] = pTxBlk->Wcid;
			pAd->vow_queue_map[token][1] = pTxBlk->QueIdx;
			pAd->vow_queue_len[pTxBlk->Wcid][pTxBlk->QueIdx]++;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[31m: enqueue wcid %d, qidx %d, queue len %d....\x1b[m\n",
				pTxBlk->Wcid, pTxBlk->QueIdx,
				pAd->vow_queue_len[pTxBlk->Wcid][pTxBlk->QueIdx]);
		}
	}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */

	return NDIS_STATUS_SUCCESS;
}

VOID mtf_dump_wtbl_base_info(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->dump_wtbl_base_info)
		return chip_dbg->dump_wtbl_base_info(pAd);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not supported\n");
}

VOID mtf_dump_wtbl_info(RTMP_ADAPTER *pAd, UINT16 wtbl_idx)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->dump_wtbl_info)
		return chip_dbg->dump_wtbl_info(pAd, wtbl_idx);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not supported\n");
}


INT mtf_get_wtbl_entry234(RTMP_ADAPTER *pAd, UINT16 widx, struct wtbl_entry *ent)
{
	struct rtmp_mac_ctrl *wtbl_ctrl;
	UINT16 wtbl_idx;

	wtbl_ctrl = &pAd->mac_ctrl;

	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		wtbl_idx = (widx < wtbl_ctrl->wtbl_entry_cnt[0] ? widx : wtbl_ctrl->wtbl_entry_cnt[0] - 1);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "pAd->mac_ctrl not init yet!\n");
		return FALSE;
	}

	ent->wtbl_idx = wtbl_idx;
	ent->wtbl_addr = wtbl_ctrl->wtbl_base_addr[0] +
					 wtbl_idx * wtbl_ctrl->wtbl_entry_size[0];
	return TRUE;
}

/* This is workaround for retrieving RSSI from WTBL entry
 * since WTBL_BASE_ADDR is changed from 0x820e0000 to 820e8000 for AXE
 * and pcie remap address is map 0x820e0000 of MAC to 0x3xxxx of PCIE
 */
#define WTBL_BASE_ADDR_OFFSET_AXE 0x8000

INT mt_wtbl_init_ByFw(struct _RTMP_ADAPTER *pAd)
{
	pAd->mac_ctrl.wtbl_base_addr[0] = (UINT32)WTBL_BASE_ADDR + WTBL_BASE_ADDR_OFFSET_AXE;
	pAd->mac_ctrl.wtbl_entry_size[0] = (UINT16)WTBL_PER_ENTRY_SIZE;
	pAd->mac_ctrl.wtbl_entry_cnt[0] = WTBL_MAX_NUM(pAd);
	return TRUE;
}

INT mtf_init_wtbl(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	pAd->mac_ctrl.wtbl_base_addr[0] = (UINT32)WTBL_BASE_ADDR + WTBL_BASE_ADDR_OFFSET_AXE;
	pAd->mac_ctrl.wtbl_entry_size[0] = (UINT16)WTBL_PER_ENTRY_SIZE;
	pAd->mac_ctrl.wtbl_entry_cnt[0] = WTBL_MAX_NUM(pAd);
	return TRUE;
}

VOID mtf_show_mac_info(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "MAC[Ver:Rev/ID=0x%08x : 0x%08x]\n",
			  pAd->MACVersion, pAd->ChipID);
}


UINT32 mtf_get_packet_type(RTMP_ADAPTER *ad, VOID *rx_packet)
{
	union _RMAC_RXD_0_UNION *rxd_0;

	rxd_0 = (union _RMAC_RXD_0_UNION *)(rx_packet);

#ifdef RT_BIG_ENDIAN
	mt_rmac_d0_endian_change(&rxd_0->word);
#endif /* RT_BIG_ENDIAN */

	return RMAC_RX_PKT_TYPE(rxd_0->word) ;
}

UINT32 mtf_get_packet_rxbytes(RTMP_ADAPTER *ad, VOID *rx_packet)
{
	union _RMAC_RXD_0_UNION *rxd_0;

	rxd_0 = (union _RMAC_RXD_0_UNION *)(rx_packet);

#ifdef RT_BIG_ENDIAN
	mt_rmac_d0_endian_change(&rxd_0->word);
#endif /* RT_BIG_ENDIAN */

	return RMAC_RX_BYTE_CNT(rxd_0->word) ;
}

/* Pre sanity the Rx pkt type */
BOOLEAN check_rx_pkt_type(UINT32  rx_pkt_type)
{
	BOOLEAN flag = TRUE;
	switch (rx_pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_NORMAL:
	case RMAC_RX_PKT_TYPE_RX_DUP_RFB:
		break;

	case RMAC_RX_PKT_TYPE_RX_TXRXV:
		break;

	case RMAC_RX_PKT_TYPE_RX_TXS:
		break;

	case RMAC_RX_PKT_TYPE_RX_EVENT:
		break;

	case RMAC_RX_PKT_TYPE_RX_TMR:
		break;

#ifdef CUT_THROUGH
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
#ifdef WFDMA_WED_COMPATIBLE
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY_V0:
#endif /*WFDMA_WED_COMPATIBLE*/
		break;
#endif /* CUT_THROUGH */
	case RMAC_RX_PKT_TYPE_RX_ICS:
		break;
	default:
		flag = FALSE;
		break;
	}
	return flag;
}

#define RMAC_INFO_BASE_SIZE 24
#define RMAC_INFO_GRP_1_SIZE 16
#define RMAC_INFO_GRP_2_SIZE 8
#define RMAC_INFO_GRP_3_SIZE 8
#define RMAC_INFO_GRP_4_SIZE 16
#define RMAC_INFO_GRP_5_SIZE 72
/*
    1'b0: the related GROUP is not present
    1'b1: the related GROUP is present

    bit[0]: indicates GROUP1 (DW10~DW13)
    bit[1]: indicates GROUP2 (DW14~DW15)
    bit[2]: indicates GROUP3 (DW16~DW17)
    bit[3]: indicates GROUP4 (DW6~DW9)
    bit[4]: indicates GROUP5 (DW18~DW35)
*/

#ifdef SNIFFER_RADIOTAP_SUPPORT
UINT32 mtf_trans_rxd_into_radiotap(RTMP_ADAPTER *pAd, VOID *rx_packet, struct _RX_BLK *rx_blk)
{
	MONITOR_STRUCT *p_monitor_ctrl = &pAd->monitor_ctrl;
	PUINT8 rmac_info;
	UINT16 rxd_len = RMAC_INFO_BASE_SIZE;
	struct rxd_grp_0 *rxd_grp0;
	struct rxd_grp_1 *rxd_grp1 = NULL;
	struct rxd_grp_2 *rxd_grp2 = NULL;
	struct rxd_grp_3 *rxd_grp3 = NULL;
	struct rxd_grp_4 *rxd_grp4 = NULL;
	struct rxd_grp_5 *rxd_grp5 = NULL;
	struct IEEE80211_RADIOTAP_INFO radiotap_info;

	rmac_info = (UCHAR *)(GET_OS_PKT_DATAPTR(rx_packet));
	rxd_grp0 = (struct rxd_grp_0 *)rmac_info;

	SET_OS_PKT_LEN(rx_packet, (rxd_grp0->rxd_0 & RXD_RX_BYTE_COUNT_MASK));

	if (rxd_grp0->rxd_1 & RXD_GROUP4_VLD) {
		rxd_grp4 = (struct rxd_grp_4 *)(rmac_info + rxd_len);
		rxd_len += RMAC_INFO_GRP_4_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP1_VLD) {
		rxd_grp1 = (struct rxd_grp_1 *)(rmac_info + rxd_len);
		rxd_len += RMAC_INFO_GRP_1_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP2_VLD) {
		rxd_grp2 = (struct rxd_grp_2 *)(rmac_info + rxd_len);
		rxd_len += RMAC_INFO_GRP_2_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP3_VLD) {
		rxd_grp3 = (struct rxd_grp_3 *)(rmac_info + rxd_len);
		rxd_len += RMAC_INFO_GRP_3_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP5_VLD) {
		rxd_grp5 = (struct rxd_grp_5 *)(rmac_info + rxd_len);
		rxd_len += RMAC_INFO_GRP_5_SIZE;
	}

	rx_blk->band = ((rxd_grp0->rxd_1 & RXD_BN) >> 28);

	if (rxd_grp2 == NULL || rxd_grp3 == NULL || rxd_grp5 == NULL)
		return NDIS_STATUS_FAILURE;


	if (HAL_MAC_CONNAC2X_RX_STATUS_GET_RXV_SEQ_NO(rxd_grp0) != 0)
		p_monitor_ctrl->u4AmpduRefNum += 1;

	radiotap_info.u2VendorLen = rxd_len + (((rxd_grp0->rxd_2 & RXD_HO_MASK) >> RXD_HO_SHIFT) * 2);
	radiotap_info.ucSubNamespace = 2; /* MTK wireshark CONNAC2 id */
	radiotap_info.u4AmpduRefNum = p_monitor_ctrl->u4AmpduRefNum;
	radiotap_info.u4Timestamp = rxd_grp2->rxd_14;
	radiotap_info.ucFcsErr = HAL_MAC_CONNAC2X_RX_STATUS_IS_FCS_ERROR(rxd_grp0);
	radiotap_info.ucFrag = HAL_MAC_CONNAC2X_RX_STATUS_IS_FRAG(rxd_grp0);
	radiotap_info.u2ChFrequency = HAL_MAC_CONNAC2X_RX_STATUS_GET_CHNL_NUM(rxd_grp0);
	radiotap_info.ucRxMode = HAL_MAC_CONNAC2X_RX_VT_GET_RX_MODE(rxd_grp5);
	radiotap_info.ucFrMode = HAL_MAC_CONNAC2X_RX_VT_GET_FR_MODE(rxd_grp5);
	radiotap_info.ucShortGI = HAL_MAC_CONNAC2X_RX_VT_GET_SHORT_GI(rxd_grp5);
	radiotap_info.ucSTBC = HAL_MAC_CONNAC2X_RX_VT_GET_STBC(rxd_grp5);
	radiotap_info.ucNess = HAL_MAC_CONNAC2X_RX_VT_GET_NESS(rxd_grp5);
	radiotap_info.ucLDPC = HAL_MAC_CONNAC2X_RX_VT_GET_LDPC(rxd_grp3);
	radiotap_info.ucMcs = HAL_MAC_CONNAC2X_RX_VT_GET_RX_RATE(rxd_grp3);
	radiotap_info.ucRcpi0 = HAL_MAC_CONNAC2X_RX_VT_GET_RCPI0(rxd_grp5);
	radiotap_info.ucTxopPsNotAllow = HAL_MAC_CONNAC2X_RX_VT_TXOP_PS_NOT_ALLOWED(rxd_grp5);
	radiotap_info.ucLdpcExtraOfdmSym = HAL_MAC_CONNAC2X_RX_VT_LDPC_EXTRA_OFDM_SYM(rxd_grp5);
	radiotap_info.ucVhtGroupId = HAL_MAC_CONNAC2X_RX_VT_GET_GROUP_ID(rxd_grp5);
	radiotap_info.ucNsts = HAL_MAC_CONNAC2X_RX_VT_GET_NSTS(rxd_grp3) + 1;
	radiotap_info.ucBeamFormed = HAL_MAC_CONNAC2X_RX_VT_GET_BEAMFORMED(rxd_grp3);
	/* HE only */
	if (radiotap_info.ucRxMode & MODE_HE_SU) {
		radiotap_info.ucPeDisamb = HAL_MAC_CONNAC2X_RX_VT_GET_PE_DIS_AMB(rxd_grp5);
		radiotap_info.ucNumUser = HAL_MAC_CONNAC2X_RX_VT_GET_NUM_USER(rxd_grp5);
		radiotap_info.ucSigBRU0 = HAL_MAC_CONNAC2X_RX_VT_GET_SIGB_RU0(rxd_grp5);
		radiotap_info.ucSigBRU1 = HAL_MAC_CONNAC2X_RX_VT_GET_SIGB_RU1(rxd_grp5);
		radiotap_info.ucSigBRU2 = HAL_MAC_CONNAC2X_RX_VT_GET_SIGB_RU2(rxd_grp5);
		radiotap_info.ucSigBRU3 = HAL_MAC_CONNAC2X_RX_VT_GET_SIGB_RU3(rxd_grp5);
		radiotap_info.u2VhtPartialAid = HAL_MAC_CONNAC2X_RX_VT_GET_PART_AID(rxd_grp5);
		radiotap_info.u2RuAllocation = ((HAL_MAC_CONNAC2X_RX_VT_GET_RU_ALLOC1(rxd_grp3) >> 1) |
				(HAL_MAC_CONNAC2X_RX_VT_GET_RU_ALLOC2(rxd_grp3) << 3));
		radiotap_info.u2BssClr = HAL_MAC_CONNAC2X_RX_VT_GET_BSS_COLOR(rxd_grp5);
		radiotap_info.u2BeamChange = HAL_MAC_CONNAC2X_RX_VT_GET_BEAM_CHANGE(rxd_grp5);
		radiotap_info.u2UlDl = HAL_MAC_CONNAC2X_RX_VT_GET_UL_DL(rxd_grp5);
		radiotap_info.u2DataDcm = HAL_MAC_CONNAC2X_RX_VT_GET_DCM(rxd_grp5);
		radiotap_info.u2SpatialReuse1 = HAL_MAC_CONNAC2X_RX_VT_GET_SPATIAL_REUSE1(rxd_grp5);
		radiotap_info.u2SpatialReuse2 = HAL_MAC_CONNAC2X_RX_VT_GET_SPATIAL_REUSE2(rxd_grp5);
		radiotap_info.u2SpatialReuse3 = HAL_MAC_CONNAC2X_RX_VT_GET_SPATIAL_REUSE3(rxd_grp5);
		radiotap_info.u2SpatialReuse4 = HAL_MAC_CONNAC2X_RX_VT_GET_SPATIAL_REUSE4(rxd_grp5);
		radiotap_info.u2Ltf = HAL_MAC_CONNAC2X_RX_VT_GET_LTF(rxd_grp5) + 1;
		radiotap_info.u2Doppler = HAL_MAC_CONNAC2X_RX_VT_GET_DOPPLER(rxd_grp5);
		radiotap_info.u2Txop = HAL_MAC_CONNAC2X_RX_VT_GET_TXOP(rxd_grp5);
	}

	radiotap_fill_field(rx_packet, &radiotap_info);



	return NDIS_STATUS_SUCCESS;
}
#endif

void mtf_get_snr(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR *pData)
{
	CHAR avgSnr = 0;
	PMAC_TABLE_ENTRY pEntry;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	pEntry = &pAd->MacTab.Content[wcid];

	if (!(IS_VALID_ENTRY(pEntry)) || (pEntry->Sst != SST_ASSOC))
		return;


	/* real SNR is read value -16.
		The real SNR may < 0, it's right, but customer may feel strange.
		so if real SNR < 0, treate it as 0. */

	/* moving average */
	if (pEntry->RssiSample.AvgSnr[0])
		pEntry->RssiSample.AvgSnr[0] = (((pEntry->RssiSample.AvgSnr[0] * 7) + avgSnr) >> 3);
	else
		pEntry->RssiSample.AvgSnr[0] = avgSnr;
}

INT32 mtf_trans_rxd_into_rxblk(RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, PNDIS_PACKET rx_pkt)
{

	UCHAR *rmac_info, *pos, *fc = NULL;
	struct rxd_grp_0 *rxd_grp0;
	struct rxd_grp_1 *rxd_grp1 = NULL;
	struct rxd_grp_2 *rxd_grp2 = NULL;
	struct rxd_grp_3 *rxd_grp3 = NULL;
	struct rxd_grp_4 *rxd_grp4 = NULL;
	struct rxd_grp_5 *rxd_grp5 = NULL;
	UINT8 i;
	INT32 rmac_info_len = RMAC_INFO_BASE_SIZE;
	UINT8 ho_len;
	UINT16 temp_fc = 0xFFFF, fn_sn = 0;
	rmac_info = (UCHAR *)(GET_OS_PKT_DATAPTR(rx_pkt));

#ifdef VERIFICATION_MODE
	if (pAd->veri_ctrl.dump_rx_debug)
		mtf_dump_rmac_info_normal(pAd, rmac_info);
#endif

	rx_blk->pRxInfo = (RXINFO_STRUC *)(&rx_blk->hw_rx_info[RXINFO_OFFSET]);
	pos = rmac_info;
	rx_blk->rmac_info = rmac_info;
	rxd_grp0 = (struct rxd_grp_0 *)rmac_info;
	pos += RMAC_INFO_BASE_SIZE;
#ifdef RT_BIG_ENDIAN
	MTMacInfoEndianChange(pAd, rmac_info, TYPE_RMACINFO, RMAC_INFO_BASE_SIZE);
#endif

	if (rxd_grp0->rxd_1 & RXD_GROUP4_VLD) {
		rxd_grp4  = (struct rxd_grp_4 *)pos;
		rmac_info_len += RMAC_INFO_GRP_4_SIZE;
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((UCHAR *)rxd_grp4, 0);
#endif
		fc = pos;
		pos += RMAC_INFO_GRP_4_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP1_VLD) {
		rxd_grp1 = (struct rxd_grp_1 *)pos;
		rmac_info_len += RMAC_INFO_GRP_1_SIZE;
		pos += RMAC_INFO_GRP_1_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP2_VLD) {
		rxd_grp2 = (struct rxd_grp_2 *)pos;
		rmac_info_len += RMAC_INFO_GRP_2_SIZE;
		pos += RMAC_INFO_GRP_2_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP3_VLD) {
		rxd_grp3 = (struct rxd_grp_3 *)pos;
		rmac_info_len += RMAC_INFO_GRP_3_SIZE;
		pos += RMAC_INFO_GRP_3_SIZE;
	}

	if (rxd_grp0->rxd_1 & RXD_GROUP5_VLD) {
		rxd_grp5 = (struct rxd_grp_5 *)pos;
		rmac_info_len += RMAC_INFO_GRP_5_SIZE;
		pos += RMAC_INFO_GRP_5_SIZE;
	}

#ifdef RT_BIG_ENDIAN

		if (rxd_grp4) {
			RTMPEndianChange((UCHAR *)rxd_grp4, RMAC_INFO_GRP_4_SIZE);
		}
		if (rxd_grp1)
			RTMPEndianChange((UCHAR *)rxd_grp1, RMAC_INFO_GRP_1_SIZE);
		if (rxd_grp2)
			RTMPEndianChange((UCHAR *)rxd_grp2, RMAC_INFO_GRP_2_SIZE);
		if (rxd_grp3)
			RTMPEndianChange((UCHAR *)rxd_grp3, RMAC_INFO_GRP_3_SIZE);
		if (rxd_grp5)
			RTMPEndianChange((UCHAR *)rxd_grp5, RMAC_INFO_GRP_5_SIZE);

#endif /* RT_BIG_ENDIAN */

	rx_blk->MPDUtotalByteCnt = (rxd_grp0->rxd_0 & RXD_RX_BYTE_COUNT_MASK) - rmac_info_len;

	ho_len = ((rxd_grp0->rxd_2 & RXD_HO_MASK) >> RXD_HO_SHIFT);

	if (ho_len) {
		rx_blk->MPDUtotalByteCnt -= (ho_len << 1);
		rmac_info_len += (ho_len << 1);
	}

	rx_blk->DataSize = rx_blk->MPDUtotalByteCnt;
	rx_blk->band = ((rxd_grp0->rxd_1 & RXD_BN) >> 28);
	rx_blk->wcid = (rxd_grp0->rxd_1 & RXD_WLAN_IDX_MASK);
	rx_blk->bss_idx = (rxd_grp0->rxd_2 & RXD_BSSID_MASK);
	rx_blk->sec_mode = ((rxd_grp0->rxd_1 & RXD_SEC_MODE_MASK) >> RXD_SEC_MODE_SHIFT);
	rx_blk->key_idx = ((rxd_grp0->rxd_1 & RXD_KID_MASK) >> RXD_KID_SHIFT);
#ifdef VLAN_SUPPORT
	rx_blk->is_htf = (rxd_grp0->rxd_2 & RXD_HTF) ? 1 : 0;
#endif

#if (defined(WIFI_DIAG) && (defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981))) || defined(TR181_SUPPORT)
	mtf_get_snr(pAd, rx_blk->wcid, (UCHAR *)rxd_grp3);
#endif
	if (rxd_grp0->rxd_3 & RXD_HTC)
		RX_BLK_SET_FLAG(rx_blk, fRX_HTC);

	if (rxd_grp0->rxd_1 & RXD_CM)
		RX_BLK_SET_FLAG(rx_blk, fRX_CM);

	if (rxd_grp0->rxd_1 & RXD_CLM)
		RX_BLK_SET_FLAG(rx_blk, fRX_CLM);

	if (rxd_grp0->rxd_1 & RXD_ICV_ERR)
		RX_BLK_SET_FLAG(rx_blk, fRX_ICV_ERR);

	if (rxd_grp0->rxd_1 & RXD_TKIPMIC_ERR)
		RX_BLK_SET_FLAG(rx_blk, fRX_TKIP_MIC_ERR);

	rx_blk->channel_freq = ((rxd_grp0->rxd_3 & RXD_CF_MASK) >> RXD_CF_SHIFT);
#ifdef CONFIG_6G_SUPPORT
	if (rx_blk->channel_freq >= CHANNEL_6G_BASE) {
		/* re-mapping 6G channen number to 1 ~ 233 */
		rx_blk->channel_freq = (((rx_blk->channel_freq - CHANNEL_6G_BASE) << 2) + 1);
	}
#endif

	rx_blk->TID = ((rxd_grp0->rxd_2 & RXD_TID_MASK) >> RXD_TID_SHIFT);

	if (rxd_grp0->rxd_2 & RXD_H)
		RX_BLK_SET_FLAG(rx_blk, fRX_HDR_TRANS);

	switch ((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) {
	case 0x1:
		rx_blk->pRxInfo->U2M = 1;
		break;
	case 0x2:
		rx_blk->pRxInfo->Mcast = 1;
		break;
	case 0x3:
		rx_blk->pRxInfo->Bcast = 1;
		break;
	}

	rx_blk->AmsduState = (rxd_grp0->rxd_4 & RXD_PF_MASK);

	if (rxd_grp0->rxd_2 & RXD_DAF)
		rx_blk->DeAmsduFail = 1;

	if (rxd_grp0->rxd_2 & RXD_FRAG)
		rx_blk->pRxInfo->FRAG = 1;

	if (rxd_grp0->rxd_2 & RXD_NULL)
		rx_blk->pRxInfo->NULLDATA = 1;

	if (!(rxd_grp0->rxd_2 & RXD_NDATA))
		rx_blk->pRxInfo->DATA = 1;

	if (rxd_grp0->rxd_3 & RXD_HTC)
		rx_blk->pRxInfo->HTC = 1;

	if (!(rxd_grp0->rxd_2 & RXD_NAMP))
		rx_blk->pRxInfo->AMPDU = 1;

	rx_blk->pRxInfo->L2PAD = 0;
	rx_blk->pRxInfo->AMSDU = 0;

	/* 0: decryption okay, 1:ICV error, 2:MIC error, 3:KEY not valid */
	rx_blk->pRxInfo->CipherErr = (((rxd_grp0->rxd_1 & RXD_ICV_ERR) ? 1 : 0) |
					(((rxd_grp0->rxd_1 & RXD_TKIPMIC_ERR) ? 1 : 0) << 1));

	if (rxd_grp0->rxd_1 & RXD_FCS_ERR)
		rx_blk->pRxInfo->Crc = 1;

	rx_blk->pRxInfo->MyBss = ((rx_blk->bss_idx == 0xf) ? 0 : 1);
	rx_blk->pRxInfo->Decrypted = 0;

	SET_OS_PKT_DATAPTR(rx_pkt, GET_OS_PKT_DATAPTR(rx_pkt) + rmac_info_len);
	SET_OS_PKT_LEN(rx_pkt, rx_blk->MPDUtotalByteCnt);

	rx_blk->pRxPacket = rx_pkt;
	rx_blk->pData = (UCHAR *)GET_OS_PKT_DATAPTR(rx_pkt);

	if (0
#ifdef QOS_R1
		|| IS_QOSR1_ENABLE(pAd)
#endif
#ifdef MAP_R2
		|| IS_MAP_R2_ENABLE(pAd)
#endif
#ifdef MAP_R3
		|| IS_MAP_R3_ENABLE(pAd)
#endif
	) {
		if (rx_blk->TID && (rx_blk->TID < 8))
			RTMP_SET_PACKET_UP(rx_blk->pRxPacket, rx_blk->TID);
	}

#ifdef VERIFICATION_MODE
	if (pAd->veri_ctrl.dump_rx_debug) {
		if (rxd_grp2)
			hex_dump("rxd_group2", (UCHAR *)rxd_grp2, RMAC_INFO_GRP_2_SIZE);

		hex_dump("rx_data", rx_blk->pData, rx_blk->MPDUtotalByteCnt);
	}
#endif

	rx_blk->AMSDU_ADDR = NULL;
	if (RX_BLK_TEST_FLAG(rx_blk, fRX_HDR_TRANS)) {
		struct wifi_dev *wdev = NULL;

		if (!fc)
			return 0;

		rx_blk->FC = fc;

		temp_fc = *((UINT16 *)fc);
		fn_sn = *((UINT16 *)(fc + 8));
#ifdef RT_BIG_ENDIAN
		temp_fc = le2cpu16(temp_fc);
		fn_sn = le2cpu16(fn_sn);
#endif
		rx_blk->FN = fn_sn & 0x000f;
		rx_blk->SN = (fn_sn & 0xfff0) >> 4;
		rx_blk->UserPriority = ((rxd_grp4->rxd_8 & RXD_TID_MASK) >> RXD_QOS_CTL_SHIFT);
		wdev = wdev_search_by_wcid(pAd, rx_blk->wcid);

		if (!wdev)
			wdev = wdev_search_by_band_omac_idx(pAd, rx_blk->band, rx_blk->bss_idx);

		if (!wdev)
			return 0;

	if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 0) && (((FRAME_CONTROL *)&temp_fc)->FrDs == 0)) {
			rx_blk->Addr1 = rx_blk->pData;
			rx_blk->Addr2 = rx_blk->pData + 6;
			rx_blk->Addr3 = wdev->bssid;
		} else if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 0) && (((FRAME_CONTROL *)&temp_fc)->FrDs == 1)) {
			rx_blk->AMSDU_ADDR = wdev->if_addr;
			rx_blk->Addr1 = rx_blk->pData;
			rx_blk->Addr2 = wdev->bssid;
			rx_blk->Addr3 = rx_blk->pData + 6;
		} else if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 1) && (((FRAME_CONTROL *)&temp_fc)->FrDs == 0)) {
			rx_blk->AMSDU_ADDR = fc + 2;
			rx_blk->Addr1 = wdev->bssid;
			rx_blk->Addr2 = rx_blk->pData + 6;
			rx_blk->Addr3 = rx_blk->pData;
		} else {
			rx_blk->Addr1 = wdev->if_addr;
			rx_blk->Addr2 = fc + 2;
			rx_blk->Addr3 = rx_blk->pData;
			rx_blk->Addr4 = rx_blk->pData + 6;
		}
	} else {
		rx_blk->FC = rx_blk->pData;

		if (rx_blk->FC) {
			temp_fc = *((UINT16 *)(rx_blk->FC));
			fn_sn = *((UINT16 *)(rx_blk->FC + 22));
		}
#ifdef RT_BIG_ENDIAN
		temp_fc = le2cpu16(temp_fc);
		fn_sn = le2cpu16(fn_sn);
#endif
		rx_blk->Duration = *((UINT16 *)(rx_blk->pData + 2));
#ifdef RT_BIG_ENDIAN
		rx_blk->Duration = le2cpu16(rx_blk->Duration);
#endif

		if ((((FRAME_CONTROL *)&temp_fc)->Type == FC_TYPE_MGMT) ||
			(((FRAME_CONTROL *)&temp_fc)->Type == FC_TYPE_DATA)) {
			rx_blk->FN = fn_sn & 0x000f;
			rx_blk->SN = (fn_sn & 0xfff0) >> 4;
		}

		rx_blk->Addr1 = rx_blk->pData + 4;
		rx_blk->Addr2 = rx_blk->pData + 10;
		rx_blk->Addr3 = rx_blk->pData + 16;


		if ((((FRAME_CONTROL *)&temp_fc)->ToDs == 1) && (((FRAME_CONTROL *)&temp_fc)->FrDs == 1))
			rx_blk->Addr4 = rx_blk->pData + 24;
	}

	if (((FRAME_CONTROL *)&temp_fc)->SubType == SUBTYPE_AUTH) {
		if ((rxd_grp0->rxd_1 & RXD_SEC_MODE_MASK) &&
			(!(rxd_grp0->rxd_1 & RXD_CM)) &&
			(!(rxd_grp0->rxd_1 & RXD_CLM))) {
			rx_blk->pRxInfo->Decrypted = 1;
		}
	}

	if (rxd_grp1) {
		UINT64 pn1 = ((rxd_grp1->rxd_11 & RXD_PN_32_47_MASK) >> RXD_PN_32_47_SHIFT);
		UINT64 pn_total = 0;

		if (pn1 != 0)
			pn_total = (rxd_grp1->rxd_10 & RXD_PN_0_31_MASK) + (pn1 << 32);
		else
			pn_total = (rxd_grp1->rxd_10 & RXD_PN_0_31_MASK);

		rx_blk->CCMP_PN = pn_total;
	}

	if (IS_MT7915_FW_VER_E1(pAd) && (!IS_MT7986(pAd)) && (!IS_MT7916(pAd)) && (!IS_MT7981(pAd))) {
		/* rcpi */
		if (rxd_grp0->rxd_1 & RXD_GROUP5_VLD) {
			rx_blk->rcpi[0] = (rxd_grp5->rxd_24 & BITS(0, 7)) >> 0;
			rx_blk->rcpi[1] = (rxd_grp5->rxd_24 & BITS(8, 15)) >> 8;
			rx_blk->rcpi[2] = (rxd_grp5->rxd_24 & BITS(16, 23)) >> 16;
			rx_blk->rcpi[3] = (rxd_grp5->rxd_24 & BITS(24, 31)) >> 24;
		}
	} else if (IS_MT7915_FW_VER_E2(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		/* rcpi */
		if (rxd_grp0->rxd_1 & RXD_GROUP3_VLD) {
			rx_blk->rcpi[0] = (rxd_grp3->rxd_17 & BITS(0, 7)) >> 0;
			rx_blk->rcpi[1] = (rxd_grp3->rxd_17 & BITS(8, 15)) >> 8;
			rx_blk->rcpi[2] = (rxd_grp3->rxd_17 & BITS(16, 23)) >> 16;
			rx_blk->rcpi[3] = (rxd_grp3->rxd_17 & BITS(24, 31)) >> 24;
		}
	} else {
		rx_blk->rcpi[0] = 0;
		rx_blk->rcpi[1] = 0;
		rx_blk->rcpi[2] = 0;
		rx_blk->rcpi[3] = 0;
	}

	for (i = 0; i < MAX_ANTENNA_NUM; i++)
		rx_blk->rx_signal.raw_rssi[i] = RCPI_TO_RSSI(rx_blk->rcpi[i]);


	/* rx rate */
	if (rxd_grp0->rxd_1 & RXD_GROUP3_VLD) {
		pAd->phy_stat_elem.rx_raw = rxd_grp3->rxd_16;
#ifdef RATE_PRIOR_SUPPORT
		rx_blk->rx_rate.field.MCS = ((rxd_grp3->rxd_16 & BITS(0, 6)) >> 0);
#endif
	}


	/* rx mode */
	if (rxd_grp0->rxd_1 & RXD_GROUP5_VLD)
		pAd->phy_stat_elem.rx_raw2 = rxd_grp5->rxd_18;

	if (((rxd_grp0->rxd_1 & (RXD_GROUP3_VLD | RXD_GROUP5_VLD)) == (RXD_GROUP3_VLD | RXD_GROUP5_VLD))
		&& pAd->tr_ctl.en_rx_profiling) {
		UINT8 rx_rate = 0, rx_nsts = 0, rx_mode = MODE_HE_SU, rx_gi = 0, dbw = 0;
		struct _rx_profiling *rx_rate_rc = NULL;
		struct _rx_mod_cnt *mpdu_cnt = NULL;
		struct _rx_mod_cnt *retry_cnt = NULL;
		BOOLEAN is_retry = FALSE;

		rx_rate = (rxd_grp3->rxd_16 & BITS(0, 6)) >> 0;
		rx_nsts = (rxd_grp3->rxd_16 & BITS(7, 9)) >> 7;

		dbw = (rxd_grp5->rxd_18 & BITS(8, 10)) >> 8;
		rx_mode = (rxd_grp5->rxd_18 & BITS(4, 7)) >> 4;
		rx_gi = (rxd_grp5->rxd_18 & BITS(13, 14)) >> 13;

		if ((rx_mode < MODE_HE_SU) && (rx_gi > 1)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "[mode(0x%x)]Invalid gi(%d)!\n", rx_mode, rx_gi);
			goto err_out;
		} else if ((rx_mode <= MODE_HE_MU) && (rx_gi > 2)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "[mode(0x%x)]Invalid gi(%d)!\n", rx_mode, rx_gi);
			goto err_out;
		} else {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "[mode(0x%x)]vlid band(%d)!\n", rx_mode, rx_gi);
		}

		if (rx_blk->band > DBDC_BAND1) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid band(%d)!\n", rx_blk->band);
			goto err_out;
		}

		rx_rate_rc = &pAd->tr_ctl.tr_cnt.rx_rate_rc[rx_blk->band];
		if (dbw >= ARRAY_SIZE(rx_rate_rc->mpdu_cnt)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Invalid BW(%d)!\n", dbw));
			goto err_out;
		}

		rx_rate_rc->total_mpdu_cnt++;
		mpdu_cnt = &rx_rate_rc->mpdu_cnt[dbw];
		retry_cnt = &rx_rate_rc->retry_cnt[dbw];

		if (rx_blk->FC) {
			temp_fc = *((UINT16 *)(rx_blk->FC));
#ifdef RT_BIG_ENDIAN
			temp_fc = le2cpu16(temp_fc);
#endif
			if (((FRAME_CONTROL *)&temp_fc)->Retry) {
				rx_rate_rc->total_retry_cnt++;
				is_retry = TRUE;
			}
		}

		switch (rx_mode) {
		case MODE_CCK:
			rx_rate = rx_rate & BITS(0, 2);

			if (rx_rate < ARRAY_SIZE(mpdu_cnt->cck)) {
				if (rx_rate & BIT2) {
					mpdu_cnt->cck[4+((rx_rate & BITS(0, 1))-1)]++;

					if (is_retry)
						retry_cnt->cck[4+((rx_rate & BITS(0, 1))-1)]++;
				} else {
					mpdu_cnt->cck[(rx_rate & BITS(0, 1))]++;

					if (is_retry)
						retry_cnt->cck[4+((rx_rate & BITS(0, 1))-1)]++;
				}
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "[CCK]Invalid rate idx(%d)!\n", rx_rate);
			break;

		case MODE_OFDM:
			switch (rx_rate & BITS(0, 3)) {
			case 0xb:
				rx_rate = 0;
				break;
			case 0xf:
				rx_rate = 1;
				break;
			case 0xa:
				rx_rate = 2;
				break;
			case 0xe:
				rx_rate = 3;
				break;
			case 0x9:
				rx_rate = 4;
				break;
			case 0xd:
				rx_rate = 5;
				break;
			case 0x8:
				rx_rate = 6;
				break;
			case 0xc:
				rx_rate = 7;
				break;
			default:
				rx_rate = 0;
			}

			if (rx_rate < ARRAY_SIZE(mpdu_cnt->ofdm)) {
				mpdu_cnt->ofdm[rx_rate]++;

				if (is_retry)
					retry_cnt->ofdm[rx_rate]++;
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "[OFDM]Invalid rate idx(%d)!\n", rx_rate);
			break;

		case MODE_HTMIX:
		case MODE_HTGREENFIELD:
			rx_rate = (rx_rate & BITS(0, 5));

			if (rx_rate < ARRAY_SIZE(mpdu_cnt->ht)) {
				mpdu_cnt->ht[rx_gi][rx_rate]++;

				if (is_retry)
					retry_cnt->ht[rx_gi][rx_rate]++;
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "[HT]Invalid rate idx(%d)!\n", rx_rate);
			break;
		case MODE_VHT:
			rx_rate = (rx_rate & BITS(0, 3));

			if ((rx_rate < ARRAY_SIZE(mpdu_cnt->vht[rx_gi][0])) && rx_nsts < 4) {
				mpdu_cnt->vht[rx_gi][rx_nsts][rx_rate]++;

				if (is_retry)
					retry_cnt->vht[rx_gi][rx_nsts][rx_rate]++;
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "[VHT]Invalid rate idx(%d)!\n", rx_rate);
			break;
		case MODE_HE_SU:
		case MODE_HE_EXT_SU:
		case MODE_HE_TRIG:
		case MODE_HE_MU:
			rx_rate = (rx_rate & BITS(0, 3));

			if ((rx_rate < ARRAY_SIZE(mpdu_cnt->he[rx_gi][0])) && rx_nsts < 4) {
				mpdu_cnt->he[rx_gi][rx_nsts][rx_rate]++;

				if (is_retry)
					retry_cnt->he[rx_gi][rx_nsts][rx_rate]++;
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "[HE]Invalid rate idx(%d)!\n", rx_rate);
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Unknown Mode(%x)!\n", rx_mode);
			break;
		}
	}

err_out:
	return rmac_info_len;
}

static VOID tx_free_v1_notify_handler(RTMP_ADAPTER *pAd, PKT_TOKEN_CB *cb,
										UINT8 *data, UINT32 token_cnt)
{
	UINT loop;
	PNDIS_PACKET pkt;
	UINT32 *token_ptr;
	UINT16 token_id;
	UINT8 type;
	struct qm_ops *qm_ops = pAd->qm_ops;

	struct token_tx_pkt_queue *que = NULL;

	for (loop = 0; loop < token_cnt; loop++) {
		token_ptr = (UINT32 *)data;
		token_id = ((*token_ptr) & TXDONE_MSDU_ID_MASK);
		data += 4;

		que = token_tx_get_queue_by_token_id(cb, token_id);
		que->total_back_cnt++;

		if (token_id < que->pkt_tkid_start || token_id > que->pkt_tkid_end)
			continue;
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
		/* already handled , do nothing */
		if (que->pkt_token[token_id].Reprocessed) {
			que->pkt_token[token_id].Reprocessed = FALSE;
			continue;
		}
#endif /* CONFIG_HOTSPOT_R2 */

		pkt = token_tx_deq(pAd, que, token_id, &type);

		if (pkt != NULL) {
#ifdef CONFIG_ATE
			if (ATE_ON(pAd)) {
#ifdef CONFIG_WLAN_SERVICE
				struct service_test *serv_test;
				struct test_configuration *configs;
				UCHAR band_idx, wdev_idx;
				struct wifi_dev *wdev = NULL;

				serv_test = (struct service_test *)(pAd->serv.serv_handle);
				wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
				if (wdev_idx >= WDEV_NUM_MAX)
					continue;

				wdev = pAd->wdev_list[wdev_idx];
				band_idx = HcGetBandByWdev(wdev);
				configs = &serv_test->test_config[band_idx];

				net_ad_post_tx(serv_test->test_winfo, configs, band_idx, pkt);
#else
				MT_ATETxControl(pAd, 0xFF, pkt);
#endif
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			} else
#endif
			{

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
				if (pAd->vow_dvt_en &&
				    (!RTMP_GET_PACKET_MGMT_PKT(pkt)) &&
				    (!RTMP_GET_PACKET_HIGH_PRIO(pkt))) {
					UINT16 wcid = pAd->vow_queue_map[token_id][0];
					UCHAR qidx = pAd->vow_queue_map[token_id][1];

					if ((wcid > 0 && wcid < 128) && (qidx >= 0 && qidx < 4)) {
						if (pAd->vow_queue_len[wcid][qidx] > 0)
							pAd->vow_queue_len[wcid][qidx]--;
						pAd->vow_queue_map[token_id][0] = 0xff;
						pAd->vow_queue_map[token_id][1] = 0xf;
						pAd->vow_tx_free[wcid]++;
					}
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"\x1b[31m%s: tokenid %d, wcid %d, qidx %d\x1b[m\n",
						 __func__, token_id, wcid, qidx);
				}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */
				if (type == TOKEN_TX_DATA)
					RELEASE_NDIS_PACKET_IRQ(pAd, pkt, NDIS_STATUS_SUCCESS);
				else
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
					 "Get a token_id[%d] = 0x%x but PktPtr is NULL!\n",
					  loop, token_id);
		}
	}

	if (token_tx_get_state(que) &&
			 (token_tx_get_free_cnt(que)
			 >= token_tx_get_hwmark(que))) {
		token_tx_set_state(que, TX_TOKEN_HIGH);
		qm_ops->schedule_tx_que(pAd, que->band_idx);
	}
}

static VOID tx_free_v2_notify_handler(RTMP_ADAPTER *pAd, PKT_TOKEN_CB *cb,
										UINT8 *data, UINT32 token_cnt)
{
	UINT loop = 0;
	PNDIS_PACKET pkt;
	UINT32 *token_ptr;
	UINT16 token_id;
	UINT8 type;
	UINT16 wlan_id;
	UINT8 q_id;
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct token_tx_pkt_queue *que = NULL;
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	while (loop < token_cnt) {
		token_ptr = (UINT32 *)data;
#ifdef RT_BIG_ENDIAN
		*token_ptr=le2cpu32(*token_ptr);
#endif
		if ((*token_ptr) & TXDONE_P) {
			wlan_id = (((*token_ptr) & TXDONE_E2_WLAN_ID_MASK) >> TXDONE_E2_WLAN_ID_SHIFT);
			q_id = (((*token_ptr) & TXDONE_E2_QID_MASK) >> TXDONE_E2_QID_SHIFT);
		} else {
			loop++;
			token_id = (((*token_ptr) & TXDONE_E2_MSDU_ID_MASK) >> TXDONE_E2_MSDU_ID_SHIFT);

			que = token_tx_get_queue_by_token_id(cb, token_id);
			que->total_back_cnt++;

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
			/* already handled , do nothing */
			if (que->band_idx == 0) {
				if (que->pkt_token[token_id].Reprocessed) {
					que->pkt_token[token_id].Reprocessed = FALSE;
					data += 4;
					continue;
				} else {
					;
				}
			} else {
				if (que->pkt_token[token_id - cap->tkn_info.band0_token_cnt].Reprocessed) {
					que->pkt_token[token_id - cap->tkn_info.band0_token_cnt].Reprocessed = FALSE;
					data += 4;
					continue;
				} else {
					;
				}
			}
#endif /* CONFIG_HOTSPOT_R2 */

			pkt = token_tx_deq(pAd, que, token_id, &type);

			if (pkt != NULL) {
#ifdef CONFIG_ATE
				if (ATE_ON(pAd)) {
#ifdef CONFIG_WLAN_SERVICE
					struct service_test *serv_test;
					struct test_configuration *configs;
					UCHAR band_idx, wdev_idx;
					struct wifi_dev *wdev = NULL;

					serv_test = (struct service_test *)(pAd->serv.serv_handle);
					wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
					if (wdev_idx >= WDEV_NUM_MAX)
						continue;

					wdev = pAd->wdev_list[wdev_idx];
					band_idx = HcGetBandByWdev(wdev);
					configs = &serv_test->test_config[band_idx];

					net_ad_post_tx(serv_test->test_winfo, configs, band_idx, pkt);
#else
					MT_ATETxControl(pAd, 0xFF, pkt);
#endif
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
				} else
#endif
				{
					if (type == TOKEN_TX_DATA) {
#ifdef EAP_STATS_SUPPORT
					USHORT wcid = RTMP_GET_PACKET_WCID(pkt);
					/*Get Mac table entry */
					PMAC_TABLE_ENTRY pEntry = NULL;
					if (VALID_UCAST_ENTRY_WCID(pAd, wcid)) {
						UINT32 pkt_time = (((*token_ptr) & TXDONE_TX_LATENCY_CNT_MASK) >> TXDONE_TX_LATENCY_CNT_SHIFT);
						UINT8 pkt_status = (((*token_ptr) & TXDONE_STAT_MASK) >> TXDONE_STAT_SHIFT);
						pEntry = &pAd->MacTab.Content[wcid];
						if (pEntry->tx_latency_max == 0) {
							pEntry->tx_latency_min = pkt_time;
							pEntry->tx_latency_max = pkt_time;
							pEntry->tx_latency_avg = pkt_time;
						} else {
							if (pkt_time > 0) {
								if (pEntry->tx_latency_min > pkt_time)
									pEntry->tx_latency_min = pkt_time;
								if (pEntry->tx_latency_max < pkt_time)
									pEntry->tx_latency_max = pkt_time;
								pEntry->tx_latency_avg = (pkt_time + pEntry->tx_latency_avg)/2;
							}
						}
						if (pkt_status == 1)
							pEntry->mpdu_xretries.QuadPart++;
					}
#endif
						RELEASE_NDIS_PACKET_IRQ(pAd, pkt, NDIS_STATUS_SUCCESS);
					}
					else
						RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
					if (pAd->vow_dvt_en &&
					    (!RTMP_GET_PACKET_MGMT_PKT(pkt)) &&
					    (!RTMP_GET_PACKET_HIGH_PRIO(pkt))) {
						UINT16 wcid = pAd->vow_queue_map[token_id][0];
						UCHAR qidx = pAd->vow_queue_map[token_id][1];

						if ((wcid > 0 && wcid < 128) && (qidx >= 0 && qidx < 4)) {
							if (pAd->vow_queue_len[wcid][qidx] > 0)
								pAd->vow_queue_len[wcid][qidx]--;
							pAd->vow_queue_map[token_id][0] = 0xff;
							pAd->vow_queue_map[token_id][1] = 0xf;
							pAd->vow_tx_free[wcid]++;
						}
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"\x1b[31m%s: tokenid %d, wcid %d, qidx %d\x1b[m\n",
							 __func__, token_id, wcid, qidx);
					}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
						 "Get a token_id[%d] = 0x%x but PktPtr is NULL!\n",
						  loop, token_id);
			}
		}

		data += 4;
	}

	if (token_tx_get_state(que) &&
			 (token_tx_get_free_cnt(que)
			 >= token_tx_get_hwmark(que))) {
		token_tx_set_state(que, TX_TOKEN_HIGH);
		qm_ops->schedule_tx_que(pAd, que->band_idx);
	}
}

static VOID tx_free_v3_notify_handler(RTMP_ADAPTER *pAd, PKT_TOKEN_CB *cb,
										UINT8 *data, UINT32 token_cnt)
{
	UINT loop = 0;
	PNDIS_PACKET pkt;
	UINT32 *token_ptr;
	UINT16 token_id;
	UINT8 type;
	UINT8 i;
	UINT16 wlan_id = GET_MAX_UCAST_NUM(pAd);
	UINT8 q_id;
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct token_tx_pkt_queue *que = NULL;
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* CONFIG_HOTSPOT_R2 */
	UINT8 pkt_stat = 0xff; /* 0, 1, 2 has its own meaning*/
	ULONG now_time;
	MAC_TABLE_ENTRY *pEntry = NULL;

	while (loop < token_cnt) {
		token_ptr = (UINT32 *)data;
		if ((*token_ptr) & TXDONE_MT7986_P) {
			wlan_id = (((*token_ptr) & TXDONE_MT7986_WLAN_ID_MASK) >> TXDONE_MT7986_WLAN_ID_SHIFT);
			q_id = (((*token_ptr) & TXDONE_MT7986_QID_MASK) >> TXDONE_MT7986_QID_SHIFT);
			MTWF_DBG(NULL, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_DEBUG, "%s: wlan_id=%d, q_id=%d\n", __func__, wlan_id, q_id);
			data += 4;
			continue;
		}
		if ((*token_ptr) & TXDONE_MT7986_H) {
					/* TxTime / AirTime / TxCount */
#ifdef EAP_STATS_SUPPORT
					/* Get Mac table entry */
					pEntry = NULL;
					/*whether pkt_time and pkt_airtime are believable*/
					BOOLEAN time_believable = TRUE;
					if (VALID_UCAST_ENTRY_WCID(pAd, wlan_id)) {
						UINT32 pkt_time = (((*token_ptr) & TXDONE_MT7986_TX_DELAY_MASK) >> TXDONE_MT7986_TX_DELAY_SHIFT);
						UINT8 pkt_status = (((*token_ptr) & TXDONE_MT7986_STAT_MASK) >> TXDONE_MT7986_STAT_SHIFT);
#ifdef EAP_ENHANCE_STATS_SUPPORT
						UINT32 pkt_airtime = (((*token_ptr) & TXDONE_MT7986_AIR_DELAY_MASK) >> TXDONE_MT7986_AIR_DELAY_SHIFT);
						UINT8 pkt_tx_cnt = (((*token_ptr) & TXDONE_MT7986_TX_CNT_MASK) >> TXDONE_MT7986_TX_CNT_SHIFT);
						UINT32 pkt_queuetime = 0;

						/* calculate pkt_queuetime*/
						if (pkt_time > pkt_airtime)
							pkt_queuetime = pkt_time - pkt_airtime;
						if (pkt_time == pkt_airtime && pkt_time != 0)
							pkt_queuetime = 1;
						if (pkt_time < pkt_airtime) {
							time_believable = FALSE;
							MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_INFO,
									 "pkt_time:%x and pkt_airtime:%x are unbelieved\n",
									 pkt_time, pkt_airtime);
						}

#endif /* EAP_ENHANCE_STATS_SUPPORT */

						pEntry = &pAd->MacTab.Content[wlan_id];

						/* TxTime */
						if (pEntry->tx_latency_max == 0) {
							pEntry->tx_latency_min = pkt_time;
							pEntry->tx_latency_max = pkt_time;
							pEntry->tx_latency_avg = pkt_time;
						} else {
							if (pkt_time > 0) {
								if (pEntry->tx_latency_min > pkt_time)
									pEntry->tx_latency_min = pkt_time;
								if (pEntry->tx_latency_max < pkt_time)
									pEntry->tx_latency_max = pkt_time;
								pEntry->tx_latency_avg = (pkt_time + pEntry->tx_latency_avg)/2;
							}
						}

#ifdef EAP_ENHANCE_STATS_SUPPORT
						/* AirTime */
						if (pEntry->air_latency_max == 0) {
							pEntry->air_latency_min = pkt_airtime;
							pEntry->air_latency_max = pkt_airtime;
							pEntry->air_latency_avg = pkt_airtime;
						} else {
							if (pkt_airtime > 0) {
								if (pEntry->air_latency_min > pkt_airtime)
									pEntry->air_latency_min = pkt_airtime;
								if (pEntry->air_latency_max < pkt_airtime)
									pEntry->air_latency_max = pkt_airtime;
								pEntry->air_latency_avg = (pkt_airtime + pEntry->air_latency_avg)/2;
							}
						}

						/* TxCount */
						if (pEntry->tx_cnt_max == 0) {
							pEntry->tx_cnt_max = pkt_tx_cnt;
							pEntry->tx_cnt_max = pkt_tx_cnt;
							pEntry->tx_cnt_max = pkt_tx_cnt;
						} else {
							if (pkt_tx_cnt > 0) {
								if (pEntry->tx_cnt_min > pkt_tx_cnt)
									pEntry->tx_cnt_min = pkt_tx_cnt;
								if (pEntry->tx_cnt_max < pkt_tx_cnt)
									pEntry->tx_cnt_max = pkt_tx_cnt;
								pEntry->tx_cnt_avg = (pkt_tx_cnt + pEntry->tx_cnt_avg)/2;
							}
						}
#endif /* EAP_ENHANCE_STATS_SUPPORT */

						if (pkt_status == 1)
							pEntry->mpdu_xretries.QuadPart++;
					}
#endif
			if (VALID_UCAST_ENTRY_WCID(pAd, wlan_id)) {
				pkt_stat = (((*token_ptr) & TXDONE_MT7986_STAT_MASK) >> TXDONE_MT7986_STAT_SHIFT);
				if (pkt_stat == 0xFF)
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"warning: pkt_stat=%x\n", pkt_stat);

				pEntry = &pAd->MacTab.Content[wlan_id];
				/* calculate every MPDU as one */
				if (pkt_stat == FLG_HW_DROP)
					pEntry->tx_fail_cnt++;
				pEntry->tx_total_cnt++;

				NdisGetSystemUpTime(&now_time);
				/* check PER every 300ms whether it is equal to 100%*/
				if (RTMP_TIME_AFTER(now_time, pEntry->last_calc_timestamp + 300 / (1000 / OS_HZ))) {
					if (pEntry->tx_fail_cnt == pEntry->tx_total_cnt) {
						pEntry->per_err_times += 1;
						pEntry->tx_contd_fail_cnt += pEntry->tx_fail_cnt;
					} else {
						pEntry->per_err_times = 0;
						pEntry->tx_contd_fail_cnt = 0;
					}

					pEntry->tx_fail_cnt = 0;
					pEntry->tx_total_cnt = 0;
					pEntry->last_calc_timestamp = now_time;
				}
			}

			data += 4;
			continue;
		}

		for (i = 0 ; i <= 1; i++) {
			token_id = (((*token_ptr) & (TXDONE_MT7986_MSDU_ID_MASK << (TXDONE_MT7986_MSDU_ID_SHIFT*i))) >> (TXDONE_MT7986_MSDU_ID_SHIFT*i));
			MTWF_DBG(NULL, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_DEBUG, "%s: token_id=%d, i=%d\n", __func__, token_id, i);
			if (token_id == 0x7fff)
				continue;
			loop++;
			que = token_tx_get_queue_by_token_id(cb, token_id);
			que->total_back_cnt++;

			if (token_id < que->pkt_tkid_start || token_id > que->pkt_tkid_end) {
				continue;
			}

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
			/* already handled , do nothing */
			if (que->band_idx == 0) {
				if (que->pkt_token[token_id].Reprocessed) {
					que->pkt_token[token_id].Reprocessed = FALSE;
					data += 4;
					continue;
				}
			} else {
				if (que->pkt_token[token_id - cap->tkn_info.band0_token_cnt].Reprocessed) {
					que->pkt_token[token_id - cap->tkn_info.band0_token_cnt].Reprocessed = FALSE;
					data += 4;
					continue;
				}
			}
#endif /* CONFIG_HOTSPOT_R2 */

			pkt = token_tx_deq(pAd, que, token_id, &type);

			if (pkt != NULL) {
#ifdef CONFIG_ATE
				if (ATE_ON(pAd)) {
#ifdef CONFIG_WLAN_SERVICE
					struct service_test *serv_test;
					struct test_configuration *configs;
					UCHAR band_idx, wdev_idx;
					struct wifi_dev *wdev = NULL;

					serv_test = (struct service_test *)(pAd->serv.serv_handle);
					wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
					if (wdev_idx >= WDEV_NUM_MAX)
						continue;

					wdev = pAd->wdev_list[wdev_idx];
					band_idx = HcGetBandByWdev(wdev);
					configs = &serv_test->test_config[band_idx];

					net_ad_post_tx(serv_test->test_winfo, configs, band_idx, pkt);
#else
					MT_ATETxControl(pAd, 0xFF, pkt);
#endif
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
				} else
#endif
				{
#ifdef APCLI_SUPPORT
#ifdef WIFI_TWT_SUPPORT
					if (RTMP_GET_PACKET_MGMT_PKT(pkt))
						twtTxDoneCheckSetupFrame(pAd, pkt);
#endif /* WIFI_TWT_SUPPORT */
#endif /* APCLI_SUPPORT */

					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
					if (pAd->vow_dvt_en &&
					    (!RTMP_GET_PACKET_MGMT_PKT(pkt)) &&
					    (!RTMP_GET_PACKET_HIGH_PRIO(pkt))) {
						UINT16 wcid = pAd->vow_queue_map[token_id][0];
						UCHAR qidx = pAd->vow_queue_map[token_id][1];

						if ((wcid > 0 && wcid < 128) && (qidx >= 0 && qidx < 4)) {
							if (pAd->vow_queue_len[wcid][qidx] > 0)
								pAd->vow_queue_len[wcid][qidx]--;
							pAd->vow_queue_map[token_id][0] = 0xff;
							pAd->vow_queue_map[token_id][1] = 0xf;
							pAd->vow_tx_free[wcid]++;
						}
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"\x1b[31m%s: tokenid %d, wcid %d, qidx %d\x1b[m\n",
							 __func__, token_id, wcid, qidx);
					}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
						 "Get a token_id[%d] = 0x%x but PktPtr is NULL!\n",
						  loop, token_id);
			}
		}

		data += 4;
	}

	if (token_tx_get_state(que) &&
			 (token_tx_get_free_cnt(que)
			 >= token_tx_get_hwmark(que))) {
		token_tx_set_state(que, TX_TOKEN_HIGH);
		qm_ops->schedule_tx_que(pAd, que->band_idx);
	}
}

static VOID tx_free_v0_notify_handler(RTMP_ADAPTER *pAd, PKT_TOKEN_CB *cb,
								UINT8 *dataPtr, UINT32 TxDCnt, UINT32 Len, UINT8 Ver)
{
	UINT loop;
	PNDIS_PACKET pkt;
	UINT16 *token_ptr, token_id;
	UINT8 Type, LenPerToken;
	struct qm_ops *qm_ops = pAd->qm_ops;
	/*struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);*/
	struct token_tx_pkt_queue *que = NULL;

	if (dataPtr == NULL) {
		ASSERT(0);
		return;
	}

	LenPerToken = (Ver == 0) ? 2 : 4;

	for (loop = 0; loop < TxDCnt; loop++) {
		if (loop * LenPerToken > Len) {
			MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
					 "token number len mismatch TxD Cnt=%d Len=%d\n",
					  loop, Len);
			hex_dump("EventTxFreeNotifyHandlerErrorFrame", dataPtr, Len);
			break;
		}

		token_ptr = (UINT16 *)dataPtr;
		token_id = le2cpu16(*token_ptr);
		que = token_tx_get_queue_by_token_id(cb, token_id);
		que->total_back_cnt++;
		/* shift position according to the version of tx done event */
		dataPtr += LenPerToken;

		if (token_id < que->pkt_tkid_start || token_id > que->pkt_tkid_end)
			continue;

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
		/* already handled , do nothing */
		if (que->pkt_token[token_id].Reprocessed) {
			que->pkt_token[token_id].Reprocessed = FALSE;
			continue;
		}
#endif /* CONFIG_HOTSPOT_R2 */
		pkt = token_tx_deq(pAd, que, token_id, &Type);

		if (pkt != NULL) {
#ifdef CONFIG_ATE
			if (ATE_ON(pAd)) {
#ifdef CONFIG_WLAN_SERVICE
				struct service_test *serv_test;
				struct test_configuration *configs;
				UCHAR band_idx, wdev_idx;
				struct wifi_dev *wdev = NULL;

				serv_test = (struct service_test *)(pAd->serv.serv_handle);
				wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
				if (wdev_idx >= WDEV_NUM_MAX)
					continue;

				wdev = pAd->wdev_list[wdev_idx];
				band_idx = HcGetBandByWdev(wdev);
				configs = &serv_test->test_config[band_idx];

				net_ad_post_tx(serv_test->test_winfo, configs, band_idx, pkt);
#else
				MT_ATETxControl(pAd, 0xFF, pkt);
#endif
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			} else
#endif
			{
#ifdef FQ_SCH_SUPPORT
				RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

				if (cap->qm == FAST_PATH_FAIR_QM)
					fq_tx_free_per_packet(pAd, RTMP_GET_PACKET_QUEIDX(pkt),
						RTMP_GET_PACKET_WCID(pkt), pkt);
#endif
				if (Type == TOKEN_TX_DATA)
					RELEASE_NDIS_PACKET_IRQ(pAd, pkt, NDIS_STATUS_SUCCESS);
				else
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
					 "Get a token_id[%d] = 0x%x but PktPtr is NULL!\n",
					  loop, token_id);
			hex_dump("EventTxFreeNotifyHandlerNullPktPtrFrame", dataPtr, Len);
		}
	}

	if (token_tx_get_state(que) &&
			 (token_tx_get_free_cnt(que)
			 >= token_tx_get_hwmark(que))) {
		token_tx_set_state(que, TX_TOKEN_HIGH);
		qm_ops->schedule_tx_que(pAd, que->band_idx);
	}



}


static UINT32 mtf_v1_txdone_handle(RTMP_ADAPTER *pAd, PKT_TOKEN_CB *cb, VOID *ptr)
{
	struct txdone_event *txdone = (struct txdone_event *)ptr;
	UINT8 *token_list;
	UINT16 rx_byte_cnt = ((txdone->txdone_0 & TXDONE_RX_BYTE_CNT_MASK) >> TXDONE_RX_BYTE_CNT_SHIFT);
	UINT8 token_cnt = ((txdone->txdone_0 & TXDONE_MSDU_ID_CNT_MASK) >> TXDONE_MSDU_ID_CNT_SHIFT);
	UINT8 pkt_type = ((txdone->txdone_0 & TXDONE_PKT_TYPE_MASK) >> TXDONE_PKT_TYPE_SHIFT);

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: rx_byte_cnt = %d,token_cnt = %d, token_cnt = %d\n",
			  __func__, rx_byte_cnt, token_cnt, token_cnt);

	if (token_cnt > 0x7f) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid token_cnt(%d)!\n", token_cnt);
		return FALSE;
	}

	switch (pkt_type) {
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
		if (((token_cnt << 2) + 8) != rx_byte_cnt) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "token_cnt(%d) and rx_byte_cnt(%d) mismatch!\n", token_cnt, rx_byte_cnt);
			hex_dump("TxFreeNotifyEventMisMatchFrame", ptr, rx_byte_cnt);
			return FALSE;
		}

		token_list = TXRX_NOTE_GET_TOKEN_LIST(ptr);

		tx_free_v1_notify_handler(pAd, cb, token_list, token_cnt);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid type(%d)!\n", pkt_type);
		break;
	}

	return TRUE;
}

static UINT32 mtf_v2_txdone_handle(RTMP_ADAPTER *pAd, PKT_TOKEN_CB *cb,
									VOID *ptr)
{
	struct txdone_event *txdone = (struct txdone_event *)ptr;
	UINT8 *token_list;
	UINT16 rx_byte_cnt = ((txdone->txdone_0 & TXDONE_RX_BYTE_CNT_MASK) >> TXDONE_RX_BYTE_CNT_SHIFT);
	UINT16 token_cnt = ((txdone->txdone_0 & TXDONE_E2_MSDU_ID_CNT_MASK) >> TXDONE_MSDU_ID_CNT_SHIFT);
	UINT8 pkt_type = ((txdone->txdone_0 & TXDONE_PKT_TYPE_MASK) >> TXDONE_PKT_TYPE_SHIFT);

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: rx_byte_cnt = %d,token_cnt = %d, token_cnt = %d\n",
			  __func__, rx_byte_cnt, token_cnt, token_cnt);

	if (token_cnt > 0x3ff) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid token_cnt(%d)!\n", token_cnt);
		return FALSE;
	}

	switch (pkt_type) {
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:

		token_list = TXRX_NOTE_GET_TOKEN_LIST(ptr);

		tx_free_v2_notify_handler(pAd, cb, token_list, token_cnt);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid type(%d)!\n", pkt_type);
		break;
	}

	return TRUE;
}


static UINT32 mtf_v3_txdone_handle(RTMP_ADAPTER *pAd, PKT_TOKEN_CB *cb,
									VOID *ptr)
{
	struct txdone_event *txdone = (struct txdone_event *)ptr;
	UINT8 *token_list;
	UINT16 rx_byte_cnt = ((txdone->txdone_0 & TXDONE_RX_BYTE_CNT_MASK) >> TXDONE_RX_BYTE_CNT_SHIFT);
	UINT16 token_cnt = ((txdone->txdone_0 & TXDONE_E2_MSDU_ID_CNT_MASK) >> TXDONE_MSDU_ID_CNT_SHIFT);
	UINT8 pkt_type = ((txdone->txdone_0 & TXDONE_PKT_TYPE_MASK) >> TXDONE_PKT_TYPE_SHIFT);

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: rx_byte_cnt = %d,token_cnt = %d, token_cnt = %d\n",
			  __func__, rx_byte_cnt, token_cnt, token_cnt);

	if (token_cnt > 0x3ff) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid token_cnt(%d)!\n", token_cnt);
		return FALSE;
	}

	switch (pkt_type) {
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:

		token_list = TXRX_NOTE_GET_TOKEN_LIST(ptr);

		tx_free_v3_notify_handler(pAd, cb, token_list, token_cnt);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid type(%d)!\n", pkt_type);
		break;
	}

	return TRUE;
}

static UINT32 mtf_v0_txdone_handle(RTMP_ADAPTER *pAd, PKT_TOKEN_CB *cb,
									VOID *ptr)
{
	UINT32 dw0 = *(UINT32 *)ptr;
	UINT32 dw1 = *((UINT32 *)ptr + 1);
	UINT8 *tokenList;
	UINT8 tokenCnt;
	UINT16 rxByteCnt;
	UINT8 pkt_type;
	UINT8 version, len_per_token;

	rxByteCnt = (dw0 & 0xffff);
	tokenCnt = ((dw0 & (0x7f << 16)) >> 16) & 0x7f;
	pkt_type = ((dw0 & TXDONE_PKT_TYPE_MASK_V0) >> TXDONE_PKT_TYPE_SHIFT_V0) & TXDONE_PKT_TYPE_MASK_VALUE_V0;
	version = ((dw1 & (0x7 << 16)) >> 16) & 0x7;
	MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "DW0=0x%08x, rxByteCnt=%d,tokenCnt= %d, pkt_type=%d\n",
			  dw0, rxByteCnt, tokenCnt, pkt_type);

	if (tokenCnt > 0x7f) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid tokenCnt(%d)!\n", tokenCnt);
		return FALSE;
	}

	switch (pkt_type) {
	case RMAC_RX_PKT_TYPE_TXRX_NOTIFY:
		len_per_token = (version == 0) ? 2 : 4;

		if ((tokenCnt * len_per_token + 8) != rxByteCnt) {
			MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "tokenCnt(%d) and rxByteCnt(%d) mismatch!\n", tokenCnt, rxByteCnt);
			hex_dump("TxFreeNotifyEventMisMatchFrame", ptr, rxByteCnt);
			return FALSE;
		}

		tokenList = TXRX_NOTE_GET_TOKEN_LIST(ptr);
		tx_free_v0_notify_handler(pAd, cb, tokenList, tokenCnt, rxByteCnt - 8, version);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid PTK_TYPE(%d) in TX_FREE_DONE_EVENT!\n", pkt_type);
		break;
	}

	return TRUE;
}

inline UINT32 mtf_txdone_handle(RTMP_ADAPTER *pAd, VOID *ptr, UINT8 resource_idx)
{
	PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	struct txdone_event *txdone = (struct txdone_event *)ptr;
#ifdef RT_BIG_ENDIAN
	txdone->txdone_1 = le2cpu32(txdone->txdone_1);
#endif /* RT_BIG_ENDIAN */

	UINT8 ver = ((txdone->txdone_1 & TXDONE_VER_MASK) >> TXDONE_VER_SHIFT);

	if (ver == MT7615_TXDONE || ver == MT7622_TXDONE)
		mtf_v0_txdone_handle(pAd, cb, ptr);
	else if (ver == MT7986_TXDONE || ver == MT7916_TXDONE || ver == MT7981_TXDONE)
		mtf_v3_txdone_handle(pAd, cb, ptr);
	else if (ver == MT7915_E2_TXDONE)
		mtf_v2_txdone_handle(pAd, cb, ptr);
	else if (ver == MT7915_E1_TXDONE)
		mtf_v1_txdone_handle(pAd, cb, ptr);
	else
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid txdone version(%d)!\n", ver);

	return TRUE;
}

#ifdef WFDMA_WED_COMPATIBLE
VOID mtf_wa_cpu_update(RTMP_ADAPTER *ad)
{
#ifdef WHNAT_SUPPORT
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	if ((ad->CommonCfg.whnat_en) &&
		!(cap->asic_caps & fASIC_CAP_TX_FREE_NOTIFY_V4))
		MtCmdCr4Capability(ad, 0);
	MtCmdCr4Set(ad, WA_SET_OPTION_WED_VERSION, ad->CommonCfg.wed_version, 0);
#endif
}

#endif /*WFDMA_WED_COMPATIBLE*/

INT32 mtf_txs_handler(RTMP_ADAPTER *pAd, VOID *rx_packet)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	struct txs_header *txs_h = (struct txs_header *)(rx_packet);
	struct txs_frame *txs_f;
	UCHAR *ptr;
	UINT8 format;
	UINT32 tid = 0;
	INT idx, txs_entry_len = 32;
	BOOLEAN me, re, le, be, txop_limit_error, baf;
	UINT32 rx_byte_cnt = (txs_h->txs_h_0 & TXS_RX_BYTE_CNT_MASK);
	UINT32 txs_cnt = ((txs_h->txs_h_0 & TXS_CNT_MASK) >> TXS_CNT_SHIFT);
	UINT32 dbg_prn = DBG_LVL_INFO;

#ifdef PKTLOSS_CHK
	if (pAd->pktloss_chk.txs_log_enable)
		dbg_prn = DBG_LVL_INFO;
#endif
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "TxS Report: Number=%d, ByteCnt=%d\n",
			 txs_cnt, rx_byte_cnt);

	if (rx_byte_cnt == 0)
		return TRUE;

	if (rx_byte_cnt != (txs_cnt * txs_entry_len + 8)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "ReceivedByteCnt not equal txs_entry required!\n");
	} else {
		ptr = ((UCHAR *)(rx_packet)) + 8;

		for (idx = 0; idx < txs_cnt; idx++) {
			txs_f = (struct txs_frame *)ptr;
#ifdef RT_BIG_ENDIAN
			RTMPEndianChange((UCHAR *)txs_f, sizeof(struct txs_frame));
#endif /* RT_BIG_ENDIAN */

			format = ((txs_f->txs_f_0 & TXS_TXSFM_MASK) >> TXS_TXSFM_SHIFT);
			tid = (txs_f->txs_f_0 & TXS_TID_MASK) >> TXS_TID_SHIFT;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
					"\t\tTXSFM = %d, TXS2M/H = %d/%d, FixRate = %d, TxRate/BW = 0x%x/%d\n",
					format,
					(txs_f->txs_f_0 & TXS_TXS2M) ? 1 : 0,
					(txs_f->txs_f_0 & TXS_TXS2H) ? 1 : 0,
					(txs_f->txs_f_0 & TXS_FR) ? 1 : 0,
					(txs_f->txs_f_0 & TXS_TX_RATE_MASK) >> TXS_TX_RATE_SHIFT,
					(txs_f->txs_f_0 & TXS_TBW_MASK) >> TXS_TBW_SHIFT);

#ifdef AUTOMATION

					if (is_frame_test(pAd, 1) != 0) {
						pAd->auto_dvt->txs.received_pid =
(txs_f->txs_f_3 & TXS_PID_MASK) >> TXS_PID_SHIFT;
						receive_del_txs_queue((txs_f->txs_f_1 & TXS_SN_MASK) >> TXS_SN_SHIFT,
(txs_f->txs_f_3 & TXS_PID_MASK) >> TXS_PID_SHIFT);
(txs_f->txs_f_3 & TXS_PID_MASK) >> TXS_PID_SHIFT,
(txs_f->txs_f_2 & TXS_WLAN_IDX_MASK) >> TXS_WLAN_IDX_SHIFT,
(txs_f->txs_f_4 & TXS_TIMESTAMP_MASK) >> TXS_TIMESTAMP_SHIFT);
					}

#endif /* AUTOMATION */


			me = ((txs_f->txs_f_0 & TXS_F_ME) ? 1 : 0);

			if (me)
				tr_cnt->me[tid]++;

			re = ((txs_f->txs_f_0 & TXS_F_RE) ? 1 : 0);

			if (re)
				tr_cnt->re[tid]++;

			le = ((txs_f->txs_f_0 & TXS_F_LE) ? 1 : 0);

			if (le)
				tr_cnt->le[tid]++;

			be = ((txs_f->txs_f_0 & TXS_F_BE) ? 1 : 0);

			if (be)
				tr_cnt->be[tid]++;

			txop_limit_error = ((txs_f->txs_f_0 & TXS_F_TXOP_LIMITE) ? 1 : 0);

			if (txop_limit_error)
				tr_cnt->txop_limit_error[tid]++;

			baf = ((txs_f->txs_f_0 & TXS_F_BAF) ? 1 : 0);

			if (baf)
				tr_cnt->baf[tid]++;

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
					"\t\tME/RE/LE/BE/TxOPLimitErr/BA-Fail = %d/%d/%d/%d/%d/%d, PS = %d, Pid = %d\n",
					me,
					re,
					le,
					be,
					txop_limit_error,
					baf,
					(txs_f->txs_f_0 & TXS_F_PS) ? 1 : 0,
					(txs_f->txs_f_3 & TXS_PID_MASK) >> TXS_PID_SHIFT);

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
					"\t\tTid = %d, AntId = %d, ETxBF/ITxBf = %d/%d\n",
					(txs_f->txs_f_0 & TXS_TID_MASK) >> TXS_TID_SHIFT,
					(txs_f->txs_f_3 & TXS_ANT_ID_MASK) >> TXS_ANT_ID_SHIFT,
					(txs_f->txs_f_2 & TXS_ETXBF) ? 1 : 0,
					(txs_f->txs_f_2 & TXS_ITXBF) ? 1 : 0);

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
					"\t\tTxPwrdBm = 0x%x, AMPDU = 0x%x\n",
					(txs_f->txs_f_1 & TXS_TX_PWR_MASK) >> TXS_TX_PWR_SHIFT,
					(txs_f->txs_f_0 & TXS_AM) ? 1 : 0);

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
					"\t\tTxDelay(32us) = 0x%x, RxVSeqNum =0x %x, Wlan Idx = 0x%x\n",
					(txs_f->txs_f_2 & TXS_TX_DELAY_MASK) >> TXS_TX_DELAY_SHIFT,
					(txs_f->txs_f_1 & TXS_RX_VECTOR_MASK) >> TXS_RX_VECTOR_SHIFT,
					(txs_f->txs_f_2 & TXS_WLAN_IDX_MASK) >> TXS_WLAN_IDX_SHIFT);

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
					"\t\tSN = 0x%x, LastTxRateIdx = %d\n",
					(txs_f->txs_f_1 & TXS_SN_MASK) >> TXS_SN_SHIFT,
					(txs_f->txs_f_2 & TXS_LAST_MCS_IDX_MASK) >> TXS_LAST_MCS_IDX_SHIFT);

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
					"\t\ttimestamp = 0x%x\n",
					(txs_f->txs_f_4 & TXS_TIMESTAMP_MASK) >> TXS_TIMESTAMP_SHIFT);

			if ((format == TXS_FORMAT0) || (format == TXS_FORMAT1)) {
				UINT32 Pid;
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
					"\t\tFrontTime(32us) = 0x%x, MPDU TxCnt =%d, Oos = %d, FinalMPDU = %d\n",
					(txs_f->txs_f_5 & TXS_FRONT_TIME_MASK) >> TXS_FRONT_TIME_SHIFT,
					(txs_f->txs_f_5 & TXS_MPDU_TX_CNT_MASK) >> TXS_MPDU_TX_CNT_SHIFT,
					(txs_f->txs_f_5 & TXS_QOS) ? 1 : 0,
					(txs_f->txs_f_5 & TXS_FM) ? 1 : 0);
				Pid = ((txs_f->txs_f_3 & TXS_PID_MASK) >> TXS_PID_SHIFT);
				if (Pid == PID_EAPOL_FRAME) {
					MAC_TABLE_ENTRY *pEntry = NULL;
					UINT16 wlanIdx;

					wlanIdx = (txs_f->txs_f_2 & TXS_WLAN_IDX_MASK) >> TXS_WLAN_IDX_SHIFT;
					pEntry = &pAd->MacTab.Content[wlanIdx];
					if (pEntry && re) {
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "EAPOL RTS Failed for WCID = %d \n", wlanIdx);
						pEntry->bLastRTSFailed = TRUE;
					}

				}
			} else if (format == TXS_FORMAT2) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
						 "\t\tPPDU MPDU TX Bytes = %d, PPDU MPDU TX Cnt = %d\n",
						 (txs_f->txs_f_5 & TXS_PPDU_MPDU_TX_BYTE_MASK) >> TXS_PPDU_MPDU_TX_BYTE_SHIFT,
						 (txs_f->txs_f_5 & TXS_PPDU_MPDU_TX_CNT_MASK) >> TXS_PPDU_MPDU_TX_CNT_SHIFT);
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
						 "\t\tPPDU MPDU Fail Bytes = %d, PPDU MPDU Fail Cnt = %d\n",
						 (txs_f->txs_f_6 & TXS_PPDU_MPDU_FAIL_BYTE_MASK) >> TXS_PPDU_MPDU_FAIL_BYTE_SHIFT,
						 (txs_f->txs_f_6 & TXS_PPDU_MPDU_FAIL_CNT_MASK) >> TXS_PPDU_MPDU_FAIL_CNT_SHIFT);
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_prn,
						 "\t\tPPDU MPDU Retry Bytes = %d, PPDU MPDU Retry Cnt = %d\n",
						 (txs_f->txs_f_7 & TXS_PPDU_MPDU_RTY_BYTE_MASK) >> TXS_PPDU_MPDU_RTY_BYTE_SHIFT,
						 (txs_f->txs_f_7 & TXS_PPDU_MPDU_RTY_CNT_MASK) >> TXS_PPDU_MPDU_RTY_CNT_SHIFT);
			} else {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Unknown TxSFormat(%d)\n", format);
			}

			ptr += txs_entry_len;
		}
	}

	return 0;
}

VOID mtf_rx_event_handler(RTMP_ADAPTER *pAd, UCHAR *data)
{
	struct cmd_msg *msg;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	EVENT_RXD *event_rxd = (EVENT_RXD *)(data + sizeof(struct rxd_grp_0));

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

#ifdef RT_BIG_ENDIAN
	event_rxd->fw_rxd_0.word = le2cpu32(event_rxd->fw_rxd_0.word);
	event_rxd->fw_rxd_1.word = le2cpu32(event_rxd->fw_rxd_1.word);
	event_rxd->fw_rxd_2.word = le2cpu32(event_rxd->fw_rxd_2.word);
#endif
	msg = AndesAllocCmdMsg(pAd, GET_EVENT_FW_RXD_LENGTH(event_rxd));

	if (!msg || !msg->net_pkt)
		return;

	AndesAppendCmdMsg(msg, (char *)(data + sizeof(struct rxd_grp_0)),
				GET_EVENT_FW_RXD_LENGTH(event_rxd));

	AndesMTRxProcessEvent(pAd, msg);
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)

	if (msg->net_pkt)
		RTMPFreeNdisPacket(pAd, msg->net_pkt);

#endif
	AndesFreeCmdMsg(msg);
}

static VOID mtf_fill_txd_header(struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	struct txd_l *txd;
	UCHAR *tmac_info;

	tmac_info = (UCHAR *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(struct txd_l));
	txd = (struct txd_l *)tmac_info;
	NdisZeroMemory(txd, sizeof(struct txd_l));

	txd->txd_0 |= (GET_OS_PKT_LEN(net_pkt) << TXD_TX_BYTE_COUNT_SHIFT);
	txd->txd_0 |= (MCU_RQ0 << TXD_Q_IDX_SHIFT);

	if (msg->attr.type == MT_FW_SCATTER)
		txd->txd_0 |= (FT_HIF_FD << TXD_PKT_FT_SHIFT);
	else
		txd->txd_0 |= (FT_HIF_CMD << TXD_PKT_FT_SHIFT);

	txd->txd_1 |= TXD_FT;
	txd->txd_1 |= (HF_CMD_FRAME << TXD_HF_SHIFT);

	/*VTA [10]*/
#ifdef EAP_STATS_SUPPORT
	txd->txd_1 |= TXD_VTA;
#endif

#ifdef RT_BIG_ENDIAN
	MTMacInfoEndianChange(NULL, (UCHAR *)tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif
	return;
}

#ifdef WIFI_UNIFIED_COMMAND
VOID mtf_fill_uni_cmd_header(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *pkt)
{
	UNI_CMD_HEADER *cmd_header = NULL;
	PNDIS_PACKET net_pkt = (PNDIS_PACKET)pkt;

	/* WFDMA arch doesn't need cmd header for image packet */
	if (pAd->MCUCtrl.fwdl_ctrl.stage == FWDL_STAGE_SCATTER)
		return;

	cmd_header = (UNI_CMD_HEADER *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(UNI_CMD_HEADER));
	mtf_fill_txd_header(msg, net_pkt);
	NdisZeroMemory(cmd_header, sizeof(UNI_CMD_HEADER));

	cmd_header->header_0.field.length = GET_OS_PKT_LEN(net_pkt) - sizeof(TMAC_TXD_L);
	cmd_header->header_0.field.cid = msg->attr.type;
	cmd_header->header_1.field.pkt_type_id = PKT_ID_CMD;
	cmd_header->header_1.field.frag_num = UNI_CMD_WRAP_FRAG_NUM(msg->total_frag, msg->frag_num);
	cmd_header->header_1.field.seq_num = msg->seq;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 "mcu_dest(%d):%s\n", msg->attr.mcu_dest,
		  (msg->attr.mcu_dest == HOST2N9) ? "HOST2N9" : "HOST2CR4N9");

	if (msg->attr.mcu_dest == HOST2N9)
		cmd_header->header_2.field.s2d_index = HOST2N9;
	else if (msg->attr.mcu_dest == HOST2CR4)
		cmd_header->header_2.field.s2d_index = HOST2CR4;
	else
		cmd_header->header_2.field.s2d_index = HOST2CR4N9;

	cmd_header->header_2.field.checksum = 0;
	cmd_header->header_2.field.option = UNI_CMD_OPT_BIT_1_UNI_CMD;
	if (IS_CMD_MSG_NEED_FW_RSP_FLAG_SET(msg))
		cmd_header->header_2.field.option |= UNI_CMD_OPT_BIT_0_ACK;
	if (IS_CMD_MSG_SET_QUERY_FLAG_SET(msg))
		cmd_header->header_2.field.option |= UNI_CMD_OPT_BIT_2_SET_QUERY;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 "cmd_header: 0x%x 0x%x 0x%x, Length=%d\n",
		  cmd_header->header_0.word, cmd_header->header_1.word,
		  cmd_header->header_2.word,
		  cmd_header->header_0.field.length);

	cmd_header->header_0.word = cpu2le32(cmd_header->header_0.word);
	cmd_header->header_1.word = cpu2le32(cmd_header->header_1.word);
	cmd_header->header_2.word = cpu2le32(cmd_header->header_2.word);
}
#endif /* WIFI_UNIFIED_COMMAND */

VOID mtf_fill_cmd_header(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *pkt)
{
	FW_TXD *fw_txd = NULL;
	PNDIS_PACKET net_pkt = (PNDIS_PACKET)pkt;

	/* WFDMA arch doesn't need cmd header for image packet */
	if (pAd->MCUCtrl.fwdl_ctrl.stage == FWDL_STAGE_SCATTER)
		return;

	fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(FW_TXD));
	mtf_fill_txd_header(msg, net_pkt);
	NdisZeroMemory(fw_txd, sizeof(FW_TXD));

	fw_txd->fw_txd_0.field.length =	GET_OS_PKT_LEN(net_pkt) - sizeof(TMAC_TXD_L);
	fw_txd->fw_txd_0.field.pq_id = msg->pq_id;
	fw_txd->fw_txd_1.field.cid = msg->attr.type;
	fw_txd->fw_txd_1.field.pkt_type_id = PKT_ID_CMD;
	fw_txd->fw_txd_1.field.set_query = IS_CMD_MSG_NA_FLAG_SET(msg) ?
						CMD_NA : IS_CMD_MSG_SET_QUERY_FLAG_SET(msg);
	fw_txd->fw_txd_1.field.seq_num = msg->seq;
	fw_txd->fw_txd_2.field.ext_cid = msg->attr.ext_type;

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: mcu_dest(%d):%s\n", __func__, msg->attr.mcu_dest,
			  (msg->attr.mcu_dest == HOST2N9) ? "HOST2N9" : "HOST2CR4");

	if (msg->attr.mcu_dest == HOST2N9)
		fw_txd->fw_txd_2.field.ucS2DIndex = HOST2N9;
	else if (msg->attr.mcu_dest == HOST2CR4)
		fw_txd->fw_txd_2.field.ucS2DIndex = HOST2CR4;
	else
		fw_txd->fw_txd_2.field.ucS2DIndex = HOST2CR4N9;

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: fw_txd: 0x%x 0x%x 0x%x, Length=%d\n", __func__,
			  fw_txd->fw_txd_0.word, fw_txd->fw_txd_1.word,
			  fw_txd->fw_txd_2.word, fw_txd->fw_txd_0.field.length);

	if ((IS_EXT_CMD_AND_SET_NEED_RSP(msg)) && !(IS_CMD_MSG_NA_FLAG_SET(msg)))
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NEED_ACK;
	else
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NO_NEED_ACK;

	fw_txd->fw_txd_0.word = cpu2le32(fw_txd->fw_txd_0.word);
	fw_txd->fw_txd_1.word = cpu2le32(fw_txd->fw_txd_1.word);
	fw_txd->fw_txd_2.word = cpu2le32(fw_txd->fw_txd_2.word);
#ifdef CONFIG_TRACE_SUPPORT
	TRACE_MCU_CMD_INFO(fw_txd->fw_txd_0.field.length,
					   fw_txd->fw_txd_0.field.pq_id, fw_txd->fw_txd_1.field.cid,
					   fw_txd->fw_txd_1.field.pkt_type_id, fw_txd->fw_txd_1.field.set_query,
					   fw_txd->fw_txd_1.field.seq_num, fw_txd->fw_txd_2.field.ext_cid,
					   fw_txd->fw_txd_2.field.ext_cid_option,
					   (char *)(GET_OS_PKT_DATAPTR(net_pkt)), GET_OS_PKT_LEN(net_pkt));
#endif /* CONFIG_TRACE_SUPPORT */
}

UINT32 mtf_rxv_handler(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, VOID *rx_packet)
{
	/* sanity check for null pointer */
	if (!rx_blk)
		return 1;

	if (!rx_packet)
		return 1;

	/* parsing rxv packet */
	chip_parse_rxv_packet(pAd, RMAC_RX_PKT_TYPE_RX_TXRXV, rx_blk, rx_packet);

	return 0;
}

VOID mtf_calculate_ecc(struct _RTMP_ADAPTER *ad, UINT32 oper, UINT32 group, UINT8 *scalar, UINT8 *point_x, UINT8 *point_y)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(ad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"oper:%d, group:%d\n", oper, group);
#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support)
		UniCmdCalculateECC(ad, oper, group, scalar, point_x, point_y);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		cmd_calculate_ecc(ad, oper, group, scalar, point_x, point_y);
}

#ifdef MGMT_TXPWR_CTRL
INT wtbl_update_pwr_offset(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	CMD_WTBL_PWR_T WtblPwrOffset = {0};
	CMD_WTBL_RATE_T WtblRateInfo = {0};
	CMD_WTBL_SPE_T WtblSpeInfo = {0};
	UINT16 ucWlanIdx = wdev->tr_tb_idx;
	UINT16 ucRate = 0;

	/* update power offset to wtbl*/
	WtblPwrOffset.u2Tag = WTBL_PWR_OFFSET;
	WtblPwrOffset.u2Length = sizeof(CMD_WTBL_PWR_T);
	WtblPwrOffset.ucPwrOffset =  wdev->TxPwrDelta;
	CmdExtWtblUpdate(pAd, ucWlanIdx, SET_WTBL, &WtblPwrOffset, sizeof(CMD_WTBL_PWR_T));

	/* update RATE Info to wtbl*/
	WtblRateInfo.u2Tag = WTBL_RATE;
	WtblRateInfo.u2Length = sizeof(CMD_WTBL_RATE_T);
	/* set wtbl rate to OFDM_6M(5G) / CCK_1M(2.4G) */
	ucRate = (wdev->channel > 14) ? 0x4B : 0x0;
	WtblRateInfo.u2Rate1 = ucRate;
	WtblRateInfo.u2Rate2 = ucRate;
	WtblRateInfo.u2Rate3 = ucRate;
	WtblRateInfo.u2Rate4 = ucRate;
	WtblRateInfo.u2Rate5 = ucRate;
	WtblRateInfo.u2Rate6 = ucRate;
	WtblRateInfo.u2Rate7 = ucRate;
	WtblRateInfo.u2Rate8 = ucRate;

	CmdExtWtblUpdate(pAd, ucWlanIdx, SET_WTBL, &WtblRateInfo, sizeof(CMD_WTBL_RATE_T));

	WtblSpeInfo.u2Tag = WTBL_SPE;
	WtblSpeInfo.u2Length = sizeof(CMD_WTBL_SPE_T);
	if (pAd->CommonCfg.bSeOff != TRUE) {
		if (HcGetBandByWdev(wdev) == BAND0)
			WtblSpeInfo.ucSpeIdx = BAND0_SPE_IDX;
		else if (HcGetBandByWdev(wdev) == BAND1)
			WtblSpeInfo.ucSpeIdx = BAND1_SPE_IDX;
	}
	CmdExtWtblUpdate(pAd, ucWlanIdx, SET_WTBL, &WtblSpeInfo, sizeof(CMD_WTBL_SPE_T));

	return TRUE;
}
#endif

#ifdef TX_POWER_CONTROL_SUPPORT
VOID mtf_txpower_boost(struct _RTMP_ADAPTER *pAd, UCHAR ucBandIdx)
{
	if (ucBandIdx >= DBDC_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"ucBandIdx incorrect!!!\n");
		return;
	}
	/* config Power boost table via profile */
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_CCK,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpCck[0]),
				SINGLE_SKU_FILL_TABLE_CCK_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_OFDM,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpOfdm[0]),
				SINGLE_SKU_FILL_TABLE_OFDM_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_HT20,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpHt20[0]),
				SINGLE_SKU_FILL_TABLE_HT20_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_HT40,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpHt40[0]),
				SINGLE_SKU_FILL_TABLE_HT40_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_VHT20,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpVht20[0]),
				SINGLE_SKU_FILL_TABLE_VHT20_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_VHT40,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpVht40[0]),
				SINGLE_SKU_FILL_TABLE_VHT40_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_VHT80,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpVht80[0]),
				SINGLE_SKU_FILL_TABLE_VHT80_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_VHT160,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpVht160[0]),
				SINGLE_SKU_FILL_TABLE_VHT160_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_RU26,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU26[0]),
				SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_RU52,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU52[0]),
				SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_RU106,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU106[0]),
				SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_RU242,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU242[0]),
				SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_RU484,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU484[0]),
				SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_RU996,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU996[0]),
				SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V1_RU996X2,
				&(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU996X2[0]),
				SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1);
}

VOID mtf_txpower_boost_ctrl(struct _RTMP_ADAPTER *pAd, UCHAR ucBandIdx, CHAR cPwrUpCat, PUCHAR pcPwrUpValue)
{
	switch (cPwrUpCat) {
	case POWER_UP_CATE_V1_CCK:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpCck, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_CCK_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_OFDM:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpOfdm, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_OFDM_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_HT20:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpHt20, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_HT20_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_HT40:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpHt40, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_HT40_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_VHT20:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpVht20, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_VHT20_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_VHT40:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpVht40, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_VHT40_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_VHT80:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpVht80, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_VHT80_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_VHT160:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpVht160, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_VHT160_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_RU26:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU26, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_RU52:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU52, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_RU106:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU106, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_RU242:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU242, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_RU484:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU484, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_RU996:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU996, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1);
		break;

	case POWER_UP_CATE_V1_RU996X2:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpRU996X2, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "set wrong parameters\n");
	}
}

BOOLEAN mtf_txpower_boost_info(struct _RTMP_ADAPTER *pAd, POWER_BOOST_TABLE_CATEGORY_V1 ePowerBoostRateType)
{
	switch (ePowerBoostRateType) {
	case POWER_UP_CATE_V1_CCK:
		MTWF_PRINT("CCK\n (M0)-(M1)-(M2)-(M3)\n");
		break;

	case POWER_UP_CATE_V1_OFDM:
		MTWF_PRINT("OFDM\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)\n");
		break;

	case POWER_UP_CATE_V1_HT20:
		MTWF_PRINT("HT20\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)\n");
		break;

	case POWER_UP_CATE_V1_HT40:
		MTWF_PRINT("HT40\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)\n");
		break;

	case POWER_UP_CATE_V1_VHT20:
		MTWF_PRINT("VHT20\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	case POWER_UP_CATE_V1_VHT40:
		MTWF_PRINT("VHT40\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	case POWER_UP_CATE_V1_VHT80:
		MTWF_PRINT("VHT80\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	case POWER_UP_CATE_V1_VHT160:
		MTWF_PRINT("VHT160\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	case POWER_UP_CATE_V1_RU26:
		MTWF_PRINT("RU26\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	case POWER_UP_CATE_V1_RU52:
		MTWF_PRINT("RU52\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	case POWER_UP_CATE_V1_RU106:
		MTWF_PRINT("RU106\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	case POWER_UP_CATE_V1_RU242:
		MTWF_PRINT("RU242\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	case POWER_UP_CATE_V1_RU484:
		MTWF_PRINT("RU484\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	case POWER_UP_CATE_V1_RU996:
		MTWF_PRINT("RU996\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	case POWER_UP_CATE_V1_RU996X2:
		MTWF_PRINT("RU996X2\n");
		MTWF_PRINT("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)-(M10)-(M11)\n");
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

BOOLEAN mtf_txpower_boost_power_cat_type(struct _RTMP_ADAPTER *pAd, UINT8 u1PhyMode, UINT8 u1Bw, PUINT8 pu1PowerBoostRateType)
{
	UINT32 u4PwrUpCat;

	if (u1PhyMode > 4) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid PhyMode!! Please Enter PhyMode as CCK:0, OFDM:1, HT:2, VHT:3, HE:4\n");
		return FALSE;
	}

	if ((u1PhyMode == 0 || u1PhyMode == 1) && (u1Bw != 0)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid BW!! For CCK/OFDM mode valid BW option is 0\n");
		return FALSE;
	} else if ((u1PhyMode == 2) && (u1Bw > 1)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid BW!! For HT mode valid BW option is 0/1\n");
		return FALSE;
	} else if ((u1PhyMode == 3) && (u1Bw > 3)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid BW!! For VHT mode valid BW option is 0/1/2/3\n");
		return FALSE;
	} else if ((u1PhyMode == 4) && (u1Bw > 6)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid BW!! For HE mode valid BW is 0/1/2/3/4/5/6\n");
		return FALSE;
	}

	if (u1PhyMode == 3) {
		u4PwrUpCat = POWER_UP_CATE_V1_VHT20 + u1Bw;
	} else if (u1PhyMode == 4) {
		u4PwrUpCat = POWER_UP_CATE_V1_RU26 + u1Bw;
	} else {
		u4PwrUpCat = u1PhyMode + u1Bw;
	}

	*pu1PowerBoostRateType = u4PwrUpCat;
	return TRUE;
}

BOOLEAN mtf_txpower_boost_rate_type(struct _RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 u1PowerBoostRateType)
{
	UINT_8 ucRateIdx, i, u1Temp = 0;
	PCHAR pcTxPwrBoost;
	UINT8 u1Length[POWER_UP_CATE_V1_NUM] = {0, SINGLE_SKU_FILL_TABLE_CCK_LENGTH_V1, SINGLE_SKU_FILL_TABLE_OFDM_LENGTH_V1, SINGLE_SKU_FILL_TABLE_HT20_LENGTH_V1,
							SINGLE_SKU_FILL_TABLE_HT40_LENGTH_V1, SINGLE_SKU_FILL_TABLE_VHT20_LENGTH_V1, SINGLE_SKU_FILL_TABLE_VHT40_LENGTH_V1, SINGLE_SKU_FILL_TABLE_VHT80_LENGTH_V1,
							SINGLE_SKU_FILL_TABLE_VHT160_LENGTH_V1,	SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1, SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1, SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1,
							SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1, SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1, SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1};

	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (u1PowerBoostRateType >= POWER_UP_CATE_V1_NUM)
		return FALSE;

	pcTxPwrBoost = &(pAd->CommonCfg.PowerBoostParamV1[ucBandIdx].cPowerUpCck[0]);

	for(i = 0; i <= u1PowerBoostRateType; i++) {
		u1Temp = u1Temp + u1Length[i];
	}

	pcTxPwrBoost = pcTxPwrBoost + u1Temp;

	if (arch_ops && arch_ops->arch_txpower_boost_info_V1) {
		if (!arch_ops->arch_txpower_boost_info_V1(pAd, u1PowerBoostRateType))
			return FALSE;

	for (ucRateIdx = 0; ucRateIdx < cap->single_sku_fill_tbl_length[u1PowerBoostRateType]; ucRateIdx++, pcTxPwrBoost++)
		MTWF_PRINT("(%2d) ", *pcTxPwrBoost);

	MTWF_PRINT("\n");
	MTWF_PRINT("-------------------------------------------------------\n");
	}
	return TRUE;
}

VOID mtf_txpower_boost_profile(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer)
{
	INT	i = 0, j = 0;
	CHAR	*value = 0, *value2 = 0;
	UINT8	ucPwrBoostReg[2] = {0};

	/* Power Boost (RU26) */
	if (RTMPGetKeyParameter("PowerUpRU26", tmpbuf, 32,
				pBuffer, TRUE)) {
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			RTMP_STRING *ptmpStr[DBDC_BAND_NUM];
			for (i = 0; i < DBDC_BAND_NUM; i++)
				ptmpStr[i] = NULL;

			/* parameter parsing (Phase I) */
			for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = value;

			/* sanity check for parameter parsing (Phase I) */
			if (i != DBDC_BAND_NUM)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_ERROR,
					"[PowerUpRU26] Input parameter incorrect!!\n");

			/* Band1 Parameter parsing (Phase II) */
			value = ptmpStr[0];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU26] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpRU26[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
			/* Band0 Parameter parsing (Phase II) */
			value = ptmpStr[1];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU26] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU26[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
		} else {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU26] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU26[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
		}
#else
		/* parameter parsing */
		for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
			for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
				ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
			if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU26] Input parameter incorrect!!\n");
			else
				pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU26[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
		}
#endif /* DBDC_MODE */
	}

	/* Power Boost (RU52) */
	if (RTMPGetKeyParameter("PowerUpRU52", tmpbuf, 32,
				pBuffer, TRUE)) {
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			RTMP_STRING *ptmpStr[DBDC_BAND_NUM];
			for (i = 0; i < DBDC_BAND_NUM; i++)
				ptmpStr[i] = NULL;

			/* parameter parsing (Phase I) */
			for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = value;

			/* sanity check for parameter parsing (Phase I) */
			if (i != DBDC_BAND_NUM)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_ERROR,
					"[PowerUpRU52] Input parameter incorrect!!\n");

			/* Band1 Parameter parsing (Phase II) */
			value = ptmpStr[0];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU52] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpRU52[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
			/* Band0 Parameter parsing (Phase II) */
			value = ptmpStr[1];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU52] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU52[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
		} else {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU52] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU52[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
		}
#else
		/* parameter parsing */
		for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
			for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
				ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
			if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU52] Input parameter incorrect!!\n");
			else
				pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU52[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
		}
#endif /* DBDC_MODE */
	}

	/* Power Boost (RU106) */
	if (RTMPGetKeyParameter("PowerUpRU106", tmpbuf, 32,
				pBuffer, TRUE)) {
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			RTMP_STRING *ptmpStr[DBDC_BAND_NUM];
			for (i = 0; i < DBDC_BAND_NUM; i++)
				ptmpStr[i] = NULL;

			/* parameter parsing (Phase I) */
			for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = value;

			/* sanity check for parameter parsing (Phase I) */
			if (i != DBDC_BAND_NUM)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_ERROR,
					"[PowerUpRU106] Input parameter incorrect!!\n");

			/* Band1 Parameter parsing (Phase II) */
			value = ptmpStr[0];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU106] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpRU106[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
			/* Band0 Parameter parsing (Phase II) */
			value = ptmpStr[1];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU106] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU106[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
		} else {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU106] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU106[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
		}
#else
		/* parameter parsing */
		for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
			for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
				ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
			if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU106] Input parameter incorrect!!\n");
			else
				pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU106[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
		}
#endif /* DBDC_MODE */
	}

	/* Power Boost (RU242) */
	if (RTMPGetKeyParameter("PowerUpRU242", tmpbuf, 32,
				pBuffer, TRUE)) {
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			RTMP_STRING *ptmpStr[DBDC_BAND_NUM];
			for (i = 0; i < DBDC_BAND_NUM; i++)
				ptmpStr[i] = NULL;

			/* parameter parsing (Phase I) */
			for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = value;

			/* sanity check for parameter parsing (Phase I) */
			if (i != DBDC_BAND_NUM)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_ERROR,
					"[PowerUpRU242] Input parameter incorrect!!\n");

			/* Band1 Parameter parsing (Phase II) */
			value = ptmpStr[0];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU242] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpRU242[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
			/* Band0 Parameter parsing (Phase II) */
			value = ptmpStr[1];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU242] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU242[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
		} else {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU242] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU242[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
		}
#else
		/* parameter parsing */
		for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
			for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
				ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
			if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU242] Input parameter incorrect!!\n");
			else
				pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU242[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
		}
#endif /* DBDC_MODE */
	}

	/* Power Boost (RU484) */
	if (RTMPGetKeyParameter("PowerUpRU484", tmpbuf, 32,
				pBuffer, TRUE)) {
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			RTMP_STRING *ptmpStr[DBDC_BAND_NUM];
			for (i = 0; i < DBDC_BAND_NUM; i++)
				ptmpStr[i] = NULL;

			/* parameter parsing (Phase I) */
			for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = value;

			/* sanity check for parameter parsing (Phase I) */
			if (i != DBDC_BAND_NUM)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_ERROR,
					"[PowerUpRU484] Input parameter incorrect!!\n");

			/* Band1 Parameter parsing (Phase II) */
			value = ptmpStr[0];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU484] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpRU484[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
			/* Band0 Parameter parsing (Phase II) */
			value = ptmpStr[1];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU484] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU484[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
		} else {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU484] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU484[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
		}
#else
		/* parameter parsing */
		for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
			for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
				ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
			if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU484] Input parameter incorrect!!\n");
			else
				pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU484[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
		}
#endif /* DBDC_MODE */
	}

	/* Power Boost (RU996) */
	if (RTMPGetKeyParameter("PowerUpRU996", tmpbuf, 32,
				pBuffer, TRUE)) {
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			RTMP_STRING *ptmpStr[DBDC_BAND_NUM];
			for (i = 0; i < DBDC_BAND_NUM; i++)
				ptmpStr[i] = NULL;

			/* parameter parsing (Phase I) */
			for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = value;

			/* sanity check for parameter parsing (Phase I) */
			if (i != DBDC_BAND_NUM)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_ERROR,
					"[PowerUpRU996] Input parameter incorrect!!\n");

			/* Band1 Parameter parsing (Phase II) */
			value = ptmpStr[0];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU996] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpRU996[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
			/* Band0 Parameter parsing (Phase II) */
			value = ptmpStr[1];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU996] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU996[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
		} else {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU996] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU996[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
		}
#else
		/* parameter parsing */
		for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
			for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
				ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
			if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU996] Input parameter incorrect!!\n");
			else
				pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU996[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
		}
#endif /* DBDC_MODE */
	}

	/* Power Boost (RU996X2) */
	if (RTMPGetKeyParameter("PowerUpRU996X2", tmpbuf, 32,
				pBuffer, TRUE)) {
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			RTMP_STRING *ptmpStr[DBDC_BAND_NUM];
			for (i = 0; i < DBDC_BAND_NUM; i++)
				ptmpStr[i] = NULL;

			/* parameter parsing (Phase I) */
			for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = value;

			/* sanity check for parameter parsing (Phase I) */
			if (i != DBDC_BAND_NUM)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
					DBG_LVL_ERROR,
					"[PowerUpRU996X2] Input parameter incorrect!!\n");

			/* Band1 Parameter parsing (Phase II) */
			value = ptmpStr[0];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU996X2] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpRU996X2[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
			/* Band0 Parameter parsing (Phase II) */
			value = ptmpStr[1];
			for (i = 0, value = rstrtok(value, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU996X2] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU996X2[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
			}
		} else {
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU996X2] Input parameter incorrect!!\n");
				else
					pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU996X2[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
		}
#else
		/* parameter parsing */
		for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1); value = rstrtok(NULL, "-"), i++) {
			for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
				ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
			if (ucPwrBoostReg[0] >= SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpRU996X2] Input parameter incorrect!!\n");
			else
				pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpRU996X2[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
		}
#endif /* DBDC_MODE */
	}
}
#endif

#ifdef SINGLE_SKU_V2
VOID mtf_txpower_sku_cfg_para(struct _RTMP_ADAPTER *pAd)
{
	UINT i;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pAd->fgPwrLimitRead[POWER_LIMIT_TABLE_TYPE_SKU] = FALSE;
	pAd->fgPwrLimitRead[POWER_LIMIT_TABLE_TYPE_BACKOFF] = FALSE;

	pAd->u1SkuParamTransOffset[0] = 0;
	pAd->u1SkuParamLen[0] = SINGLE_SKU_PARSE_TABLE_LENGTH[0];
	pAd->u1SkuChBandNeedParse[0] = TABLE_PARSE_G_A_BAND;
	for (i = 1; i < SINGLE_SKU_TYPE_PARSE_NUM_V1; i++) {
		pAd->u1SkuParamLen[i] = SINGLE_SKU_PARSE_TABLE_LENGTH[i];
		pAd->u1SkuParamTransOffset[i] = pAd->u1SkuParamTransOffset[i-1] + SINGLE_SKU_PARSE_TABLE_LENGTH[i-1];
		pAd->u1SkuChBandNeedParse[i] = TABLE_PARSE_G_A_BAND;
	}

	for (i = 0; i < SINGLE_SKU_TYPE_NUM_V1; i++)
		pAd->u1SkuFillParamLen[i] = cap->single_sku_fill_tbl_length[i];

	pAd->u1BackoffParamTransOffset[0] = 0;
	pAd->u1BackoffParamLen[0] = BACKOFF_TABLE_BF_LENGTH[0];
	pAd->u1BackoffChBandNeedParse[0] = TABLE_PARSE_G_A_BAND;
	for (i = 1; i < BACKOFF_TYPE_PARSE_NUM_V1; i++) {
		pAd->u1BackoffParamLen[i] = BACKOFF_TABLE_BF_LENGTH[i];
		pAd->u1BackoffParamTransOffset[i] = pAd->u1BackoffParamTransOffset[i-1] + BACKOFF_TABLE_BF_LENGTH[i-1];
		pAd->u1BackoffChBandNeedParse[i] = TABLE_PARSE_G_A_BAND;
	}

	for (i = 0; i < BACKOFF_TYPE_NUM_V1; i++)
		pAd->u1BackoffFillParamLen[i] = BACKOFF_FILL_TABLE_BF_LENGTH[i];
}
#endif
VOID mtf_txpower_show_info(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TXPOWER_INFO_V1_T prEventTxPowerInfo;

	/* get event info buffer contents */
	prEventTxPowerInfo = (P_EXT_EVENT_TXPOWER_INFO_V1_T)Data;

	MTWF_PRINT("=============================================================================\n");
	MTWF_PRINT(" 							 BASIC INFO\n");
	MTWF_PRINT("=============================================================================\n");
	MTWF_PRINT("  Band Index: %d,  Channel Band: %s\n",
			 prEventTxPowerInfo->u1BandIdx, (prEventTxPowerInfo->u1ChBand) ? ((prEventTxPowerInfo->u1ChBand == CH_6G_BAND)?("6G"):("5G")) : ("2G"));
	MTWF_PRINT("  PA Type: %s,  LNA Type: %s\n",
			 (prEventTxPowerInfo->fgPaType) ? ("ePA") : ("iPA"),
			 (prEventTxPowerInfo->fgLnaType) ? ("eLNA") : ("iLNA"));
	MTWF_PRINT("-----------------------------------------------------------------------------\n");
	MTWF_PRINT("  Sku: %s\n",
			 (prEventTxPowerInfo->fgSkuEnable) ? ("Enable") : ("Disable"));
	MTWF_PRINT("  Percentage: %s\n",
			 (prEventTxPowerInfo->fgPercentageEnable) ? ("Enable") : ("Disable"));
	MTWF_PRINT("  Power Drop: %d [dbm]\n",
			 prEventTxPowerInfo->cPowerDrop >> 1);
	MTWF_PRINT("  Backoff: %s\n",
			 (prEventTxPowerInfo->fgBackoffEnable) ? ("Enable") : ("Disable"));
	MTWF_PRINT("  FrontEnd Loss (Tx): %d, %d, %d, %d\n",
			 prEventTxPowerInfo->cFrondEndLossTx[WF0], prEventTxPowerInfo->cFrondEndLossTx[WF1],
			 prEventTxPowerInfo->cFrondEndLossTx[WF2], prEventTxPowerInfo->cFrondEndLossTx[WF3]);
	MTWF_PRINT("  FrontEnd Loss (Rx): %d, %d, %d, %d\n",
			 prEventTxPowerInfo->cFrondEndLossRx[WF0], prEventTxPowerInfo->cFrondEndLossRx[WF1],
			 prEventTxPowerInfo->cFrondEndLossRx[WF2], prEventTxPowerInfo->cFrondEndLossRx[WF3]);
	MTWF_PRINT("  Mu Tx Power Manual Mode: %d\n",
			 prEventTxPowerInfo->fgMuTxPwrManEn);
	MTWF_PRINT("  Mu Tx Power (Auto): %d, Mu Tx Power (Manual): %d\n",
			 prEventTxPowerInfo->cMuTxPwr, prEventTxPowerInfo->cMuTxPwrMan);
	MTWF_PRINT("  Thermal compensation: %s\n",
			 (prEventTxPowerInfo->fgThermalCompEnable) ? ("Enable") : ("Disable"));
	MTWF_PRINT("  Theraml compensation value: %d\n",
			 prEventTxPowerInfo->cThermalCompValue);
	MTWF_PRINT("=============================================================================\n");

}

VOID mtf_txpower_all_rate_info(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TXPOWER_ALL_RATE_POWER_INFO_T prEventTxPowerAllRateInfo;
	UINT8 u1TxPwrIdx, u1RateIdx;
	UINT8 u1TxPwrCckRate[MODULATION_SYSTEM_CCK_NUM] = {1, 2, 5, 11};
	UINT8 u1TxPwrOfdmRate[MODULATION_SYSTEM_OFDM_NUM] = {6, 9, 12, 18, 24, 36, 48, 54};
	UINT8 u1TxPwrHt20Rate[MODULATION_SYSTEM_HT20_NUM] = {0, 1, 2, 3, 4, 5, 6, 7};
	UINT8 u1TxPwrHt40Rate[MODULATION_SYSTEM_HT40_NUM] = {0, 1, 2, 3, 4, 5, 6, 7, 32};
#ifdef DATA_TXPWR_CTRL
	UINT8 u1TxPwrMax = 0, u1BwIdx = 0, u1McsIdx = 0;
#endif

	/* get event info buffer contents */
	prEventTxPowerAllRateInfo = (P_EXT_EVENT_TXPOWER_ALL_RATE_POWER_INFO_T)Data;

#ifdef MGMT_TXPWR_CTRL
		if (prEventTxPowerAllRateInfo->u1ChBand)
			pAd->ApCfg.MgmtTxPwr[prEventTxPowerAllRateInfo->u1BandIdx] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[TXPOWER_RATE_OFDM_OFFSET][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		else
			pAd->ApCfg.MgmtTxPwr[prEventTxPowerAllRateInfo->u1BandIdx] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[TXPOWER_RATE_CCK_OFFSET][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;

		pAd->ApCfg.EpaFeGain[prEventTxPowerAllRateInfo->u1BandIdx] = 0;

		MTWF_PRINT("[%s] band_idx:%d pwr:%d ChBand:%s ePAGain:%d\n",  __func__, prEventTxPowerAllRateInfo->u1BandIdx,
			pAd->ApCfg.MgmtTxPwr[prEventTxPowerAllRateInfo->u1BandIdx], (prEventTxPowerAllRateInfo->u1ChBand) ? ("5G") : ("2G"), prEventTxPowerAllRateInfo->u1EpaFeGain);
#endif
#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
		if (prEventTxPowerAllRateInfo->u1ChBand) {
			pAd->CommonCfg.ctrlTPC.pwr_ofdm6m =\
				prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[TXPOWER_RATE_OFDM_OFFSET][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
			pAd->CommonCfg.ctrlTPC.pwr_mcs11 =\
				prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[TXPOWER_RATE_VHT80_OFFSET + 11][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		}
#endif
#endif
#ifdef DATA_TXPWR_CTRL
	for (u1BwIdx = 0; u1BwIdx < DATA_TXPOWER_MAX_BW_NUM; u1BwIdx++) {
		for (u1McsIdx = 0; u1McsIdx < DATA_TXPOWER_MAX_MCS_NUM; u1McsIdx++) {
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][u1BwIdx][u1McsIdx] = 100;
		}
	}
#endif

#if defined(MGMT_TXPWR_CTRL) || (defined (TPC_SUPPORT) && defined (TPC_MODE_CTRL))
		if (g_fgCommand == FALSE)
			return;

		g_fgCommand = FALSE;
#endif

	MTWF_PRINT("=============================================================================\n");
	MTWF_PRINT("							  TX POWER INFO 							 \n");
	MTWF_PRINT("=============================================================================\n");
	MTWF_PRINT("  Band Index: %d,  Channel Band: %s ePAGain:%d\n",
		 prEventTxPowerAllRateInfo->u1BandIdx, (prEventTxPowerAllRateInfo->u1ChBand) ? ("5G") : ("2G"), prEventTxPowerAllRateInfo->u1EpaFeGain);
	MTWF_PRINT("-----------------------------------------------------------------------------\n");

	/* CCK */
	for (u1TxPwrIdx = TXPOWER_RATE_CCK_OFFSET; u1TxPwrIdx < TXPOWER_RATE_OFDM_OFFSET; u1TxPwrIdx++) {
		MTWF_PRINT("  [CCK_%02dM]: 0x%02x (%03d)\n",
		u1TxPwrCckRate[u1TxPwrIdx - TXPOWER_RATE_CCK_OFFSET],
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* OFDM */
	for (u1TxPwrIdx = TXPOWER_RATE_OFDM_OFFSET; u1TxPwrIdx < TXPOWER_RATE_HT20_OFFSET; u1TxPwrIdx++) {
		MTWF_PRINT("  [OFDM_%02dM]: 0x%02x (%03d)\n",
		u1TxPwrOfdmRate[u1TxPwrIdx - TXPOWER_RATE_OFDM_OFFSET],
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	MTWF_PRINT("-----------------------------------------------------------------------------\n");

	/* HT20 */
	for (u1TxPwrIdx = TXPOWER_RATE_HT20_OFFSET; u1TxPwrIdx < TXPOWER_RATE_HT40_OFFSET; u1TxPwrIdx++) {
		MTWF_PRINT("  [HT20_M%02d]: 0x%02x (%03d)\n",
		u1TxPwrHt20Rate[u1TxPwrIdx - TXPOWER_RATE_HT20_OFFSET],
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		if (g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][0][u1TxPwrIdx - TXPOWER_RATE_HT20_OFFSET] > prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][0][u1TxPwrIdx - TXPOWER_RATE_HT20_OFFSET] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* HT40 */
	for (u1TxPwrIdx = TXPOWER_RATE_HT40_OFFSET; u1TxPwrIdx < TXPOWER_RATE_VHT20_OFFSET; u1TxPwrIdx++) {
		MTWF_PRINT("  [HT40_M%02d]: 0x%02x (%03d)\n",
		u1TxPwrHt40Rate[u1TxPwrIdx - TXPOWER_RATE_HT40_OFFSET],
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		if (g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][1][u1TxPwrIdx - TXPOWER_RATE_HT40_OFFSET] > prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][1][u1TxPwrIdx - TXPOWER_RATE_HT40_OFFSET] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	MTWF_PRINT("-----------------------------------------------------------------------------\n");

	/* VHT20 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_VHT20_OFFSET; u1TxPwrIdx < TXPOWER_RATE_VHT40_OFFSET; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [VHT20_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		if (g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][0][u1TxPwrIdx - TXPOWER_RATE_VHT20_OFFSET] > prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][0][u1TxPwrIdx - TXPOWER_RATE_VHT20_OFFSET] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* VHT40 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_VHT40_OFFSET; u1TxPwrIdx < TXPOWER_RATE_VHT80_OFFSET; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [VHT40_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		if (g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][1][u1TxPwrIdx - TXPOWER_RATE_VHT40_OFFSET] > prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][1][u1TxPwrIdx - TXPOWER_RATE_VHT40_OFFSET] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* VHT80 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_VHT80_OFFSET; u1TxPwrIdx < TXPOWER_RATE_VHT160_OFFSET; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [VHT80_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		if (g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][2][u1TxPwrIdx - TXPOWER_RATE_VHT80_OFFSET] > prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][2][u1TxPwrIdx - TXPOWER_RATE_VHT80_OFFSET] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* VHT160 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_VHT160_OFFSET; u1TxPwrIdx < TXPOWER_RATE_VHT160_OFFSET + MODULATION_SYSTEM_VHT160_NUM; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [VHT160_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		if (g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][3][u1TxPwrIdx - TXPOWER_RATE_VHT160_OFFSET] > prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][3][u1TxPwrIdx - TXPOWER_RATE_VHT160_OFFSET] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	MTWF_PRINT("-----------------------------------------------------------------------------\n");

	/* HE26 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_HE26_OFFSET; u1TxPwrIdx < TXPOWER_RATE_HE52_OFFSET; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [HE26_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* HE52 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_HE52_OFFSET; u1TxPwrIdx < TXPOWER_RATE_HE106_OFFSET; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [HE52_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* HE106 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_HE106_OFFSET; u1TxPwrIdx < TXPOWER_RATE_HE242_OFFSET; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [HE106_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* HE242 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_HE242_OFFSET; u1TxPwrIdx < TXPOWER_RATE_HE484_OFFSET; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [HE242_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		if (g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][0][u1TxPwrIdx - TXPOWER_RATE_HE242_OFFSET] > prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][0][u1TxPwrIdx - TXPOWER_RATE_HE242_OFFSET] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* HE484 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_HE484_OFFSET; u1TxPwrIdx < TXPOWER_RATE_HE996_OFFSET; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [HE484_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		if (g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][1][u1TxPwrIdx - TXPOWER_RATE_HE484_OFFSET] > prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][1][u1TxPwrIdx - TXPOWER_RATE_HE484_OFFSET] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* HE996 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_HE996_OFFSET; u1TxPwrIdx < TXPOWER_RATE_HE996X2_OFFSET; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [HE996_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		if (g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][2][u1TxPwrIdx - TXPOWER_RATE_HE996_OFFSET] > prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][2][u1TxPwrIdx - TXPOWER_RATE_HE996_OFFSET] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

	/* HE996X2 */
	for (u1RateIdx = 0, u1TxPwrIdx = TXPOWER_RATE_HE996X2_OFFSET; u1TxPwrIdx < TXPOWER_RATE_NUM; u1TxPwrIdx++, u1RateIdx++) {
		MTWF_PRINT("  [HE996X2_M%02d]: 0x%02x (%03d)\n",
		u1RateIdx,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm,
		prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm);
#ifdef DATA_TXPWR_CTRL
		if (u1TxPwrMax < prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			u1TxPwrMax = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
		if (g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][3][u1TxPwrIdx - TXPOWER_RATE_HE996X2_OFFSET] > prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm)
			g_u1MinPwrlimitperRate[prEventTxPowerAllRateInfo->u1BandIdx][3][u1TxPwrIdx - TXPOWER_RATE_HE996X2_OFFSET] = prEventTxPowerAllRateInfo->rRatePowerInfo.ai1FramePowerConfig[u1TxPwrIdx][prEventTxPowerAllRateInfo->u1BandIdx].i1FramePowerDbm;
#endif
	}

#ifdef DATA_TXPWR_CTRL
	//get max base power (per band)
	pAd->ApCfg.MaxBaseTxPwr[prEventTxPowerAllRateInfo->u1BandIdx] = u1TxPwrMax;
	if (pAd->ApCfg.data_pwr_cmd_flag)
		RTMP_OS_COMPLETE(&pAd->ApCfg.get_tx_pwr_aync_done);
#endif

	MTWF_PRINT("-----------------------------------------------------------------------------\n");
	MTWF_PRINT("  [MAX][Bound]: 0x%02x (%03d)\n",
		prEventTxPowerAllRateInfo->i1PwrMaxBnd,
		prEventTxPowerAllRateInfo->i1PwrMaxBnd);
	MTWF_PRINT("  [MIN][Bound]: 0x%02x (%03d)\n",
		prEventTxPowerAllRateInfo->i1PwrMinBnd,
		prEventTxPowerAllRateInfo->i1PwrMinBnd);
	MTWF_PRINT("=============================================================================\n");
}

