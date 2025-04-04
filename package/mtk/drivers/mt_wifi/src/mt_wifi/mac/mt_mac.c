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

static const UCHAR wmm_swq_2_hw_ac_queue[18] = {
	Q_IDX_AC0, /* QID_AC_BK */
	Q_IDX_AC1, /* QID_AC_BE */
	Q_IDX_AC2, /* QID_AC_VI */
	Q_IDX_AC3, /* QID_AC_VO */
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	17,
};

VOID mt_dump_rxinfo(RTMP_ADAPTER *pAd, UCHAR *rxinfo)
{
	RXINFO_STRUC *pRxInfo = (RXINFO_STRUC *) rxinfo;

	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(RXINFO_STRUC));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxInfo Fields:\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBA=%d\n", pRxInfo->BA));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDATA=%d\n", pRxInfo->DATA));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tNULLDATA=%d\n", pRxInfo->NULLDATA));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFRAG=%d\n", pRxInfo->FRAG));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tU2M=%d\n", pRxInfo->U2M));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMcast=%d\n", pRxInfo->Mcast));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBcast=%d\n", pRxInfo->Bcast));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMyBss=%d\n", pRxInfo->MyBss));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCrc=%d\n", pRxInfo->Crc));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCipherErr=%d\n", pRxInfo->CipherErr));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMSDU=%d\n", pRxInfo->AMSDU));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHTC=%d\n", pRxInfo->HTC));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRSSI=%d\n", pRxInfo->RSSI));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tL2PAD=%d\n", pRxInfo->L2PAD));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAMPDU=%d\n", pRxInfo->AMPDU));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDecrypted=%d\n", pRxInfo->Decrypted));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBssIdx3=%d\n", pRxInfo->BssIdx3));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twapi_kidx=%d\n", pRxInfo->wapi_kidx));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpn_len=%d\n", pRxInfo->pn_len));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_fc_type0=%d\n", pRxInfo->sw_fc_type0));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsw_fc_type1=%d\n", pRxInfo->sw_fc_type1));
}

static char *hdr_fmt_str[] = {
	"Non-80211-Frame",
	"Command-Frame",
	"Normal-80211-Frame",
	"enhanced-80211-Frame",
};


static char *p_idx_str[] = {"LMAC", "MCU"};
static char *q_idx_lmac_str[] = {"AC0", "AC1", "AC2", "AC3", "AC4", "AC5", "AC6",
								 "BCN", "BMC",
								 "AC10", "AC11", "AC12", "AC13", "AC14",
								 "Invalid"
								};
static char *q_idx_mcu_str[] = {"RQ0", "RQ1", "RQ2", "RQ3", "Invalid"};
#ifndef COMPOS_WIN
VOID dump_tmac_info(RTMP_ADAPTER *pAd, UCHAR *tmac_info)
{
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)tmac_info;
	TMAC_TXD_0 *txd_0 = (TMAC_TXD_0 *)tmac_info;
	TMAC_TXD_1 *txd_1 = (TMAC_TXD_1 *)(tmac_info + sizeof(TMAC_TXD_0));
	UCHAR q_idx = 0, pIdx = (UCHAR)txd_0->PIdx;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	hex_dump("TMAC_Info Raw Data: ", (UCHAR *)tmac_info, cap->TXWISize);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_TXD Fields:\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_0:\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPortID=%d(%s)\n", pIdx,
			 pIdx < 2 ? p_idx_str[pIdx] : "Invalid"));

	if (pIdx == P_IDX_LMAC)
		q_idx = txd_0->QIdx % 0xf;
	else
		q_idx = txd_0->QIdx % 0x5;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tQueID=%d(%s %s)\n", txd_0->QIdx,
			 (pIdx == P_IDX_LMAC ? "LMAC" : "MCU"),
			 pIdx == P_IDX_LMAC ? q_idx_lmac_str[q_idx] : q_idx_mcu_str[q_idx]));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxByteCnt=%d\n", txd_0->TxByteCount));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_1:\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWlan_idx=%d\n", txd_1->WlanIdx));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrInfo=0x%x\n", txd_1->HdrInfo));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrFmt=%d(%s)\n",
			 txd_1->HdrFmt,
			 txd_1->HdrFmt < 4 ? hdr_fmt_str[txd_1->HdrFmt] : "N/A"));

	switch (txd_1->HdrFmt) {
	case TMI_HDR_FT_NON_80211:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMRD=%d, EOSP=%d, RMVL=%d, VLAN=%d, ETYP=%d\n",
				 txd_1->HdrInfo & (1 << TMI_HDR_INFO_0_BIT_MRD),
				 txd_1->HdrInfo & (1 << TMI_HDR_INFO_0_BIT_EOSP),
				 txd_1->HdrInfo & (1 << TMI_HDR_INFO_0_BIT_RMVL),
				 txd_1->HdrInfo & (1 << TMI_HDR_INFO_0_BIT_VLAN),
				 txd_1->HdrInfo & (1 << TMI_HDR_INFO_0_BIT_ETYP)));
		break;

	case TMI_HDR_FT_CMD:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tRsvd=0x%x\n", txd_1->HdrInfo));
		break;

	case TMI_HDR_FT_NOR_80211:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tHeader Len=%d(WORD)\n",
				 txd_1->HdrInfo & TMI_HDR_INFO_2_MASK_LEN));
		break;

	case TMI_HDR_FT_ENH_80211:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tEOSP=%d, AMS=%d\n",
				 txd_1->HdrInfo & (1 << TMI_HDR_INFO_3_BIT_EOSP),
				 txd_1->HdrInfo & (1 << TMI_HDR_INFO_3_BIT_AMS)));
		break;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFormatType=%d(%s format)\n", txd_1->TxDFmt,
			 txd_1->TxDFmt == TMI_FT_LONG ? "Long - 8 DWORD" : "Short - 3 DWORD"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrPad=%d\n", txd_1->HdrPad));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNoAck=%d\n", txd_1->NoAck));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTID=%d\n", txd_1->Tid));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tProtect Frame=%d\n", txd_1->ProtectFrame));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\town_mac=%d\n", txd_1->OwnMacAddr));

	if (txd_s->TxD1.TxDFmt == TMI_FT_LONG) {
		TMAC_TXD_L *txd_l = (TMAC_TXD_L *)tmac_info;
		TMAC_TXD_2 *txd_2 = &txd_l->TxD2;
		TMAC_TXD_3 *txd_3 = &txd_l->TxD3;
		TMAC_TXD_4 *txd_4 = &txd_l->TxD4;
		TMAC_TXD_5 *txd_5 = &txd_l->TxD5;
		TMAC_TXD_6 *txd_6 = &txd_l->TxD6;

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_2:\n"));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tReamingLife/MaxTx time=%d\n", txd_2->MaxTxTime));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNDP=%d\n", txd_2->Ndp));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNDPA=%d\n", txd_2->Ndpa));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSounding=%d\n", txd_2->Sounding));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbc_mc_pkt=%d\n", txd_2->BmcPkt));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBIP=%d\n", txd_2->Bip));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tDuration=%d\n", txd_2->Duration));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHE(HTC Exist)=%d\n", txd_2->HtcExist));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFRAG=%d\n", txd_2->Frag));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpwr_offset=%d\n", txd_2->PwrOffset));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tba_disable=%d\n", txd_2->BaDisable));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttiming_measure=%d\n", txd_2->TimingMeasure));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfix_rate=%d\n", txd_2->FixRate));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttype(%d)\n", txd_2->FrmType));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsubtype(%d)\n", txd_2->SubType));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_3:\n"));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_cnt=%d\n", txd_3->TxCnt));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tremain_tx_cnt=%d\n", txd_3->RemainTxCnt));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsn=%d\n", txd_3->Sn));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_vld=%d\n", txd_3->PnVld));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tsn_vld=%d\n", txd_3->SnVld));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_4:\n"));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_low=0x%x\n", txd_4->PktNumLow));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_5:\n"));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpwr_mgmt=%d\n", txd_5->PwrMgmt));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_2_host=%d\n", txd_5->TxS2Host));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_2_mcu=%d\n", txd_5->TxS2Mcu));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_status_fmt=%d\n", txd_5->TxSFmt));

		if (txd_5->TxS2Host || txd_5->TxS2Mcu)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpid=%d\n", txd_5->PktId));

		if (txd_2->FixRate)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tda_select=%d\n", txd_5->DataSrcSelect));

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpn_high=0x%x\n", txd_5->PktNumHigh));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTMAC_TXD_6:\n"));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_rate=0x%x\n", txd_6->RateToBeFixed));

		if (txd_2->FixRate) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tfix_rate_mode=%d\n", txd_6->FixedRateMode));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tant_id=%d\n", txd_6->AntIdx));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbw=%d\n", txd_6->FixedBwMode));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tspe_en=%d\n", txd_6->SpeExtEnable));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tant_pri=%d\n", txd_6->AntPri));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tdyn_bw=%d\n", txd_6->DynamicBw));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tETxBF=%d\n", txd_6->ExplicitBf));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tITxBF=%d\n", txd_6->ImplicitBf));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tldpc=%d(%s)\n", txd_6->LDPC, txd_6->LDPC == 0 ? "BCC" : "LDPC"));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGI=%d(%s)\n", txd_6->GI, txd_6->GI == 0 ? "LONG" : "SHORT"));
		}
	}
}
#endif /* COMPOS_WIN */


