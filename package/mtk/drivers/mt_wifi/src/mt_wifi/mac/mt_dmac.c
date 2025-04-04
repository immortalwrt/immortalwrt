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
#include "mac/mac_mt/dmac/mt_dmac.h"
#ifdef TXRX_STAT_SUPPORT
#include "hdev/hdev_basic.h"
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

VOID mtd_dump_rxinfo(RTMP_ADAPTER *pAd, UCHAR *rxinfo)
{
	RXINFO_STRUC *pRxInfo = (RXINFO_STRUC *) rxinfo;

	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(RXINFO_STRUC));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxInfo Fields:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBA=%d\n", pRxInfo->BA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDATA=%d\n", pRxInfo->DATA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tNULLDATA=%d\n", pRxInfo->NULLDATA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFRAG=%d\n", pRxInfo->FRAG));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tU2M=%d\n", pRxInfo->U2M));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMcast=%d\n", pRxInfo->Mcast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBcast=%d\n", pRxInfo->Bcast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMyBss=%d\n", pRxInfo->MyBss));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCrc=%d\n", pRxInfo->Crc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCipherErr=%d\n", pRxInfo->CipherErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMSDU=%d\n", pRxInfo->AMSDU));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHTC=%d\n", pRxInfo->HTC));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRSSI=%d\n", pRxInfo->RSSI));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tL2PAD=%d\n", pRxInfo->L2PAD));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMPDU=%d\n", pRxInfo->AMPDU));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDecrypted=%d\n", pRxInfo->Decrypted));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBssIdx3=%d\n", pRxInfo->BssIdx3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twapi_kidx=%d\n", pRxInfo->wapi_kidx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpn_len=%d\n", pRxInfo->pn_len));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_fc_type0=%d\n", pRxInfo->sw_fc_type0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_fc_type1=%d\n", pRxInfo->sw_fc_type1));
}

static char *p_idx_str[] = {"LMAC", "MCU"};
static char *q_idx_lmac_str[] = {"WMM0_AC0", "WMM0_AC1", "WMM0_AC2", "WMM0_AC3",
								 "WMM1_AC0", "WMM1_AC1", "WMM1_AC2", "WMM1_AC3",
								 "WMM2_AC0", "WMM2_AC1", "WMM2_AC2", "WMM2_AC3",
								 "WMM3_AC0", "WMM3_AC1", "WMM3_AC2", "WMM3_AC3",
								 "Band0_ALTX", "Band0_BMC", "Band0_BNC", "Band0_PSMP",
								 "Band1_ALTX", "Band1_BMC", "Band1_BNC", "Band1_PSMP",
								 "Invalid"
								};
static char *q_idx_mcu_str[] = {"RQ0", "RQ1", "RQ2", "RQ3", "PDA", "Invalid"};

VOID mtd_dump_tmac_info(RTMP_ADAPTER *pAd, UCHAR *tmac_info)
{
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)tmac_info;
	TMAC_TXD_0 *txd_0 = (TMAC_TXD_0 *)tmac_info;
	TMAC_TXD_1 *txd_1 = (TMAC_TXD_1 *)(tmac_info + sizeof(TMAC_TXD_0));
	UINT32 *txd_7 = &txd_s->TxD7;
	UCHAR q_idx = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	hex_dump("TMAC_Info Raw Data: ", (UCHAR *)tmac_info, cap->TXWISize);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_TXD Fields:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_0:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPortID=%d(%s)\n",
			txd_0->p_idx, p_idx_str[txd_0->p_idx]));

	if (txd_0->p_idx == P_IDX_LMAC)
		q_idx = txd_0->q_idx % 0x18;
	else
		q_idx = ((txd_0->q_idx == TxQ_IDX_MCU_PDA) ? txd_0->q_idx : (txd_0->q_idx % 0x4));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tQueID=0x%x(%s %s)\n", txd_0->q_idx,
			 (txd_0->p_idx == P_IDX_LMAC ? "LMAC" : "MCU"),
			 txd_0->p_idx == P_IDX_LMAC ? q_idx_lmac_str[q_idx] : q_idx_mcu_str[q_idx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxByteCnt=%d\n", txd_0->TxByteCount));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_1:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\twlan_idx=%d\n", txd_1->wlan_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrFmt=%d(%s)\n",
			 txd_1->hdr_format, hdr_fmt_str[txd_1->hdr_format]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrInfo=0x%x\n", txd_1->hdr_info));

	switch (txd_1->hdr_format) {
	case TMI_HDR_FT_NON_80211:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMRD=%d, EOSP=%d, RMVL=%d, VLAN=%d, ETYP=%d\n",
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_MRD),
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_EOSP),
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_RMVL),
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_VLAN),
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_ETYP)));
		break;

	case TMI_HDR_FT_CMD:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tRsvd=0x%x\n", txd_1->hdr_info));
		break;

	case TMI_HDR_FT_NOR_80211:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tHeader Len=%d(WORD)\n",
				 txd_1->hdr_info & TMI_HDR_INFO_2_MASK_LEN));
		break;

	case TMI_HDR_FT_ENH_80211:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tEOSP=%d, AMS=%d\n",
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_3_BIT_EOSP),
				 txd_1->hdr_info & (1 << TMI_HDR_INFO_3_BIT_AMS)));
		break;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxDFormatType=%d(%s format)\n", txd_1->ft,
			 txd_1->ft == TMI_FT_LONG ? "Long - 8 DWORD" : "Short - 3 DWORD"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttxd_len=%d page(%d DW)\n",
			 txd_1->txd_len == 0 ? 1 : 2, (txd_1->txd_len + 1) * 16));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrPad=%d(Padding Mode: %s, padding bytes: %d)\n",
			 txd_1->hdr_pad, ((txd_1->hdr_pad & (TMI_HDR_PAD_MODE_TAIL << 1)) ? "tail" : "head"),
			 ((txd_1->hdr_pad & 0x1) ? 2 : 0)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tUNxV=%d\n", txd_1->UNxV));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tamsdu=%d\n", txd_1->amsdu));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTID=%d\n", txd_1->tid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpkt_ft=%d(%s)\n",
			 txd_1->pkt_ft, pkt_ft_str[txd_1->pkt_ft]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\town_mac=%d\n", txd_1->OwnMacAddr));

	if (txd_s->TxD1.ft == TMI_FT_LONG) {
		TMAC_TXD_L *txd_l = (TMAC_TXD_L *)tmac_info;
		TMAC_TXD_2 *txd_2 = &txd_l->TxD2;
		TMAC_TXD_3 *txd_3 = &txd_l->TxD3;
		TMAC_TXD_4 *txd_4 = &txd_l->TxD4;
		TMAC_TXD_5 *txd_5 = &txd_l->TxD5;
		TMAC_TXD_6 *txd_6 = &txd_l->TxD6;

		txd_7 = &txd_l->TxD7;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_2:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsub_type=%d\n", txd_2->sub_type));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfrm_type=%d\n", txd_2->frm_type));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNDP=%d\n", txd_2->ndp));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNDPA=%d\n", txd_2->ndpa));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSounding=%d\n", txd_2->sounding));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRTS=%d\n", txd_2->rts));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbc_mc_pkt=%d\n", txd_2->bc_mc_pkt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBIP=%d\n", txd_2->bip));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tDuration=%d\n", txd_2->duration));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHE(HTC Exist)=%d\n", txd_2->htc_vld));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFRAG=%d\n", txd_2->frag));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tReamingLife/MaxTx time=%d\n", txd_2->max_tx_time));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpwr_offset=%d\n", txd_2->pwr_offset));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tba_disable=%d\n", txd_2->ba_disable));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttiming_measure=%d\n", txd_2->timing_measure));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfix_rate=%d\n", txd_2->fix_rate));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_3:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNoAck=%d\n", txd_3->no_ack));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPF=%d\n", txd_3->protect_frm));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_cnt=%d\n", txd_3->tx_cnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tremain_tx_cnt=%d\n", txd_3->remain_tx_cnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsn=%d\n", txd_3->sn));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_vld=%d\n", txd_3->pn_vld));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsn_vld=%d\n", txd_3->sn_vld));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_4:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_low=0x%x\n", txd_4->pn_low));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_5:\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_2_host=%d\n", txd_5->tx_status_2_host));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_2_mcu=%d\n", txd_5->tx_status_2_mcu));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_fmt=%d\n", txd_5->tx_status_fmt));

		if (txd_5->tx_status_2_host || txd_5->tx_status_2_mcu)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpid=%d\n", txd_5->pid));

		if (txd_2->fix_rate)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tda_select=%d\n", txd_5->da_select));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpwr_mgmt=0x%x\n", txd_5->pwr_mgmt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_high=0x%x\n", txd_5->pn_high));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_6:\n"));

		if (txd_2->fix_rate) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfix_rate_mode=%d\n", txd_6->fix_rate_mode));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGI=%d(%s)\n", txd_6->gi, txd_6->gi == 0 ? "LONG" : "SHORT"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tldpc=%d(%s)\n", txd_6->ldpc, txd_6->ldpc == 0 ? "BCC" : "LDPC"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxBF=%d\n", txd_6->TxBF));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_rate=0x%x\n", txd_6->tx_rate));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tant_id=%d\n", txd_6->ant_id));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tdyn_bw=%d\n", txd_6->dyn_bw));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbw=%d\n", txd_6->bw));
		}
	}

	if (!IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		UINT32 i;
		MAC_TX_PKT_T *txp = (MAC_TX_PKT_T *)(tmac_info + cap->tx_hw_hdr_len);

		for (i = 0; i < NUM_OF_MSDU_ID_IN_TXD; i++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tID:%u, token id: %u\n", i, txp->au2MsduId[i]));

		}

		for (i = 0; i < (TXD_MAX_BUF_NUM / 2); i++) {
			TXD_PTR_LEN_T *txd_ptr_len = &txp->arPtrLen[i];

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tID:%u, u4Ptr0 = 0x%x, u2Len0 = %u, u4Ptr1 = 0x%x,\
				u2Len1 = %u\n", i, txd_ptr_len->u4Ptr0, txd_ptr_len->u2Len0, txd_ptr_len->u4Ptr1, txd_ptr_len->u2Len1));
		}
	}
}

static VOID mtd_dump_rmac_info_normal(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;

	hex_dump("RMAC_Info Raw Data: ", rmac_info, sizeof(RXD_BASE_STRUCT));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxData_BASE:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPktType=%d(%s)\n",
			 rxd_base->RxD0.PktType,
			 rxd_pkt_type_str(rxd_base->RxD0.PktType)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGroupValid=%x\n", rxd_base->RxD0.RfbGroupVld));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRxByteCnt=%d\n", rxd_base->RxD0.RxByteCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tIP/UT=%d/%d\n", rxd_base->RxD0.IpChkSumOffload, rxd_base->RxD0.UdpTcpChkSumOffload));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEtherTypeOffset=%d(WORD)\n", rxd_base->RxD0.EthTypeOffset));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHTC/UC2ME/MC/BC=%d/%d/%d/%d\n",
			 rxd_base->RxD1.HTC,
			 (rxd_base->RxD1.a1_type == 0x1 ? 1 : 0),
			 (rxd_base->RxD1.a1_type == 0x2 ? 1 : 0),
			 rxd_base->RxD1.a1_type == 0x3 ? 1 : 0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBeacon with BMCast/Ucast=%d/%d\n",
			 rxd_base->RxD1.BcnWithBMcst, rxd_base->RxD1.BcnWithUCast));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tKeyID=%d\n", rxd_base->RxD1.KeyId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tChFrequency=%x\n", rxd_base->RxD1.ChFreq));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdearLength(MAC)=%d\n", rxd_base->RxD1.MacHdrLen));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHeaerOffset(HO)=%d\n", rxd_base->RxD1.HdrOffset));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHeaerTrans(H)=%d\n", rxd_base->RxD1.HdrTranslation));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPayloadFormat(PF)=%d\n", rxd_base->RxD1.PayloadFmt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBSSID=%d\n", rxd_base->RxD1.RxDBssidIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWlanIndex=%d\n", rxd_base->RxD2.RxDWlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTID=%d\n", rxd_base->RxD2.RxDTid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSEC Mode=%d\n", rxd_base->RxD2.SecMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSW BIT=%d\n", rxd_base->RxD2.SwBit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFCE Error(FC)=%d\n", rxd_base->RxD2.FcsErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCipher Mismatch(CM)=%d\n", rxd_base->RxD2.CipherMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCipher Length Mismatch(CLM)=%d\n", rxd_base->RxD2.CipherLenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tICV Err(I)=%d\n", rxd_base->RxD2.IcvErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTKIP MIC Err(T)=%d\n", rxd_base->RxD2.TkipMicErr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tLength Mismatch(LM)=%d\n", rxd_base->RxD2.LenMis));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tDeAMSDU Fail(DAF)=%d\n", rxd_base->RxD2.DeAmsduFail));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tExceedMax Rx Length(EL)=%d\n", rxd_base->RxD2.ExMaxRxLen));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrTransFail(HTF)=%d\n", rxd_base->RxD2.HdrTransFail));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tInterested Frame(INTF)=%d\n", rxd_base->RxD2.InterestedFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFragment Frame(FRAG)=%d\n", rxd_base->RxD2.FragFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNull Frame(NULL)=%d\n", rxd_base->RxD2.NullFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon Data Frame(NDATA)=%d\n", rxd_base->RxD2.NonDataFrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon-AMPDU Subframe(NASF)=%d\n", rxd_base->RxD2.NonAmpduSfrm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon AMPDU(NAMP)=%d\n", rxd_base->RxD2.NonAmpduFrm));
}

VOID mtd_dump_rmac_info_for_ICVERR(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;
	int LogDbgLvl = DBG_LVL_WARN;

	if (!IS_HIF_TYPE(pAd, HIF_MT))
		return;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);

	if (pkt_type != RMAC_RX_PKT_TYPE_RX_NORMAL)
		return;


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ("\tHTC/UC2ME/MC/BC=%d/%d/%d/%d",
				rxd_base->RxD1.HTC,
				(rxd_base->RxD1.a1_type == 0x1 ? 1 : 0),
				(rxd_base->RxD1.a1_type == 0x2 ? 1 : 0),
				rxd_base->RxD1.a1_type == 0x3 ? 1 : 0));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", WlanIndex=%d", rxd_base->RxD2.RxDWlanIdx);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", SEC Mode=%d\n", rxd_base->RxD2.SecMode);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, "\tFCE Error(FC)=%d", rxd_base->RxD2.FcsErr);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", CM=%d", rxd_base->RxD2.CipherMis);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", CLM=%d", rxd_base->RxD2.CipherLenMis);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", I=%d", rxd_base->RxD2.IcvErr);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", T=%d", rxd_base->RxD2.TkipMicErr);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, ", LM=%d\n", rxd_base->RxD2.LenMis);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, LogDbgLvl, "\tFragment Frame(FRAG)=%d\n", rxd_base->RxD2.FragFrm);

}


static VOID dump_rmac_info_txs(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* TXS_FRM_STRUC *txs_frm = (TXS_FRM_STRUC *)rmac_info; */
}


static VOID dump_rmac_info_rxv(RTMP_ADAPTER *pAd, UCHAR *Data)
{
#if RXV_REFINE_OK
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TA_0_31=%d\n", DW0->TA_0_31));

	DW1 = (RXV_DWORD1 *)(Data + 4);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TA_32_47=%d", DW1->TA_32_47));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxvSn=%d", DW1->RxvSn));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TR=%d\n", DW1->TR));


	RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TxRate=%d", RXV1_1ST_CYCLE->TxRate));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtStbc=%d", RXV1_1ST_CYCLE->HtStbc));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtAdCode=%d", RXV1_1ST_CYCLE->HtAdCode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtExtltf=%d", RXV1_1ST_CYCLE->HtExtltf));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", TxMode=%d", RXV1_1ST_CYCLE->TxMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FrMode=%d\n", RXV1_1ST_CYCLE->FrMode));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHTA1_B22=%d", RXV1_1ST_CYCLE->VHTA1_B22));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtAggregation=%d", RXV1_1ST_CYCLE->HtAggregation));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtShortGi=%d", RXV1_1ST_CYCLE->HtShortGi));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtSmooth=%d", RXV1_1ST_CYCLE->HtSmooth));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtNoSound=%d", RXV1_1ST_CYCLE->HtNoSound));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", NumRx=%d\n", RXV1_1ST_CYCLE->NumRx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHTA2_B8_B3=%d", RXV1_1ST_CYCLE->VHTA2_B8_B3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", ACID_DET_LOWER=%d", RXV1_1ST_CYCLE->ACID_DET_LOWER));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", ACID_DET_UPPER=%d\n", RXV1_1ST_CYCLE->ACID_DET_UPPER));

	RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Length=%d", RXV1_2ND_CYCLE->Length));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", GroupId=%d", RXV1_2ND_CYCLE->GroupId));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", NstsField=%d", RXV1_2ND_CYCLE->NstsField));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxValidIndicator=%d", RXV1_2ND_CYCLE->RxValidIndicator));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", SelAnt=%d\n", RXV1_2ND_CYCLE->SelAnt));

	RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(Data + 16);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHTA1_B21_B10=%d", RXV1_3TH_CYCLE->VHTA1_B21_B10));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", BFAgcApply=%d", RXV1_3TH_CYCLE->BFAgcApply));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", POPEverTrig=%d", RXV1_3TH_CYCLE->POPEverTrig));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FgacCalLnaRx=%d", RXV1_3TH_CYCLE->FgacCalLnaRx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", IBRssiRx=%d", RXV1_3TH_CYCLE->IBRssiRx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", WBRssiRx=%d\n", RXV1_3TH_CYCLE->WBRssiRx));

	RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 20);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI0=%d", RXV1_4TH_CYCLE->RCPI0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI1=%d", RXV1_4TH_CYCLE->RCPI1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI2=%d", RXV1_4TH_CYCLE->RCPI2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RCPI3=%d\n", RXV1_4TH_CYCLE->RCPI3));

	RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(Data + 24);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FagcLnaGainx=%d", RXV1_5TH_CYCLE->FagcLnaGainx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FagcLpfGainx=%d", RXV1_5TH_CYCLE->FagcLpfGainx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", NoiseBalanceEnable=%d", RXV1_5TH_CYCLE->NoiseBalanceEnable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", MISC1=%d\n", RXV1_5TH_CYCLE->MISC1));

	RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(Data + 28);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf0=%d", RXV1_6TH_CYCLE->Nf0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf1=%d", RXV1_6TH_CYCLE->Nf1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf2=%d", RXV1_6TH_CYCLE->Nf2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Nf3=%d\n", RXV1_6TH_CYCLE->Nf3));

	RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PrimItfrEnv=%d", RXV2_1ST_CYCLE->PrimItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", SecItfrEnv=%d", RXV2_1ST_CYCLE->SecItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Sec40ItfrEnv=%d", RXV2_1ST_CYCLE->Sec40ItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", Sec80ItfrEnv=%d", RXV2_1ST_CYCLE->Sec80ItfrEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxLQ=%d", RXV2_1ST_CYCLE->RxLQ));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", BtEnv=%d", RXV2_1ST_CYCLE->BtEnv));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", ScrambleSeed=%d\n", RXV2_1ST_CYCLE->ScrambleSeed));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", VHT_A2=%d", RXV2_1ST_CYCLE->VHT_A2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxCERmsd=%d", RXV2_1ST_CYCLE->RxCERmsd));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FCSErr=%d\n", RXV2_1ST_CYCLE->FCSErr));

	RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PostTMD=%d", RXV2_2ND_CYCLE->PostTMD));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", RxBW=%d", RXV2_2ND_CYCLE->RxBW));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PostBWDSecCh=%d", RXV2_2ND_CYCLE->PostBWDSecCh));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", PostDewSecCh=%d", RXV2_2ND_CYCLE->PostDewSecCh));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", HtSTFDet=%d", RXV2_2ND_CYCLE->HtSTFDet));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", CagcSTFDet=%d", RXV2_2ND_CYCLE->CagcSTFDet));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", IBRssi0=%d", RXV2_2ND_CYCLE->IBRssi0));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", WBRssi0=%d", RXV2_2ND_CYCLE->WBRssi0));

	RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);
#endif /*RXV_REFINE_OK*/
}


static VOID dump_rmac_info_rfb(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* RXD_BASE_STRUCT *rfb_frm = (RXD_BASE_STRUCT *)rmac_info; */
}


static VOID dump_rmac_info_tmr(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* TMR_FRM_STRUC *rxd_base = (TMR_FRM_STRUC *)rmac_info; */
}