static char *rmac_info_type_str[] = {
	"TXS",
	"RXV",
	"RxNormal",
	"DupRFB",
	"TMR",
	"Unknown",
};

static inline char *rxd_pkt_type_str(INT pkt_type)
{
	if (pkt_type >= 0 && pkt_type <= 0x04)
		return rmac_info_type_str[pkt_type];
	else
		return rmac_info_type_str[5];
}


VOID dump_rmac_info_normal(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;

	hex_dump("RMAC_Info Raw Data: ", rmac_info, sizeof(RXD_BASE_STRUCT));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxData_BASE:\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPktType=%d(%s)\n",
			 rxd_base->RxD0.PktType,
			 rxd_pkt_type_str(rxd_base->RxD0.PktType)));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGroupValid=%x\n", rxd_base->RxD0.RfbGroupVld));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRxByteCnt=%d\n", rxd_base->RxD0.RxByteCnt));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tIP/UT=%d/%d\n", rxd_base->RxD0.IpChkSumOffload, rxd_base->RxD0.UdpTcpChkSumOffload));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEtherTypeOffset=%d(WORD)\n", rxd_base->RxD0.EthTypeOffset));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHTC/UC2ME/MC/BC=%d/%d/%d/%d\n",
			 rxd_base->RxD1.HTC, rxd_base->RxD1.UcastToMe,
			 rxd_base->RxD1.Mcast, rxd_base->RxD1.Bcast));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBeacon with BMCast/Ucast=%d/%d\n",
			 rxd_base->RxD1.BcnWithBMcst, rxd_base->RxD1.BcnWithUCast));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tKeyID=%d\n", rxd_base->RxD1.KeyId));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tChFrequency=%x\n", rxd_base->RxD1.ChFreq));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdearLength(MAC)=%d\n", rxd_base->RxD1.MacHdrLen));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrOffset(HO)=%d\n", rxd_base->RxD1.HdrOffset));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrTranslation(H)=%d\n", rxd_base->RxD1.HdrTranslation));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPayloadFormat(PF)=%d\n", rxd_base->RxD1.PayloadFmt));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBSSID=%d\n", rxd_base->RxD1.RxDBssidIdx));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWlanIndex=%d\n", rxd_base->RxD2.RxDWlanIdx));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTID=%d\n", rxd_base->RxD2.RxDTid));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSEC Mode=%d\n", rxd_base->RxD2.SecMode));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSW BIT=%d\n", rxd_base->RxD2.SwBit));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFCE Error(FC)=%d\n", rxd_base->RxD2.FcsErr));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCipher Mismatch(CM)=%d\n", rxd_base->RxD2.CipherMis));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCipher Length Mismatch(CLM)=%d\n", rxd_base->RxD2.CipherLenMis));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tICV Err(I)=%d\n", rxd_base->RxD2.IcvErr));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTKIP MIC Err(T)=%d\n", rxd_base->RxD2.TkipMicErr));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tLength Mismatch(LM)=%d\n", rxd_base->RxD2.LenMis));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tDeAMSDU Fail(DAF)=%d\n", rxd_base->RxD2.DeAmsduFail));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tExceedMax Rx Length(EL)=%d\n", rxd_base->RxD2.ExMaxRxLen));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tLLC-SNAP mismatch(LLC-MIS, for HdrTrans)=%d\n", rxd_base->RxD2.LlcMis));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tUndefined VLAN tagged Type(UDF_VLT)=%d\n", rxd_base->RxD2.UdfVlanType));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFragment Frame(FRAG)=%d\n", rxd_base->RxD2.FragFrm));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNull Frame(NULL)=%d\n", rxd_base->RxD2.NullFrm));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon Data Frame(NDATA)=%d\n", rxd_base->RxD2.NonDataFrm));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon-AMPDU Subframe(NASF)=%d\n", rxd_base->RxD2.NonAmpduSfrm));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNon AMPDU(NAMP)=%d\n", rxd_base->RxD2.NonAmpduFrm));
}

VOID mt_dump_rmac_info_for_ICVERR(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;
	union _RMAC_RXD_0_UNION *rxd_0;
	UINT32 pkt_type;

	if (!IS_HIF_TYPE(pAd, HIF_MT))
		return;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);

	if (pkt_type != RMAC_RX_PKT_TYPE_RX_NORMAL)
		return;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tHTC/UC2ME/MC/BC=%d/%d/%d/%d",
			 rxd_base->RxD1.HTC, rxd_base->RxD1.UcastToMe,
			 rxd_base->RxD1.Mcast, rxd_base->RxD1.Bcast));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (", WlanIndex=%d", rxd_base->RxD2.RxDWlanIdx));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (", SEC Mode=%d\n", rxd_base->RxD2.SecMode));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tFCE Error(FC)=%d", rxd_base->RxD2.FcsErr));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (", CM=%d", rxd_base->RxD2.CipherMis));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (", CLM=%d", rxd_base->RxD2.CipherLenMis));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (", I=%d", rxd_base->RxD2.IcvErr));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (", T=%d", rxd_base->RxD2.TkipMicErr));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (", LM=%d\n", rxd_base->RxD2.LenMis));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tFragment Frame(FRAG)=%d\n", rxd_base->RxD2.FragFrm));
}



static VOID dump_rmac_info_txs(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* TXS_FRM_STRUC *txs_frm = (TXS_FRM_STRUC *)rmac_info; */
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)rmac_info;

	hex_dump("RMAC_Info Raw Data: ", rmac_info, sizeof(RXD_BASE_STRUCT));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxData_BASE:\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPktType=%d(%s)\n",
			 rxd_base->RxD0.PktType,
			 rxd_pkt_type_str(rxd_base->RxD0.PktType)));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tGroupValid=%x\n", rxd_base->RxD0.RfbGroupVld));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRxByteCnt=%d\n", rxd_base->RxD0.RxByteCnt));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tIP/UT=%d/%d\n", rxd_base->RxD0.IpChkSumOffload, rxd_base->RxD0.UdpTcpChkSumOffload));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEtherTypeOffset=%d(WORD)\n", rxd_base->RxD0.EthTypeOffset));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHTC/UC2ME/MC/BC=%d/%d/%d/%d\n",
			 rxd_base->RxD1.HTC, rxd_base->RxD1.UcastToMe,
			 rxd_base->RxD1.Mcast, rxd_base->RxD1.Bcast));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBeacon with BMCast/Ucast=%d/%d\n",
			 rxd_base->RxD1.BcnWithBMcst, rxd_base->RxD1.BcnWithUCast));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tKeyID=%d\n", rxd_base->RxD1.KeyId));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tChFrequency=%x\n", rxd_base->RxD1.ChFreq));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdearLength(MAC)=%d\n", rxd_base->RxD1.MacHdrLen));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrOffset(HO)=%d\n", rxd_base->RxD1.HdrOffset));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHdrTranslation(H)=%d\n", rxd_base->RxD1.HdrTranslation));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPayloadFormat(PF)=%d\n", rxd_base->RxD1.PayloadFmt));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBSSID=%d\n", rxd_base->RxD1.RxDBssidIdx));
}


static VOID dump_rmac_info_rxv(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* RXV_FRM_STRUC *rxv_frm = (RXV_FRM_STRUC *)rmac_info; */
}


static VOID dump_rmac_info_rfb(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* RXD_BASE_STRUCT *rfb_frm = (RXD_BASE_STRUCT *)rmac_info; */
}


static VOID dump_rmac_info_tmr(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	/* TMR_FRM_STRUC *rxd_base = (TMR_FRM_STRUC *)rmac_info; */
}


VOID dump_rmac_info(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	union _RMAC_RXD_0_UNION *rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	INT pkt_type;

	rxd_0 = (union _RMAC_RXD_0_UNION *)rmac_info;
	pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RMAC_RXD Header Format :%s\n",
			 rxd_pkt_type_str(pkt_type)));

	switch (pkt_type) {
	case RMAC_RX_PKT_TYPE_RX_TXS:
		dump_rmac_info_txs(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_TXRXV:
		dump_rmac_info_rxv(pAd, rmac_info);
		break;

	case RMAC_RX_PKT_TYPE_RX_NORMAL:
		dump_rmac_info_normal(pAd, rmac_info);
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



VOID mt_dump_txs(struct _RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->TxSD0;
	TXS_D_1 *txs_d1 = &txs_entry->TxSD1;
	TXS_D_2 *txs_d2 = &txs_entry->TxSD2;
	TXS_D_3 *txs_d3 = &txs_entry->TxSD3;
	TXS_D_4 *txs_d4 = &txs_entry->TxSD4;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\t(TXSFM=%d, TXS2M=%d, TXS2H=%d)\n", txs_d0->TxSFmt, txs_d0->TxS2M, txs_d0->TxS2H));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tFR=%d, TxRate=0x%x\n", txs_d0->TxS_FR, txs_d0->TxRate));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tME=%d, RE=%d, LE=%d, BE=%d, TxOPLimitErr=%d\n",
			  txs_d0->ME, txs_d0->RE, txs_d0->LE, txs_d0->BE, txs_d0->TxOp));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tPS=%d, BA Fail=%d, tid=%d, Ant_Id=%d\n",
			  txs_d0->PSBit, txs_d0->BAFail, txs_d0->Tid, txs_d0->AntId));

	if (Format == TXS_FORMAT0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tTimeStamp=0x%x, FrontTime=0x%x(unit 32us)\n",
				  txs_d1->TimeStamp, txs_d2->field_ft.FrontTime));
	} else if (Format == TXS_FORMAT1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tNoise0=0x%x, Noise1=0x%x, Noise2=0x%x\n",
				  txs_d1->FieldNoise.Noise0, txs_d1->FieldNoise.Noise1,
				  txs_d1->FieldNoise.Noise2));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t\tRCPI0=0x%x, RCPI1=0x%x, RCPI2=0x%x\n",
				  txs_d2->field_rcpi.RCPI0, txs_d2->field_rcpi.RCPI1,
				  txs_d2->field_rcpi.RCPI2));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tTxPwr(dbm)=0x%x\n", txs_d2->field_ft.TxPwrdBm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tTxDelay=0x%x(unit 32us), RxVSeqNum=0x%x, Wlan Idx=0x%x\n",
			  txs_d3->TxDelay, txs_d3->RXV_SN, txs_d3->TxS_WlanIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t\tSN=0x%x, TxBW=0x%x, AMPDU=%d, Final MPDU=%d PID=0x%x, MPDU TxCnt=%d, MCS Idx=%d\n",
			  txs_d4->TxS_SN_TSSI, txs_d4->TxS_TxBW, txs_d4->TxS_AMPDU, txs_d4->TxS_FianlMPDU,
			  txs_d4->TxS_Pid, txs_d4->TxS_MpduTxCnt, txs_d4->TxS_LastTxMcsIdx));
}

static VOID DumpPseFrameInfo(RTMP_ADAPTER *pAd, UINT8 PID, UINT8 QID)
{
	UINT32 FirstFID, CurFID, NextFID, FrameNums = 0;
	UINT32 Value;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PID = %d, QID = %d\n", PID, QID));
	HW_IO_READ32(pAd->hdev_ctrl, C_GFF, &Value);
	Value &= ~GET_QID_MASK;
	Value |= SET_GET_QID(QID);
	Value &= ~GET_PID_MASK;
	Value |= SET_GET_PID(PID);
	HW_IO_WRITE32(pAd->hdev_ctrl, C_GFF, Value);
	HW_IO_READ32(pAd->hdev_ctrl, C_GFF, &Value);
	FirstFID = GET_FIRST_FID(Value);

	if (FirstFID == 0xfff) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("FrameNums = %d\n", FrameNums));
		return;
	}

	CurFID = FirstFID;
	FrameNums++;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("FID = %d", CurFID));

	while (1) {
		HW_IO_READ32(pAd->hdev_ctrl, C_GF, &Value);
		Value &= ~CURR_FID_MASK;
		Value |= SET_CURR_FID(CurFID);
		HW_IO_WRITE32(pAd->hdev_ctrl, C_GF, Value);
		HW_IO_READ32(pAd->hdev_ctrl, C_GF, &Value);
		NextFID = GET_RETURN_FID(Value);

		if (NextFID == 0xfff) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nFrameNums = %d\n", FrameNums));
			return;
		} else {
			CurFID = NextFID;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (", FID = %d\n", CurFID));
			FrameNums++;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nFrameNums = %d\n", FrameNums));
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
VOID mt_update_raw_counters(RTMP_ADAPTER *pAd)
{
	UINT32 OldValue, ampdu_range_cnt[4];
	UINT32 mac_val, rx_err_cnt, fcs_err_cnt;
	/* UINT32 TxSuccessCount = 0, TxRetryCount = 0; */
#ifdef CONFIG_STA_SUPPORT
	UINT8 i = 0;
#endif
#ifdef DBG_DIAGNOSE
	UINT32 bss_tx_cnt;
	UINT32 TxFailCount = 0;
#endif /* DBG_DIAGNOSE */
#ifdef COMPOS_WIN
	COUNTER_MTK *pPrivCounters;
#else
	COUNTER_RALINK *pPrivCounters;
#endif
	COUNTER_802_11 *wlanCounter;
	COUNTER_802_3 *dot3Counters;
	/* for PER debug */
	UINT32 AmpduTxCount = 0;
	UINT32 AmpduTxSuccessCount = 0;
#ifndef COMPOS_TESTMODE_WIN

#ifdef CONFIG_STA_SUPPORT
	for (i = 0; i < pAd->MSTANum; i++)
		if ((pAd->StaCfg[i].wdev.DevInfo.WdevActive) && (pAd->StaCfg[i].PwrMgmt.bDoze)) {
			MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%d): H/W in PM4, return\n", __LINE__);
			return;
		}
#endif /* CONFIG_STA_SUPPORT */

#endif /* COMPOS_TESTMODE_WIN */
#ifdef COMPOS_WIN
	wlanCounter = &pAd->Counter.WlanCounters;
	pPrivCounters = &pAd->Counter.MTKCounters;
	dot3Counters = &pAd->Counter.Counters8023;
#else
	pPrivCounters = &pAd->RalinkCounters;
	wlanCounter = pAd->WlanCounters;
	dot3Counters = &pAd->Counters8023;
#endif /* COMPOS_WIN */
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_MSDR14, &AmpduTxCount);
	AmpduTxCount &= 0xFFFFFF;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_MSDR15, &AmpduTxSuccessCount);
	AmpduTxSuccessCount &= 0xFFFFFF;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_MSDR4, &rx_err_cnt);
	fcs_err_cnt = (rx_err_cnt >> 16) & 0xffff;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_MDR2, &mac_val);
	ampdu_range_cnt[0] = mac_val & 0xffff;
	ampdu_range_cnt[1] =  (mac_val >> 16) & 0xffff;
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_MDR3, &mac_val);
	ampdu_range_cnt[2] = mac_val & 0xffff;
	ampdu_range_cnt[3] =  (mac_val >> 16) & 0xffff;
#ifdef DBG_DIAGNOSE
	MAC_IO_READ32(pAd->hdev_ctrl, MIB_MB0SDR1, &mac_val);
	TxFailCount = (mac_val >> 16) & 0xffff;
	/* TODO: shiang, now only check BSS0 */
	MAC_IO_READ32(pAd->hdev_ctrl, WTBL_BTCRn, &bss_tx_cnt);
	bss_tx_cnt = (bss_tx_cnt >> 16) & 0xffff;
#endif /* DBG_DIAGNOSE */
#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters[0].AmpduSuccessCount.u.LowPart += AmpduTxSuccessCount;
	pAd->WlanCounters[0].AmpduFailCount.u.LowPart += (AmpduTxCount - AmpduTxSuccessCount);
#endif /* STATS_COUNT_SUPPORT */
	pPrivCounters->OneSecRxFcsErrCnt += fcs_err_cnt;
#ifdef STATS_COUNT_SUPPORT
	/* Update FCS counters*/
	OldValue = pAd->WlanCounters[0].FCSErrorCount.u.LowPart;
	pAd->WlanCounters[0].FCSErrorCount.u.LowPart += fcs_err_cnt; /* >> 7);*/

	if (pAd->WlanCounters[0].FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters[0].FCSErrorCount.u.HighPart++;

#endif /* STATS_COUNT_SUPPORT */
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		pAd->ATECtrl.rx_stat.RxMacFCSErrCount += fcs_err_cnt;

#endif
	/* Add FCS error count to private counters*/
	pPrivCounters->OneSecRxFcsErrCnt += fcs_err_cnt;
	OldValue = pPrivCounters->RealFcsErrCount.u.LowPart;
	pPrivCounters->RealFcsErrCount.u.LowPart += fcs_err_cnt;

	if (pPrivCounters->RealFcsErrCount.u.LowPart < OldValue)
		pPrivCounters->RealFcsErrCount.u.HighPart++;

	dot3Counters->RxNoBuffer += (rx_err_cnt & 0xffff);
	wlanCounter->TxAggRange1Count.u.LowPart += ampdu_range_cnt[0];
	wlanCounter->TxAggRange2Count.u.LowPart += ampdu_range_cnt[1];
	wlanCounter->TxAggRange3Count.u.LowPart += ampdu_range_cnt[2];
	wlanCounter->TxAggRange4Count.u.LowPart += ampdu_range_cnt[3];
#ifdef DBG_DIAGNOSE
	{
		RtmpDiagStruct *pDiag;
		UINT8 ArrayCurIdx;
		struct dbg_diag_info *diag_info;

		pDiag = &pAd->DiagStruct;
		ArrayCurIdx = pDiag->ArrayCurIdx;

		if (pDiag->inited == 0) {
			os_zero_mem(pDiag, sizeof(struct _RtmpDiagStrcut_));
			pDiag->ArrayStartIdx = pDiag->ArrayCurIdx = 0;
			pDiag->wcid = 0;
			pDiag->inited = 1;
			pDiag->diag_cond = 0;
		} else {
			diag_info = &pDiag->diag_info[ArrayCurIdx];

			if ((pDiag->wcid == 0) && (pAd->MacTab.Size > 0)) {
				UINT16 idx;
				MAC_TABLE_ENTRY *pEntry;

				for (idx = 0; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++) {
					pEntry = &pAd->MacTab.Content[idx];

					if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
						pDiag->wcid = idx;
						MTWF_DBG_NP(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(): DBG_DIAGNOSE Start to monitor the SwQ depth of WCID[%d], ArrayCurIdx=%d\n",
								 pDiag->wcid, pDiag->ArrayCurIdx);
						break;
					}
				}
			}

			/* Tx*/
			diag_info->TxFailCnt = TxFailCount;
#ifdef DBG_TX_AGG_CNT
			diag_info->TxAggCnt = AmpduTxCount;
			diag_info->TxNonAggCnt = bss_tx_cnt - AmpduTxCount; /* only useful when only one BSS case */
			diag_info->TxAMPDUCnt[0] = ampdu_range_cnt[0];
			diag_info->TxAMPDUCnt[1] = ampdu_range_cnt[1];
			diag_info->TxAMPDUCnt[2] = ampdu_range_cnt[2];
			diag_info->TxAMPDUCnt[3] = ampdu_range_cnt[3];
#endif /* DBG_TX_AGG_CNT */
			diag_info->RxCrcErrCnt = fcs_err_cnt;
			INC_RING_INDEX(pDiag->ArrayCurIdx,  DIAGNOSE_TIME);
			ArrayCurIdx = pDiag->ArrayCurIdx;
			os_zero_mem(&pDiag->diag_info[ArrayCurIdx], sizeof(pDiag->diag_info[ArrayCurIdx]));

			if (pDiag->ArrayCurIdx == pDiag->ArrayStartIdx)
				INC_RING_INDEX(pDiag->ArrayStartIdx,  DIAGNOSE_TIME);
		}
	}