VOID mtd_dump_rmac_info(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RMAC_RXD Header Format :%s\n",
			 rxd_pkt_type_str(pkt_type)));

	switch (pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_TXS:
		dump_rmac_info_txs(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXRXV:
		dump_rmac_info_rxv(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_NORMAL:
		mtd_dump_rmac_info_normal(pAd, rmac_info);
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


VOID mtd_dump_txs(struct _RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
	TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
	TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
	TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
	UINT32 tid = 0;

	/* TXS_D_4 *TxSD4 = &txs_entry->TxSD4; */
	/* TXS_D_5 *TxSD5 = &txs_entry->TxSD5; */
	/* TXS_D_6 *TxSD6 = &txs_entry->TxSD6; */

	tid = TxSD1->TxS_Tid;

	if (TxSD0->ME)
		tr_cnt->me[tid]++;

	if (TxSD0->RE)
		tr_cnt->re[tid]++;

	if (TxSD0->LE)
		tr_cnt->le[tid]++;

	if (TxSD0->BE)
		tr_cnt->be[tid]++;

	if (TxSD0->TxOp)
		tr_cnt->txop_limit_error[tid]++;

	if (TxSD0->BAFail)
		tr_cnt->baf[tid]++;

	if (Format == TXS_FORMAT0) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\tType=TimeStamp/FrontTime Mode\n");
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tTXSFM=%d, TXS2M/H=%d/%d, FixRate=%d, TxRate/BW=0x%x/%d\n",
				  TxSD0->TxSFmt, TxSD0->TxS2M, TxSD0->TxS2H,
				  TxSD0->TxS_FR, TxSD0->TxRate, TxSD1->TxS_TxBW);
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tME/RE/LE/BE/TxOPLimitErr/BA-Fail=%d/%d/%d/%d/%d/%d, PS=%d, Pid=%d\n",
				  TxSD0->ME, TxSD0->RE, TxSD0->LE, TxSD0->BE, TxSD0->TxOp,
				  TxSD0->BAFail, TxSD0->PSBit, TxSD0->TxS_PId);
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tTid=%d, AntId=%d, ETxBF/ITxBf=%d/%d\n",
				  TxSD1->TxS_Tid, TxSD1->TxS_AntId,
				  TxSD1->TxS_ETxBF, TxSD1->TxS_ITxBF);
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tTxPwrdBm=0x%x, FinalMPDU=0x%x, AMPDU=0x%x\n",
				  TxSD1->TxPwrdBm, TxSD1->TxS_FianlMPDU, TxSD1->TxS_AMPDU);
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tTxDelay=0x%x, RxVSeqNum=0x%x, Wlan Idx=0x%x\n",
				  TxSD2->TxS_TxDelay, TxSD2->TxS_RxVSN, TxSD2->TxS_WlanIdx);
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tSN=0x%x, MPDU TxCnt=%d, LastTxRateIdx=%d\n",
				  TxSD3->type_0.TxS_SN, TxSD3->type_0.TxS_MpduTxCnt,
				  TxSD3->type_0.TxS_LastTxRateIdx);
	} else if (Format == TXS_FORMAT1) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\tType=Noisy/RCPI Mode\n");
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tTXSFM=%d, TXS2M/H=%d/%d, FixRate=%d, TxRate/BW=0x%x/%d\n",
				  TxSD0->TxSFmt, TxSD0->TxS2M, TxSD0->TxS2H,
				  TxSD0->TxS_FR, TxSD0->TxRate, TxSD1->TxS_TxBW);
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tME/RE/LE/BE/TxOPLimitErr/BA-Fail=%d/%d/%d/%d/%d/%d, PS=%d, Pid=%d\n",
				  TxSD0->ME, TxSD0->RE, TxSD0->LE, TxSD0->BE, TxSD0->TxOp,
				  TxSD0->BAFail, TxSD0->PSBit, TxSD0->TxS_PId);
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tTid=%d, AntId=%d, ETxBF/ITxBf=%d/%d\n",
				  TxSD1->TxS_Tid, TxSD1->TxS_AntId,
				  TxSD1->TxS_ETxBF, TxSD1->TxS_ITxBF);
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tTxPwrdBm=0x%x, FinalMPDU=0x%x, AMPDU=0x%x\n",
				  TxSD1->TxPwrdBm, TxSD1->TxS_FianlMPDU, TxSD1->TxS_AMPDU);
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tTxDelay=0x%x, RxVSeqNum=0x%x, Wlan Idx=0x%x\n",
				  TxSD2->TxS_TxDelay, TxSD2->TxS_RxVSN, TxSD2->TxS_WlanIdx);
		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\t\tMPDU TxCnt=%d, LastTxRateIdx=%d\n",
				  TxSD3->type_1.TxS_MpduTxCnt,
				  TxSD3->type_1.TxS_LastTxRateIdx);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Unknown TxSFormat(%d)\n", Format);
	}
}

INT mtd_dump_dmac_amsdu_info(RTMP_ADAPTER *pAd)
{
#define MSDU_CNT_CR_NUMBER 8
	UINT32 msdu_cnt[MSDU_CNT_CR_NUMBER] = {0};
	UINT idx = 0;
	PMAC_TABLE_ENTRY pEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_1_MSDU_CNT, &msdu_cnt[0]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_2_MSDU_CNT, &msdu_cnt[1]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_3_MSDU_CNT, &msdu_cnt[2]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_4_MSDU_CNT, &msdu_cnt[3]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_5_MSDU_CNT, &msdu_cnt[4]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_6_MSDU_CNT, &msdu_cnt[5]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_7_MSDU_CNT, &msdu_cnt[6]);
	RTMP_IO_READ32(pAd->hdev_ctrl, AMSDU_PACK_8_MSDU_CNT, &msdu_cnt[7]);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=== HW_AMSDU INFO.===\n"));
	for (idx = 0; idx < MSDU_CNT_CR_NUMBER; idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PACK_%d_MSDU_CNT=%d\n", (idx+1), msdu_cnt[idx]));
	}
	for (idx = 0; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++) {
		pEntry = &pAd->MacTab.Content[idx];
		tr_entry = &tr_ctl->tr_entry[idx];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if ((pEntry->Sst == SST_ASSOC) &&
			(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) &&
			(pEntry->tx_amsdu_bitmap != 0)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Wcid%03d: %02x", idx, pEntry->tx_amsdu_bitmap));
		}
	}
	return TRUE;
}
#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(TXRX_STAT_SUPPORT)
VOID Reset_MIB_Update_Counters(RTMP_ADAPTER *pAd, UCHAR Idx)
{
	pAd->ChannelStats.MibUpdateOBSSAirtime[Idx] = 0;
	pAd->ChannelStats.MibUpdateMyTxAirtime[Idx] = 0;
	pAd->ChannelStats.MibUpdateMyRxAirtime[Idx] = 0;
}
#endif

VOID mtd_update_mib_bucket(RTMP_ADAPTER *pAd)
{
	UINT32  CrValue, PhyBandOffset = BandOffset;
	UCHAR   i = 0;
	UCHAR   CurrIdx = 0;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);
#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(TXRX_STAT_SUPPORT)
	SCAN_CTRL *ScanCtrl = NULL;
#endif
	pAd->MsMibBucket.CurIdx++;
	if (pAd->MsMibBucket.CurIdx >= 2)
		pAd->MsMibBucket.CurIdx = 0;
	CurrIdx = pAd->MsMibBucket.CurIdx;

	{
		pAd->MsMibBucket.RO_BAND0_PHYCTRL_STS0_OFFSET = 0;
		pAd->MsMibBucket.RO_BAND0_PHYCTRL_STS5_OFFSET = 0;
		pAd->MsMibBucket.PHY_BAND0_PHYMUX_5_OFFSET = 0;
	}

	for (i = 0; i < concurrent_bands; i++) {
		if (pAd->MsMibBucket.Enabled == TRUE) {
			/* Channel Busy Time */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR9 + (i * BandOffset), &CrValue);
			pAd->MsMibBucket.ChannelBusyTimeCcaNavTx[i][CurrIdx] = CrValue;
			/* Primary Channel Busy Time */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR16 + (i * BandOffset), &CrValue);
			pAd->MsMibBucket.ChannelBusyTime[i][CurrIdx] = CrValue;
#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(TXRX_STAT_SUPPORT)
			ScanCtrl = &pAd->ScanCtrl[i];
			if ((ScanCtrl->SyncFsm.CurrState == SYNC_FSM_IDLE) || (ScanCtrl->SyncFsm.CurrState == SYNC_FSM_PENDING)) {
				pAd->MsMibBucket.OBSSAirtime[i][CurrIdx] = pAd->ChannelStats.MibUpdateOBSSAirtime[i];
				pAd->MsMibBucket.MyTxAirtime[i][CurrIdx] = pAd->ChannelStats.MibUpdateMyTxAirtime[i];
				pAd->MsMibBucket.MyRxAirtime[i][CurrIdx] = pAd->ChannelStats.MibUpdateMyRxAirtime[i];
				Reset_MIB_Update_Counters(pAd, i);
		   } else
#endif
			{
				/* OBSS Air time */
				HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME5 + i * 4, &CrValue);
				pAd->MsMibBucket.OBSSAirtime[i][CurrIdx] = CrValue;
				/* My Tx Air time */
				HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR36 + (i * BandOffset), &CrValue);
				pAd->MsMibBucket.MyTxAirtime[i][CurrIdx] = CrValue;
				/* My Rx Air time */
				HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR37 + (i * BandOffset), &CrValue);
				pAd->MsMibBucket.MyRxAirtime[i][CurrIdx] = CrValue;
			}
			/* EDCCA time */
			HW_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR18 + (i * BandOffset), &CrValue);
			pAd->MsMibBucket.EDCCAtime[i][CurrIdx] = CrValue;

			HW_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS0 + pAd->MsMibBucket.RO_BAND0_PHYCTRL_STS0_OFFSET + (i * PhyBandOffset), &CrValue); /* PD count */
			pAd->MsMibBucket.PdCount[i][CurrIdx] = CrValue;
			HW_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS5 + pAd->MsMibBucket.RO_BAND0_PHYCTRL_STS5_OFFSET + (i * PhyBandOffset), &CrValue); /* MDRDY count */
			pAd->MsMibBucket.MdrdyCount[i][CurrIdx] = CrValue;

			HW_IO_READ32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5 + pAd->MsMibBucket.PHY_BAND0_PHYMUX_5_OFFSET + (i * PhyBandOffset), &CrValue);
			CrValue &= 0xff8fffff;
			HW_IO_WRITE32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5 + pAd->MsMibBucket.PHY_BAND0_PHYMUX_5_OFFSET + (i * PhyBandOffset), CrValue); /* Reset */
			CrValue |= 0x500000;
			HW_IO_WRITE32(pAd->hdev_ctrl, PHY_BAND0_PHYMUX_5 + pAd->MsMibBucket.PHY_BAND0_PHYMUX_5_OFFSET + (i * PhyBandOffset), CrValue); /* Enable */
		}
	}
	if (pAd->MsMibBucket.Enabled == TRUE) {
		/* Reset OBSS Air time */
		HW_IO_READ32(pAd->hdev_ctrl, RMAC_MIBTIME0, &CrValue);
		CrValue |= 1 << RX_MIBTIME_CLR_OFFSET;
		CrValue |= 1 << RX_MIBTIME_EN_OFFSET;
		HW_IO_WRITE32(pAd->hdev_ctrl, RMAC_MIBTIME0, CrValue);
	}
}

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
static VOID NICUpdateAmpduRawCounters(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	/* for PER debug */
	UINT32 AmpduTxCount = 0;
	UINT32 AmpduTxSuccessCount = 0;
	COUNTER_802_11 *wlanCounter;
	UINT32 mac_val, ampdu_range_cnt[4] = {0, 0, 0, 0};
	UINT32 Offset = 0x200 * BandIdx;

	wlanCounter = &pAd->WlanCounters[BandIdx];
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR14 + Offset, &AmpduTxCount);
	AmpduTxCount &= 0xFFFFFF;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR15 + Offset, &AmpduTxSuccessCount);
	AmpduTxSuccessCount &= 0xFFFFFF;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0DR2 + Offset, &mac_val);
	ampdu_range_cnt[0] = mac_val & 0xffff;
	ampdu_range_cnt[1] =  (mac_val >> 16) & 0xffff;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0DR3 + Offset, &mac_val);
	ampdu_range_cnt[2] = mac_val & 0xffff;
	ampdu_range_cnt[3] =  (mac_val >> 16) & 0xffff;
#ifdef STATS_COUNT_SUPPORT
	wlanCounter->AmpduSuccessCount.u.LowPart += AmpduTxSuccessCount;
	wlanCounter->AmpduFailCount.u.LowPart += (AmpduTxCount - AmpduTxSuccessCount);
#endif /* STATS_COUNT_SUPPORT */
	wlanCounter->TxAggRange1Count.u.LowPart += ampdu_range_cnt[0];
	wlanCounter->TxAggRange2Count.u.LowPart += ampdu_range_cnt[1];
	wlanCounter->TxAggRange3Count.u.LowPart += ampdu_range_cnt[2];
	wlanCounter->TxAggRange4Count.u.LowPart += ampdu_range_cnt[3];

	/* update amsdu */
}

VOID mtd_update_raw_counters(RTMP_ADAPTER *pAd)
{
#ifdef STATS_COUNT_SUPPORT
	UINT32 OldValue;
#endif
#ifdef TXRX_STAT_SUPPORT
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
#endif
	UINT32 i;
	UINT32 rx_err_cnt, fcs_err_cnt, mdrdy_cnt = 0, fcs_err_cnt_band1 = 0, mdrdy_cnt_band1 = 0;
	/* UINT32 TxSuccessCount = 0, TxRetryCount = 0; */
	COUNTER_RALINK *pPrivCounters;
	COUNTER_802_11 *wlanCounter;
	COUNTER_802_3 *dot3Counters;
#ifdef ERR_RECOVERY
	if (IsStopingPdma(&pAd->ErrRecoveryCtl))
		return;
#endif /* ERR_RECOVERY */
	pPrivCounters = &pAd->RalinkCounters;
	wlanCounter = &pAd->WlanCounters[0];
	dot3Counters = &pAd->Counters8023;

	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR3, &rx_err_cnt);
	fcs_err_cnt = rx_err_cnt & 0xffff;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR4, &rx_err_cnt);
#ifdef TXRX_STAT_SUPPORT
#ifdef DBDC_MODE
#endif /* DBDC_MODE */
#endif
	if (pAd->parse_rxv_stat_enable) {
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR10, &mdrdy_cnt);
		mdrdy_cnt = (mdrdy_cnt & 0x3FFFFFF); /* [25:0] Mac Mdrdy*/

#ifdef DBDC_MODE
#endif /* DBDC_MODE */
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR3, &fcs_err_cnt_band1);
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR10, &mdrdy_cnt_band1);
	}
#ifdef TXRX_STAT_SUPPORT
	ctrl->rdev[DBDC_BAND0].pRadioCtrl->RxCRCErrorCount.QuadPart += fcs_err_cnt;
	if (pAd->CommonCfg.dbdc_mode)
		ctrl->rdev[DBDC_BAND1].pRadioCtrl->RxCRCErrorCount.QuadPart += fcs_err_cnt_band1;
#endif

	pPrivCounters->OneSecRxFcsErrCnt += fcs_err_cnt;

	if (pAd->parse_rxv_stat_enable) {
		pAd->AccuOneSecRxBand0FcsErrCnt += fcs_err_cnt; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand0MdrdyCnt += mdrdy_cnt; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand1FcsErrCnt += fcs_err_cnt_band1; /*Used for rx_statistic*/
		pAd->AccuOneSecRxBand1MdrdyCnt += mdrdy_cnt_band1; /*Used for rx_statistic*/
	}

#ifdef STATS_COUNT_SUPPORT
	/* Update FCS counters*/
	OldValue = pAd->WlanCounters[0].FCSErrorCount.u.LowPart;
	pAd->WlanCounters[0].FCSErrorCount.u.LowPart += fcs_err_cnt; /* >> 7);*/

	if (pAd->WlanCounters[0].FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters[0].FCSErrorCount.u.HighPart++;

#ifdef DBDC_MODE
#if (defined(MT7615) || defined(MT7626)) && defined(CONFIG_ATE)
	if (IS_ATE_DBDC(pAd)) {
		/* Update FCS counters of band1 */
		OldValue = pAd->WlanCounters[1].FCSErrorCount.u.LowPart;
		pAd->WlanCounters[1].FCSErrorCount.u.LowPart += fcs_err_cnt_band1;

		if (pAd->WlanCounters[1].FCSErrorCount.u.LowPart < OldValue)
			pAd->WlanCounters[1].FCSErrorCount.u.HighPart++;
	}
#endif /* defined(MT7615) || defined(MT7626) */
#endif /* DBDC_MODE */

#endif /* STATS_COUNT_SUPPORT */
	/* Add FCS error count to private counters*/
	pPrivCounters->OneSecRxFcsErrCnt += fcs_err_cnt;
	OldValue = pPrivCounters->RealFcsErrCount.u.LowPart;
	pPrivCounters->RealFcsErrCount.u.LowPart += fcs_err_cnt;

	if (pPrivCounters->RealFcsErrCount.u.LowPart < OldValue)
		pPrivCounters->RealFcsErrCount.u.HighPart++;

	dot3Counters->RxNoBuffer += (rx_err_cnt & 0xffff);

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
	((TMI_TX_RATE_MODE_CCK << TMI_TX_RATE_BIT_MODE) | (_mcs))

#define TMI_TX_RATE_OFDM_VAL(_mcs) \
	((TMI_TX_RATE_MODE_OFDM << TMI_TX_RATE_BIT_MODE) | (_mcs))

#define TMI_TX_RATE_HT_VAL(_mode, _mcs, _stbc) \
	(((_stbc) << TMI_TX_RATE_BIT_STBC) |\
	 ((_mode) << TMI_TX_RATE_BIT_MODE) | \
	 (_mcs))

#define TMI_TX_RATE_VHT_VAL(_nss, _mcs, _stbc) \
	(((_stbc) << TMI_TX_RATE_BIT_STBC) |\
	 (((_nss - 1) & (TMI_TX_RATE_MASK_NSS)) << TMI_TX_RATE_BIT_NSS) | \
	 (TMI_TX_RATE_MODE_VHT << TMI_TX_RATE_BIT_MODE) | \
	 (_mcs))


UINT16 mtd_tx_rate_to_tmi_rate(UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble)
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

		tmi_rate = (TMI_TX_RATE_MODE_CCK << TMI_TX_RATE_BIT_MODE) | (mcs_id);
		break;

	case MODE_OFDM:
		if (mcs < tmi_rate_map_ofdm_size) {
			mcs_id = tmi_rate_map_ofdm[mcs];
			tmi_rate = (TMI_TX_RATE_MODE_OFDM << TMI_TX_RATE_BIT_MODE) | (mcs_id);
		}

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		tmi_rate = ((USHORT)(stbc << TMI_TX_RATE_BIT_STBC)) |
				   (((nss - 1) & TMI_TX_RATE_MASK_NSS) << TMI_TX_RATE_BIT_NSS) |
				   ((USHORT)(mode << TMI_TX_RATE_BIT_MODE)) |
				   ((USHORT)(mcs));
		/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): mode=%d, mcs=%d, stbc=%d converted tmi_rate=0x%x\n", */
		/* __FUNCTION__, mode, mcs, stbc, tmi_rate)); */
		break;

	case MODE_VHT:
		tmi_rate = TMI_TX_RATE_VHT_VAL(nss, mcs, stbc);
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid mode(mode=%d)\n",
				 __func__, mode));
		break;
	}

	return tmi_rate;
}