#endif /* DBG_DIAGNOSE */
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

UCHAR tmi_rate_map_cck_lp[] = {
	TMI_TX_RATE_CCK_1M_LP,
	TMI_TX_RATE_CCK_2M_LP,
	TMI_TX_RATE_CCK_5M_LP,
	TMI_TX_RATE_CCK_11M_LP,
};

UCHAR tmi_rate_map_cck_sp[] = {
	TMI_TX_RATE_CCK_2M_SP,
	TMI_TX_RATE_CCK_2M_SP,
	TMI_TX_RATE_CCK_5M_SP,
	TMI_TX_RATE_CCK_11M_SP,
};

UCHAR tmi_rate_map_ofdm[] = {
	TMI_TX_RATE_OFDM_6M,
	TMI_TX_RATE_OFDM_9M,
	TMI_TX_RATE_OFDM_12M,
	TMI_TX_RATE_OFDM_18M,
	TMI_TX_RATE_OFDM_24M,
	TMI_TX_RATE_OFDM_36M,
	TMI_TX_RATE_OFDM_48M,
	TMI_TX_RATE_OFDM_54M,
};


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


UINT16 tx_rate_to_tmi_rate(UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble)
{
	UINT16 tmi_rate = 0, mcs_id = 0;

	stbc = (stbc == TRUE) ? 1 : 0;

	switch (mode) {
	case MODE_CCK:
		if (preamble)
			mcs_id = tmi_rate_map_cck_lp[mcs];
		else
			mcs_id = tmi_rate_map_cck_sp[mcs];

		tmi_rate = (TMI_TX_RATE_MODE_CCK << TMI_TX_RATE_BIT_MODE) | (mcs_id);
		break;

	case MODE_OFDM:
		mcs_id = tmi_rate_map_ofdm[mcs];
		tmi_rate = (TMI_TX_RATE_MODE_OFDM << TMI_TX_RATE_BIT_MODE) | (mcs_id);
		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		tmi_rate = ((USHORT)(stbc << TMI_TX_RATE_BIT_STBC)) |
				   (((nss - 1) & TMI_TX_RATE_MASK_NSS) << TMI_TX_RATE_BIT_NSS) |
				   ((USHORT)(mode << TMI_TX_RATE_BIT_MODE)) |
				   ((USHORT)(mcs));
		/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): mode=%d, mcs=%d, stbc=%d converted tmi_rate=0x%x\n", */
		/* __FUNCTION__, mode, mcs, stbc, tmi_rate)); */
		break;

	case MODE_VHT:
		tmi_rate = TMI_TX_RATE_VHT_VAL(nss, mcs, stbc);
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid mode(mode=%d)\n",
				 __func__, mode));
		break;
	}

	return tmi_rate;
}


UCHAR get_nsts_by_mcs(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss)
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


static VOID MtWriteTMacInfo(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *buf,
	IN TMAC_INFO * TxInfo)
{
	TMAC_TXD_S *txd_s = (TMAC_TXD_S *)buf;
	TMAC_TXD_L *txd_l = (TMAC_TXD_L *)buf;
	TMAC_TXD_0 *txd_0 = &txd_s->TxD0;
	TMAC_TXD_1 *txd_1 = &txd_s->TxD1;
	TMAC_TXD_2 *txd_2 = NULL;
	TMAC_TXD_3 *txd_3 = NULL;
	TMAC_TXD_5 *txd_5 = NULL;
	TMAC_TXD_6 *txd_6 = NULL;
	UCHAR txd_size;
	UCHAR stbc = 0, bw = BW_20, mcs = 0, nss = 1, sgi = 0, phy_mode = 0, preamble = 1, ldpc = 0, expBf = 0, impBf = 0;
	UCHAR vht_nss = 1;
	TX_RADIO_SET_T *pTxRadioSet = NULL;
	/*DWORD 0*/
	txd_0->PIdx = TxInfo->PortIdx;
	txd_0->QIdx = TxInfo->QueIdx;

	if (TxInfo->QueIdx < 4)
		txd_0->QIdx = wmm_swq_2_hw_ac_queue[TxInfo->QueIdx];
	else {
		/* TODO: shiang-usw, consider about MCC case! */
		txd_0->QIdx = TxInfo->QueIdx;
	}

	/*DWORD 1*/
	txd_1->WlanIdx = TxInfo->Wcid;

	if (!TxInfo->NeedTrans) {
		txd_1->HdrFmt = TMI_HDR_FT_NOR_80211;
		TMI_HDR_INFO_VAL(txd_1->HdrFmt, 0, 0, 0, 0, 0, TxInfo->WifiHdrLen, 0, txd_1->HdrInfo);

		/* TODO: depends on QoS to decide if need to padding */
		if (TxInfo->HdrPad)
			txd_1->HdrPad = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | TxInfo->HdrPad;
	} else {
		txd_1->HdrFmt = TMI_HDR_FT_NON_80211;
		TMI_HDR_INFO_VAL(txd_1->HdrFmt, TxInfo->MoreData, TxInfo->Eosp, 1, TxInfo->VlanFrame, TxInfo->EtherFrame,
						 TxInfo->WifiHdrLen, 0, txd_1->HdrInfo);

		if (TxInfo->HdrPad)
			txd_1->HdrPad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | TxInfo->HdrPad;
	}

	txd_1->NoAck = (TxInfo->bAckRequired ? 0 : 1);
	txd_1->Tid = TxInfo->UserPriority;

	if (IS_CIPHER_NONE(TxInfo->CipherAlg))
		txd_1->ProtectFrame = 0;
	else
		txd_1->ProtectFrame = 1;

	txd_1->OwnMacAddr = TxInfo->OwnMacIdx;

	if (TxInfo->LongFmt == FALSE) {
		txd_1->TxDFmt = TMI_FT_SHORT;
		txd_size = sizeof(TMAC_TXD_S);
	} else {
		txd_2 = &txd_l->TxD2;
		txd_3 = &txd_l->TxD3;
		txd_5 = &txd_l->TxD5;
		txd_6 = &txd_l->TxD6;
		txd_1->TxDFmt = TMI_FT_LONG;
		txd_size = sizeof(TMAC_TXD_L);
		pTxRadioSet = &TxInfo->TxRadioSet;
		ldpc = pTxRadioSet->Ldpc;
		mcs = pTxRadioSet->RateCode;
		sgi = pTxRadioSet->ShortGI;

		if ((pAd->Antenna.field.TxPath == 1) && (TxInfo->FixRate))
			stbc = 0;
		else
			stbc = pTxRadioSet->Stbc;

		phy_mode = pTxRadioSet->PhyMode;
		bw = pTxRadioSet->CurrentPerPktBW;
		expBf = pTxRadioSet->EtxBFEnable;
		impBf = pTxRadioSet->ItxBFEnable;
		vht_nss = TxInfo->VhtNss ? TxInfo->VhtNss : 1;
		nss = get_nsts_by_mcs(phy_mode, mcs, stbc, vht_nss);
		/*DW2*/
		txd_2->MaxTxTime = TxInfo->MaxTxTime;
		txd_2->BmcPkt = TxInfo->BmcPkt;
		txd_2->FixRate = TxInfo->FixRate;
		txd_2->FrmType = TxInfo->FrmType;
		txd_2->SubType = TxInfo->SubType;

		if (TxInfo->NeedTrans)
			txd_2->HtcExist = 0;

		txd_2->Frag = TxInfo->FragIdx;
		txd_2->TimingMeasure = TxInfo->TimingMeasure;
		txd_2->BaDisable = TxInfo->BaDisable;
		/*DW3*/
		txd_3->RemainTxCnt = TxInfo->RemainTxCnt;
		txd_3->Sn = TxInfo->Sn;
		/*DW5*/
		txd_5->PktId = TxInfo->Pid;
		txd_5->TxSFmt = TxInfo->TxSFmt;
		txd_5->TxS2Host = TxInfo->TxS2Host;
		txd_5->TxS2Mcu = TxInfo->TxS2Mcu;

		if (TxInfo->NeedTrans)
			txd_5->DataSrcSelect = TMI_DAS_FROM_MPDU;

		txd_5->BarSsnCtrl = TxInfo->BarSsnCtrl;
		/* For  MT STA LP control, use H/W control mode for PM bit */
#if defined(CONFIG_STA_SUPPORT) && defined(CONFIG_PM_BIT_HW_MODE)
		txd_5->PwrMgmt = TMI_PM_BIT_CFG_BY_HW;
#else
		txd_5->PwrMgmt = TMI_PM_BIT_CFG_BY_SW;
#endif /* CONFIG_STA_SUPPORT && CONFIG_PM_BIT_HW_MODE */

		/* txd_5->PktNumHigh = 0; */

		/* DW6 */
		if (txd_2->FixRate) {
			txd_6->FixedRateMode = TMI_FIX_RATE_BY_TXD;
			txd_6->AntPri = TxInfo->AntPri;
			txd_6->SpeExtEnable = TxInfo->SpeEn;
			txd_6->FixedBwMode = ((1 << 2) | bw);
			txd_6->DynamicBw = 0;
			txd_6->ExplicitBf = expBf;
			txd_6->ImplicitBf = impBf;
			txd_6->LDPC = ldpc;
			txd_6->GI = sgi;

			if (txd_6->FixedRateMode == TMI_FIX_RATE_BY_TXD) {
				preamble = TxInfo->TxRadioSet.Premable;
				txd_6->RateToBeFixed = tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
			}
		}
	}

	txd_0->TxByteCount = txd_size + TxInfo->PktLen;
#ifdef CONFIG_CSO_SUPPORT
	txd_0->UTChkSum = 1;
	txd_0->IpChkSum = 1;

	if (txd_0->IpChkSum || txd_0->UTChkSum) {
		UCHAR tmp = 0;

		if (!TxInfo->NeedTrans)  /*802.11*/
			tmp = (txd_size + TxInfo->WifiHdrLen + 6) >> 1;
		else
			tmp = (txd_size + LENGTH_802_3) >> 1;

		txd_0->EthTypeOffset = tmp;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():TxInfo->WifiHdrLen is %x tmp length is %d\n",
				 __func__, TxInfo->WifiHdrLen, tmp));
	}

#endif /* CONFIG_CSO_SUPPORT */
#ifdef CONFIG_TRACE_SUPPORT

	if (txd_1->TxDFmt == TMI_FT_SHORT)
		TRACE_TX_MAC_SINFO(txd_s);
	else
		TRACE_TX_MAC_LINFO(txd_l);

#endif
}

#ifdef COMPOS_WIN
VOID
XmitWriteTxD(
	IN PMP_ADAPTER        pAd,
	IN UCHAR *buf,
	IN HARD_TRANSMIT_INFO TxInfo
)
{
	UINT8 tmpQIdx = 0;
	/* ============== */
	TxInfo.TMacInfo.PortIdx = P_IDX_LMAC;
	tmpQIdx = TxInfo.TMacInfo.QueIdx;

	if (TxInfo.bWfdPkt)
		TxInfo.TMacInfo.QueIdx = DMA_Q9; /* WFD packet always use Q9 */
	else
		TxInfo.TMacInfo.QueIdx;

	if (TxInfo.bShortFmt)
		TxInfo.TMacInfo.LongFmt = FALSE;
	else
		TxInfo.TMacInfo.LongFmt = TRUE;

	if (TxInfo.TMacInfo.LongFmt == TRUE) {
		if (TxInfo.bMgmtPacket ||
			TxInfo.bMcast ||
			TxInfo.bDHCPFrame ||
			TxInfo.bEAPOLFrame ||
			((TxInfo.MimoPs == MMPS_STATIC) && (pAd->RegistryCfg.APTGN == TRUE))) {
			TxInfo.TMacInfo.BaDisable = TRUE;
			TxInfo.TMacInfo.MaxTxTime = 0;
			TxInfo.TMacInfo.FixRate = 1;
			MTWF_DBG_NP(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Force to Fixed Rate, reason:(Mgmt:%d, Mcast:%d, DHCP:%d, EAPOL:%d, MMPS_S:%d\n",
					  TxInfo.bMgmtPacket,
					  TxInfo.bMcast,
					  TxInfo.bDHCPFrame,
					  TxInfo.bEAPOLFrame,
					  TxInfo.MimoPs);
		}

		if (TxInfo.TMacInfo.QueIdx == 0x7 || TxInfo.TMacInfo.QueIdx == 0x8)
			TxInfo.TMacInfo.RemainTxCnt = 0x1f;
		else
			TxInfo.TMacInfo.RemainTxCnt = 0xf;

		if (TxInfo.TMacInfo.Pid == PID_NOT_RESPONSE)
			TxInfo.TMacInfo.TxS2Host = 0; /* 0 : not report status to host */
		else
			TxInfo.TMacInfo.TxS2Host = 1; /* 1 : report status to host */

		TxInfo.TMacInfo.BarSsnCtrl = TMI_BSN_CFG_BY_HW;
	}

	TxInfo.TMacInfo.NeedTrans = FALSE;
	MtWriteTMacInfo(pAd, buf, &TxInfo.TMacInfo);
	TxInfo.TMacInfo.QueIdx = tmpQIdx;
	/* ============== */
}

#endif /* COMPOS_WIN */

INT mt_mac_pse_init(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 pse_addr = cap->WtblPseAddr;
	/* TODO: shiang-7603 */
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%d): pse addr=%x!\n", __LINE__, pse_addr);
	return TRUE;
}


VOID dump_wtbl_1_info(RTMP_ADAPTER *pAd, struct wtbl_1_struc *tb)
{
	union WTBL_1_DW0 *wtbl_1_d0 = (union WTBL_1_DW0 *)&tb->wtbl_1_d0.word;
	union WTBL_1_DW1 *wtbl_1_d1 = (union WTBL_1_DW1 *)&tb->wtbl_1_d1.word;
	union WTBL_1_DW2 *wtbl_1_d2 = (union WTBL_1_DW2 *)&tb->wtbl_1_d2.word;
	union WTBL_1_DW3 *wtbl_1_d3 = (union WTBL_1_DW3 *)&tb->wtbl_1_d3.word;
	union WTBL_1_DW4 *wtbl_1_d4 = (union WTBL_1_DW4 *)&tb->wtbl_1_d4.word;
	UINT8 addr[6];

	os_move_mem(&addr[0], &wtbl_1_d1->word, 4);
	addr[0] = wtbl_1_d1->field.addr_0 & 0xff;
	addr[1] = ((wtbl_1_d1->field.addr_0 & 0xff00) >> 8);
	addr[2] = ((wtbl_1_d1->field.addr_0 & 0xff0000) >> 16);
	addr[3] = ((wtbl_1_d1->field.addr_0 & 0xff000000) >> 24);
	/* addr[4] = wtbl_1_d0->field.addr_32 & 0xff; */
	/* addr[5] = (wtbl_1_d0->field.addr_32 & 0xff00 >> 8); */
	addr[4] = wtbl_1_d0->field.addr_4 & 0xff;
	addr[5] = wtbl_1_d0->field.addr_5 & 0xff;
	hex_dump("WTBL Segment 1 Raw Data", (UCHAR *)tb, sizeof(struct wtbl_1_struc));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WTBL Segment 1 Fields:\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAddr: "MACSTR"\n", MAC2STR(addr)));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMUAR_Idx:%d\n", wtbl_1_d0->field.muar_idx));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\trc_a1:%d/rc_a2:%d/kid:%d/rkv:%d/rv:%d/rc_id:%d/sw:%d\n",
			 wtbl_1_d0->field.rc_a1, wtbl_1_d0->field.rc_a2, wtbl_1_d0->field.kid, wtbl_1_d0->field.rkv, wtbl_1_d0->field.rv, wtbl_1_d0->field.rc_id, wtbl_1_d0->field.sw));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twm/mm:%d/%d\n", wtbl_1_d0->field.wm, wtbl_1_d2->field.mm));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\trx_ps:%d\n", wtbl_1_d2->field.rx_ps));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\taf:%d\n", wtbl_1_d2->field.af));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tcipher_suit:%d\n", wtbl_1_d2->field.cipher_suit));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\ttd/fd:%d/%d\n", wtbl_1_d2->field.td, wtbl_1_d2->field.fd));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tdis_rhtr:%d\n", wtbl_1_d2->field.dis_rhtr));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tr:%d\n", wtbl_1_d2->field.r));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\trts:%d\n", wtbl_1_d2->field.rts));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tcf_ack:%d\n", wtbl_1_d2->field.cf_ack));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\trdg_ba:%d\n", wtbl_1_d2->field.rdg_ba));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsmps:%d\n", wtbl_1_d2->field.smps));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tbaf_en:%d\n", wtbl_1_d2->field.baf_en));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tht/vht/ldpc/dyn_bw:%d/%d/%d/%d\n",
			 wtbl_1_d2->field.ht, wtbl_1_d2->field.vht,
			 wtbl_1_d2->field.ldpc, wtbl_1_d2->field.dyn_bw));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxBF(tibf/tebf):%d / %d\n", wtbl_1_d2->field.tibf, wtbl_1_d2->field.tebf));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\ttxop_ps_cap:%d\n", wtbl_1_d2->field.txop_ps_cap));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tmesh:%d\n", wtbl_1_d2->field.mesh));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tqos:%d\n", wtbl_1_d2->field.qos));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tadm:%d\n", wtbl_1_d2->field.adm));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tgid:%d\n", wtbl_1_d2->field.gid));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twtbl2_fid:%d/wtbl2_eid:%d\n", wtbl_1_d3->field.wtbl2_fid, wtbl_1_d3->field.wtbl2_eid));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twtbl3_fid:%d/wtbl3_eid:%d\n", wtbl_1_d4->field.wtbl3_fid, wtbl_1_d4->field.wtbl3_eid));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twtbl4_fid:%d/wtbl4_eid:%d\n", wtbl_1_d3->field.wtbl4_fid, wtbl_1_d4->field.wtbl4_eid));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tchk_per:%d\n", wtbl_1_d3->field.chk_per));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tdu_i_psm:%d\n", wtbl_1_d3->field.du_i_psm));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\ti_psm:%d\n", wtbl_1_d3->field.i_psm));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpsm:%d\n", wtbl_1_d3->field.psm));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tskip_tx:%d\n", wtbl_1_d3->field.skip_tx));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tpartial_aid:%d\n", wtbl_1_d4->field.partial_aid));
}