UCHAR mtd_get_nsts_by_mcs(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss)
{
	UINT8 nsts = 1;

	switch (phy_mode) {
	case MODE_VHT:
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

static const  UCHAR dmac_wmm_aci_2_hw_ac_que[4][4] = {
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

const static UCHAR dmac_wmm_swq_2_hw_ac_que[4][4] = {
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

UINT32 mtd_get_hwq_from_ac(UINT8 wmm_idx, UINT8 wmm_ac)
{
	if (wmm_idx >= 4 || wmm_ac > QID_AC_VO)
		return TxQ_IDX_AC0;
	return dmac_wmm_aci_2_hw_ac_que[wmm_idx][wmm_ac];
}


static VOID MtWriteTMacInfo(RTMP_ADAPTER *pAd, UCHAR *buf, TMAC_INFO *TxInfo)
{
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)buf;
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_s->TxD0;
	TMAC_TXD_1 *txd_1 = &txd_s->TxD1;
	TMAC_TXD_2 *txd_2 = NULL;
	TMAC_TXD_3 *txd_3 = NULL;
	TMAC_TXD_5 *txd_5 = NULL;
	TMAC_TXD_6 *txd_6 = NULL;
	UINT32 *txd_7 = NULL;
	UCHAR txd_size;
	UCHAR stbc = 0, bw = BW_20, mcs = 0, nss = 1, sgi = 0, phy_mode = 0, preamble = 1, ldpc = 0, expBf = 0, impBf = 0, vht_nss = 1;
	TX_RADIO_SET_T *pTxRadioSet = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/*DWORD 0*/
	txd_0->p_idx = TxInfo->PortIdx;
	txd_0->q_idx = TxInfo->QueIdx;

	if ((TxInfo->QueIdx < 4) && (TxInfo->WmmSet < 4))
		txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[TxInfo->WmmSet][TxInfo->QueIdx];
	else {
		/* TODO: shiang-usw, consider about MCC case! */
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s: Non-WMM Q, TxInfo->WmmSet/QueIdx(%d/%d)!\n",
			__func__, TxInfo->WmmSet, TxInfo->QueIdx);
		txd_0->q_idx = TxInfo->QueIdx;
	}

#ifdef CONFIG_CSO_SUPPORT
	txd_0->UdpTcpChkSumOffload = 0;
	txd_0->IpChkSumOffload = 0;

	if (txd_0->IpChkSumOffload || txd_0->UdpTcpChkSumOffload)
		txd_0->EthTypeOffset = 8; /* 8 is (14+2)/2, it is  7615 setting just for 802.3 frame not for 802.11 */

#endif /* CONFIG_CSO_SUPPORT */
	/*DWORD 1*/
	txd_1->wlan_idx = TxInfo->Wcid;

	if (!IS_WCID_VALID(pAd, txd_1->wlan_idx)) {
		pAd->wrong_wlan_idx_num++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong wlan index = %d\n", txd_1->wlan_idx));
		dump_stack();
	}

	if (!TxInfo->NeedTrans) {
		txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
		TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, TxInfo->WifiHdrLen, 0, txd_1->hdr_info);

		/* TODO: depends on QoS to decide if need to padding */
		if (TxInfo->HdrPad)
			txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | TxInfo->HdrPad;
	} else {
		txd_1->hdr_format = TMI_HDR_FT_NON_80211;
		TMI_HDR_INFO_VAL(txd_1->hdr_format, TxInfo->MoreData, TxInfo->Eosp, 1, TxInfo->VlanFrame, TxInfo->EtherFrame,
						 TxInfo->WifiHdrLen, 0, txd_1->hdr_info);

		if (TxInfo->HdrPad)
			txd_1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | TxInfo->HdrPad;
	}

	txd_1->tid = TxInfo->UserPriority;
	txd_1->OwnMacAddr = TxInfo->OwnMacIdx;

	if (TxInfo->LongFmt == FALSE) {
		txd_1->ft = TMI_FT_SHORT;
		txd_size = sizeof(TMAC_TXD_S);
		txd_7 = &txd_s->TxD7;
	} else {
		txd_2 = &txd_l->TxD2;
		txd_3 = &txd_l->TxD3;
		txd_5 = &txd_l->TxD5;
		txd_6 = &txd_l->TxD6;
		txd_7 = &txd_l->TxD7;
		txd_1->ft = TMI_FT_LONG;
		txd_size = sizeof(TMAC_TXD_L);
		pTxRadioSet = &TxInfo->TxRadioSet;
		ldpc = pTxRadioSet->Ldpc;
		mcs = pTxRadioSet->RateCode;
		sgi = pTxRadioSet->ShortGI;
		stbc = pTxRadioSet->Stbc;
		phy_mode = pTxRadioSet->PhyMode;
		bw = pTxRadioSet->CurrentPerPktBW;
		expBf = pTxRadioSet->EtxBFEnable;
		impBf = pTxRadioSet->ItxBFEnable;
		vht_nss = TxInfo->VhtNss ? TxInfo->VhtNss : 1;
		nss = mtd_get_nsts_by_mcs(phy_mode, mcs, stbc, vht_nss);
		/*DW2*/
		txd_2->max_tx_time = TxInfo->MaxTxTime;
		txd_2->bc_mc_pkt = TxInfo->BmcPkt;
		txd_2->fix_rate = TxInfo->FixRate;
		txd_2->frm_type = TxInfo->FrmType;
		txd_2->sub_type = TxInfo->SubType;

		if (TxInfo->NeedTrans)
			txd_2->htc_vld = 0;

		txd_2->frag = TxInfo->FragIdx;
		txd_2->timing_measure = TxInfo->TimingMeasure;
		txd_2->ba_disable = TxInfo->BaDisable;
		txd_2->pwr_offset = TxInfo->PowerOffset;
		/*DW3*/
		txd_3->remain_tx_cnt = TxInfo->RemainTxCnt;
		txd_3->sn = TxInfo->Sn;
		txd_3->no_ack = (TxInfo->bAckRequired ? 0 : 1);
		txd_3->protect_frm = (TxInfo->CipherAlg != CIPHER_NONE) ? 1 : 0;
		/*DW5*/
		txd_5->pid = TxInfo->Pid;
		txd_5->tx_status_fmt = TxInfo->TxSFmt;
		txd_5->tx_status_2_host = TxInfo->TxS2Host;
		txd_5->tx_status_2_mcu = TxInfo->TxS2Mcu;

		if (TxInfo->NeedTrans)
			txd_5->da_select = TMI_DAS_FROM_MPDU;

		/* txd_5->BarSsnCtrl = TxInfo->BarSsnCtrl; */
		/* For  MT STA LP control, use H/W control mode for PM bit */
#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
		txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW;
#else
		txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
#endif /* CONFIG_STA_SUPPORT && CONFIG_PM_BIT_HW_MODE */

		/* DW6 */
		if (txd_2->fix_rate == 1) {
			txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
			/* TODO: MT7615 fix me! the AntPri is 8 bits, but currently hardware "ant_id" can support up to 12 bits!! */
			txd_6->ant_id = TxInfo->AntPri;
			/* TODO: MT7615, fix me! how to support SpeExtEnable in MT7615?? */
			/* txd_6->spe_en = TxInfo->SpeEn; */
			txd_6->bw = ((1 << 2) | bw);
			txd_6->dyn_bw = 0;
			txd_6->TxBF = ((impBf | expBf) ? 1 : 0);
			txd_6->ldpc = ldpc;
			txd_6->gi = sgi;

			if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD) {
				preamble = TxInfo->TxRadioSet.Premable;
				txd_6->tx_rate = mtd_tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
			}
		}
	}

	/* DWORD 7 */
	if (cap->txd_type == TXD_V1) {
		*txd_7 &= ~SPE_IDX_MASK;
		*txd_7 |= SPE_IDX(TxInfo->AntPri);
	} else {
		*txd_7 &= ~CON_SPE_IDX_MASK;
		*txd_7 |= CON_SPE_IDX(TxInfo->AntPri);
	}
	*txd_7 |= PP_REF_SUBTYPE(TxInfo->SubType);
	*txd_7 |= PP_REF_TYPE(TxInfo->FrmType);

	txd_0->TxByteCount = txd_size + TxInfo->PktLen;
}


#ifdef CONFIG_ATE
INT32 mt_ct_ate_hw_tx(RTMP_ADAPTER *pAd, TMAC_INFO *info, TX_BLK *tx_blk)
{
	UINT16 free_cnt = 1;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MtWriteTMacInfo(pAd, tx_blk->pSrcBufHeader, info);
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHeader_802_11, DIR_WRITE, FALSE);
	MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif
	tx_blk->pSrcBufData = tx_blk->pSrcBufHeader;
	NdisCopyMemory(&tx_blk->HeaderBuf[0], tx_blk->pSrcBufData, cap->tx_hw_hdr_len);
	tx_blk->pSrcBufData += cap->tx_hw_hdr_len;
	tx_blk->SrcBufLen -= cap->tx_hw_hdr_len;
	tx_blk->MpduHeaderLen = 0;
	tx_blk->wifi_hdr_len = 0;
	tx_blk->HdrPadLen = 0;
	tx_blk->hw_rsv_len = 0;
	/* Indicate which Tx ring to be sent by band_idx */
	ret = asic_write_txp_info(pAd, &tx_blk->HeaderBuf[cap->tx_hw_hdr_len], tx_blk);

	if (ret != NDIS_STATUS_SUCCESS)
		return ret;

	asic_write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	return NDIS_STATUS_SUCCESS;
}
#endif /*CONFIG_ATE*/

/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.

	Arguments:
		pTxWI		Pointer to head of each MPDU to HW.
		Ack		Setting for Ack requirement bit
		Fragment	Setting for Fragment bit
		RetryMode	Setting for retry mode
		Ifs		Setting for IFS gap
		Rate		Setting for transmit rate
		Service	Setting for service
		Length		Frame length
		TxPreamble	Short or Long preamble when using CCK rates
		QueIdx - 0-3, according to 802.11e/d4.4 June/2003

	Return Value:
		None

	See also : BASmartHardTransmit()    !!!

	========================================================================
*/

VOID mtd_write_tmac_info_fixed_rate(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *tmac_info,
	IN MAC_TX_INFO * info,
	IN HTTRANSMIT_SETTING * pTransmit)
{
	MAC_TABLE_ENTRY *mac_entry = NULL;
	UCHAR stbc, bw, mcs, nss = 1, sgi, phy_mode, ldpc = 0, preamble = LONG_PREAMBLE;
	UCHAR /*to_mcu = FALSE,*/ q_idx = info->q_idx;
	TMAC_TXD_L txd;
	TMAC_TXD_0 *txd_0 = &txd.TxD0;
	TMAC_TXD_1 *txd_1 = &txd.TxD1;
	TMAC_TXD_2 *txd_2 = &txd.TxD2;
	TMAC_TXD_3 *txd_3 = &txd.TxD3;
	TMAC_TXD_5 *txd_5 = &txd.TxD5;
	TMAC_TXD_6 *txd_6 = &txd.TxD6;
	UINT32 *txd_7 = &txd.TxD7;
	INT txd_size = sizeof(TMAC_TXD_S);
	STA_TR_ENTRY *tr_entry = NULL;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

#ifdef CONFIG_AP_SUPPORT
	struct wifi_dev *wdev = NULL;
#endif /* CONFIG_AP_SUPPORT */

	if (VALID_UCAST_ENTRY_WCID(pAd, info->WCID))
		mac_entry = &pAd->MacTab.Content[info->WCID];

	NdisZeroMemory(&txd, sizeof(TMAC_TXD_L));
	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	sgi = pTransmit->field.ShortGI;
	stbc = pTransmit->field.STBC;
	phy_mode = pTransmit->field.MODE;
	bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_ATE

	if (!ATE_ON(pAd))
#endif
	{
		if (mac_entry && !IS_ENTRY_NONE(mac_entry)) {
			UCHAR MaxMcs_1ss;
#ifdef DOT11_VHT_AC

			if (IS_VHT_STA(mac_entry))
				MaxMcs_1ss = 9;
			else
#endif /* DOT11_VHT_AC */
				MaxMcs_1ss = 7;

			if ((pAd->CommonCfg.bMIMOPSEnable) && (mac_entry->MmpsMode == MMPS_STATIC)
				&& (pTransmit->field.MODE >= MODE_HTMIX && pTransmit->field.MCS > MaxMcs_1ss))
				mcs = MaxMcs_1ss;
		}
	}

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT

	if (pAd->CommonCfg.VoPwrConstraintTest == TRUE) {
		info->AMPDU = 0;
		mcs = 0;
		ldpc = 0;
		bw = 0;
		sgi = 0;
		stbc = 0;
		phy_mode = MODE_OFDM;
	}

#endif /* DOT11K_RRM_SUPPORT */
	/* DWORD 0 */
	/* txd_0->p_idx = (to_mcu ? P_IDX_MCU : P_IDX_LMAC); */
	txd_0->p_idx = P_IDX_LMAC;/* to_mcu==FALSE*/
	if (q_idx < WMM_QUE_NUM)
		txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[info->wmm_set][q_idx];
	else
		txd_0->q_idx = q_idx;

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s(%d): TxBlk->wmm_set= %d, QID = %d\n",
			  __func__, __LINE__, info->wmm_set, txd_0->q_idx);
#ifdef CONFIG_CSO_SUPPORT
	/* Leonardo: checksumm offload is not necessary for management frame */
	txd_0->UdpTcpChkSumOffload = 0;
	txd_0->IpChkSumOffload = 0;
#endif /* CONFIG_CSO_SUPPORT */
	/* DWORD 1 */
	txd_1->wlan_idx = info->WCID;

	if (!IS_WCID_VALID(pAd, txd_1->wlan_idx)) {
		pAd->wrong_wlan_idx_num++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong wlan index = %d\n", txd_1->wlan_idx));
		dump_stack();
	}

	txd_1->ft = TMI_FT_LONG;
	txd_1->txd_len = 0;
	txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
	TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, info->hdr_len, 0, txd_1->hdr_info);

	if (info->hdr_pad)  /* TODO: depends on QoS to decide if need to padding */
		txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1;

	txd_1->tid = info->TID;

	if (info->IsOffloadPkt == TRUE)
		txd_1->pkt_ft = TMI_PKT_FT_MCU_FW;
	else
		txd_1->pkt_ft = TMI_PKT_FT_HIF_CT; /* cut-through */

	txd_1->UNxV = 0;	/* Chagne UNxV to TXDEXTLEN for non-usb */
	txd_1->OwnMacAddr = info->OmacIdx;

	/*
	  repeater entry doesn't have real wdev could bind.
	  so reference the OmacIdx which is stored in tr_entry.

	  other types entry could reference the OmacIdx of wdev which is connected to the pEntry.
	*/
	if (mac_entry && IS_ENTRY_REPEATER(mac_entry)) {
		tr_entry = &tr_ctl->tr_entry[mac_entry->wcid];
		txd_1->OwnMacAddr = tr_entry->OmacIdx;
	} else if (mac_entry && !IS_ENTRY_NONE(mac_entry) && !IS_ENTRY_MCAST(mac_entry))
		txd_1->OwnMacAddr = mac_entry->wdev->OmacIdx;

	if (txd_1->ft == TMI_FT_LONG) {
		txd_size = sizeof(TMAC_TXD_L);

		/* DWORD 2 */
		if (info->IsOffloadPkt == TRUE) {
			txd_2->sub_type = info->SubType;
			txd_2->frm_type = info->Type;
		}

		txd_2->ndp = 0;
		txd_2->ndpa = 0;
		txd_2->sounding = 0;
		txd_2->rts = 0;
		txd_2->bc_mc_pkt = info->BM;
		txd_2->bip = 0;
		txd_2->fix_rate = 1;

		if (info->sw_duration)
			txd_2->duration = 1;
		else
			txd_2->duration = 0;
		txd_2->htc_vld = 0;
		txd_2->frag = info->FRAG; /* 0: no frag, 1: 1st frag, 2: mid frag, 3: last frag */
		txd_2->max_tx_time = info->tx_lifetime;
		txd_2->pwr_offset = info->txpwr_offset;
		txd_2->ba_disable = 1;
		txd_2->timing_measure = 0;

		if (info->IsAutoRate)
			txd_2->fix_rate = 0;
		else
			txd_2->fix_rate = 1;

		if ((pAd->pTmrCtrlStruct != NULL)
			&& (pAd->pTmrCtrlStruct->TmrEnable == TMR_INITIATOR)) {
			if ((info->Ack == 1) && (txd_2->bc_mc_pkt == 0))
				txd_2->timing_measure = 1;
		}


		if ((pAd->pTmrCtrlStruct != NULL)
			&& (pAd->pTmrCtrlStruct->TmrEnable == TMR_INITIATOR))
		{
			if (info->IsTmr)
				txd_2->timing_measure = 1;
		}

		/* DWORD 3 */
		if (txd_0->q_idx == TxQ_IDX_BCN0 || txd_0->q_idx == TxQ_IDX_BCN1)
			txd_3->remain_tx_cnt = MT_TX_RETRY_UNLIMIT;
		else {
				txd_3->remain_tx_cnt = MT_TX_SHORT_RETRY;
		}

		txd_3->no_ack = (info->Ack ? 0 : 1);

		if (0 /* bar_sn_ctrl */)
			txd_3->sn_vld = 1;

		if (info->NSeq) {
			txd_3->sn_vld = 1;
			txd_3->sn = info->assigned_seq;
		}

		/* DWORD 4 */
		/* DWORD 5 */
#ifdef OCE_FILS_SUPPORT
		if ((info->Type == FC_TYPE_MGMT) &&
			(info->SubType == SUBTYPE_BEACON)) {
			txd_5->pid = info->PID;
			txd_5->tx_status_fmt = TXS_FORMAT0;
			txd_5->tx_status_2_mcu = 0;
			txd_5->tx_status_2_host = 1;
		}
#endif /* OCE_FILS_SUPPORT */

		if ((info->txs2m) || (info->txs2h)) {
			txd_5->pid = info->PID;
			txd_5->tx_status_fmt = TXS_FORMAT0;
			txd_5->tx_status_2_mcu = info->txs2m;
			txd_5->tx_status_2_host = info->txs2h;
		}


#ifdef AUTOMATION
		if (is_frame_test(pAd, 0) == 2) {
			if (info->Type == FC_TYPE_MGMT && info->SubType == SUBTYPE_BEACON)
				;
			else {
				txd_5->pid = pAd->auto_dvt->txs.pid;
				txd_5->tx_status_fmt = pAd->auto_dvt->txs.format;
				txd_5->tx_status_2_mcu = 0;
				txd_5->tx_status_2_host = 1;
				send_add_txs_queue(pAd->auto_dvt->txs.pid, info->WCID);
			}
		}
#endif /* AUTOMATION */
#ifdef HDR_TRANS_TX_SUPPORT
		/* txd_5->da_select = TMI_DAS_FROM_MPDU; */
#endif /* HDR_TRANS_TX_SUPPORT */
		/* TODO: shiang-MT7615, fix me! bar_sn_ctrl = Write SSN to SN field and set sn_vld bit to 1 */
		/* txd_5->bar_sn_ctrl = 1; */

		if (info->PsmBySw)
			txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
		else
			txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW;

		/* txd_5->pn_high = 0; */

		/* DWORD 6 */
		if (txd_2->fix_rate == 1) {
			txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
			txd_6->bw = ((1 << 2) | bw);
			txd_6->dyn_bw = 0;
			txd_6->ant_id = 0; /* Smart Antenna indicator */
			txd_6->TxBF = 0;
			txd_6->ldpc = ldpc;
			txd_6->gi = sgi;

			if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD) {
				if (phy_mode == MODE_CCK)
					preamble = info->Preamble;

				txd_6->tx_rate = mtd_tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
			}
		}


		if (info->prot == 1)
			txd_3->protect_frm = 1;
		else if (info->prot == 2 || info->prot == 3) {
#ifdef CONFIG_AP_SUPPORT

			if (mac_entry) {
				wdev = mac_entry->wdev;
				GET_GroupKey_WCID(wdev, info->WCID);
			}

#ifdef DOT11W_PMF_SUPPORT
			if (info->prot == 2)
				txd_2->bip = 1;
#endif /* DOT11W_PMF_SUPPORT */
#else
			{
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[info->WCID];

				info->WCID = pEntry->wdev->bss_info_argument.bmc_wlan_idx;
			}
#endif
			txd_1->wlan_idx = info->WCID;

			if (!IS_WCID_VALID(pAd, txd_1->wlan_idx)) {
				pAd->wrong_wlan_idx_num++;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong wlan index = %d\n", txd_1->wlan_idx));
				dump_stack();
			}
		} else
			txd_3->protect_frm = 0;
	}

	/* DWORD 7 */
	if (cap->txd_type == TXD_V1) {
		*txd_7 &= ~SPE_IDX_MASK;
		*txd_7 |= SPE_IDX(info->AntPri);

		*txd_7 &= ~PP_REF_SUBTYPE_MASK;
		*txd_7 |= PP_REF_SUBTYPE(info->SubType);

		*txd_7 &= ~PP_REF_TYPE_MASK;
		*txd_7 |= PP_REF_TYPE(info->Type);
	} else {
		*txd_7 &= ~CON_SPE_IDX_MASK;
		*txd_7 |= CON_SPE_IDX(info->AntPri);
		/*
		   align mtd_write_tmac_info_by_host() :
		   DW7's type & subtype to let PP reasign to DW2's type & subtype
		*/
		*txd_7 &= ~CON_PP_REF_SUBTYPE_MASK;
		*txd_7 |= CON_PP_REF_SUBTYPE(info->SubType);
		*txd_7 &= ~CON_PP_REF_TYPE_MASK;
		*txd_7 |= CON_PP_REF_TYPE(info->Type);
	}

	txd_0->TxByteCount = txd_size + info->Length;

	NdisMoveMemory(tmac_info, &txd, sizeof(TMAC_TXD_L));

#ifdef RED_SUPPORT_BY_HOST
	if (!pAd->red_mcu_offload)
		RedRecordForceRateFromDriver(pAd, info->WCID);
#endif
}

VOID mtd_write_tmac_info(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_MCU_OFFLOAD))
		mtd_write_tmac_info_by_cr4(pAd, buf, pTxBlk);
	else
		mtd_write_tmac_info_by_host(pAd, buf, pTxBlk);
}

VOID mtd_write_tmac_info_by_cr4(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_l->TxD0;
	TMAC_TXD_1 *txd_1 = &txd_l->TxD1;

	txd_0->p_idx = P_IDX_LMAC;
	txd_0->q_idx = 0;
	txd_0->TxByteCount = sizeof(TMAC_TXD_L) +
						 pTxBlk->MpduHeaderLen +
						 pTxBlk->HdrPadLen +
						 pTxBlk->SrcBufLen;
	txd_1->ft = TMI_FT_LONG;
	txd_1->txd_len = 0;
	txd_1->pkt_ft = TMI_PKT_FT_HIF_CT;
	txd_1->hdr_format = TMI_HDR_FT_NON_80211;
	TMI_HDR_INFO_VAL(TMI_HDR_FT_NON_80211, 0, 0, 0, 0, 0, 0, 0, txd_1->hdr_info);

	if (pTxBlk->HdrPadLen)
		txd_1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1;

}


VOID mtd_write_tmac_info_by_host(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	UCHAR stbc = 0, bw = BW_20, mcs = 0, nss = 1, sgi = 0, phy_mode = 0, preamble = 1, ldpc = 0;
	UINT16 wcid;
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)buf;
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_s->TxD0;
	TMAC_TXD_1 *txd_1 = &txd_s->TxD1;
	UINT32 *txd_7 = &txd_s->TxD7;
	STA_TR_ENTRY *tr_entry = NULL;
	TMAC_TXD_2 *txd_2 = &txd_l->TxD2;
	TMAC_TXD_3 *txd_3 = &txd_l->TxD3;
	TMAC_TXD_5 *txd_5 = &txd_l->TxD5;
	TMAC_TXD_6 *txd_6 = &txd_l->TxD6;
	HTTRANSMIT_SETTING *pTransmit = pTxBlk->pTransmit;
	struct wifi_dev *wdev = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	UCHAR band = 0, vow_dvt_apply = 0;
	static UCHAR ucPid;
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	wcid = pTxBlk->Wcid;
	wdev = pTxBlk->wdev;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (pAd->CommonCfg.dbdc_mode)
		band = WMODE_CAP_5G(wdev->PhyMode) ? 1 : 0;
	if (pAd->vow_dvt_en) {
		if ((!RTMP_GET_PACKET_MGMT_PKT(pTxBlk->pPacket)) &&
			(RTMP_GET_PACKET_TYPE(pTxBlk->pPacket) != TX_ALTX) &&
			(!RTMP_GET_PACKET_HIGH_PRIO(pTxBlk->pPacket))) {
			if ((wcid == pAd->vow_cloned_wtbl_from[band]) ||
				((wcid >= pAd->vow_cloned_wtbl_start[band]) &&
				(wcid <= pAd->vow_cloned_wtbl_max[band])))
				vow_dvt_apply = 1;
		}
	}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	NdisZeroMemory(txd_l, sizeof(TMAC_TXD_L));

	txd_0 = &txd_s->TxD0;
	txd_1 = &txd_s->TxD1;

	/* DWORD 0 */
	txd_0->p_idx = P_IDX_LMAC;
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (vow_dvt_apply) {
		pTxBlk->wmm_set = pAd->vow_sta_wmm[wcid];
		pTxBlk->QueIdx = pAd->vow_sta_ac[wcid];
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wmm set %d,\
				QueIdx %d\n", pTxBlk->wmm_set, pTxBlk->QueIdx);
	}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	if (pTxBlk->QueIdx < 4) {
		txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[pTxBlk->wmm_set][pTxBlk->QueIdx];
	} else {
		/* TODO: shiang-usw, consider about MCC case! */
		txd_0->q_idx = pTxBlk->QueIdx;
	}

#ifdef RANDOM_PKT_GEN
	random_write_qidx(pAd, buf, pTxBlk);
#endif

#if (defined(VOW_SUPPORT) && defined(VOW_DVT))
	if (vow_dvt_apply) {
		txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[pTxBlk->wmm_set][pTxBlk->QueIdx];

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\x1b[31m%s: q_idx %d, \
		resource_idx %d\x1b[m\n", __func__, txd_0->q_idx, pTxBlk->resource_idx);

	if (pTxBlk->wmm_set == 0) {/* the same band */
		if (pAd->vow_bcmc_en == 1)
			txd_0->q_idx = TxQ_IDX_BMC0;
		else if (pAd->vow_bcmc_en == 2)
			txd_0->q_idx = TxQ_IDX_BMC1;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%d): TxBlk->wmm_set= %d,\
		QID = %d\n", __LINE__, pTxBlk->wmm_set, txd_0->q_idx);
	}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	txd_0->TxByteCount = pTxBlk->tx_bytes_len;
	txd_1->wlan_idx = wcid;

	if (!IS_WCID_VALID(pAd, txd_1->wlan_idx)) {
		pAd->wrong_wlan_idx_num++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 2 wrong wlan index = %d (%d)\n", __func__, txd_1->wlan_idx, HcGetMaxStaNum(pAd)));
		dump_stack();
	}

	if (pTxBlk->TxFrameType == TX_AMSDU_FRAME)
		txd_1->amsdu = 1;

	txd_1->pkt_ft = TMI_PKT_FT_HIF_CT;


	if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
		txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
		TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, pTxBlk->wifi_hdr_len, 0, txd_1->hdr_info);

		if (pTxBlk->HdrPadLen)  /* TODO: depends on QoS to decide if need to padding */
			txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1;

	} else {
		BOOLEAN is_vlan = FALSE;
		BOOLEAN is_etype = ((RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket) <= 1500) ? 0 : 1);
#ifdef VLAN_SUPPORT
		BOOLEAN is_rmvl = (wdev->bVLAN_Tag) ? FALSE : TRUE;
#else
		BOOLEAN is_rmvl = TRUE;
#endif /*VLAN_SUPPORT*/
		if (is_rmvl)
			is_vlan = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) != 0) ? TRUE:FALSE;
#ifdef AUTOMATION
		if (pAd->fpga_ctl.txrx_dbg_type == 1) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: Eth type=0x%04X, is_vlan = %d, is_etype = %d, is_rmvl=%d\n",
				__func__, (UINT16) RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket),
				is_vlan, is_etype, is_rmvl));
		} else if (pAd->fpga_ctl.txrx_dbg_type == 5) {
			is_rmvl = 0; /* keep vlan */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: Eth type=0x%04X, is_vlan = %d, is_etype = %d, is_rmvl=%d\n",
				__func__, (UINT16) RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket),
				is_vlan, is_etype, is_rmvl));
		}
#endif /* AUTOMATION */
		txd_1->hdr_format = TMI_HDR_FT_NON_80211;
		TMI_HDR_INFO_VAL(txd_1->hdr_format,
						 TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData),
						 TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP),
						 is_rmvl,
						 is_vlan,
						 is_etype,
						 pTxBlk->wifi_hdr_len, 0, txd_1->hdr_info);

		if (pTxBlk->HdrPadLen)
			txd_1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1;
	}

	txd_1->tid = pTxBlk->UserPriority;

#ifdef CONFIG_CSO_SUPPORT
	if (IS_ASIC_CAP(pAd, fASIC_CAP_CSO)) {
		txd_0->UdpTcpChkSumOffload = 1;
		txd_0->IpChkSumOffload = 1;

		if (txd_0->IpChkSumOffload || txd_0->UdpTcpChkSumOffload) {
			if (txd_1->hdr_format == TMI_HDR_FT_NOR_80211)
				txd_0->EthTypeOffset = (pTxBlk->MpduHeaderLen + 2)>>1;
			else if (txd_1->hdr_format == TMI_HDR_FT_NON_80211)
				txd_0->EthTypeOffset = (LENGTH_802_3 + 2)>>1; /* 8 is (14+2)/2, it is	7615 setting just for 802.3 frame not for 802.11 */
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: txd_1->hdr_format is unexpected\n", __func__));
		}
	}
#endif /* CONFIG_CSO_SUPPORT */

	/*
	  repeater entry doesn't have real wdev could bind.
	  so reference the OmacIdx which is stored in tr_entry.
	  other types entry could reference the OmacIdx of wdev which is connected to the pEntry.
	*/
	if (pMacEntry && IS_ENTRY_REPEATER(pMacEntry)) {
		if (pTxBlk->tr_entry) {
			tr_entry = pTxBlk->tr_entry;
			txd_1->OwnMacAddr = tr_entry->OmacIdx;
		}
	} else if (pMacEntry) {
		txd_1->OwnMacAddr = wdev->OmacIdx;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (vow_dvt_apply) {
		if (pAd->vow_sta_mbss[pTxBlk->Wcid]) {
			if (wcid <= (WTBL_MAX_NUM(pAd) - MAX_MBSSID_NUM(pAd)))
				txd_1->OwnMacAddr = pAd->vow_sta_mbss[wcid]+16;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[31m%s: wcid %d, OM %d\x1b[m\n", __func__,
					pTxBlk->Wcid, txd_1->OwnMacAddr);
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\x1b[31m%s: wcid %d, OM %d\x1b[m\n", __func__, pTxBlk->Wcid,
			txd_1->OwnMacAddr);
	}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */
	} else {
		wdev = pTxBlk->wdev;
		txd_1->OwnMacAddr = wdev->OmacIdx;
	}

	txd_7 = &txd_l->TxD7;
	txd_1->ft = TMI_FT_LONG;

	if (pTransmit) {
		ldpc = pTransmit->field.ldpc;
		phy_mode = pTransmit->field.MODE;
		mcs = phy_mode == MODE_VHT ? pTransmit->field.MCS & 0xf : pTransmit->field.MCS;
		sgi = pTransmit->field.ShortGI;
		stbc = pTransmit->field.STBC;
		bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
		nss = mtd_get_nsts_by_mcs(phy_mode, mcs, stbc,
							  phy_mode == MODE_VHT ? ((pTransmit->field.MCS & (0x3 << 4)) >> 4) + 1 : 0);
	}

	txd_l->TxD2.max_tx_time = 0;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (vow_dvt_apply) {
		txd_l->TxD2.max_tx_time = pAd->vow_life_time;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"wcid %d, lifetime %d\n", wcid, txd_l->TxD2.max_tx_time);
	}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	txd_l->TxD2.bc_mc_pkt = (pTxBlk->TxFrameType == TX_MCAST_FRAME ? 1 : 0);

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_ForceRate)) {
		txd_l->TxD2.fix_rate = 1;
		if (pAd->CommonCfg.bSeOff != TRUE) {
			if (HcGetBandByWdev(pTxBlk->wdev) == BAND0) {
				if (cap->txd_type == TXD_V1) {
					*txd_7 &= ~SPE_IDX_MASK;
					*txd_7 |= SPE_IDX(BAND0_SPE_IDX);
				} else {
					*txd_7 &= ~CON_SPE_IDX_MASK;
					*txd_7 |= CON_SPE_IDX(BAND0_SPE_IDX);
				}
			} else if (HcGetBandByWdev(pTxBlk->wdev) == BAND1) {
				if (cap->txd_type == TXD_V1) {
					*txd_7 &= ~SPE_IDX_MASK;
					*txd_7 |= SPE_IDX(BAND1_SPE_IDX);
				} else {
					*txd_7 &= ~CON_SPE_IDX_MASK;
					*txd_7 |= CON_SPE_IDX(BAND1_SPE_IDX);
				}
			}
		}
	}

	txd_l->TxD2.frag = pTxBlk->FragIdx;
	txd_2->pwr_offset = 0;

#ifdef CONFIG_AP_SUPPORT
	if (txd_2->frm_type == FC_TYPE_MGMT)
		txd_2->pwr_offset = wdev->mgmt_txd_txpwr_offset;
#endif

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired)) {
		txd_3->no_ack = 0;
		txd_l->TxD3.remain_tx_cnt = MT_TX_LONG_RETRY;
	} else {
		txd_3->no_ack = 1;
		txd_l->TxD3.remain_tx_cnt = MT_TX_RETRY_UNLIMIT;
	}

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (vow_dvt_apply) {
		txd_3->no_ack = (pAd->vow_sta_ack[wcid] ? 1 : 0);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"wcid %d, no ack %d\n", wcid, txd_3->no_ack);
	}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

	if (cap->TmrEnable == 1) {
		if (txd_3->no_ack == 0)
			txd_l->TxD2.timing_measure = 1;
	}

	if (IS_CIPHER_NONE(pTxBlk->CipherAlg))
		txd_3->protect_frm = 0;
	else
		txd_3->protect_frm = 1;

	txd_l->TxD5.pid = pTxBlk->Pid;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
		if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_MCAST_CLONE))
			txd_5->da_select = TMI_DAS_FROM_MPDU;
		else
			txd_5->da_select = TMI_DAS_FROM_WTBL;

#if (defined(VOW_SUPPORT) && defined(VOW_DVT))
	if (vow_dvt_apply)
		txd_5->da_select = TMI_DAS_FROM_WTBL;
#endif /* (defined(VOW_SUPPORT) && defined(VOW_DVT)) */
	}

	if (0 /* bar_sn_ctrl */)
		txd_3->sn_vld = 1;

#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
	txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW;
#else
	txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
#endif

#if (defined(VOW_SUPPORT) && defined(VOW_DVT))
	if (vow_dvt_apply) {
		txd_l->TxD5.tx_status_fmt = TXS_FORMAT0;
		txd_l->TxD5.tx_status_2_mcu = 0;
		if (pAd->vow_txs_en)
			txd_l->TxD5.tx_status_2_host = 1;
		txd_l->TxD5.pid = ucPid++%255;
	}
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\x1b[32m%s: wcid %d, pid %d\x1b[m\n", __func__, wcid, txd_l->TxD5.pid);
#endif /* (defined(VOW_SUPPORT) && defined(VOW_DVT)) */
#ifdef AUTOMATION
	if (is_frame_test(pAd, 0) == 1) {
		txd_5->pid = pAd->auto_dvt->txs.pid;
		txd_5->tx_status_fmt = pAd->auto_dvt->txs.format;
		txd_5->tx_status_2_mcu = 0;
		txd_5->tx_status_2_host = 1;
		send_add_txs_queue(pAd->auto_dvt->txs.pid, wcid);
	}
#endif /* AUTOMATION */
	/* DWORD 6 */
	if (txd_2->fix_rate == 1) {
		txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
		txd_6->bw = ((1 << 2) | bw);
		txd_6->dyn_bw = 0;
		txd_6->TxBF = 0;
		txd_6->ldpc = ldpc;
		txd_6->gi = sgi;

#ifdef GN_MIXMODE_SUPPORT
		 if (pAd->CommonCfg.GNMixMode
			&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
				|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
				|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))) {
			phy_mode = MODE_OFDM;
			mcs = 0;
		}
#endif /*GN_MIXMODE_SUPPORT*/
		if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD) {
			if (phy_mode == MODE_CCK) {

				/*
					Should not happen here, ad the last sanity check !!
				*/
				if (WMODE_CAP_5G(wdev->PhyMode)) {
					phy_mode = MODE_OFDM;
				}

				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
					preamble = SHORT_PREAMBLE;
				else
					preamble = LONG_PREAMBLE;
			}

			txd_6->tx_rate = mtd_tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
		}
	}

	/* DWORD 7 */
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (vow_dvt_apply) {
		if (cap->txd_type == TXD_V1) {
			*txd_7 &= ~SW_TX_TIME_MASK;
			*txd_7 |= SW_TX_TIME(0);
		} else {
			*txd_7 &= ~CON_SW_TX_TIME_MASK;
			*txd_7 |= CON_SW_TX_TIME(0);
		}

	}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT)*/

	if (cap->txd_type == TXD_V2) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HW_AMSDU)) {
			*txd_7 &= ~CON_HW_AMSDU_CAP_MASK;
			*txd_7 |= CON_HW_AMSDU_CAP(1);
		}

		*txd_7 &= ~CON_PP_REF_SUBTYPE_MASK;
		*txd_7 |= CON_PP_REF_SUBTYPE(pTxBlk->dot11_subtype);

		*txd_7 &= ~CON_PP_REF_TYPE_MASK;
		*txd_7 |= CON_PP_REF_TYPE(pTxBlk->dot11_type);
	}

	/*mtd_dump_tmac_info(pAd, buf);*/

#ifdef RED_SUPPORT_BY_HOST
	if (!pAd->red_mcu_offload) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_ForceRate))
			RedRecordForceRateFromDriver(pAd, pTxBlk->Wcid);
	}
#endif
}

#ifdef MT7626_REDUCE_TX_OVERHEAD
TMAC_TXD_L txd_l_cache_ar[MT7626_MT_WTBL_SIZE+1] = {0};
UINT16 cur_wcid_ar[MT7626_MT_WTBL_SIZE+1] = {0}, last_wcid_ar[MT7626_MT_WTBL_SIZE+1] = {0};
UINT8 cached_flag_ar[MT7626_MT_WTBL_SIZE+1] = {0};
VOID mtd_write_tmac_info_by_host_cached(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	UCHAR stbc = 0, bw = BW_20, mcs = 0, nss = 1, sgi = 0, phy_mode = 0, preamble = 1, ldpc = 0;
	UINT16 wcid;
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)buf;
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_s->TxD0;
	TMAC_TXD_1 *txd_1 = &txd_s->TxD1;
	UINT32 *txd_7 = &txd_s->TxD7;
	STA_TR_ENTRY *tr_entry = NULL;
	TMAC_TXD_2 *txd_2 = &txd_l->TxD2;
	TMAC_TXD_3 *txd_3 = &txd_l->TxD3;
	TMAC_TXD_5 *txd_5 = &txd_l->TxD5;
	TMAC_TXD_6 *txd_6 = &txd_l->TxD6;
	HTTRANSMIT_SETTING *pTransmit = pTxBlk->pTransmit;
	struct wifi_dev *wdev = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	UCHAR band = 0, vow_dvt_apply = 0;
	static UCHAR ucPid;
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */
	BOOLEAN is_etype = FALSE;

	wcid = pTxBlk->Wcid;
	wdev = pTxBlk->wdev;

	last_wcid_ar[wcid] = cur_wcid_ar[wcid];
	cur_wcid_ar[wcid] = wcid;
	if ((cached_flag_ar[wcid] == 1) &&
		(pTxBlk->TxFrameType != TX_MCAST_FRAME) &&
		TX_BLK_TEST_FLAG(pTxBlk, fTX_HW_AMSDU) &&
		TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS) &&
		!TX_BLK_TEST_FLAG(pTxBlk, fTX_MCAST_CLONE) &&
		!TX_BLK_TEST_FLAG(pTxBlk, fTX_ForceRate)) {
		/* cache sanity pass */
	} else {
		cached_flag_ar[wcid] = 0;
	}

	if (cached_flag_ar[wcid]) {
		/* NdisCopyMemory(txd_l,&txd_l_cache,sizeof(TMAC_TXD_L)); */
		*txd_l = txd_l_cache_ar[wcid];

		if (pTxBlk->QueIdx < 4) {
			txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[pTxBlk->wmm_set][pTxBlk->QueIdx];
		} else {
			/* TODO: shiang-usw, consider about MCC case! */
			txd_0->q_idx = pTxBlk->QueIdx;
		}

		txd_0->TxByteCount = pTxBlk->tx_bytes_len;
		txd_1->tid = pTxBlk->UserPriority;
	} else {
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		if (pAd->CommonCfg.dbdc_mode)
			band = WMODE_CAP_5G(wdev->PhyMode) ? 1 : 0;
		if (pAd->vow_dvt_en) {
			if ((!RTMP_GET_PACKET_MGMT_PKT(pTxBlk->pPacket)) &&
					(RTMP_GET_PACKET_TYPE(pTxBlk->pPacket) != TX_ALTX) &&
			(!RTMP_GET_PACKET_HIGH_PRIO(pTxBlk->pPacket))) {
				if ((wcid == pAd->vow_cloned_wtbl_from[band]) ||
						((wcid >= pAd->vow_cloned_wtbl_start[band]) &&
						 (wcid <= pAd->vow_cloned_wtbl_max[band])))
					vow_dvt_apply = 1;
			}
		}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

		NdisZeroMemory(txd_l, sizeof(TMAC_TXD_L));

		txd_0 = &txd_s->TxD0;
		txd_1 = &txd_s->TxD1;

		/* DWORD 0 */
		txd_0->p_idx = P_IDX_LMAC;
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		if (vow_dvt_apply) {
			pTxBlk->wmm_set = pAd->vow_sta_wmm[wcid];
			pTxBlk->QueIdx = pAd->vow_sta_ac[wcid];
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wmm set %d,\
					 QueIdx %d\n", pTxBlk->wmm_set, pTxBlk->QueIdx);
		}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

		if (pTxBlk->QueIdx < 4) {
			txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[pTxBlk->wmm_set][pTxBlk->QueIdx];
		} else {
			/* TODO: shiang-usw, consider about MCC case! */
			txd_0->q_idx = pTxBlk->QueIdx;
		}

#ifdef RANDOM_PKT_GEN
		random_write_qidx(pAd, buf, pTxBlk);
#endif

#if (defined(VOW_SUPPORT) && defined(VOW_DVT))
		if (vow_dvt_apply) {
			txd_0->q_idx = dmac_wmm_swq_2_hw_ac_que[pTxBlk->wmm_set][pTxBlk->QueIdx];

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\x1b[31m%s: q_idx %d, \
					 resource_idx %d\x1b[m\n", __func__, txd_0->q_idx, pTxBlk->resource_idx);

			if (pTxBlk->wmm_set == 0) {/* the same band */
				if (pAd->vow_bcmc_en == 1)
					txd_0->q_idx = TxQ_IDX_BMC0;
				else if (pAd->vow_bcmc_en == 2)
					txd_0->q_idx = TxQ_IDX_BMC1;
			}

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%d): TxBlk->wmm_set= %d,\
					 QID = %d\n", __LINE__, pTxBlk->wmm_set, txd_0->q_idx);
		}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

		txd_0->TxByteCount = pTxBlk->tx_bytes_len;

		txd_1->wlan_idx = wcid;

		if (!IS_WCID_VALID(pAd, txd_1->wlan_idx)) {
			pAd->wrong_wlan_idx_num++;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 2 wrong wlan index = %d (%d)\n", __func__, txd_1->wlan_idx, HcGetMaxStaNum(pAd)));
			dump_stack();
		}

		if (pTxBlk->TxFrameType & TX_AMSDU_FRAME)
			txd_1->amsdu = 1;


		txd_1->pkt_ft = TMI_PKT_FT_HIF_CT;

		if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
			txd_1->hdr_format = TMI_HDR_FT_NOR_80211;
			TMI_HDR_INFO_VAL(txd_1->hdr_format, 0, 0, 0, 0, 0, pTxBlk->wifi_hdr_len, 0, txd_1->hdr_info);

			if (pTxBlk->HdrPadLen)  /* TODO: depends on QoS to decide if need to padding */
				txd_1->hdr_pad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | 0x1;

		} else {
			BOOLEAN is_vlan = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) != 0) ? TRUE:FALSE;
			BOOLEAN is_rmvl = 1;

			is_etype = ((RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket) <= 1500) ? 0 : 1);