static UCHAR *bw_str[] = {"20", "40", "80", "160"};
VOID dump_wtbl_2_info(RTMP_ADAPTER *pAd, struct wtbl_2_struc *tb)
{
	union WTBL_2_DW0 *dw_0 = &tb->wtbl_2_d0;
	union WTBL_2_DW1 *dw_1 = &tb->wtbl_2_d1;
	union WTBL_2_DW2 *dw_2 = &tb->wtbl_2_d2;
	union WTBL_2_DW3 *dw_3 = &tb->wtbl_2_d3;
	union WTBL_2_DW4 *dw_4 = &tb->wtbl_2_d4;
	union WTBL_2_DW5 *dw_5 = &tb->wtbl_2_d5;
	union WTBL_2_DW6 *dw_6 = &tb->wtbl_2_d6;
	union WTBL_2_DW7 *dw_7 = &tb->wtbl_2_d7;
	union WTBL_2_DW8 *dw_8 = &tb->wtbl_2_d8;
	union WTBL_2_DW9 *dw_9 = &tb->wtbl_2_d9;
	union WTBL_2_DW10 *dw_10 = &tb->wtbl_2_d10;
	union WTBL_2_DW11 *dw_11 = &tb->wtbl_2_d11;
	union WTBL_2_DW12 *dw_12 = &tb->wtbl_2_d12;
	union WTBL_2_DW13 *dw_13 = &tb->wtbl_2_d13;
	union WTBL_2_DW14 *dw_14 = &tb->wtbl_2_d14;
	union WTBL_2_DW15 *dw_15 = &tb->wtbl_2_d15;
	UINT32 idx, ba_size_idx, ba_size = 0;
	BOOLEAN ba_en;
	UINT32 rate_info[8];
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR *ba_range = cap->ppdu.ba_range;

	hex_dump("WTBL Segment 2 Raw Data", (UCHAR *)tb, sizeof(struct wtbl_2_struc));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WTBL Segment 2 Fields:\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPN_0-31:0x%x\n", dw_0->pn_0));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPN_32-48:0x%x\n", dw_1->field.pn_32));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSN(NonQos/Mgmt Frame):%d\n", dw_1->field.com_sn));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tSN(TID0~7 QoS Frame):%d - %d - %d - %d - %d - %d - %d - %d\n",
			 dw_2->field.tid_ac_0_sn, dw_2->field.tid_ac_1_sn,
			 dw_2->field.tid_ac_2_sn_0 | (dw_3->field.tid_ac_2_sn_9 << 9),
			 dw_3->field.tid_ac_3_sn, dw_3->field.tid_4_sn,
			 dw_3->field.tid_5_sn_0 | (dw_4->field.tid_5_sn_5 << 5),
			 dw_4->field.tid_6_sn, dw_4->field.tid_7_sn));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxRateCnt(1-5):%d(%d) - %d - %d - %d - %d\n",
			 dw_5->field.rate_1_tx_cnt, dw_5->field.rate_1_fail_cnt,
			 dw_6->field.rate_2_tx_cnt, dw_6->field.rate_3_tx_cnt,
			 dw_6->field.rate_4_tx_cnt, dw_6->field.rate_5_tx_cnt));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxBwCnt(Current-Other):%d(%d) - %d(%d)\n",
			 dw_7->field.current_bw_tx_cnt, dw_7->field.current_bw_fail_cnt,
			 dw_8->field.other_bw_tx_cnt, dw_8->field.other_bw_fail_cnt));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFreqCap:%d(%sMHz)\n",
			 dw_9->field.fcap, bw_str[dw_9->field.fcap]));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRateIdx/CBRN/CCBW_SEL/SPE_EN: %d/%d/%d/%d\n",
			 dw_9->field.rate_idx, dw_9->field.cbrn, dw_9->field.ccbw_sel, dw_9->field.spe_en));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMpduCnt(Fail/OK):%d-%d\n",
			 dw_9->field.mpdu_fail_cnt, dw_9->field.mpdu_ok_cnt));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxRate Info: G2/G4/G8/G16=%d/%d/%d/%d\n",
			 dw_9->field.g2, dw_9->field.g4, dw_9->field.g8, dw_9->field.g16));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxRate Info: RateIdx/STBC/Nsts/PhyMode/TxRate (RawData)\n"));
	rate_info[0] = (dw_10->word & 0xfff);
	rate_info[1] = (dw_10->word & 0xfff000) >> 12;
	rate_info[2] = ((dw_10->word & 0xff000000) >> 24) | ((dw_11->word & 0xf) << 8);
	rate_info[3] = ((dw_11->word & 0xfff0) >> 4);
	rate_info[4] = ((dw_11->word & 0xfff0000) >> 16);
	rate_info[5] = ((dw_11->word & 0xf0000000) >> 28) | ((dw_12->word & 0xff) << 4);
	rate_info[6] = ((dw_12->word & 0xfff00) >> 8);
	rate_info[7] = ((dw_12->word & 0xfff00000) >> 20);

	for (idx = 0; idx <= 7; idx++) {
		UCHAR stbc, nss, phy_mode, rate;
		UINT32 raw_data;

		raw_data = rate_info[idx] & 0xfff;
		stbc = (raw_data & 0x800) ? 1 : 0;
		nss = (raw_data & 0x600) >> 9;
		phy_mode = (raw_data & 0x1c0) >> 6;
		rate = (raw_data & 0x3f);
		/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t%d/%d/%d/%d/MCS%d 0x%x\n", idx + 1, stbc, nss,  phy_mode, rate, rate_info[idx])); */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t%d/%d/%d/", idx + 1, stbc, nss));

		if (phy_mode == MODE_CCK)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CCK/"));
		else if (phy_mode == MODE_OFDM)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("OFDM/"));
		else if (phy_mode == MODE_HTMIX)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT/"));
		else if (phy_mode == MODE_HTGREENFIELD)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("GF/"));
		else if (phy_mode == MODE_VHT)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT/"));
		else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("unkonw/"));

		if (phy_mode == MODE_CCK) {
			if (rate == TMI_TX_RATE_CCK_1M_LP)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("1M"));
			else if (rate == TMI_TX_RATE_CCK_2M_LP)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("2M"));
			else if (rate == TMI_TX_RATE_CCK_5M_LP)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("5M"));
			else if (rate == TMI_TX_RATE_CCK_11M_LP)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("11M"));
			else if (rate == TMI_TX_RATE_CCK_2M_SP)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("2M"));
			else if (rate == TMI_TX_RATE_CCK_5M_SP)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("5M"));
			else if (rate == TMI_TX_RATE_CCK_11M_SP)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("11M"));
			else
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("unkonw"));
		} else if (phy_mode == MODE_OFDM) {
			if (rate == TMI_TX_RATE_OFDM_6M)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("6M"));
			else if (rate == TMI_TX_RATE_OFDM_9M)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("9M"));
			else if (rate == TMI_TX_RATE_OFDM_12M)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("12M"));
			else if (rate == TMI_TX_RATE_OFDM_18M)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("18M"));
			else if (rate == TMI_TX_RATE_OFDM_24M)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("24M"));
			else if (rate == TMI_TX_RATE_OFDM_36M)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("36M"));
			else if (rate == TMI_TX_RATE_OFDM_48M)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("48M"));
			else if (rate == TMI_TX_RATE_OFDM_54M)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("54M"));
			else
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("unkonw"));
		} else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCS%d", rate));

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 0x%x\n", rate_info[idx]));
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tResp_RCPI0/Resp_RCPI1/Resp_RCPI2=0x%x/0x%x/0x%x\n",
			 dw_13->field.resp_rcpi_0, dw_13->field.resp_rcpi_1, dw_13->field.resp_rcpi_2));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t1CC(Noise)/2CC(Noise)/3CC(Noise)/CE_RMSD/CC_Sel/Ant_Sel=0x%x/0x%x/0x%x/0x%x/%d/%d\n",
			 dw_14->field.sts_1_ch_cap_noise, dw_14->field.sts_2_ch_cap_noise, dw_14->field.sts_3_ch_cap_noise,
			 dw_14->field.ce_rmsd, dw_14->field.cc_noise_sel, dw_14->field.ant_sel));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBA Info: TID/BA_En/BAWinSizeIdx(Range)\n"));

	for (idx = 0; idx <= 7; idx++) {
		ba_en = (dw_15->field.ba_en & (1 << idx)) ? 1 : 0;
		ba_size_idx = (dw_15->field.ba_win_size_tid & (0x7 << (idx * 3))) >> ((idx * 3));

		if (ba_size_idx < 8)
			ba_size = ba_range[ba_size_idx];

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t%d/%d/%d(%d)\n", idx, ba_en, ba_size_idx,  ba_size));
	}
}


VOID dump_wtbl_3_info(RTMP_ADAPTER *pAd, union wtbl_3_struc *tb)
{
	hex_dump("WTBL Segment 3 Raw Data", (UCHAR *)tb, sizeof(union wtbl_3_struc));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WTBL Segment 3 Fields:\n"));
}


VOID dump_wtbl_4_info(RTMP_ADAPTER *pAd, struct wtbl_4_struc *tb)
{
	/* hex_dump("WTBL Segment 4 Raw Data", (UCHAR *)tb, sizeof(struct wtbl_4_struc)); */
	/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WTBL Segment 4 Fields:\n")); */
}


VOID dump_wtbl_base_info(RTMP_ADAPTER *pAd)
{
	INT idx;

	for (idx = 0; idx < 4; idx++) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWTBL Segment %d info:\n", idx + 1));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tMemBaseAddr/FID:0x%x/%d\n",
				 pAd->mac_ctrl.wtbl_base_addr[idx],
				 pAd->mac_ctrl.wtbl_base_fid[idx]));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEntrySize/Cnt:%d/%d\n",
				 pAd->mac_ctrl.wtbl_entry_size[idx],
				 pAd->mac_ctrl.wtbl_entry_cnt[idx]));
	}
}

VOID dump_wtbl_info(RTMP_ADAPTER *pAd, UINT wtbl_idx)
{
	INT idx, start_idx, end_idx, tok;
	UINT32 addr, val[16];
	struct wtbl_1_struc wtbl_1;
	struct wtbl_2_struc wtbl_2;
	union wtbl_3_struc wtbl_3;
	struct wtbl_4_struc wtbl_4;
	UINT32 wtbl_num = WTBL_MAX_NUM(pAd);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx));

	if (wtbl_idx == WCID_ALL(pAd)) {
		start_idx = 0;
		end_idx = (wtbl_num - 1);
	} else if (wtbl_idx < wtbl_num)
		start_idx = end_idx = wtbl_idx;
	else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid WTBL index(%d)!\n",
				 __func__, wtbl_idx));
		return;
	}

	for (idx = start_idx; idx <= end_idx; idx++) {
		/* Read WTBL 1 */
		os_zero_mem((UCHAR *)&wtbl_1, sizeof(struct wtbl_1_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WTBL Segment 1 HW Addr:0x%x\n", addr));
		HW_IO_READ32(pAd->hdev_ctrl, addr, &wtbl_1.wtbl_1_d0.word);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 4, &wtbl_1.wtbl_1_d1.word);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 8, &wtbl_1.wtbl_1_d2.word);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 12, &wtbl_1.wtbl_1_d3.word);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 16, &wtbl_1.wtbl_1_d4.word);
		dump_wtbl_1_info(pAd, &wtbl_1);
		/* Read WTBL 2 */
		os_zero_mem((UCHAR *)&wtbl_2, sizeof(struct wtbl_2_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[1] + idx * pAd->mac_ctrl.wtbl_entry_size[1];
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WTBL Segment 2 HW Addr:0x%x\n", addr));

		for (tok = 0; tok < sizeof(struct wtbl_2_struc) / 4; tok++)
			HW_IO_READ32(pAd->hdev_ctrl, addr + tok * 4, &val[tok]);

		dump_wtbl_2_info(pAd, (struct wtbl_2_struc *)&val[0]);
		/* Read WTBL 3 */
		os_zero_mem((UCHAR *)&wtbl_3, sizeof(union wtbl_3_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[2] + idx * pAd->mac_ctrl.wtbl_entry_size[2];
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WTBL Segment 3 HW Addr:0x%x\n", addr));

		for (tok = 0; tok < sizeof(union wtbl_3_struc) / 4; tok++)
			HW_IO_READ32(pAd->hdev_ctrl, addr + tok * 4, &val[tok]);

		dump_wtbl_3_info(pAd, (union wtbl_3_struc *)&val[0]);
		/* HW_IO_READ32(pAd->hdev_ctrl, addr, wtbl_3.); */
		/* dump_wtbl_3_info(pAd, &wtbl_3); */
		/* Read WTBL 4 */
		os_zero_mem((UCHAR *)&wtbl_4, sizeof(struct wtbl_4_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[3] + idx * pAd->mac_ctrl.wtbl_entry_size[3];
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WTBL Segment 4 HW Addr:0x%x\n", addr));
		HW_IO_READ32(pAd->hdev_ctrl, addr, &wtbl_4.ac0.word[0]);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 4, &wtbl_4.ac0.word[1]);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 8, &wtbl_4.ac1.word[0]);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 12, &wtbl_4.ac1.word[1]);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 16, &wtbl_4.ac2.word[0]);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 20, &wtbl_4.ac2.word[1]);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 24, &wtbl_4.ac3.word[0]);
		HW_IO_READ32(pAd->hdev_ctrl, addr + 28, &wtbl_4.ac3.word[1]);
		dump_wtbl_4_info(pAd, &wtbl_4);
	}
}


VOID dump_wtbl_entry(RTMP_ADAPTER *pAd, struct wtbl_entry *ent)
{
	INT idx;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL SW Entry[%d] Cache info\n", ent->wtbl_idx));

	for (idx = 0; idx < 4; idx++) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWTBL %d info:\n", idx + 1));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tAddr=0x%x, FID=%d, EID=%d\n",
				 ent->wtbl_addr[idx], ent->wtbl_fid[idx], ent->wtbl_eid[idx]));

		switch (idx) {
		case 0:
			dump_wtbl_1_info(pAd, &ent->wtbl_1);
			break;

		case 1:
			dump_wtbl_2_info(pAd, &ent->wtbl_2);
			break;

		case 2:
			dump_wtbl_3_info(pAd, &ent->wtbl_3);
			break;

		case 3:
			dump_wtbl_4_info(pAd, &ent->wtbl_4);
			break;

		default:
			break;
		}
	}
}


INT mt_wtbl_get_entry234(RTMP_ADAPTER *pAd, UCHAR widx, struct wtbl_entry *ent)
{
	struct rtmp_mac_ctrl *wtbl_ctrl;
	UINT8 wtbl_idx, ecnt_per_page;
	UINT32 page_offset, element_offset, idx;

	wtbl_ctrl = &pAd->mac_ctrl;

	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		wtbl_idx = (widx < wtbl_ctrl->wtbl_entry_cnt[0] ? widx : wtbl_ctrl->wtbl_entry_cnt[0] - 1);
	else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():PSE not init yet!\n", __func__));
		return FALSE;
	}

	ent->wtbl_idx = wtbl_idx;

	for (idx = 0; idx < 4; idx++) {
		if (idx == 0) {
			/* WTBL 1 */
			ent->wtbl_addr[idx] = wtbl_ctrl->wtbl_base_addr[0] +
								  wtbl_idx * wtbl_ctrl->wtbl_entry_size[0];
			ent->wtbl_fid[idx] = 0;
			ent->wtbl_eid[idx] = 0;
		} else {
			/* WTBL 2/3/4 */
			ecnt_per_page = wtbl_ctrl->page_size / wtbl_ctrl->wtbl_entry_size[idx];
			page_offset = wtbl_idx / ecnt_per_page;
			element_offset = wtbl_idx % ecnt_per_page;
			ent->wtbl_fid[idx] = wtbl_ctrl->wtbl_base_fid[idx] + page_offset;

			if (idx == 2)
				ent->wtbl_eid[idx] = (UINT16)(element_offset * 2);
			else
				ent->wtbl_eid[idx] = (UINT16)element_offset;

			ent->wtbl_addr[idx] = wtbl_ctrl->wtbl_base_addr[idx] +
								  page_offset * wtbl_ctrl->page_size +
								  element_offset * wtbl_ctrl->wtbl_entry_size[idx];
		}
	}

	return TRUE;
}