#ifdef AUTOMATION
			if (pAd->fpga_ctl.txrx_dbg_type == 1) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s: Eth type=0x%04X, is_vlan = %d, is_etype = %d, is_rmvl=%d\n",
						  __func__, (UINT16) RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket),
						  is_vlan, is_etype, is_rmvl));
			} else if (pAd->fpga_ctl.txrx_dbg_type == 5) {
				is_rmvl = 0; /* keep vlan */
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s: Eth type=0x%04X, is_vlan = %d, is_etype = %d, is_rmvl=%d\n",
						  __func__, (UINT16) RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket),
						  is_vlan, is_etype, is_rmvl));
			}
#endif /* AUTOMATION */
			txd_1->hdr_format = TMI_HDR_FT_NON_80211;
			TMI_HDR_INFO_VAL(txd_1->hdr_format,
							 TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData),
							 TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP),
							 is_rmvl,
							 is_vlan,
							 is_etype,
							 pTxBlk->wifi_hdr_len, 0, txd_1->hdr_info);

			if (pTxBlk->HdrPadLen)
				txd_1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1;
		}

		txd_1->tid = pTxBlk->UserPriority;

#ifdef CONFIG_CSO_SUPPORT
		if (IS_ASIC_CAP(pAd, fASIC_CAP_CSO)) {
			txd_0->UdpTcpChkSumOffload = 1;
			txd_0->IpChkSumOffload = 1;

			if (txd_0->IpChkSumOffload || txd_0->UdpTcpChkSumOffload) {
				if (txd_1->hdr_format == TMI_HDR_FT_NOR_80211)
					txd_0->EthTypeOffset = (pTxBlk->MpduHeaderLen + 2)>>1;
				else if (txd_1->hdr_format == TMI_HDR_FT_NON_80211)
					txd_0->EthTypeOffset = (LENGTH_802_3 + 2)>>1; /* 8 is (14+2)/2, it is	7615 setting just for 802.3 frame not for 802.11 */
				else
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: txd_1->hdr_format is unexpected\n", __func__));
			}
		}
#endif /* CONFIG_CSO_SUPPORT */

		/*
		  repeater entry doesn't have real wdev could bind.
		  so reference the OmacIdx which is stored in tr_entry.
		  other types entry could reference the OmacIdx of wdev which is connected to the pEntry.
		*/
		if (pMacEntry && IS_ENTRY_REPEATER(pMacEntry)) {
			if (pTxBlk->tr_entry) {
				tr_entry = pTxBlk->tr_entry;
				txd_1->OwnMacAddr = tr_entry->OmacIdx;
			}
		} else if (pMacEntry) {
			txd_1->OwnMacAddr = wdev->OmacIdx;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
			if (vow_dvt_apply) {
				if (pAd->vow_sta_mbss[pTxBlk->Wcid]) {
					if (wcid <= (WTBL_MAX_NUM(pAd) - MAX_MBSSID_NUM(pAd)))
						txd_1->OwnMacAddr = pAd->vow_sta_mbss[wcid]+16;
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "\x1b[31m%s: wcid %d, OM %d\x1b[m\n", __func__,
							  pTxBlk->Wcid, txd_1->OwnMacAddr);
				}

				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "\x1b[31m%s: wcid %d, OM %d\x1b[m\n", __func__, pTxBlk->Wcid,
						  txd_1->OwnMacAddr);
			}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */
		} else {
			wdev = pTxBlk->wdev;
			txd_1->OwnMacAddr = wdev->OmacIdx;
		}

		txd_2->pwr_offset = 0;

#ifdef CONFIG_AP_SUPPORT
		if (txd_2->frm_type == FC_TYPE_MGMT)
			txd_2->pwr_offset = wdev->mgmt_txd_txpwr_offset;
#endif

		txd_7 = &txd_l->TxD7;
		txd_1->ft = TMI_FT_LONG;

		if (pTransmit) {
			ldpc = pTransmit->field.ldpc;
			phy_mode = pTransmit->field.MODE;
			mcs = phy_mode == MODE_VHT ? pTransmit->field.MCS & 0xf : pTransmit->field.MCS;
			sgi = pTransmit->field.ShortGI;
			stbc = pTransmit->field.STBC;
			bw = (phy_mode <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
			nss = mtd_get_nsts_by_mcs(phy_mode, mcs, stbc,
								  phy_mode == MODE_VHT ? ((pTransmit->field.MCS & (0x3 << 4)) >> 4) + 1 : 0);
		}

		txd_l->TxD2.max_tx_time = 0;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		if (vow_dvt_apply) {
			txd_l->TxD2.max_tx_time = pAd->vow_life_time;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "wcid %d, lifetime %d\n", wcid, txd_l->TxD2.max_tx_time);
		}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

		txd_l->TxD2.bc_mc_pkt = (pTxBlk->TxFrameType == TX_MCAST_FRAME ? 1 : 0);

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_ForceRate)) {
			txd_l->TxD2.fix_rate = 1;
			if (pAd->CommonCfg.bSeOff != TRUE) {
				if (HcGetBandByWdev(pTxBlk->wdev) == BAND0) {
					if (cap->txd_type == TXD_V1) {
						*txd_7 &= ~SPE_IDX_MASK;
						*txd_7 |= SPE_IDX(BAND0_SPE_IDX);
					} else {
						*txd_7 &= ~CON_SPE_IDX_MASK;
						*txd_7 |= CON_SPE_IDX(BAND0_SPE_IDX);
					}
				} else if (HcGetBandByWdev(pTxBlk->wdev) == BAND1) {
					if (cap->txd_type == TXD_V1) {
						*txd_7 &= ~SPE_IDX_MASK;
						*txd_7 |= SPE_IDX(BAND1_SPE_IDX);
					} else {
						*txd_7 &= ~CON_SPE_IDX_MASK;
						*txd_7 |= CON_SPE_IDX(BAND1_SPE_IDX);
					}
				}
			}
		}

		txd_l->TxD2.frag = pTxBlk->FragIdx;

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired)) {
			txd_3->no_ack = 0;
			txd_l->TxD3.remain_tx_cnt = MT_TX_LONG_RETRY;
		} else {
			txd_3->no_ack = 1;
			txd_l->TxD3.remain_tx_cnt = MT_TX_RETRY_UNLIMIT;
		}

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		if (vow_dvt_apply) {
			txd_3->no_ack = (pAd->vow_sta_ack[wcid] ? 1 : 0);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "wcid %d, no ack %d\n", wcid, txd_3->no_ack);
		}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

		if (cap->TmrEnable == 1) {
			if (txd_3->no_ack == 0)
				txd_l->TxD2.timing_measure = 1;
		}

		if (IS_CIPHER_NONE(pTxBlk->CipherAlg))
			txd_3->protect_frm = 0;
		else
			txd_3->protect_frm = 1;

		txd_l->TxD5.pid = pTxBlk->Pid;

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
			if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_MCAST_CLONE))
				txd_5->da_select = TMI_DAS_FROM_MPDU;
			else
				txd_5->da_select = TMI_DAS_FROM_WTBL;

#if (defined(VOW_SUPPORT) && defined(VOW_DVT))
			if (vow_dvt_apply)
				txd_5->da_select = TMI_DAS_FROM_WTBL;
#endif /* (defined(VOW_SUPPORT) && defined(VOW_DVT)) */
		}

		if (0 /* bar_sn_ctrl */)
			txd_3->sn_vld = 1;

#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
		txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW;
#else
		txd_5->pwr_mgmt = TMI_PM_BIT_CFG_BY_SW;
#endif

#if (defined(VOW_SUPPORT) && defined(VOW_DVT))
		if (vow_dvt_apply) {
			txd_l->TxD5.tx_status_fmt = TXS_FORMAT0;
			txd_l->TxD5.tx_status_2_mcu = 0;
			if (pAd->vow_txs_en)
				txd_l->TxD5.tx_status_2_host = 1;
			txd_l->TxD5.pid = ucPid++%255;
		}
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\x1b[32m%s: wcid %d, pid %d\x1b[m\n", __func__, wcid, txd_l->TxD5.pid);
#endif /* (defined(VOW_SUPPORT) && defined(VOW_DVT)) */
#ifdef AUTOMATION
		if (is_frame_test(pAd, 0) == 1) {
			txd_5->pid = pAd->auto_dvt->txs.pid;
			txd_5->tx_status_fmt = pAd->auto_dvt->txs.format;
			txd_5->tx_status_2_mcu = 0;
			txd_5->tx_status_2_host = 1;
			send_add_txs_queue(pAd->auto_dvt->txs.pid, wcid);
		}
#endif /* AUTOMATION */
		/* DWORD 6 */
		if (txd_2->fix_rate == 1) {
			txd_6->fix_rate_mode = TMI_FIX_RATE_BY_TXD;
			txd_6->bw = ((1 << 2) | bw);
			txd_6->dyn_bw = 0;
			txd_6->TxBF = 0;
			txd_6->ldpc = ldpc;
			txd_6->gi = sgi;

#ifdef GN_MIXMODE_SUPPORT
			 if (pAd->CommonCfg.GNMixMode
				&& (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
					|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
					|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)))) {
				phy_mode = MODE_OFDM;
				mcs = 0;
			}
#endif /*GN_MIXMODE_SUPPORT*/
			if (txd_6->fix_rate_mode == TMI_FIX_RATE_BY_TXD) {
				if (phy_mode == MODE_CCK) {
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
						preamble = SHORT_PREAMBLE;
					else
						preamble = LONG_PREAMBLE;

					/*
						Should not happen here, ad the last sanity check !!
					*/
					if (WMODE_CAP_5G(wdev->PhyMode)) {
						phy_mode = MODE_OFDM;
					}
				}
				txd_6->tx_rate = mtd_tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
			}
		}

		/* DWORD 7 */
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		if (vow_dvt_apply) {
			if (cap->txd_type == TXD_V1) {
				*txd_7 &= ~SW_TX_TIME_MASK;
				*txd_7 |= SW_TX_TIME(0);
			} else {
				*txd_7 &= ~CON_SW_TX_TIME_MASK;
				*txd_7 |= CON_SW_TX_TIME(0);
			}

		}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT)*/

		if (cap->txd_type == TXD_V2) {
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HW_AMSDU)) {
				*txd_7 &= ~CON_HW_AMSDU_CAP_MASK;
				*txd_7 |= CON_HW_AMSDU_CAP(1);

				/* only data frame will trigger cache mechanism */
				if ((txd_l->TxD2.fix_rate != 1) &&
					(is_etype == TRUE)) {
					cached_flag_ar[wcid] = 1;
				}
			}
			*txd_7 &= ~CON_PP_REF_SUBTYPE_MASK;
			*txd_7 |= CON_PP_REF_SUBTYPE(pTxBlk->dot11_subtype);

			*txd_7 &= ~CON_PP_REF_TYPE_MASK;
			*txd_7 |= CON_PP_REF_TYPE(pTxBlk->dot11_type);
		}

		/* mtd_dump_tmac_info(pAd, buf); */

#ifdef RED_SUPPORT_BY_HOST
		if (!pAd->red_mcu_offload) {
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_ForceRate))
				RedRecordForceRateFromDriver(pAd, pTxBlk->Wcid);
		}
#endif
		if (cached_flag_ar[wcid] == 1) {
			txd_l_cache_ar[wcid] = *txd_l ;
		}
	}
}
#endif

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

#ifdef CUT_THROUGH
INT32 mtd_write_txp_info_by_host(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *tx_blk)
{
	MAC_TX_PKT_T *txp = (MAC_TX_PKT_T *)buf;
	TXD_PTR_LEN_T *txp_ptr_len;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	struct wifi_dev *wdev = tx_blk->wdev;
	UINT8 band_idx = HcGetBandByWdev(wdev);
	struct token_tx_pkt_queue *que = token_tx_get_queue_by_band(cb, band_idx);
	UINT16 token;
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	UCHAR band = 0, vow_dvt_apply = 0;
	UINT16 wcid;
	struct wifi_dev *wdev = NULL;

	wcid = tx_blk->Wcid;
	wdev = tx_blk->wdev;

	if (pAd->CommonCfg.dbdc_mode)
		band = WMODE_CAP_5G(wdev->PhyMode) ? 1 : 0;
	if (pAd->vow_dvt_en) {
		if ((!RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) &&
			(RTMP_GET_PACKET_TYPE(tx_blk->pPacket) != TX_ALTX) &&
			(!RTMP_GET_PACKET_HIGH_PRIO(tx_blk->pPacket))) {
			if ((wcid == pAd->vow_cloned_wtbl_from[band]) ||
				((wcid >= pAd->vow_cloned_wtbl_start[band]) &&
				(wcid <= pAd->vow_cloned_wtbl_max[band])))
				vow_dvt_apply = 1;
		}
	}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	/* check queue status */
	if (vow_dvt_apply && vow_is_queue_full(pAd, tx_blk->Wcid, tx_blk->QueIdx)) {
		pAd->vow_need_drop_cnt[tx_blk->Wcid]++;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */

	if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
		(tx_blk->amsdu_state == TX_AMSDU_ID_FIRST))
		NdisZeroMemory(txp, sizeof(MAC_TX_PKT_T));

	txp_ptr_len = &txp->arPtrLen[tx_blk->frame_idx / 2];

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

		txp_ptr_len->u2Len0 = (tx_blk->SrcBufLen | TXD_LEN_ML);

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			txp_ptr_len->u2Len0 |= TXD_LEN_AL;

		txp_ptr_len->u2Len0 = cpu2le16(txp_ptr_len->u2Len0);
	} else {
		txp_ptr_len->u4Ptr1 = cpu2le32(PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE));

		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = token_tx_enq(pAd, que, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = token_tx_enq(pAd, que, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len1 = (tx_blk->SrcBufLen | TXD_LEN_ML);

		if (tx_blk->amsdu_state == TX_AMSDU_ID_LAST)
			txp_ptr_len->u2Len1 |= TXD_LEN_AL;

		txp_ptr_len->u2Len1 = cpu2le16(txp_ptr_len->u2Len1);
	}
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (vow_dvt_apply) {
		pAd->vow_queue_map[token][0] = tx_blk->Wcid;
		pAd->vow_queue_map[token][1] = tx_blk->QueIdx;
		pAd->vow_queue_len[tx_blk->Wcid][tx_blk->QueIdx]++;
		/*printk("\x1b[31m%s: enqueue wcid %d, qidx %d, queue len %d....\x1b[m\n",
			__FUNCTION__, tx_blk->Wcid, tx_blk->QueIdx, pAd->vow_queue_len[tx_blk->Wcid][tx_blk->QueIdx]);*/
	}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */
	txp->au2MsduId[tx_blk->frame_idx] = cpu2le16(token | TXD_MSDU_ID_VLD);
	tx_blk->txp_len = sizeof(MAC_TX_PKT_T);
	return NDIS_STATUS_SUCCESS;
}

INT32 mtd_write_txp_info_by_host_v2(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *tx_blk)
{
	MAC_TX_PKT_T *txp = (MAC_TX_PKT_T *)buf;
	TXD_PTR_LEN_T *txp_ptr_len;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	struct wifi_dev *wdev = tx_blk->wdev;
	UINT8 band_idx = HcGetBandByWdev(wdev);
	struct token_tx_pkt_queue *que = token_tx_get_queue_by_band(cb, band_idx);
	UINT16 token;
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	UCHAR band = 0, vow_dvt_apply = 0;
	UINT16 wcid;
	struct wifi_dev *wdev = NULL;

	wcid = tx_blk->Wcid;
	wdev = tx_blk->wdev;

	if (pAd->CommonCfg.dbdc_mode)
		band = WMODE_CAP_5G(wdev->PhyMode) ? 1 : 0;
	if (pAd->vow_dvt_en) {
		if ((!RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) &&
			(RTMP_GET_PACKET_TYPE(tx_blk->pPacket) != TX_ALTX) &&
			(!RTMP_GET_PACKET_HIGH_PRIO(tx_blk->pPacket))) {
			if ((wcid == pAd->vow_cloned_wtbl_from[band]) ||
				((wcid >= pAd->vow_cloned_wtbl_start[band]) &&
				(wcid <= pAd->vow_cloned_wtbl_max[band])))
				vow_dvt_apply = 1;
		}
	}
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	/* check queue status */
	if (vow_dvt_apply && vow_is_queue_full(pAd, tx_blk->Wcid, tx_blk->QueIdx)) {
		pAd->vow_need_drop_cnt[tx_blk->Wcid]++;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */

	if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
		(tx_blk->amsdu_state == TX_AMSDU_ID_FIRST))
		NdisZeroMemory(txp, sizeof(MAC_TX_PKT_T));

	txp_ptr_len = &txp->arPtrLen[tx_blk->frame_idx / 2];

	RTMP_SET_BAND_IDX(tx_blk->pPacket, band_idx);

	if ((tx_blk->frame_idx & 0x1) == 0x0) {
		txp_ptr_len->u4Ptr0 = PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE);
		txp_ptr_len->u4Ptr0 = cpu2le32(txp_ptr_len->u4Ptr0);

		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = token_tx_enq(pAd, que, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr0, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = token_tx_enq(pAd, que, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr0, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len0 = ((tx_blk->SrcBufLen & TXD_LEN_MASK_V2) | TXD_LEN_ML_V2);

		txp_ptr_len->u2Len0 = cpu2le16(txp_ptr_len->u2Len0);
	} else {
		txp_ptr_len->u4Ptr1 = PCI_MAP_SINGLE(pAd, tx_blk, 0, 1, RTMP_PCI_DMA_TODEVICE);
		txp_ptr_len->u4Ptr1 = cpu2le32(txp_ptr_len->u4Ptr1);

		if (RTMP_GET_PACKET_MGMT_PKT(tx_blk->pPacket)) {
			token = token_tx_enq(pAd, que, tx_blk->pPacket, TOKEN_TX_MGT, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		} else {
			token = token_tx_enq(pAd, que, tx_blk->pPacket, TOKEN_TX_DATA, tx_blk->Wcid,
									   txp_ptr_len->u4Ptr1, GET_OS_PKT_LEN(tx_blk->pPacket));
		}

		txp_ptr_len->u2Len1 = ((tx_blk->SrcBufLen & TXD_LEN_MASK_V2) | TXD_LEN_ML_V2);

		txp_ptr_len->u2Len1 = cpu2le16(txp_ptr_len->u2Len1);
	}
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
	if (vow_dvt_apply) {
		pAd->vow_queue_map[token][0] = tx_blk->Wcid;
		pAd->vow_queue_map[token][1] = tx_blk->QueIdx;
		pAd->vow_queue_len[tx_blk->Wcid][tx_blk->QueIdx]++;
		/*printk("\x1b[31m%s: enqueue wcid %d, qidx %d, queue len %d....\x1b[m\n",
			__FUNCTION__, tx_blk->Wcid, tx_blk->QueIdx, pAd->vow_queue_len[tx_blk->Wcid][tx_blk->QueIdx]);*/
	}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */
	txp->au2MsduId[tx_blk->frame_idx] = cpu2le16(token | TXD_MSDU_ID_VLD);
	tx_blk->txp_len = sizeof(MAC_TX_PKT_T);
	return NDIS_STATUS_SUCCESS;
}


INT32 mtd_write_txp_info_by_cr4(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	CR4_TXP_MSDU_INFO *cr4_txp_msdu_info = (CR4_TXP_MSDU_INFO *)buf;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	struct wifi_dev *wdev = pTxBlk->wdev;
	UINT8 band_idx = HcGetBandByWdev(wdev);
	struct token_tx_pkt_queue *que = token_tx_get_queue_by_band(cb, band_idx);
	UINT16 token;
	UCHAR BssInfoIdx = 0;

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

	cr4_txp_msdu_info->buf_ptr[0] = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);

	RTMP_SET_BAND_IDX(pTxBlk->pPacket, band_idx);

	if (RTMP_GET_PACKET_MGMT_PKT(pTxBlk->pPacket)) {
		cr4_txp_msdu_info->type_and_flags |= CT_INFO_MGN_FRAME;
		token = token_tx_enq(pAd, que, pTxBlk->pPacket, TOKEN_TX_MGT, pTxBlk->Wcid,
								   cr4_txp_msdu_info->buf_ptr[0], GET_OS_PKT_LEN(pTxBlk->pPacket));
	} else {
		token = token_tx_enq(pAd, que, pTxBlk->pPacket, TOKEN_TX_DATA, pTxBlk->Wcid,
								   cr4_txp_msdu_info->buf_ptr[0], GET_OS_PKT_LEN(pTxBlk->pPacket));
	}

#ifdef MAC_REPEATER_SUPPORT

	if (pTxBlk->pMacEntry && IS_ENTRY_REPEATER(pTxBlk->pMacEntry))
		cr4_txp_msdu_info->rept_wds_wcid = pTxBlk->pMacEntry->wcid;
	else
		/*TODO: WDS case.*/
#endif
#ifdef A4_CONN
	if (pTxBlk->pMacEntry && IS_ENTRY_A4(pTxBlk->pMacEntry))
		cr4_txp_msdu_info->rept_wds_wcid = pTxBlk->pMacEntry->wcid;
	else
#endif /* A4_CONN */
	{
		cr4_txp_msdu_info->rept_wds_wcid = 0xff;
	}

	BssInfoIdx = wdev->bss_info_argument.ucBssIndex;

	if (token == que->pkt_tkid_invalid) {
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	cr4_txp_msdu_info->msdu_token = token;
	cr4_txp_msdu_info->bss_index = BssInfoIdx;
	cr4_txp_msdu_info->buf_num = 1; /* os get scatter. */
	cr4_txp_msdu_info->buf_len[0] = pTxBlk->SrcBufLen;
#ifdef VLAN_SUPPORT
	if (pAd->CommonCfg.bEnableVlan && RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) != 0) {
		UINT8 VlanPcp;
		VlanPcp = RTMP_GET_VLAN_PCP(pTxBlk->pPacket);

		if ((VlanPcp >= 0) && (VlanPcp <= 7))
			cr4_txp_msdu_info->reserved = VlanPcp;

	}
#endif /*VLAN_SUPPORT*/

#ifdef DSCP_PRI_SUPPORT
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		/*handle DSCP-UP Mapping in CR4*/
	} else
#endif
	{
		if ((pAd->ApCfg.DscpPriMapSupport) && (pTxBlk->DscpMappedPri >= 0)  && (pTxBlk->DscpMappedPri <= 7)) {
			cr4_txp_msdu_info->reserved = pTxBlk->DscpMappedPri;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "cr4->reserved:%d\n",
									cr4_txp_msdu_info->reserved);
		}
	}
#endif /*DSCP_PRI_SUPPORT*/
	pTxBlk->txp_len = sizeof(CR4_TXP_MSDU_INFO);

	if (cr4_txp_msdu_info->type_and_flags == 0) {
		pTxBlk->dbdc_band = pTxBlk->resource_idx;
		pTxBlk->DropPkt = FALSE;
		WLAN_HOOK_CALL(WLAN_HOOK_TX, pAd, pTxBlk);
		if (pTxBlk->DropPkt == TRUE) {
			UINT8 Type;

			token_tx_deq(pAd, que, token, &Type);
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_SUCCESS);
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s drop keep alive pkt\n", __func__);
			return NDIS_STATUS_FAILURE;
		}
	}
#ifdef RT_BIG_ENDIAN
	cr4_txp_msdu_info->msdu_token = cpu2le16(cr4_txp_msdu_info->msdu_token);
	cr4_txp_msdu_info->buf_ptr[0] = cpu2le32(cr4_txp_msdu_info->buf_ptr[0]);
	cr4_txp_msdu_info->buf_len[0] = cpu2le16(cr4_txp_msdu_info->buf_len[0]);
	cr4_txp_msdu_info->type_and_flags = cpu2le16(cr4_txp_msdu_info->type_and_flags);
#endif

	return NDIS_STATUS_SUCCESS;
}

INT dump_txp_info(RTMP_ADAPTER *pAd, CR4_TXP_MSDU_INFO *txp_info)
{
	INT cnt;

	hex_dump("CT_TxP_Info Raw Data: ", (UCHAR *)txp_info, sizeof(CR4_TXP_MSDU_INFO));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXP_Fields:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\ttype_and_flags=0x%x\n", txp_info->type_and_flags));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tmsdu_token=%d\n", txp_info->msdu_token));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tbss_index=%d\n", txp_info->bss_index));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tbuf_num=%d\n", txp_info->buf_num));

	for (cnt = 0; cnt < txp_info->buf_num; cnt++) {
		if (cnt >= MAX_BUF_NUM_PER_PKT)
			break;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tbuf_ptr=0x%x\n", txp_info->buf_ptr[cnt]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tbuf_len=%d(0x%x)\n", txp_info->buf_len[cnt], txp_info->buf_len[cnt]));
	}

	return 0;
}
#endif /*CUT_THROUGH*/

VOID mtd_write_tmac_info_mgmt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR sub_type, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen)
{
	MAC_TX_INFO mac_info;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));
	mac_info.Type = FC_TYPE_MGMT;
	mac_info.SubType = sub_type;
	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = TRUE;
	mac_info.AMPDU = FALSE;
	mac_info.BM = 1;
	mac_info.Ack = FALSE;
	mac_info.NSeq = TRUE;
	mac_info.BASize = 0;
	mac_info.WCID = 0;
	mac_info.Length = frmLen;
	mac_info.TID = 0;
	mac_info.TxRate = 0;
	mac_info.Txopmode = IFS_HTTXOP;
	mac_info.hdr_len = 24;
	mac_info.bss_idx = wdev->func_idx;
	mac_info.SpeEn = 1;
	mac_info.q_idx = HcGetBcnQueueIdx(pAd, wdev);
	mac_info.TxSPriv = wdev->func_idx;
	mac_info.OmacIdx = wdev->OmacIdx;
	mac_info.txpwr_offset = wdev->mgmt_txd_txpwr_offset;

	if (wdev->bcn_buf.BcnUpdateMethod == BCN_GEN_BY_FW)
		mac_info.IsOffloadPkt = TRUE;
	else
		mac_info.IsOffloadPkt = FALSE;

	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;

	if (pAd->CommonCfg.bSeOff != TRUE) {
		if (HcGetBandByWdev(wdev) == BAND0)
			mac_info.AntPri = BAND0_SPE_IDX;
		else if (HcGetBandByWdev(wdev) == BAND1)
			mac_info.AntPri = BAND1_SPE_IDX;
	}
	NdisZeroMemory(tmac_buf, sizeof(TMAC_TXD_L));
	mtd_write_tmac_info_fixed_rate(pAd, tmac_buf, &mac_info, BeaconTransmit);
#ifdef RT_BIG_ENDIAN

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MTMacInfoEndianChange(pAd, tmac_buf, TYPE_TXWI, sizeof(TMAC_TXD_L));

#endif
}

#define HW_TX_RATE_TO_MODE(_x)			(((_x) & (0x7 << 6)) >> 6)
#define HW_TX_RATE_TO_MCS(_x, _mode)		((_x) & (0x3f))
#define HW_TX_RATE_TO_NSS(_x)				(((_x) & (0x3 << 9)) >> 9)
#define HW_TX_RATE_TO_STBC(_x)			(((_x) & (0x1 << 11)) >> 11)

#define MAX_TX_MODE 5
static char *HW_TX_MODE_STR[] = {"CCK", "OFDM", "HT-Mix", "HT-GF", "VHT", "N/A"};
static char *HW_TX_RATE_CCK_STR[] = {"1M", "2M", "5.5M", "11M", "N/A"};
static char *HW_TX_RATE_OFDM_STR[] = {"6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M", "N/A"};

static char *hw_rate_ofdm_str(UINT16 ofdm_idx)
{
	switch (ofdm_idx) {
	case 11: /* 6M */
		return HW_TX_RATE_OFDM_STR[0];

	case 15: /* 9M */
		return HW_TX_RATE_OFDM_STR[1];

	case 10: /* 12M */
		return HW_TX_RATE_OFDM_STR[2];

	case 14: /* 18M */
		return HW_TX_RATE_OFDM_STR[3];

	case 9: /* 24M */
		return HW_TX_RATE_OFDM_STR[4];

	case 13: /* 36M */
		return HW_TX_RATE_OFDM_STR[5];

	case 8: /* 48M */
		return HW_TX_RATE_OFDM_STR[6];

	case 12: /* 54M */
		return HW_TX_RATE_OFDM_STR[7];

	default:
		return HW_TX_RATE_OFDM_STR[8];
	}
}

static char *hw_rate_str(UINT8 mode, UINT16 rate_idx)
{
	if (mode == 0)
		return rate_idx < 4 ? HW_TX_RATE_CCK_STR[rate_idx] : HW_TX_RATE_CCK_STR[4];
	else if (mode == 1)
		return hw_rate_ofdm_str(rate_idx);
	else
		return "MCS";
}


static VOID dump_wtbl_basic_info(RTMP_ADAPTER *pAd, struct wtbl_struc *tb)
{
	struct wtbl_basic_info *basic_info = &tb->peer_basic_info;
	struct wtbl_tx_rx_cap *trx_cap = &tb->trx_cap;
	struct wtbl_rate_tb *rate_info = &tb->auto_rate_tb;
	UCHAR addr[MAC_ADDR_LEN];
	UINT16 txrate[8], rate_idx, txmode, mcs, nss, stbc;

	NdisMoveMemory(&addr[0], (UCHAR *)basic_info, 4);
	addr[0] = basic_info->wtbl_d1.field.addr_0 & 0xff;
	addr[1] = ((basic_info->wtbl_d1.field.addr_0 & 0xff00) >> 8);
	addr[2] = ((basic_info->wtbl_d1.field.addr_0 & 0xff0000) >> 16);
	addr[3] = ((basic_info->wtbl_d1.field.addr_0 & 0xff000000) >> 24);
	addr[4] = basic_info->wtbl_d0.field.addr_4 & 0xff;
	addr[5] = basic_info->wtbl_d0.field.addr_5 & 0xff;
	hex_dump("WTBL Raw Data", (UCHAR *)tb, sizeof(struct wtbl_struc));
	/* Basic Info (DW0~DW1) */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Basic Info(DW0~DW1):\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tAddr: "MACSTR"(D0[B0~15], D1[B0~31])\n",
			  MAC2STR(addr)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tMUAR_Idx(D0[B16~21]):%d\n",
			  basic_info->wtbl_d0.field.muar_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\trc_a1/rc_a2:%d/%d(D0[B22]/D0[B29])\n",
			  basic_info->wtbl_d0.field.rc_a1, basic_info->wtbl_d0.field.rc_a2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tKID:%d/RCID:%d/RKV:%d/RV:%d/IKV:%d/WPI_FLAG:%d(D0[B23~24], D0[B25], D0[B26], D0[B28], D0[B30])\n",
			  basic_info->wtbl_d0.field.kid, basic_info->wtbl_d0.field.rc_id,
			  basic_info->wtbl_d0.field.rkv, basic_info->wtbl_d0.field.rv,
			  basic_info->wtbl_d0.field.ikv, basic_info->wtbl_d0.field.wpi_flg));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tGID_SU:%d(D0[B31])\n", basic_info->wtbl_d0.field.gid_su));
	/* TRX Cap(DW2~5) */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("TRX Cap(DW2~5):\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tsw/DIS_RHTR:%d/%d\n",
			  trx_cap->wtbl_d2.field.SW, trx_cap->wtbl_d2.field.dis_rhtr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tHT/VHT/HT-LDPC/VHT-LDPC/DYN_BW/MMSS:%d/%d/%d/%d/%d/%d\n",
			  trx_cap->wtbl_d2.field.ht, trx_cap->wtbl_d2.field.vht,
			  trx_cap->wtbl_d5.field.ldpc, trx_cap->wtbl_d5.field.ldpc_vht,
			  trx_cap->wtbl_d5.field.dyn_bw, trx_cap->wtbl_d5.field.mm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tTx Power Offset:0x%x(%d)\n",
			  trx_cap->wtbl_d5.field.txpwr_offset, ((trx_cap->wtbl_d5.field.txpwr_offset == 0x0) ? 0 : (trx_cap->wtbl_d5.field.txpwr_offset - 0x20))));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tFCAP/G2/G4/G8/G16/CBRN:%d/%d/%d/%d/%d/%d\n",
			  trx_cap->wtbl_d5.field.fcap, trx_cap->wtbl_d5.field.g2,
			  trx_cap->wtbl_d5.field.g4, trx_cap->wtbl_d5.field.g8,
			  trx_cap->wtbl_d5.field.g16, trx_cap->wtbl_d5.field.cbrn));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tHT-TxBF(tibf/tebf):%d/%d, VHT-TxBF(tibf/tebf):%d/%d, PFMU_IDX=%d\n",
			  trx_cap->wtbl_d2.field.tibf, trx_cap->wtbl_d2.field.tebf,
			  trx_cap->wtbl_d2.field.tibf_vht, trx_cap->wtbl_d2.field.tebf_vht,
			  trx_cap->wtbl_d2.field.pfmu_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tSPE_IDX=%d\n", trx_cap->wtbl_d3.field.spe_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tBA Enable:0x%x, BAFail Enable:%d\n",
			  trx_cap->wtbl_d4.field.ba_en, trx_cap->wtbl_d3.field.baf_en));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tQoS Enable:%d\n",
			  trx_cap->wtbl_d5.field.qos));

	if (trx_cap->wtbl_d4.field.ba_en) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 0 - %d, TID 1 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_0,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_1));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 2 - %d, TID 3 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_2,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_3));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 4 - %d, TID 5 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_4,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_5));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tBA WinSize: TID 6 - %d, TID 7 - %d\n",
				  trx_cap->wtbl_d4.field.ba_win_size_tid_6,
				  trx_cap->wtbl_d4.field.ba_win_size_tid_7));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpartial_aid:%d\n", trx_cap->wtbl_d2.field.partial_aid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twpi_even:%d\n", trx_cap->wtbl_d2.field.wpi_even));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAAD_OM/CipherSuit:%d/%d\n",
			 trx_cap->wtbl_d2.field.AAD_OM, trx_cap->wtbl_d2.field.cipher_suit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\taf:%d\n", trx_cap->wtbl_d3.field.af));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\trdg_ba:%d/rdg capability:%d\n", trx_cap->wtbl_d3.field.rdg_ba, trx_cap->wtbl_d3.field.r));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tcipher_suit:%d\n", trx_cap->wtbl_d2.field.cipher_suit));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFromDS:%d\n", trx_cap->wtbl_d5.field.fd));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tToDS:%d\n", trx_cap->wtbl_d5.field.td));
	/* Rate Info (DW6~8) */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rate Info (DW6~8):\n"));
	txrate[0] = (rate_info->wtbl_d6.word & 0xfff);
	txrate[1] = (rate_info->wtbl_d6.word & (0xfff << 12)) >> 12;
	txrate[2] = ((rate_info->wtbl_d6.word & (0xff << 24)) >> 24) | ((rate_info->wtbl_d7.word & 0xf) << 8);
	txrate[3] = ((rate_info->wtbl_d7.word & (0xfff << 4)) >> 4);
	txrate[4] = ((rate_info->wtbl_d7.word & (0xfff << 16)) >> 16);
	txrate[5] = ((rate_info->wtbl_d7.word & (0xf << 28)) >> 28) | ((rate_info->wtbl_d8.word & (0xff)) << 4);
	txrate[6] = ((rate_info->wtbl_d8.word & (0xfff << 8)) >> 8);
	txrate[7] = ((rate_info->wtbl_d8.word & (0xfff << 20)) >> 20);

	for (rate_idx = 0; rate_idx < 8; rate_idx++) {
		txmode = HW_TX_RATE_TO_MODE(txrate[rate_idx]);
		mcs = HW_TX_RATE_TO_MCS(txrate[rate_idx], txmode);
		nss = HW_TX_RATE_TO_NSS(txrate[rate_idx]);
		stbc = HW_TX_RATE_TO_STBC(txrate[rate_idx]);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tRate%d(0x%x):TxMode=%d(%s), TxRate=%d(%s), Nsts=%d, STBC=%d\n",
				  rate_idx + 1, txrate[rate_idx],
				  txmode, (txmode < MAX_TX_MODE ? HW_TX_MODE_STR[txmode] : HW_TX_MODE_STR[MAX_TX_MODE]),
				  mcs, hw_rate_str(txmode, mcs), nss, stbc));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpsm:%d\n", trx_cap->wtbl_d3.field.psm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tskip_tx:%d\n", trx_cap->wtbl_d3.field.skip_tx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tdu_i_psm:%d\n", trx_cap->wtbl_d3.field.du_i_psm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\ti_psm:%d\n", trx_cap->wtbl_d3.field.i_psm));
}



VOID mtd_dump_wtbl_base_info(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWTBL Basic Info:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tMemBaseAddr:0x%x\n",
			 pAd->mac_ctrl.wtbl_base_addr[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEntrySize/Cnt:%d/%d\n",
			 pAd->mac_ctrl.wtbl_entry_size[0],
			 pAd->mac_ctrl.wtbl_entry_cnt[0]));
}


VOID mtd_dump_wtbl_info(RTMP_ADAPTER *pAd, UINT16 wtbl_idx)
{
	INT idx, start_idx, end_idx, wtbl_len;
	UINT32 wtbl_offset, addr;
	UCHAR *wtbl_raw_dw = NULL;
	struct wtbl_entry wtbl_ent;
	struct wtbl_struc *wtbl = &wtbl_ent.wtbl;
	UINT16 wtbl_num = WTBL_MAX_NUM(pAd);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx));

	if (wtbl_idx == WCID_ALL) {
		start_idx = 0;
		end_idx = (wtbl_num - 1);
	} else if (wtbl_idx < wtbl_num)
		start_idx = end_idx = wtbl_idx;
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():Invalid WTBL index(%d)!\n",
				  __func__, wtbl_idx));
		return;
	}

	wtbl_len = sizeof(WTBL_STRUC);
	os_alloc_mem(pAd, (UCHAR **)&wtbl_raw_dw, wtbl_len);

	if (!wtbl_raw_dw) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():AllocMem fail(%d)!\n",
				  __func__, wtbl_idx));
		return;
	}

	for (idx = start_idx; idx <= end_idx; idx++) {
		wtbl_ent.wtbl_idx = idx;
		wtbl_ent.wtbl_addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];

		/* Read WTBL Entries */
		for (wtbl_offset = 0; wtbl_offset <= wtbl_len; wtbl_offset += 4) {
			addr = wtbl_ent.wtbl_addr + wtbl_offset;
			HW_IO_READ32(pAd->hdev_ctrl, addr, (UINT32 *)(&wtbl_raw_dw[wtbl_offset]));
		}

		NdisCopyMemory((UCHAR *)wtbl, &wtbl_raw_dw[0], sizeof(struct wtbl_struc));
		dump_wtbl_entry(pAd, &wtbl_ent);
	}

	os_free_mem(wtbl_raw_dw);
}


VOID dump_wtbl_entry(RTMP_ADAPTER *pAd, struct wtbl_entry *ent)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL SW Entry[%d] info\n", ent->wtbl_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWTBL info:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tAddr=0x%x\n", ent->wtbl_addr));
	dump_wtbl_basic_info(pAd, &ent->wtbl);
}


INT mtd_get_wtbl_entry234(RTMP_ADAPTER *pAd, UINT16 widx, struct wtbl_entry *ent)
{
	struct rtmp_mac_ctrl *wtbl_ctrl;
	UINT16 wtbl_idx;

	wtbl_ctrl = &pAd->mac_ctrl;

	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		wtbl_idx = (widx < wtbl_ctrl->wtbl_entry_cnt[0] ? widx : wtbl_ctrl->wtbl_entry_cnt[0] - 1);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():pAd->mac_ctrl not init yet!\n", __func__));
		return FALSE;
	}

	ent->wtbl_idx = wtbl_idx;
	ent->wtbl_addr = wtbl_ctrl->wtbl_base_addr[0] +
		wtbl_idx * wtbl_ctrl->wtbl_entry_size[0];
	return TRUE;
}

INT mtd_init_wtbl(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	pAd->mac_ctrl.wtbl_base_addr[0] = (UINT32)WTBL_BASE_ADDR;
	pAd->mac_ctrl.wtbl_entry_size[0] = (UINT16)WTBL_PER_ENTRY_SIZE;
	pAd->mac_ctrl.wtbl_entry_cnt[0] = WTBL_MAX_NUM(pAd);
	return TRUE;
}

VOID mtd_show_mac_info(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "MAC[Ver:Rev/ID=0x%08x : 0x%08x]\n",
			  pAd->MACVersion, pAd->ChipID);
}

UINT32 mtd_get_packet_type(RTMP_ADAPTER *ad, VOID *rx_packet)
{
	union _RMAC_RXD_0_UNION *rxd_0;

	rxd_0 = (union _RMAC_RXD_0_UNION *)(rx_packet);

#ifdef RT_BIG_ENDIAN
	mt_rmac_d0_endian_change(&rxd_0->word);
#endif /* RT_BIG_ENDIAN */

	return RMAC_RX_PKT_TYPE(rxd_0->word) ;
}

#define RMAC_INFO_BASE_SIZE	16
#define RMAC_INFO_GRP_1_SIZE    16
#define RMAC_INFO_GRP_2_SIZE    8
#define RMAC_INFO_GRP_3_SIZE    24
#define RMAC_INFO_GRP_4_SIZE    16