INT mt_wtbl_init_ByDriver(RTMP_ADAPTER *pAd)
{
	UINT32 page_cnt, ecnt_per_page, offset;
	UINT16 cnt = 0;
	struct wtbl_entry tb_entry;
	union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
	union WTBL_1_DW4 *dw4 = (union WTBL_1_DW4 *)&tb_entry.wtbl_1.wtbl_1_d4.word;
	UINT16 wtbl_num = WTBL_MAX_NUM(pAd);

	pAd->mac_ctrl.page_size = MT_PSE_PAGE_SIZE; /* size in bytes of each PSE page */
	pAd->mac_ctrl.wtbl_base_addr[0] = WTBL_WTBL1DW0; /* Start address of WTBL2 in host view */
	pAd->mac_ctrl.wtbl_entry_size[0] = sizeof(struct wtbl_1_struc);
	pAd->mac_ctrl.wtbl_entry_cnt[0] = wtbl_num;
	pAd->mac_ctrl.wtbl_base_fid[0] = 0;
	pAd->mac_ctrl.wtbl_base_addr[1] = MT_PCI_REMAP_ADDR_1; /* Start address of WTBL2 in host view */
	pAd->mac_ctrl.wtbl_entry_size[1] = sizeof(struct wtbl_2_struc);
	pAd->mac_ctrl.wtbl_entry_cnt[1] = wtbl_num;
	pAd->mac_ctrl.wtbl_base_fid[1] = 0;
	ecnt_per_page = pAd->mac_ctrl.page_size / sizeof(struct wtbl_2_struc);
	page_cnt = (pAd->mac_ctrl.wtbl_entry_cnt[1]  + ecnt_per_page - 1) / ecnt_per_page;
	offset = page_cnt * pAd->mac_ctrl.page_size;
	pAd->mac_ctrl.wtbl_base_fid[2] = pAd->mac_ctrl.wtbl_base_fid[1] + page_cnt;
	pAd->mac_ctrl.wtbl_base_addr[2] = pAd->mac_ctrl.wtbl_base_addr[1] + offset;
	pAd->mac_ctrl.wtbl_entry_size[2] = sizeof(union wtbl_3_struc);
	pAd->mac_ctrl.wtbl_entry_cnt[2] = wtbl_num;
	ecnt_per_page = pAd->mac_ctrl.page_size / sizeof(union wtbl_3_struc);
	page_cnt = (pAd->mac_ctrl.wtbl_entry_cnt[2]  + ecnt_per_page - 1) / ecnt_per_page;
	offset = page_cnt * pAd->mac_ctrl.page_size;
	pAd->mac_ctrl.wtbl_base_fid[3] = pAd->mac_ctrl.wtbl_base_fid[2] + page_cnt;
	pAd->mac_ctrl.wtbl_base_addr[3] = pAd->mac_ctrl.wtbl_base_addr[2] + offset;
	pAd->mac_ctrl.wtbl_entry_size[3] = sizeof(struct wtbl_4_struc);
	pAd->mac_ctrl.wtbl_entry_cnt[3] = wtbl_num;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): page_size(%d)\n", __func__, __LINE__, pAd->mac_ctrl.page_size));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): wtbl_base_addr[0]=0x%x, wtbl_entry_size[0]=%d, wtbl_entry_cnt[0]=%d, wtbl_base_fid[0]=%d\n",
			 __func__, __LINE__,
			 pAd->mac_ctrl.wtbl_base_addr[0], pAd->mac_ctrl.wtbl_entry_size[0], pAd->mac_ctrl.wtbl_entry_cnt[0], pAd->mac_ctrl.wtbl_base_fid[0]));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): wtbl_base_addr[1]=0x%x, wtbl_entry_size[1]=%d, wtbl_entry_cnt[1]=%d, wtbl_base_fid[1]=%d\n",
			 __func__, __LINE__,
			 pAd->mac_ctrl.wtbl_base_addr[1], pAd->mac_ctrl.wtbl_entry_size[1], pAd->mac_ctrl.wtbl_entry_cnt[1], pAd->mac_ctrl.wtbl_base_fid[1]));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): wtbl_base_addr[2]=0x%x, wtbl_entry_size[2]=%d, wtbl_entry_cnt[2]=%d, wtbl_base_fid[2]=%d\n",
			 __func__, __LINE__,
			 pAd->mac_ctrl.wtbl_base_addr[2], pAd->mac_ctrl.wtbl_entry_size[2], pAd->mac_ctrl.wtbl_entry_cnt[2], pAd->mac_ctrl.wtbl_base_fid[2]));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): wtbl_base_addr[3]=0x%x, wtbl_entry_size[3]=%d, wtbl_entry_cnt[3]=%d, wtbl_base_fid[3]=%d\n",
			 __func__, __LINE__,
			 pAd->mac_ctrl.wtbl_base_addr[3], pAd->mac_ctrl.wtbl_entry_size[3], pAd->mac_ctrl.wtbl_entry_cnt[3], pAd->mac_ctrl.wtbl_base_fid[3]));

	for (cnt = 0; cnt < wtbl_num; cnt++) {
		os_zero_mem(&tb_entry, sizeof(tb_entry));

		if (asic_get_wtbl_entry234(pAd, cnt, &tb_entry) == FALSE) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
					 __func__, cnt));
			return FALSE;
		}

		dw3->field.wtbl2_fid = tb_entry.wtbl_fid[1];
		dw3->field.wtbl2_eid = tb_entry.wtbl_eid[1];
		dw3->field.wtbl4_fid = tb_entry.wtbl_fid[3];
		dw4->field.wtbl3_fid = tb_entry.wtbl_fid[2];
		dw4->field.wtbl3_eid = tb_entry.wtbl_eid[2];
		dw4->field.wtbl4_eid = tb_entry.wtbl_eid[3];
		HW_IO_WRITE32(pAd->hdev_ctrl, tb_entry.wtbl_addr[0] + 12, dw3->word);
		HW_IO_WRITE32(pAd->hdev_ctrl, tb_entry.wtbl_addr[0] + 16, dw4->word);
		tb_entry.wtbl_1.wtbl_1_d0.field.rc_a2 = 1;
		tb_entry.wtbl_1.wtbl_1_d0.field.rv = 1;
		HW_IO_WRITE32(pAd->hdev_ctrl, tb_entry.wtbl_addr[0], tb_entry.wtbl_1.wtbl_1_d0.word);
	}

	dump_wtbl_base_info(pAd);
	return TRUE;
}

INT mt_wtbl_init(RTMP_ADAPTER *pAd)
{
	mt_wtbl_init_ByDriver(pAd);
	return TRUE;
}

INT mt_hw_tb_init(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	MT_WCID_TABLE_INFO_T WtblInfo;
	/* TODO: shiang-7603 */
	mt_wtbl_init(pAd);
	/* Create default entry for rx packets which A2 is not in our table */
	os_zero_mem(&WtblInfo, sizeof(MT_WCID_TABLE_INFO_T));
	os_move_mem(WtblInfo.Addr, BROADCAST_ADDR, MAC_ADDR_LEN);
#ifdef COMPOS_WIN
	WtblInfo.Wcid = 0;
#else
	WtblInfo.Wcid = 0;
#endif
	WtblInfo.WcidType  = MT_WCID_TYPE_BMCAST;
	MtAsicUpdateRxWCIDTable(pAd, WtblInfo);
	return TRUE;
}

VOID mt_show_mac_info(RTMP_ADAPTER *pAd)
{
#ifndef COMPOS_WIN
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MAC[Ver:Rev/ID=0x%08x : 0x%08x]\n",
			 pAd->MACVersion, pAd->ChipID);
#endif
}



INT mt_mlme_hw_tx(RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *transmit, TX_BLK *tx_blk)
{
	UINT16 free_cnt = 1;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;

	asic_write_tmac_info_fixed_rate(pAd, tmac_info, info, transmit);

	tx_blk->pSrcBufData = tx_blk->pSrcBufHeader;
	NdisCopyMemory(&tx_blk->HeaderBuf[0], tx_blk->pSrcBufData, tx_hw_hdr_len);
	tx_blk->pSrcBufData += tx_hw_hdr_len;
	tx_blk->SrcBufLen -= tx_hw_hdr_len;
	tx_blk->MpduHeaderLen = 0;
	tx_blk->wifi_hdr_len = 0;
	tx_blk->HdrPadLen = 0;
	tx_blk->hw_rsv_len = 0;
	asic_write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	return NDIS_STATUS_SUCCESS;
}

INT32 txs_handler_v1(RTMP_ADAPTER *pAd, VOID *rx_packet)
{
	RMAC_RXD_0_TXS *txs = (RMAC_RXD_0_TXS *)(rx_packet);
	UCHAR *ptr;
	INT idx, txs_entry_len = 20;

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "TxS Report: Number=%d, ByteCnt=%d\n",
			  txs->TxSCnt, txs->TxSRxByteCnt);

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

	if (txs->TxSRxByteCnt != (txs->TxSCnt * txs_entry_len + 4)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("ReceivedByteCnt not equal txs_entry required!\n"));
	} else {
		ptr = ((UCHAR *)(rx_packet)) + 4;

		for (idx = 0; idx < txs->TxSCnt; idx++) {
			TXS_STRUC *txs_entry = (TXS_STRUC *)ptr;
			TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
#ifdef RT_BIG_ENDIAN
			TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
			TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
			TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
#endif /* RT_BIG_ENDIAN */
			TXS_D_4 *TxSD4 = &txs_entry->TxSD4;
#ifdef RT_BIG_ENDIAN
			*(((UINT32 *)TxSD0)) = SWAP32(*(((UINT32 *)TxSD0)));
			*(((UINT32 *)TxSD1)) = SWAP32(*(((UINT32 *)TxSD1)));
			*(((UINT32 *)TxSD2)) = SWAP32(*(((UINT32 *)TxSD2)));
			*(((UINT32 *)TxSD3)) = SWAP32(*(((UINT32 *)TxSD3)));
			*(((UINT32 *)TxSD4)) = SWAP32(*(((UINT32 *)TxSD4)));
#endif /* RT_BIG_ENDIAN */

			ParseTxSPacket_v1(pAd, TxSD4->TxS_Pid, TxSD0->TxSFmt, ptr);

			ptr += txs_entry_len;
		}
	}

	return TRUE;
}

#ifdef MGMT_TXPWR_CTRL
INT wtbl_update_pwr_offset(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	CMD_WTBL_PWR_T WtblPwrOffset = {0};
	CMD_WTBL_RATE_T WtblRateInfo = {0};
	UINT8 ucWlanIdx = wdev->tr_tb_idx;
	UINT16 ucRate = 0;

	/* update power offset to wtbl*/
	WtblPwrOffset.u2Tag = WTBL_PWR_OFFSET;
	WtblPwrOffset.u2Length = sizeof(CMD_WTBL_PWR_T);
	WtblPwrOffset.ucPwrOffset = wdev->TxPwrDelta;
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

	return TRUE;
}
#endif