/*
    1'b0: the related GROUP is not present
    1'b1: the related GROUP is present

    bit[0]: indicates GROUP1 (DW8~DW11)
    bit[1]: indicates GROUP2 (DW12~DW13)
    bit[2]: indicates GROUP3 (DW14~DW19)
    bit[3]: indicates GROUP4 (DW4~DW7)
*/
static INT32 RMACInfoGrpToLen[] = {
	/* 0: base only */
	RMAC_INFO_BASE_SIZE,
	/* 1: [bit 0] base + group 1 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE,
	/* 2: [bit 1] base + group 2 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_2_SIZE,
	/* 3: [bit 0 + bit 1] base + group 1 + group 2 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_2_SIZE,
	/* 4: [bit 2] base + group 3 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_3_SIZE,
	/* 5: [bit 0 + bit 2] base + group 1 + group 3 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_3_SIZE,
	/* 6: [bit 1 + bit 2] base + group 2 + group 3 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_3_SIZE,
	/* 7: [bit 0 + bit 1 + bit 2] base + group 1 + group 2 + group 3 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_3_SIZE,
	/* 8: [bit 3 ] base + group 4 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_4_SIZE,
	/* 9: [bit 0 + bit 3 ] base + group 1 + group 4 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_4_SIZE,
	/* 10: [bit 1 + bit 3 ] base + group 2 + group 4 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_4_SIZE,
	/* 11: [bit 0 + bit 1 + bit 3 ] base + group 1 + group 2 + group 4 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_4_SIZE,
	/* 12: [bit 2 + bit 3 ] base + group 3 + group 4 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_3_SIZE + RMAC_INFO_GRP_4_SIZE,
	/* 13: [bit 0 + bit 2 + bit 3 ] base + group 1 + group 3 + group 4 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_3_SIZE + RMAC_INFO_GRP_4_SIZE,
	/* 14: [bit 1 + bit 2 + bit 3 ] base + group 2 + group 3 + group 4 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_3_SIZE + RMAC_INFO_GRP_4_SIZE,
	/* 15: [bit 0 + bit 1 + bit 2 + bit 3 ] base + group 1 + group 2 + group 3 + group 4 */
	RMAC_INFO_BASE_SIZE + RMAC_INFO_GRP_1_SIZE + RMAC_INFO_GRP_2_SIZE + RMAC_INFO_GRP_3_SIZE + RMAC_INFO_GRP_4_SIZE,
};

#ifdef MAC_REPEATER_SUPPORT
#endif /* MAC_REPEATER_SUPPORT */

INT32 mtd_trans_rxd_into_rxblk(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket)
{
	UCHAR *RMACInfo, *Pos;
	INT32 RMACInfoLen;
	struct _RXD_BASE_STRUCT *rx_base;
	RXD_GRP4_STRUCT *RxdGrp4 = NULL;
	RXD_GRP1_STRUCT *RxdGrp1 = NULL;
	RXD_GRP2_STRUCT *RxdGrp2 = NULL;
	RXD_GRP3_STRUCT *RxdGrp3 = NULL;
	UCHAR *FC = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	pRxBlk->pRxInfo = (RXINFO_STRUC *)(&pRxBlk->hw_rx_info[RXINFO_OFFSET]);
	RMACInfo = (UCHAR *)(GET_OS_PKT_DATAPTR(pRxPacket));
	Pos = RMACInfo;
	pRxBlk->rmac_info = RMACInfo;
	rx_base = (struct _RXD_BASE_STRUCT *)RMACInfo;
	Pos += RMAC_INFO_BASE_SIZE;

	if (rx_base->RxD0.RfbGroupVld & RXS_GROUP4) {
		RxdGrp4 = (RXD_GRP4_STRUCT *)Pos;
		FC = Pos;
		Pos += RMAC_INFO_GRP_4_SIZE;
	}

	if (rx_base->RxD0.RfbGroupVld & RXS_GROUP1) {
		RxdGrp1 = (RXD_GRP1_STRUCT *)Pos;
		Pos += RMAC_INFO_GRP_1_SIZE;
	}

	if (rx_base->RxD0.RfbGroupVld & RXS_GROUP2) {
		RxdGrp2 = (RXD_GRP2_STRUCT *)Pos;
		pRxBlk->TimeStamp = RxdGrp2->rxd_12.Timestamp;
		Pos += RMAC_INFO_GRP_2_SIZE;
	}

	if (rx_base->RxD0.RfbGroupVld & RXS_GROUP3) {
		RxdGrp3 = (RXD_GRP3_STRUCT *)Pos;
		Pos += RMAC_INFO_GRP_3_SIZE;
	}

	RMACInfoLen = RMACInfoGrpToLen[rx_base->RxD0.RfbGroupVld];
#ifdef RT_BIG_ENDIAN

	if ((RMACInfoLen - 4) > 0)
		MTMacInfoEndianChange(pAd, RMACInfo, TYPE_RMACINFO, RMAC_INFO_BASE_SIZE);

	if (RxdGrp4) {
		/*In RxdGrp4, byte 2,3,4,5,6,7 is TA, should not do Endianchange*/
		*((UINT16 *)FC) =  SWAP16(*((UINT16 *)FC));
		RTMPEndianChange((UCHAR *)RxdGrp4 + 8, RMAC_INFO_GRP_4_SIZE - 8);
	}
	if (RxdGrp1)
		RTMPEndianChange((UCHAR *)RxdGrp1, RMAC_INFO_GRP_1_SIZE);
	if (RxdGrp2)
		RTMPEndianChange((UCHAR *)RxdGrp2, RMAC_INFO_GRP_2_SIZE);
	if (RxdGrp3)
		RTMPEndianChange((UCHAR *)RxdGrp3, RMAC_INFO_GRP_3_SIZE);
#endif /* RT_BIG_ENDIAN */
	/* dump_rmac_info(pAd, RMACInfo); */
	pRxBlk->MPDUtotalByteCnt = rx_base->RxD0.RxByteCnt - RMACInfoLen;

	if (rx_base->RxD1.HdrOffset == 1) {
		pRxBlk->MPDUtotalByteCnt -= 2;
		RMACInfoLen += 2;
	}

	pRxBlk->DataSize = pRxBlk->MPDUtotalByteCnt;
	pRxBlk->wcid = rx_base->RxD2.RxDWlanIdx;
	pRxBlk->bss_idx = rx_base->RxD1.RxDBssidIdx;
	pRxBlk->key_idx = rx_base->RxD1.KeyId;
	pRxBlk->channel_freq = rx_base->RxD1.ChFreq;
	pRxBlk->TID = rx_base->RxD2.RxDTid;
	{
		pRxBlk->pRxInfo->U2M = rx_base->RxD1.UcastToMe;
		pRxBlk->pRxInfo->Mcast = rx_base->RxD1.Mcast;
		pRxBlk->pRxInfo->Bcast = rx_base->RxD1.Bcast;
	}

	if (RxdGrp1 != NULL) {
		UINT64 pn1 = RxdGrp1->rxd_9.RscPn1.PN;
		UINT64 pnTotal = 0;

		if (pn1 != 0)
			pnTotal =  RxdGrp1->rxd_8.RscPn0 + (pn1 << 32);
		else
			pnTotal =  RxdGrp1->rxd_8.RscPn0;

		pRxBlk->CCMP_PN = pnTotal;
	}
	pRxBlk->AmsduState = rx_base->RxD1.PayloadFmt;
	pRxBlk->DeAmsduFail = rx_base->RxD2.DeAmsduFail;
	pRxBlk->pRxInfo->FRAG = rx_base->RxD2.FragFrm;
	pRxBlk->pRxInfo->NULLDATA = rx_base->RxD2.NullFrm;
	pRxBlk->pRxInfo->DATA = !(rx_base->RxD2.NonDataFrm);
	pRxBlk->pRxInfo->HTC = rx_base->RxD1.HTC;
	pRxBlk->pRxInfo->AMPDU = !(rx_base->RxD2.NonAmpduFrm);
	pRxBlk->pRxInfo->L2PAD = 0;
	pRxBlk->pRxInfo->AMSDU = 0; /* TODO: */
	pRxBlk->pRxInfo->CipherErr = rx_base->RxD2.IcvErr | (rx_base->RxD2.TkipMicErr << 1);/* 0: decryption okay, 1:ICV error, 2:MIC error, 3:KEY not valid */

	pRxBlk->sec_mode = rx_base->RxD2.SecMode;

	if (rx_base->RxD1.HTC)
		RX_BLK_SET_FLAG(pRxBlk, fRX_HTC);

	if (rx_base->RxD2.CipherMis)
		RX_BLK_SET_FLAG(pRxBlk, fRX_CM);

	if (rx_base->RxD2.CipherLenMis)
		RX_BLK_SET_FLAG(pRxBlk, fRX_CLM);

	if (rx_base->RxD2.IcvErr)
		RX_BLK_SET_FLAG(pRxBlk, fRX_ICV_ERR);

	if (rx_base->RxD2.TkipMicErr)
		RX_BLK_SET_FLAG(pRxBlk, fRX_TKIP_MIC_ERR);

	pRxBlk->pRxInfo->Crc = rx_base->RxD2.FcsErr;
	pRxBlk->pRxInfo->MyBss = ((rx_base->RxD1.RxDBssidIdx == 0xf) ? 0 : 1);
	pRxBlk->pRxInfo->Decrypted = 0; /* TODO: */
#ifdef SNIFFER_MT7615

	if (IS_MT7615(pAd)) {
		OS_PKT_RESERVE(pRxPacket, RMACInfoLen);
		OS_PKT_TAIL_BUF_EXTEND(pRxPacket, pRxBlk->MPDUtotalByteCnt);
	} else {
#endif
		SET_OS_PKT_DATAPTR(pRxPacket, GET_OS_PKT_DATAPTR(pRxPacket) + RMACInfoLen);
		SET_OS_PKT_LEN(pRxPacket, pRxBlk->MPDUtotalByteCnt);
#ifdef SNIFFER_MT7615
	}

#endif

#ifdef VLAN_SUPPORT
	/*For IOT, remove unused fields*/
	if (pRxPacket) {
		UCHAR byte0, byte1, extra_field_offset;
		extra_field_offset = IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(pRxPacket)) ? (2 * ETH_ALEN+LENGTH_802_1Q) : (2 * ETH_ALEN);

		/*Remove the extra field (2 Bytes) which is added by hardware*/
		/*The added info is the length of actual data (without overhead)*/
		byte0 = GET_OS_PKT_DATAPTR(pRxPacket)[extra_field_offset];
		byte1 = GET_OS_PKT_DATAPTR(pRxPacket)[extra_field_offset+1];
		/*If there is the extra field, remove it*/
		if (((byte0<<8) | byte1) == GET_OS_PKT_LEN(pRxPacket) - extra_field_offset - 2) { /*2 : len of extra field*/
			memmove(GET_OS_PKT_DATAPTR(pRxPacket) + 2, GET_OS_PKT_DATAPTR(pRxPacket), extra_field_offset);
			RtmpOsSkbPullRcsum(pRxPacket, 2);
			RtmpOsSkbResetNetworkHeader(pRxPacket);
			RtmpOsSkbResetTransportHeader(pRxPacket);
			RtmpOsSkbResetMacLen(pRxPacket);
			pRxBlk->DataSize -= 2;
		}
		/*End of remove extra field*/
	}
#endif /*VLAN_SUPPORT*/

	pRxBlk->pRxPacket = pRxPacket;
	pRxBlk->pData = (UCHAR *)GET_OS_PKT_DATAPTR(pRxPacket);
#ifdef HDR_TRANS_RX_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		struct wifi_dev *wdev = NULL;

		if (!FC)
			return 0;

		pRxBlk->FC = FC;
		pRxBlk->FN = RxdGrp4->rxd_6.Frag;
		pRxBlk->SN = RxdGrp4->rxd_6.Seq;
		pRxBlk->UserPriority = (RxdGrp4->rxd_6.QoS & QOS_USER_PRIORITY_MASK);
		wdev = wdev_search_by_wcid(pAd, pRxBlk->wcid);

		if (wdev == NULL)
			wdev = wdev_search_by_omac_idx(pAd, pRxBlk->bss_idx);

		if (wdev == NULL)
			return 0;

		if ((((FRAME_CONTROL *)FC)->ToDs == 0) && (((FRAME_CONTROL *)FC)->FrDs == 0)) {
			pRxBlk->Addr1 = pRxBlk->pData;
			pRxBlk->Addr2 = pRxBlk->pData + 6;
			pRxBlk->Addr3 = wdev->bssid;
		} else if ((((FRAME_CONTROL *)FC)->ToDs == 0) && (((FRAME_CONTROL *)FC)->FrDs == 1)) {
			pRxBlk->Addr1 = pRxBlk->pData;
			pRxBlk->Addr2 = wdev->bssid;
			pRxBlk->Addr3 = pRxBlk->pData + 6;
		} else if ((((FRAME_CONTROL *)FC)->ToDs == 1) && (((FRAME_CONTROL *)FC)->FrDs == 0)) {
			pRxBlk->Addr1 = wdev->bssid;
			pRxBlk->Addr2 = pRxBlk->pData + 6;
			pRxBlk->Addr3 = pRxBlk->pData;
		} else {
			pRxBlk->Addr1 = wdev->if_addr;
			pRxBlk->Addr2 = FC + 2;
			pRxBlk->Addr3 = pRxBlk->pData;
			pRxBlk->Addr4 = pRxBlk->pData + 6;
		}
	} else
#endif
	{
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, pRxBlk->pData, DIR_READ, TRUE);
#endif /* RT_BIG_ENDIAN */
		pRxBlk->FC = pRxBlk->pData;
		pRxBlk->Duration = *((UINT16 *)(pRxBlk->pData + 2));

		if ((((FRAME_CONTROL *)pRxBlk->FC)->Type == FC_TYPE_MGMT) || (((FRAME_CONTROL *)pRxBlk->FC)->Type == FC_TYPE_DATA)) {
			pRxBlk->FN = *((UINT16 *)(pRxBlk->pData + 22)) & 0x000f;
			pRxBlk->SN = (*((UINT16 *)(pRxBlk->pData + 22)) & 0xfff0) >> 4;
		}

		pRxBlk->Addr1 = pRxBlk->pData + 4;
		pRxBlk->Addr2 = pRxBlk->pData + 10;
		pRxBlk->Addr3 = pRxBlk->pData + 16;

		if ((((FRAME_CONTROL *)pRxBlk->FC)->ToDs == 1) && (((FRAME_CONTROL *)pRxBlk->FC)->FrDs == 1))
			pRxBlk->Addr4 = pRxBlk->pData + 24;
	}

	if (rx_base->RxD0.RfbGroupVld & RXS_GROUP3)
		chip_parse_rxv_packet(pAd, RMAC_RX_PKT_TYPE_RX_NORMAL, pRxBlk, (UCHAR *)RxdGrp3);

	if (((FRAME_CONTROL *)pRxBlk->FC)->SubType == SUBTYPE_AUTH) {
		/*
			If HW already decrypted this packet, SW doesn't need to decrypt again.
			@20150708
		*/
		if ((rx_base->RxD2.SecMode != 0) &&
			(rx_base->RxD2.CipherMis == 0) &&
			(rx_base->RxD2.CipherLenMis == 0))
			pRxBlk->pRxInfo->Decrypted = 1;

		MTWF_DBG_NP(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "(): SecMode = 0x%x, CipherMis = %d, CipherLenMis = %d, TkipMicErr = %d, IcvErr = %d\n",
				  rx_base->RxD2.SecMode, rx_base->RxD2.CipherMis, rx_base->RxD2.CipherLenMis, rx_base->RxD2.TkipMicErr, rx_base->RxD2.IcvErr);
	}

#ifdef AUTOMATION
	automation_dump_rxd_rxblk(pAd, (CHAR *)__func__, (INT)__LINE__, pRxBlk, rx_base);
	rxd_wcid_check(pAd, rx_base->RxD2.RxDWlanIdx);
#endif /* AUTOMATION */

#ifdef MAC_REPEATER_SUPPORT
#endif /* MAC_REPEATER_SUPPORT */

#ifdef CONFIG_CSO_SUPPORT

	if ((pChipCap->asic_caps & fASIC_CAP_CSO) == fASIC_CAP_CSO) {
		if ((!rx_base->RxD0.IpChkSumOffload) || (!rx_base->RxD0.UdpTcpChkSumOffload))
			RTMP_SET_TCP_CHKSUM_FAIL(pRxBlk->pRxPacket, TRUE);
	}

#endif
	return RMACInfoLen;
}

#define GET_RX_ORDER_TOKEN_ID(_ptr, _idx) le2cpu16(*(UINT16 *)(((UINT8 *)(_ptr)) + 4 * (_idx)))
#define GET_RX_ORDER_DROP(_ptr, _idx) ((*(UINT8 *)(((UINT8 *)(_ptr)) + 4 * (_idx) + 3)) & 0x01)

#ifdef CUT_THROUGH
static VOID EventTxFreeNotifyHandler(RTMP_ADAPTER *pAd, PKT_TOKEN_CB *cb,
								UINT8 *dataPtr, UINT32 TxDCnt, UINT32 Len, UINT8 Ver)
{
	UINT loop;
	PNDIS_PACKET pkt;
	UINT16 *token_ptr, token_id;
	UINT8 type, LenPerToken;
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct token_tx_pkt_queue *que = NULL;

	if (dataPtr == NULL) {
		ASSERT(0);
		return;
	}

	LenPerToken = (Ver == 0) ? 2 : 4;

	for (loop = 0; loop < TxDCnt; loop++) {
		if (loop * LenPerToken > Len) {
			MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
					 ("%s: token number len mismatch TxD Cnt=%d Len=%d\n",
					  __func__, loop, Len));
			hex_dump("EventTxFreeNotifyHandlerErrorFrame", dataPtr, Len);
			break;
		}

		token_ptr = (UINT16 *)dataPtr;
		token_id = le2cpu16(*token_ptr);
		que = token_tx_get_queue_by_token_id(cb, token_id);
		que->total_back_cnt++;
		/* shift position according to the version of tx done event */
		dataPtr += LenPerToken;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		{
			UINT16 wcid = pAd->vow_queue_map[token_id][0];
			UCHAR qidx = pAd->vow_queue_map[token_id][1];

			if ((wcid > 0 && wcid < 128) && (qidx >= 0 && qidx < 4)) {
				pAd->vow_queue_len[wcid][qidx]--;
				pAd->vow_queue_map[token_id][0] = 0xff;
				pAd->vow_queue_map[token_id][1] = 0xf;

				pAd->vow_tx_free[wcid]++;
			}
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[31m%s: tokenid %d, wcid %d, qidx %d\x1b[m\n", __func__, token_id, wcid, qidx);
		}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */


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
#ifdef CUT_THROUGH_DBG
		/* debug only: measure the time interval between Pkt sended to free-notity comed back. */
		NdisGetSystemUpTime(&pktTokenCb->tx_id_list.list->pkt_token[token_id].endTime);
		MTWF_DBG(pAd, DBG_CAT_TOKEN, TOKEN_PROFILE, DBG_LVL_INFO,
				 "tx time token_id[%d] = %ld %ld %ld OS_HZ=%d\n",
				  token_id,
				  pktTokenCb->tx_id_list.list->pkt_token[token_id].endTime -
				  pktTokenCb->tx_id_list.list->pkt_token[token_id].startTime,
				  pktTokenCb->tx_id_list.list->pkt_token[token_id].endTime,
				  pktTokenCb->tx_id_list.list->pkt_token[token_id].startTime,
				  OS_HZ);
		MTWF_DBG(pAd, pAd, DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_INFO,
				 "token_id[%d] = 0x%x, try to free TxPkt(%p)\n",
				  loop, token_id, pkt);
#endif

#ifdef CONFIG_ATE
		if (ATE_ON(pAd)) {
			/* Note: temp for wlan_service/original coexistence */
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
#endif /* CONFIG_WLAN_SERVICE */
		}
#endif /* CONFIG_ATE */

		if (pkt != NULL) {
#ifdef RED_SUPPORT_BY_HOST
			/* maintain RA/AC Tail Drop counter */
			if (!pAd->red_mcu_offload)
				red_tx_free_handle(pAd, pkt);
#endif

#ifdef CONFIG_ATE
			if (ATE_ON(pAd))
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			else
#endif
			{
#ifdef FQ_SCH_SUPPORT
				RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

				if (cap->qm == FAST_PATH_FAIR_QM)
					fq_tx_free_per_packet(pAd, RTMP_GET_PACKET_QUEIDX(pkt),
						RTMP_GET_PACKET_WCID(pkt), pkt);
#endif
				if (type == TOKEN_TX_DATA)
					RELEASE_NDIS_PACKET_IRQ(pAd, pkt, NDIS_STATUS_SUCCESS);
				else
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			}
		} else {
			MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_ERROR,
					 ("%s: Get a token_id[%d] = 0x%x but PktPtr is NULL!\n",
					  __func__, loop, token_id));
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
#endif /*CUT_THROUGH*/

#define TXRX_NOTE_GET_EVENT_TYPE(_ptr) ((UINT8)(((UINT32)(((UINT8 *)(_ptr)) + 3) & 0x1e000000) >> 25))
#define TXRX_NOTE_GET_TXDCNT(_ptr) (le2cpu16((UINT16)(((UINT32)(((UINT8 *)(_ptr)) + 2) & 0x01ff0000) >> 16)))
#define TXRX_NOTE_GET_LEN(_ptr) (le2cpu16((UINT16)(((UINT32)(((UINT8 *)(_ptr)) + 1) & 0x0000ffff))))
#define TXRX_NOTE_GET_DATA(_ptr) ((UINT8 *)(((UINT8 *)(_ptr)) + 8))
#define TXRX_NOTE_GET_TOKEN_LIST(_ptr) ((UINT8 *)(((UINT8 *)(_ptr)) + 8))

#ifdef CUT_THROUGH
UINT32 mtd_txdone_handle(RTMP_ADAPTER *ad, VOID *ptr, UINT8 resource_idx)
{
	PKT_TOKEN_CB *cb = hc_get_ct_cb(ad->hdev_ctrl);
	UINT32 dw0 = *(UINT32 *)ptr;
	UINT32 dw1 = *((UINT32 *)ptr + 1);
	UINT8 *tokenList;
	UINT8 tokenCnt;
	UINT16 rxByteCnt;
	UINT8 report_type;
	UINT8 version, len_per_token;

	rxByteCnt = (dw0 & 0xffff);
	tokenCnt = ((dw0 & (0x7f << 16)) >> 16) & 0x7f;
	report_type = ((dw0 & (0x3f << 23)) >> 23) & 0x3f;
	version = ((dw1 & (0x7 << 16)) >> 16) & 0x7;
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: DW0=0x%08x, rxByteCnt=%d,tokenCnt= %d, ReportType=%d\n",
			  __func__, dw0, rxByteCnt, tokenCnt, report_type);

	if (tokenCnt > 0x7f) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Invalid tokenCnt(%d)!\n", tokenCnt));
		return FALSE;
	}

	switch (report_type) {
	case TX_FREE_NOTIFY:
		len_per_token = (version == 0) ? 2 : 4;

		if ((tokenCnt * len_per_token + 8) != rxByteCnt) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tokenCnt(%d) and rxByteCnt(%d) mismatch!\n", tokenCnt, rxByteCnt));
			hex_dump("TxFreeNotifyEventMisMatchFrame", ptr, rxByteCnt);
			return FALSE;
		}
#if defined(RED_SUPPORT) && (defined(MT7622) || defined(P18) || defined(MT7663))
		if (IS_MT7622(ad) || IS_P18(ad) || IS_MT7663(ad)) {
			/*For maintain RA/AC Tail Drop counter, we need to know WlanIdx & AC by parsing DW*/
			/*RedTxFreeEventNotifyHandler((UINT32 *)ptr, tokenCnt, version, ad);*/
		}
#endif
		tokenList = TXRX_NOTE_GET_TOKEN_LIST(ptr);
		EventTxFreeNotifyHandler(ad, cb, tokenList, tokenCnt, rxByteCnt - 8, version);
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Invalid type(%d)!\n", report_type));
		break;
	}

	return TRUE;
}
#endif /*CUT_THROUGH*/

INT32 mtd_txs_handler(RTMP_ADAPTER *pAd, VOID *rx_packet)
{
	RMAC_RXD_0_TXS *txs = (RMAC_RXD_0_TXS *)(rx_packet);
	UCHAR *ptr;
	INT idx, txs_entry_len = 20;
	BOOLEAN dump_all = FALSE;

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "TxS Report: Number=%d, ByteCnt=%d\n",
			  txs->TxSCnt, txs->TxSRxByteCnt);

	if (txs->TxSRxByteCnt == 0)
		return TRUE;

#if (ENABLE_RXD_LOG)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("TxS Report:TxSPktType =%d, Number=%d, ByteCnt=%d\n",
			  txs->TxSPktType, txs->TxSCnt, txs->TxSRxByteCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_0_TXS: 0x%x\n", *((UINT32 *)txs + 0)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_1_TXS: 0x%x\n", *((UINT32 *)txs + 1)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_2_TXS: 0x%x\n", *((UINT32 *)txs + 2)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_3_TXS: 0x%x\n", *((UINT32 *)txs + 3)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_4_TXS: 0x%x\n", *((UINT32 *)txs + 4)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_5_TXS: 0x%x\n", *((UINT32 *)txs + 5)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_6_TXS: 0x%x\n", *((UINT32 *)txs + 6)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_7_TXS: 0x%x\n", *((UINT32 *)txs + 7)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_8_TXS: 0x%x\n", *((UINT32 *)txs + 8)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_9_TXS: 0x%x\n", *((UINT32 *)txs + 9)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_10_TXS: 0x%x\n", *((UINT32 *)txs + 10)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_11_TXS: 0x%x\n", *((UINT32 *)txs + 11)));
#endif

	txs_entry_len = 28; /* sizeof(TXS_STRUC); // 28 bytes */


	if (txs->TxSRxByteCnt != (txs->TxSCnt * txs_entry_len + 4)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("ReceivedByteCnt not equal txs_entry required!\n"));
	} else {
		ptr = ((UCHAR *)(rx_packet)) + 4;

		for (idx = 0; idx < txs->TxSCnt; idx++) {
			TXS_STRUC *txs_entry = (TXS_STRUC *)ptr;
			TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
			INT32 stat;
#ifdef RT_BIG_ENDIAN
			TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
			TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
			TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
#endif /* RT_BIG_ENDIAN */
#ifdef RT_BIG_ENDIAN
			*(((UINT32 *)TxSD0)) = SWAP32(*(((UINT32 *)TxSD0)));
			*(((UINT32 *)TxSD1)) = SWAP32(*(((UINT32 *)TxSD1)));
			*(((UINT32 *)TxSD2)) = SWAP32(*(((UINT32 *)TxSD2)));
			*(((UINT32 *)TxSD3)) = SWAP32(*(((UINT32 *)TxSD3)));
#endif /* RT_BIG_ENDIAN */
#ifdef RT_BIG_ENDIAN
			{
				TXS_D_5 *TxSD5 = &txs_entry->TxSD5;
				TXS_D_6 *TxSD6 = &txs_entry->TxSD6;
				*(((UINT32 *)TxSD5)) = SWAP32(*(((UINT32 *)TxSD5)));
				*(((UINT32 *)TxSD6)) = SWAP32(*(((UINT32 *)TxSD6)));
			}
#endif /* RT_BIG_ENDIAN */
			stat = ParseTxSPacket_v2(pAd, TxSD0->TxS_PId, TxSD0->TxSFmt, ptr);

			if (stat == -1)
				dump_all = TRUE;


			ptr += txs_entry_len;
		}

		if (dump_all == TRUE) {
			printk("%s(): Previous received TxS has error, dump raw data!\n", __func__);
			printk("\tTxS Report: Cnt=%d, ByteCnt=%d\n", txs->TxSCnt, txs->TxSRxByteCnt);
			hex_dump("Raw data", ((UCHAR *)(rx_packet)),
					 txs->TxSRxByteCnt > 400 ? 400 : txs->TxSRxByteCnt);
		}
	}

	return TRUE;
}

VOID mtd_rx_event_handler(RTMP_ADAPTER *pAd, UCHAR *data)
{
	struct cmd_msg *msg;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	EVENT_RXD *event_rxd = (EVENT_RXD *)(data + sizeof(RXD_BASE_STRUCT));

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	event_rxd->fw_rxd_0.word = le2cpu32(event_rxd->fw_rxd_0.word);
	event_rxd->fw_rxd_1.word = le2cpu32(event_rxd->fw_rxd_1.word);
	event_rxd->fw_rxd_2.word = le2cpu32(event_rxd->fw_rxd_2.word);

	msg = AndesAllocCmdMsg(pAd, GET_EVENT_FW_RXD_LENGTH(event_rxd));

	if (!msg)
		return;

	AndesAppendCmdMsg(msg, (char *)(data + sizeof(RXD_BASE_STRUCT)), GET_EVENT_FW_RXD_LENGTH(event_rxd));
	AndesMTRxProcessEvent(pAd, msg);

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	if (msg->net_pkt)
		RTMPFreeNdisPacket(pAd, msg->net_pkt);
#endif

	AndesFreeCmdMsg(msg);
}

static VOID mtd_fill_txd_header(struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	TMAC_TXD_L *txd;
	UCHAR *tmac_info;

	tmac_info = (UCHAR *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(TMAC_TXD_L));
	txd = (TMAC_TXD_L *)tmac_info;
	NdisZeroMemory(txd, sizeof(TMAC_TXD_L));
	txd->TxD0.TxByteCount = GET_OS_PKT_LEN(net_pkt);
	txd->TxD0.p_idx = (msg->pq_id & 0x8000) >> 15;
	txd->TxD0.q_idx = (msg->pq_id & 0x7c00) >> 10;
	txd->TxD1.ft = 0x1;
	txd->TxD1.hdr_format = 0x1;

	if (msg->attr.type == MT_FW_SCATTER)
		txd->TxD1.pkt_ft = TMI_PKT_FT_HIF_FW;
	else
		txd->TxD1.pkt_ft = TMI_PKT_FT_HIF_CMD;

#ifdef RT_BIG_ENDIAN
	MTMacInfoEndianChange(NULL, (UCHAR *)tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif
	return;
}

VOID mtd_fill_cmd_header(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *pkt)
{
	FW_TXD *fw_txd = NULL;
	PNDIS_PACKET net_pkt = (PNDIS_PACKET) pkt;

	fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(FW_TXD));
	mtd_fill_txd_header(msg, net_pkt);
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
	else
		fw_txd->fw_txd_2.field.ucS2DIndex = HOST2CR4;

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

UINT32 mtd_rxv_handler(RTMP_ADAPTER *pAd, RX_BLK *rx_blk, VOID *rx_packet)
{
	RMAC_RXD_0_TXRXV *rxv = (RMAC_RXD_0_TXRXV *)(rx_packet);
	UCHAR *ptr;
	INT idx;

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "RxV Report: Number=%d, ByteCnt=%d\n",
			  rxv->RxvCnt, rxv->RxByteCnt);
#if (ENABLE_RXD_LOG)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RxV Report: Number=%d, ByteCnt=%d\n",
			  rxv->RxvCnt, rxv->RxByteCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_0_RxV: 0x%x\n", *((UINT32 *)rxv + 0)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_1_RxV: 0x%x\n", *((UINT32 *)rxv + 1)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_2_RxV: 0x%x\n", *((UINT32 *)rxv + 2)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_3_RxV: 0x%x\n", *((UINT32 *)rxv + 3)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_4_RxV: 0x%x\n", *((UINT32 *)rxv + 4)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_5_RxV: 0x%x\n", *((UINT32 *)rxv + 5)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_6_RxV: 0x%x\n", *((UINT32 *)rxv + 6)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_7_RxV: 0x%x\n", *((UINT32 *)rxv + 7)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_8_RxV: 0x%x\n", *((UINT32 *)rxv + 8)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_9_RxV: 0x%x\n", *((UINT32 *)rxv + 9)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_10_RxV: 0x%x\n", *((UINT32 *)rxv + 10)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RMAC_RXD_11_RxV: 0x%x\n", *((UINT32 *)rxv + 11)));
#endif

	if (rxv->RxByteCnt != (rxv->RxvCnt * 44 + 4)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("ReceivedByteCnt not equal rxv_entry required!\n"));
	} else {
		ptr = (UCHAR *)(rx_packet + 4);
#ifdef RT_BIG_ENDIAN

		if ((rxv->RxByteCnt - 4) > 0)
			MTMacInfoEndianChange(pAd, ptr, TYPE_RMACINFO, rxv->RxByteCnt);

#endif /* RT_BIG_ENDIAN */

		for (idx = 0; idx < rxv->RxvCnt; idx++) {
			chip_parse_rxv_packet(pAd, RMAC_RX_PKT_TYPE_RX_TXRXV, rx_blk, ptr);
			ptr += 44;
		}
	}

	return TRUE;
}

#ifdef ERR_RECOVERY
VOID mtd_dump_ser_stat(RTMP_ADAPTER *pAd, UINT8 dump_lvl)
{
	UINT32 reg_tmp_val;

	MAC_IO_READ32(pAd->hdev_ctrl, MCU_COM_REG1, &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, MCU_COM_REG1=0x%08X\n", __func__, reg_tmp_val));

	if (reg_tmp_val == MCU_COM_REG1_SER_PSE) {
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe064, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c064=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe068, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c068=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe06C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c06C=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x8244, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x82060244=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x8248, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x82060248=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x8258, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x82060258=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x825C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206025C=0x%08X\n", __func__, reg_tmp_val));
	} else if (reg_tmp_val == MCU_COM_REG1_SER_LMAC_TX) {
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe064, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c064=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe068, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c068=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe06C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c06C=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0xe070, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x8206c070=0x%08X\n", __func__, reg_tmp_val));
	} else if (reg_tmp_val == MCU_COM_REG1_SER_SEC_RF_RX) {
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21620, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6020=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21624, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6024=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21628, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6028=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x2162C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F602C=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21630, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6030=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21634, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6034=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x21638, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F6038=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x2163C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F603C=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20824, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1024=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20924, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1124=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20a24, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1224=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20b24, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1324=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20820, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1020=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20920, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1120=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20a20, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1220=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20b20, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1320=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20840, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1040=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20844, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1044=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20848, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1048=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x2084C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F104C=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20940, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1140=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20944, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1144=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x20948, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F1148=0x%08X\n", __func__, reg_tmp_val));
		MAC_IO_READ32(pAd->hdev_ctrl, 0x2094C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s,::E  R	, 0x820F114C=0x%08X\n", __func__, reg_tmp_val));
	}

	MAC_IO_READ32(pAd->hdev_ctrl, (0xc1e4), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, 0x820681e4=0x%08X\n", __func__, reg_tmp_val));
	MAC_IO_READ32(pAd->hdev_ctrl, (0xc1e8), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, 0x820681e8=0x%08X\n", __func__, reg_tmp_val));
	MAC_IO_READ32(pAd->hdev_ctrl, (0xc2e8), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, 0x820682e8=0x%08X\n", __func__, reg_tmp_val));
	MAC_IO_READ32(pAd->hdev_ctrl, (0xc2ec), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, 0x820682ec=0x%08X\n", __func__, reg_tmp_val));
	MAC_IO_READ32(pAd->hdev_ctrl, (0x20afc), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s,::E  R	, 0x820f20fc=0x%08X\n", __func__, reg_tmp_val));

	/* Add the EDCCA Time for Debug */
	Show_MibBucket_Proc(pAd, "");
}
#endif
#ifdef TX_POWER_CONTROL_SUPPORT
VOID mtd_txpower_boost(struct _RTMP_ADAPTER *pAd, UCHAR ucBandIdx)
{
	/* config Power boost table via profile */
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V0_CCK,
				&(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpCck),
				SINGLE_SKU_FILL_TABLE_CCK_LENGTH_V0);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V0_OFDM,
				&(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpOfdm),
				SINGLE_SKU_FILL_TABLE_OFDM_LENGTH_V0);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V0_HT20,
				&(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpHt20),
				SINGLE_SKU_FILL_TABLE_HT20_LENGTH_V0);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V0_HT40,
				&(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpHt40),
				SINGLE_SKU_FILL_TABLE_HT40_LENGTH_V0);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V0_VHT20,
				&(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpVht20),
				SINGLE_SKU_FILL_TABLE_VHT20_LENGTH_V0);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V0_VHT40,
				&(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpVht40),
				SINGLE_SKU_FILL_TABLE_VHT40_LENGTH_V0);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V0_VHT80,
				&(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpVht80),
				SINGLE_SKU_FILL_TABLE_VHT80_LENGTH_V0);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_V0_VHT160,
				&(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpVht160),
				SINGLE_SKU_FILL_TABLE_VHT160_LENGTH_V0);
}

VOID mtd_txpower_boost_ctrl(struct _RTMP_ADAPTER *pAd, UCHAR ucBandIdx, CHAR cPwrUpCat, PUCHAR pcPwrUpValue)
{
	switch (cPwrUpCat) {
	case POWER_UP_CATE_V0_CCK:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpCck, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_CCK_LENGTH_V0);
		break;

	case POWER_UP_CATE_V0_OFDM:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpOfdm, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_OFDM_LENGTH_V0);
		break;

	case POWER_UP_CATE_V0_HT20:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpHt20, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_HT20_LENGTH_V0);
		break;

	case POWER_UP_CATE_V0_HT40:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpHt40, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_HT40_LENGTH_V0);
		break;

	case POWER_UP_CATE_V0_VHT20:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpVht20, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_VHT20_LENGTH_V0);
		break;

	case POWER_UP_CATE_V0_VHT40:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpVht40, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_VHT40_LENGTH_V0);
		break;

	case POWER_UP_CATE_V0_VHT80:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpVht80, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_VHT80_LENGTH_V0);
		break;

	case POWER_UP_CATE_V0_VHT160:
		os_move_mem(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpVht160, pcPwrUpValue, sizeof(CHAR) * SINGLE_SKU_FILL_TABLE_VHT160_LENGTH_V0);
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
	}
}

BOOLEAN mtd_txpower_boost_info(struct _RTMP_ADAPTER *pAd, POWER_BOOST_TABLE_CATEGORY_V0 ePowerBoostRateType)
{
	switch (ePowerBoostRateType) {
	case POWER_UP_CATE_V0_CCK:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CCK\n (M0)-(M1)-(M2)-(M3)\n"));
		break;

	case POWER_UP_CATE_V0_OFDM:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("OFDM\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)\n"));
		break;

	case POWER_UP_CATE_V0_HT20:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT20\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)\n"));
		break;

	case POWER_UP_CATE_V0_HT40:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT40\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)\n"));
		break;

	case POWER_UP_CATE_V0_VHT20:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT20\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)\n"));
		break;

	case POWER_UP_CATE_V0_VHT40:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT40\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)\n"));
		break;

	case POWER_UP_CATE_V0_VHT80:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT80\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)\n"));
		break;

	case POWER_UP_CATE_V0_VHT160:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT160\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("(M0)-(M1)-(M2)-(M3)-(M4)-(M5)-(M6)-(M7)-(M8)-(M9)\n"));
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

BOOLEAN mtd_txpower_boost_power_cat_type(struct _RTMP_ADAPTER *pAd, UINT8 u1PhyMode, UINT8 u1Bw, PUINT8 pu1PowerBoostRateType)
{
	UINT32 u4PwrUpCat;

	if (u1PhyMode > 3) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid PhyMode!! Please Enter PhyMode as CCK:0, OFDM:1, HT:2, VHT:3\n", __func__));
		return FALSE;
	}

	if ((u1PhyMode == 0 || u1PhyMode == 1) && (u1Bw != 0)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid BW!! For CCK/OFDM mode valid BW option is 0\n", __func__));
		return FALSE;
	} else if ((u1PhyMode == 2) && (u1Bw > 1)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid BW!! For HT mode valid BW option is 0/1\n", __func__));
		return FALSE;
	} else if ((u1PhyMode == 3) && (u1Bw > 3)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid BW!! For VHT mode valid BW option is 0/1/2/3\n", __func__));
		return FALSE;
	}

	if (u1PhyMode == 3) {
		u4PwrUpCat = POWER_UP_CATE_V1_VHT20 + u1Bw;
	} else {
		u4PwrUpCat = u1PhyMode + u1Bw;
	}

	*pu1PowerBoostRateType = u4PwrUpCat;
	return TRUE;
}

BOOLEAN mtd_txpower_boost_rate_type(struct _RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 u1PowerBoostRateType)
{
	UINT_8 ucRateIdx, i, u1Temp = 0;
	PCHAR pcTxPwrBoost;
	UINT8 u1Length[POWER_UP_CATE_V0_NUM] = {0, SINGLE_SKU_FILL_TABLE_CCK_LENGTH_V0, SINGLE_SKU_FILL_TABLE_OFDM_LENGTH_V0, SINGLE_SKU_FILL_TABLE_HT20_LENGTH_V0,
					SINGLE_SKU_FILL_TABLE_HT40_LENGTH_V0, SINGLE_SKU_FILL_TABLE_VHT20_LENGTH_V0, SINGLE_SKU_FILL_TABLE_VHT40_LENGTH_V0,
					SINGLE_SKU_FILL_TABLE_VHT80_LENGTH_V0};

	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (u1PowerBoostRateType > POWER_UP_CATE_V0_NUM)
		return FALSE;

	pcTxPwrBoost = &(pAd->CommonCfg.PowerBoostParamV0[ucBandIdx].cPowerUpCck[0]);

	for(i = 0; i <= u1PowerBoostRateType; i++) {
		u1Temp = u1Temp + u1Length[i];
	}

	pcTxPwrBoost = pcTxPwrBoost + u1Temp;

	if (arch_ops && arch_ops->arch_txpower_boost_info_V0) {
	if (!arch_ops->arch_txpower_boost_info_V0(pAd, u1PowerBoostRateType))
		return FALSE;

	for (ucRateIdx = 0; ucRateIdx < cap->single_sku_fill_tbl_length[u1PowerBoostRateType]; ucRateIdx++, pcTxPwrBoost++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%2d) ", *pcTxPwrBoost));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-------------------------------------------------------\n"));
	}
	return TRUE;
}

#endif
