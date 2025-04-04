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
	cmm_txbf_mt.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_txbf_mt.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif
#include "hdev/hdev.h"

#ifdef TXBF_SUPPORT

typedef enum _BF_SND_CNT_CONDITION
{
    SND_CNT_CONDI_DEFAULT = 0,
    SND_CNT_CONDI_8RU = BIT(0),
    SND_CNT_CONDI_MANUAL = BIT(7)
} BF_SND_CNT_CONDITION;

UINT32 g_u4TxBfOui;
VOID txbf_set_oui(
	UINT8 u1BfOui)
{
	g_u4TxBfOui |= BIT(u1BfOui);
	MTWF_DBG(NULL, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "u1BfOui:%u, g_u4TxBfOui: %u\n", u1BfOui, g_u4TxBfOui);
}

VOID txbf_clear_oui(
	VOID)
{
	MTWF_DBG(NULL, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\n");
	g_u4TxBfOui = 0;
}

UINT32 txbf_get_oui(
	VOID)
{
	return g_u4TxBfOui;
}

#define ETXBF_PROBE_TIME (RA_INTERVAL-100)	/* Wait for Sounding Response will time out 100msec before end of RA interval */

#ifdef VHT_TXBF_SUPPORT
struct TXBF_BFEE_CHECK {
	BOOLEAN valid;
	UINT8 bfer_cap_su;
	UINT8 num_snd_dimension;
};

static struct TXBF_BFEE_CHECK txbf_bfee_check = {
	.valid = FALSE,
	.bfer_cap_su = 0,
	.num_snd_dimension = 0
	};

VOID txbf_bfee_cap_unset(
	VOID)
{
	txbf_bfee_check.valid = FALSE;
	txbf_bfee_check.bfer_cap_su = 0;
	txbf_bfee_check.num_snd_dimension = 0;
}

UINT8 txbf_bfee_get_bfee_sts(
	UINT8 bfee_sts)
{
	UINT8 new_bfee_sts = bfee_sts;

	if (txbf_bfee_check.valid && txbf_bfee_check.bfer_cap_su && txbf_bfee_check.num_snd_dimension > 0)
		new_bfee_sts = min(new_bfee_sts, txbf_bfee_check.num_snd_dimension);

	MTWF_DBG(NULL, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"bfee_sts=%u, valid=%u, bfer_cap_su=%u, num_snd_dimension=%u, new_bfee_sts=%u\n",
			bfee_sts,
			txbf_bfee_check.valid,
			txbf_bfee_check.bfer_cap_su,
			txbf_bfee_check.num_snd_dimension,
			new_bfee_sts);

	txbf_bfee_cap_unset();
	return new_bfee_sts;
}

VOID txbf_bfee_cap_set(
	BOOLEAN valid,
	UINT8 bfer_cap_su,
	UINT8 num_snd_dimension)
{
	MTWF_DBG(NULL, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"valid=%u, bfer_cap_su=%u, num_snd_dimension=%u\n",
		valid,
		bfer_cap_su,
		num_snd_dimension);

	txbf_bfee_check.valid = valid;
	txbf_bfee_check.bfer_cap_su = bfer_cap_su;
	txbf_bfee_check.num_snd_dimension = num_snd_dimension;
}
#endif /* VHT_TXBF_SUPPORT */

#ifdef MFB_SUPPORT

UCHAR mcsToLowerMcs[] = {
	/*   originalMfb, newMfb1s, newMfb2s, newMfb3s*/
	0,	0,	0,	0,
	1,	1,	1,	1,
	2,	2,	2,	2,
	3,	3,	3,	3,
	4,	4,	4,	4,
	5,	5,	5,	5,
	6,	6,	6,	6,
	7,	7,	7,	7,
	8,	0,	8,	8,
	9,	1,	9,	9,
	10,	2,	10,	10,
	11,	3,	11,	11,
	12,	4,	12,	12,
	13,	5,	13,	13,
	14,	6,	14,	14,
	15,	7,	15,	15,
	16,	0,	8,	16,
	17,	1,	9,	17,
	18,	2,	10,	18,
	19,	3,	11,	19,
	20,	4,	12,	20,
	21,	5,	13,	21,
	22,	6,	14,	22,
	23,	7,	15,	23,
	24,	0,	8,	16,
	25,	1,	9,	17,
	26,	2,	10,	18,
	27,	3,	11,	19,
	28,	4,	12,	20,
	29,	5,	13,	21,
	30,	6,	14,	22,
	31,	7,	15,	23,
	32,	0,	0,	0,
	33,	3,	3,	3,
	34,	3,	3,	3,
	35,	3,	11,	11,
	36,	4,	4,	4,
	37,	6,	6,	6,
	38,	6,	12,	12,
	39,	3,	3,	17,
	40,	3,	11,	11,
	41,	3,	3,	17,
	42,	3,	11,	11,
	43,	3,	11,	19,
	44,	3,	11,	11,
	45,	3,	11,	19,
	46,	4,	4,	18,
	47,	4,	12,	12,
	48,	6,	6,	6,
	49,	6,	12,	12,
	50,	6,	12,	20,
	51,	6,	14,	14,
	52,	6,	14,	14,
	53,	3,	3,	17,
	54,	3,	11,	11,
	55,	3,	11,	19,
	56,	3,	3,	17,
	57,	3,	11,	11,
	58,	3,	11,	19,
	59,	3,	11,	19,
	60,	3,	11,	11,
	61,	3,	11,	19,
	62,	3,	11,	19,
	63,	3,	11,	19,
	64,	3,	11,	19,
	65,	4,	4,	18,
	66,	4,	12,	12,
	67,	4,	12,	20,
	68,	6,	6,	6,
	69,	6,	12,	12,
	70,	6,	12,	20,
	71,	6,	12,	20,
	72,	6,	14,	14,
	73,	6,	14,	14,
	74,	6,	14,	14,
	75,	6,	14,	22,
	76,	6,	14,	22
};
#endif /* MFB_SUPPORT */

#if defined(MT_MAC)
/*
 * ==========================================================================
 * Description:
 * Enable sounding trigger
 *
 * Return:
 * TRUE if all parameters are OK, FALSE otherwise
 * ==========================================================================
 */
INT mt_Trigger_Sounding_Packet(
	IN    PRTMP_ADAPTER        pAd,
	IN    UCHAR                SndgEn,
	IN    UINT32               u4SNDPeriod,
	IN    UCHAR                ucSu_Mu,
	IN    UCHAR                ucMuNum,
	IN    PUCHAR               pWlanId)
{
	/* Enable sounding trigger in FW */
	return CmdETxBfSoundingPeriodicTriggerCtrl(pAd,
			SndgEn,
			u4SNDPeriod,
			ucSu_Mu,
			ucMuNum,
			pWlanId);
}

#ifdef CONFIG_STA_SUPPORT
/*
 * ==========================================================================
 * Description:
 * Judge sounding whether shall be triggered on STA mode
 *
 * Return:
 * None
 * ==========================================================================
 */
VOID mt_BfSoundingAdjust(
	IN RTMP_ADAPTER * pAd,
	IN UINT8 ConnectionState,
	IN struct wifi_dev *wdev
)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	MAC_TABLE_ENTRY *pPeerEntry = NULL;
	UINT16 u2Wcid = 0;

	pEntry = GetAssociatedAPByWdev(pAd, wdev);

	if ((!pEntry) || (!wdev))
		return;
	u2Wcid = pEntry->wcid;

	MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "WlanIdx=%d, ConnectionState=%d\n",
			u2Wcid, ConnectionState);

	if (!VALID_UCAST_ENTRY_WCID(pAd, u2Wcid)) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pEntry is NULL!!\n");
		return;
	}
	pPeerEntry = &pAd->MacTab.Content[u2Wcid];

	if (ConnectionState == STATE_CONNECTED) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"[ETxBfCap=%d\n", IS_ETXBF_SUP(pPeerEntry->rStaRecBf.u1TxBfCap));

		if (IS_ETXBF_SUP(pPeerEntry->rStaRecBf.u1TxBfCap)) {
			MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"[STA mode][Conn]Start BF process timer\n");
			CmdETxBfSoundingPeriodicTriggerCtrl(pAd,
												TRUE,
												0,
												BF_PROCESSING,
												0,
												NULL);
		} else {
			MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"[STA mode][Conn]Stop BF process timer\n");
			CmdETxBfSoundingPeriodicTriggerCtrl(pAd,
												FALSE,
												0,
												BF_PROCESSING,
												0,
												NULL);
		}
	}

	if (ConnectionState == STATE_DISCONNECT) {
		if (IS_ETXBF_SUP(pPeerEntry->rStaRecBf.u1TxBfCap)) {
			MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"[STA mode][DisConn]Stop BF process timer\n");
			CmdETxBfSoundingPeriodicTriggerCtrl(pAd,
												FALSE,
												0,
												BF_PROCESSING,
												0,
												NULL);
		}
	}
}
#endif /* CONFIG_STA_SUPPORT*/
#endif /* MT_MAC */


#ifdef MT_MAC
/*
 * Set_StaETxBfEnCond_Proc - enable/disable ETxBF
 * usage: iwpriv ra0 set StaETxBfEnCond=dd
 * 0=>disable, 1=>enable
 * Note: After use this command, need to re-run StaStartup()/LinkUp() operations to sync all status.
 * If ETxBfIncapable!=0 then we don't need to reassociate.
 */
UCHAR AsicTxBfEnCondProc(
	IN	RTMP_ADAPTER * pAd,
	IN	TXBF_STATUS_INFO * pTxBfInfo)
{
	MAC_TABLE_ENTRY		  *pEntry;
	UCHAR ucETxBfEn;
	UINT16 u2WlanIdx;

	for (u2WlanIdx = 0; VALID_UCAST_ENTRY_WCID(pAd, u2WlanIdx); u2WlanIdx++) {
		pEntry = &pAd->MacTab.Content[u2WlanIdx];

		if (!IS_ENTRY_NONE(pEntry)) {
#ifdef VHT_TXBF_SUPPORT

			if (WMODE_CAP_AC(pTxBfInfo->ucPhyMode) && (pTxBfInfo->u2Channel > 14)) {
				ucETxBfEn = mt_WrapClientSupportsVhtETxBF(pAd, pTxBfInfo->pVhtTxBFCap) ? TRUE : FALSE;
				MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG, "VHT mode!\n");
				MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG,
					"STA : Bfee Cap =%d, Bfer Cap =%d!\n",
					pTxBfInfo->pVhtTxBFCap->bfee_cap_su,  pTxBfInfo->pVhtTxBFCap->bfer_cap_su);
			} else
#endif
			{
				ucETxBfEn = mt_WrapClientSupportsETxBF(pAd, pTxBfInfo->pHtTxBFCap) ? TRUE : FALSE;
				MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG, "HT mode!\n");
			}

			MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG,
				"Final ETxBF status =%d!\n", ucETxBfEn);

			if (ucETxBfEn) {
				CmdTxBfTxApplyCtrl(pAd,
								   u2WlanIdx,
								   TRUE,
								   FALSE,
								   TRUE,
								   FALSE);
			}
		}
	}

	return TRUE;
}
#endif /* MT_MAC */

/*
 * TxBFInit - Intialize TxBF fields in pEntry
 * supportsETxBF - TRUE if client supports ETxBF
 */
VOID mt_TxBFInit(
	IN PRTMP_ADAPTER	pAd,
	IN TXBF_STATUS_INFO * pTxBfInfo,
	IN TXBF_MAC_TABLE_ENTRY * pEntryTxBf,
	IN BOOLEAN			supportsETxBF)
{
	pEntryTxBf->eTxBfEnCond = supportsETxBF ? pTxBfInfo->ucETxBfTxEn : 0;
}


/* TxBF Fw Init */
VOID mt_TxBFFwInit(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->CommonCfg.ETxBfEnCond) {
#if defined(MT_MAC)
		/* Enable periodic sounding */
		mt_Trigger_Sounding_Packet(pAd,
								   TRUE,
								   0,
								   4,
								   0,
								   NULL);
#endif
	} else {
#if defined(MT_MAC)
		/* Disable periodic sounding */
		mt_Trigger_Sounding_Packet(pAd,
								   FALSE,
								   0,
								   0,
								   0,
								   NULL);
#endif
	}
}


/* clientSupportsETxBF - returns true if client supports compatible Sounding */
BOOLEAN mt_clientSupportsETxBF(
	IN	PRTMP_ADAPTER	 pAd,
	IN	HT_BF_CAP * pTxBFCap,
	IN  BOOLEAN          ETxBfNoncompress)
{
	BOOLEAN compCompat, noncompCompat;

	compCompat    = (pTxBFCap->ExpComBF > 0) &&
					/*(pTxBFCap->ComSteerBFAntSup+1 >= pAd->Antenna.field.TxPath) && */
					(ETxBfNoncompress == 0);
	noncompCompat = (pTxBFCap->ExpNoComBF > 0)
					/* && (pTxBFCap->NoComSteerBFAntSup+1 >= pAd->Antenna.field.TxPath)*/;
	return pTxBFCap->RxNDPCapable == 1 && (compCompat || noncompCompat);
}


#ifdef VHT_TXBF_SUPPORT
/* clientSupportsETxBF - returns true if client supports compatible Sounding */
BOOLEAN mt_clientSupportsVhtETxBF(
	IN	PRTMP_ADAPTER	pAd,
	IN	VHT_CAP_INFO * pTxBFCap)
{
	BOOLEAN fgBfeeCap = FALSE;

#ifdef CFG_SUPPORT_MU_MIMO
	fgBfeeCap = pTxBFCap->bfee_cap_su || pTxBFCap->bfee_cap_mu;
#else
	fgBfeeCap = pTxBFCap->bfee_cap_su;
#endif

	return fgBfeeCap;
}
#endif /* VHT_TXBF_SUPPORT */

#ifdef HE_TXBF_SUPPORT
BOOLEAN txbf_peer_he_bfer_cap(
	struct he_bf_info *he_bf_struct)
{
	return (he_bf_struct->bf_cap & HE_SU_BFER) ? TRUE:FALSE;
}

BOOLEAN txbf_peer_he_bfee_cap(
	struct he_bf_info *he_bf_struct)
{
	return (he_bf_struct->bf_cap & HE_SU_BFEE) ? TRUE:FALSE;
}

BOOLEAN txbf_peer_he_su_ng16_cap(
	struct he_bf_info *he_bf_struct)
{
	return (he_bf_struct->bf_cap & HE_BFEE_NG_16_SU_FEEDBACK) ? TRUE:FALSE;
}

BOOLEAN txbf_peer_he_mu_ng16_cap(
	struct he_bf_info *he_bf_struct)
{
	return (he_bf_struct->bf_cap & HE_BFEE_NG_16_MU_FEEDBACK) ? TRUE:FALSE;
}

BOOLEAN txbf_peer_he_su_codebook42_cap(
	struct he_bf_info *he_bf_struct)
{
	return (he_bf_struct->bf_cap & HE_BFEE_CODEBOOK_SU_FEEDBACK) ? TRUE:FALSE;
}

BOOLEAN txbf_peer_he_mu_codebook75_cap(
	struct he_bf_info *he_bf_struct)
{
	return (he_bf_struct->bf_cap & HE_BFEE_CODEBOOK_MU_FEEDBACK) ? TRUE:FALSE;
}

BOOLEAN txbf_peer_he_trigger_su_feedback(
	struct he_bf_info *he_bf_struct)
{
	return (he_bf_struct->bf_cap & HE_TRIG_SU_BFEE_FEEDBACK) ? TRUE:FALSE;
}

BOOLEAN txbf_peer_he_trigger_mu_feedback(
	struct he_bf_info *he_bf_struct)
{
	return (he_bf_struct->bf_cap & HE_TRIG_MU_BFEE_FEEDBACK) ? TRUE:FALSE;
}

BOOLEAN txbf_peer_he_ndp_ltf(
	enum he_gi_caps he_gi)
{
	return (he_gi & HE_NDP_4x_LTF_3DOT2MS_GI) ? TRUE:FALSE;
}

#endif /* HE_TXBF_SUPPORT */

/*
 * ==========================================================================
 * Description:
 * Get STA's TxBF Cap info: BFer and Nr
 *
 * param[in] PMAC_TABLE_ENTRY pEntry
 *         [out] BOOLEAN *bfer  1: has BFer cap, 0: otherwise
 *     	[out] UCHAR *nr  STA BFer's Nr
 * Return:
 * TRUE if success, FALSE otherwise
 * ==========================================================================
 */
static BOOLEAN get_sta_bfer_nr(
	IN PMAC_TABLE_ENTRY pEntry,
	OUT BOOLEAN *bfer,
	OUT UCHAR *nr)
{
	*bfer = FALSE;
	*nr = 0;

	if (!pEntry)
		return FALSE;

	switch (pEntry->MaxHTPhyMode.field.MODE) {
#ifdef DOT11_HE_AX
#ifdef HE_TXBF_SUPPORT
	case MODE_HE:
		*bfer = txbf_peer_he_bfer_cap(&pEntry->cap.he_bf);
		*nr = pEntry->cap.he_bf.snd_dim_le_eq_bw80;
		MTWF_DBG(NULL, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"sta %u HE BFer:%u, Nr=%u\n", pEntry->wcid, *bfer, *nr);
		break;
#endif
#endif
#ifdef DOT11_VHT_AC
	case MODE_VHT:
		*bfer = pEntry->vht_cap_ie.vht_cap.bfer_cap_su;
		*nr = pEntry->vht_cap_ie.vht_cap.num_snd_dimension;
		MTWF_DBG(NULL, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"sta %u VHT BFer:%u, Nr=%u\n", pEntry->wcid, *bfer, *nr);
		break;
#endif /* DOT11_VHT_AC */
	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		*bfer = pEntry->HTCapability.TxBFCap.TxNDPCapable;
		*nr = pEntry->HTCapability.TxBFCap.ChanEstimation;
		MTWF_DBG(NULL, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"sta %u HT BFer:%u, Nr=%u\n", pEntry->wcid, *bfer, *nr);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

/* Nr,Nc */
UINT_8   g_ru2PfmuMemReq[4][4] = {
	{0,  0,  0, 0}, /* None,None,None,None */
	{1,  1,  0, 0}, /* 2//x1,2x2,2x3,2x4 */
	{2,  4,  4, 0}, /* 3//x1,3x2,3x3,3x4 */
	{3,  5,  6, 0}
};/* 4x1,4x2,4x3,4x4 */

UINT_8  g_aPfmuTimeOfMem20M[4] = {0, 1, 2, 2};

#if defined(DOT11_HE_AX) && defined(HE_TXBF_SUPPORT)
UINT8 TxBfGetHeNcForIot(
	UINT8 u1OrigNc,
	PUINT8 he80_rx_nss_mcs
	)
{
	UINT8 u1PeerMaxRxNss;
	UINT8 u1NewNc = u1OrigNc;

	for (u1PeerMaxRxNss = 0; u1PeerMaxRxNss < DOT11AX_MAX_STREAM; u1PeerMaxRxNss++) {
		if (he80_rx_nss_mcs[u1PeerMaxRxNss] == 3) /* 3 for not support */
			break;
	}

	/* Recognize S10 Peer: Broadcomm/Epigram and Rx Steam=2 and Max Nc=0 */
	if ((txbf_get_oui() & BIT(ENUM_BF_OUI_BROADCOM)) &&
		u1PeerMaxRxNss == 2 && u1OrigNc == 0) {
		u1NewNc = u1PeerMaxRxNss-1;
	}

	MTWF_DBG(NULL, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
			"u1PeerMaxRxNss: %u, txbf_get_oui: %u, u1OrigNc: %u, u1NewNc:%u\n",
			u1PeerMaxRxNss, txbf_get_oui(), u1OrigNc, u1NewNc);

	return u1NewNc;
}
#endif /* defined(DOT11_HE_AX) && defined(HE_TXBF_SUPPORT) */

INT32 mt_AsicBfStaRecUpdate(
	RTMP_ADAPTER * pAd,
	UCHAR        ucPhyMode,
	UCHAR        ucBssIdx,
	UINT16       u2WlanIdx)
{
	PMAC_TABLE_ENTRY pEntry;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef DOT11_HE_AX
#ifdef HE_TXBF_SUPPORT
	struct txbf_starec_conf *man_bf_sta_rec = &pAd->manual_bf_sta_rec;
#endif
#endif
	UCHAR   ucBFerMaxNr,       ucBFeeMaxNr,          ucBFerCurrNr;
	UCHAR   ucStatus   = TRUE;
	UCHAR   ucPeerRxNumSupport;
	UCHAR   ucTxMCSSetdefined, ucTxRxMCSSetNotEqual, ucTxMaxNumSpatilStream;
	BOOLEAN fgETxBfCap = FALSE, fgITxBfCap = FALSE;
	UINT8   ucTxPath = pAd->Antenna.field.TxPath;
	HT_CAPABILITY_IE *ht_cap;
	VHT_CAP_INFO vht_cap;
#ifdef DOT11_HE_AX
#ifdef HE_TXBF_SUPPORT
	struct he_bf_info he_bf_struct;
	struct mcs_nss_caps *mcs_nss;
	UCHAR bfer_max_nr, bfee_max_nr;
	UINT8 he_bw;
	UINT8 tx_nss_bw160;
	UINT8 i, u1PeerMaxRxNss, u1PeerMaxBw;
#endif
#endif
	UINT8 band_idx = 0;
	UINT8 index1, index2;

	pEntry = &pAd->MacTab.Content[u2WlanIdx];

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		band_idx = HcGetBandByWdev(pEntry->wdev);

		if (band_idx == DBDC_BAND0)
			ucTxPath = pAd->dbdc_band0_tx_path;
		else
			ucTxPath = pAd->dbdc_band1_tx_path;
	}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(pEntry->wdev);
		if (pAd->bAntennaSetAPEnable[BandIdx])
			ucTxPath = pAd->TxStream[BandIdx];
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	switch (pEntry->MaxHTPhyMode.field.MODE) {
#ifdef DOT11_HE_AX
#ifdef HE_TXBF_SUPPORT
	case MODE_HE:
		NdisZeroMemory(&he_bf_struct, sizeof(struct he_bf_info));
		mt_wrap_get_he_bf_cap(pEntry->wdev, &he_bf_struct);

		mcs_nss = wlan_config_get_mcs_nss_caps(pEntry->wdev);
		he_bw = wlan_operate_get_he_bw(pEntry->wdev);

		if (he_bw > HE_BW_80)
			tx_nss_bw160 = min(ucTxPath, (UINT8)(mcs_nss->bw160_max_nss + 1));

		if (wlan_config_get_etxbf(pEntry->wdev) == SUBF_ALL || wlan_config_get_etxbf(pEntry->wdev) == SUBF_BFER)
			fgETxBfCap = txbf_peer_he_bfee_cap(&pEntry->cap.he_bf);
		else
			fgETxBfCap = FALSE;

		if (fgETxBfCap)
			pEntry->rStaRecBf.u1TxBfCap = TXBF_PFMU_STA_ETXBF_SUP;
		else
			pEntry->rStaRecBf.u1TxBfCap = 0;

		pEntry->rStaRecBf.fgSU_MU = FALSE;
		pEntry->rStaRecBf.ucTxMode = MODE_HE_SU;

		u1PeerMaxBw = BW_20;

		if ((pEntry->wdev->channel < 14) && WMODE_CAP_2G(pEntry->wdev->PhyMode)) {
			/* 2G */
			if (pEntry->cap.ch_bw.he_ch_width & SUPP_40M_CW_IN_24G_BAND)
				u1PeerMaxBw = BW_40;

		} else if (WMODE_CAP_5G(pEntry->wdev->PhyMode) || WMODE_CAP_6G(pEntry->wdev->PhyMode)) {
			/* 5G and 6G */
			if (pEntry->cap.ch_bw.he_ch_width & SUPP_40M_80M_CW_IN_5G_BAND) {
				u1PeerMaxBw = BW_80;
				if (pEntry->cap.ch_bw.he_ch_width & (SUPP_160M_CW_IN_5G_BAND|SUPP_160M_8080M_CW_IN_5G_BAND))
					u1PeerMaxBw = BW_160;
			}

			if (WMODE_CAP_6G(pEntry->wdev->PhyMode) && (pEntry->cap.he6g_op_present == 1)) {
				/*6G with 6G operation info*/
				u1PeerMaxBw = pEntry->cap.ch_bw.he6g_ch_width;
			}
		}

		pEntry->rStaRecBf.ucCBW = min(u1PeerMaxBw, he_bw);

		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
			"pEntry->MaxHTPhyMode.field.BW =%u\n", pEntry->MaxHTPhyMode.field.BW);
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
			"pEntry->cap.ch_bw.he_ch_width =%u\n", pEntry->cap.ch_bw.he_ch_width);
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
			"he_bw                         =%u\n", he_bw);
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
			"pEntry->rStaRecBf.ucCBW       =%u\n", pEntry->rStaRecBf.ucCBW);

		if (fgETxBfCap) {
			pEntry->rStaRecBf.ucSoundingPhy = 1;       /* OFDM */
			pEntry->rStaRecBf.ucNdpaRate = 73;         /* OFDM 24M */
			pEntry->rStaRecBf.ucNdpRate = 0;           /* MCS0 */
			pEntry->rStaRecBf.ucReptPollRate = 73;     /* OFDM 24M */

			ucBFerMaxNr = he_bf_struct.snd_dim_le_eq_bw80;
			ucBFeeMaxNr = pEntry->cap.he_bf.bfee_sts_le_eq_bw80;

			/* BW80 */
			for (u1PeerMaxRxNss = 0; u1PeerMaxRxNss < DOT11AX_MAX_STREAM; u1PeerMaxRxNss++) {
				if (pEntry->cap.rate.he80_tx_nss_mcs[u1PeerMaxRxNss] == 3) /* 3 for not support */
					break;
			}
			ucPeerRxNumSupport = u1PeerMaxRxNss - 1;

			pEntry->rStaRecBf.ucNr = min(ucBFerMaxNr, ucBFeeMaxNr);
			pEntry->rStaRecBf.ucNc = min(ucPeerRxNumSupport, pEntry->rStaRecBf.ucNr);
			/*
			  *  Currently, the Nc value only references BFee's "Max Rx Nss", which is correct for all HE BFees we've seen.
			  *  The Nc value doesn't reference "BFee Max Nc" since many Broadcom 2x2 BFees' "Max Nc" are 0, which are wrong.
			  *
			pEntry->rStaRecBf.ucNc = min(pEntry->cap.he_bf.bfee_max_nc, pEntry->rStaRecBf.ucNr);
			pEntry->rStaRecBf.ucNc = TxBfGetHeNcForIot(pEntry->rStaRecBf.ucNc, (PUINT8)&pEntry->cap.rate.he80_rx_nss_mcs);
			*/

			pEntry->rStaRecBf.uciBfDBW = pEntry->rStaRecBf.ucCBW;
			pEntry->rStaRecBf.uciBfNcol = pEntry->rStaRecBf.ucNc;
			pEntry->rStaRecBf.uciBfNrow = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfTimeOut = 0x18;

			if ((he_bw > HE_BW_80) && (pEntry->MaxHTPhyMode.field.BW == BW_160)) {
				bfer_max_nr = he_bf_struct.snd_dim_gt_bw80;
				bfee_max_nr = pEntry->cap.he_bf.bfee_sts_gt_bw80;
				pEntry->rStaRecBf.nr_bw160 = min(bfer_max_nr, bfee_max_nr);
				/*bw80+80*/
				if (pEntry->cap.ch_bw.he_ch_width & SUPP_160M_8080M_CW_IN_5G_BAND) {
					for (i = 1; i < DOT11AX_MAX_STREAM; i++) {
						if (pEntry->cap.rate.he8080_rx_nss_mcs[i] == 3)
							break;
					}
					ucPeerRxNumSupport = i - 1;

					if (pEntry->rStaRecBf.nc_bw160 > 0)
						pEntry->rStaRecBf.nc_bw160 = min(pEntry->rStaRecBf.nc_bw160, ucPeerRxNumSupport);
					else
						pEntry->rStaRecBf.nc_bw160 = ucPeerRxNumSupport;
				}
				/* bw160 */
				if (pEntry->cap.ch_bw.he_ch_width & SUPP_160M_CW_IN_5G_BAND) {
					for (i = 1; i < DOT11AX_MAX_STREAM; i++) {
						if (pEntry->cap.rate.he160_rx_nss_mcs[i] == 3)
							break;
					}
					ucPeerRxNumSupport = i - 1;
					pEntry->rStaRecBf.nc_bw160 = ucPeerRxNumSupport;
				}
			}

			pEntry->rStaRecBf.trigger_su = txbf_peer_he_trigger_su_feedback(&pEntry->cap.he_bf);
			pEntry->rStaRecBf.trigger_mu = txbf_peer_he_trigger_mu_feedback(&pEntry->cap.he_bf);
			/*
			 * We use higher precision in AP sounding.
			 * For test, we use StaRecBfHeUpdate to change to lower precision.
			 */
			 /*
			pEntry->rStaRecBf.ng16_su = txbf_peer_he_su_ng16_cap(&pEntry->cap.he_bf);
			pEntry->rStaRecBf.ng16_mu = txbf_peer_he_mu_ng16_cap(&pEntry->cap.he_bf);
			pEntry->rStaRecBf.codebook42_su = txbf_peer_he_su_codebook42_cap(&pEntry->cap.he_bf);
			pEntry->rStaRecBf.codebook75_mu = txbf_peer_he_mu_codebook75_cap(&pEntry->cap.he_bf);
			*/
			/* BF_TBD - Wait for CSD estimated result */
			/* pEntry->rStaRecBf.he_ltf = txbf_peer_he_ndp_ltf(pEntry->cap.he_gi); */
			pEntry->rStaRecBf.he_ltf = 0;

			if (man_bf_sta_rec->conf & BIT(MANUAL_HE_RU_RANGE)) {
				pEntry->rStaRecBf.ru_start_idx = man_bf_sta_rec->conf_ru_start_idx;
				pEntry->rStaRecBf.ru_end_idx = man_bf_sta_rec->conf_ru_end_idx;
			}

			if (man_bf_sta_rec->conf & BIT(MANUAL_HE_TRIGGER)) {
				pEntry->rStaRecBf.trigger_su = man_bf_sta_rec->conf_trigger_su;
				pEntry->rStaRecBf.trigger_mu = man_bf_sta_rec->conf_trigger_mu;
			}

			if (man_bf_sta_rec->conf & BIT(MANUAL_HE_NG16)) {
				pEntry->rStaRecBf.ng16_su = man_bf_sta_rec->conf_ng16_su;
				pEntry->rStaRecBf.ng16_mu = man_bf_sta_rec->conf_ng16_mu;
			}

			if (man_bf_sta_rec->conf & BIT(MANUAL_HE_CODEBOOK)) {
				pEntry->rStaRecBf.codebook42_su = man_bf_sta_rec->conf_codebook42_su;
				pEntry->rStaRecBf.codebook75_mu = man_bf_sta_rec->conf_codebook75_mu;
			}

		} else {
			pEntry->rStaRecBf.ucNr = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfDBW = pEntry->MaxHTPhyMode.field.BW;
			pEntry->rStaRecBf.uciBfNrow = ucTxPath - 1;

			/* bw80 */
			for (i = 1; i < DOT11AX_MAX_STREAM; i++) {
				if (pEntry->cap.rate.he80_tx_nss_mcs[i] == 3)
					break;
			}
			ucPeerRxNumSupport = i - 1;
			pEntry->rStaRecBf.ucNc = min(ucPeerRxNumSupport, pEntry->rStaRecBf.ucNr);
			pEntry->rStaRecBf.uciBfNcol = ucPeerRxNumSupport;
			pEntry->rStaRecBf.uciBfTimeOut = 0x18;

			if ((he_bw > HE_BW_80) && (pEntry->MaxHTPhyMode.field.BW == BW_160)) {

				pEntry->rStaRecBf.nr_bw160 = tx_nss_bw160 - 1;
				/*bw80+80*/
				if (pEntry->cap.ch_bw.he_ch_width & SUPP_160M_8080M_CW_IN_5G_BAND) {
					for (i = 1; i < DOT11AX_MAX_STREAM; i++) {
						if (pEntry->cap.rate.he8080_rx_nss_mcs[i] == 3)
							break;
					}
					ucPeerRxNumSupport = i - 1;

					if (pEntry->rStaRecBf.nc_bw160 > 0)
						pEntry->rStaRecBf.nc_bw160 = min(pEntry->rStaRecBf.nc_bw160, ucPeerRxNumSupport);
					else
						pEntry->rStaRecBf.nc_bw160 = ucPeerRxNumSupport;
				}
				/* bw160 */
				if (pEntry->cap.ch_bw.he_ch_width & SUPP_160M_CW_IN_5G_BAND) {
					for (i = 1; i < DOT11AX_MAX_STREAM; i++) {
						if (pEntry->cap.rate.he160_rx_nss_mcs[i] == 3)
							break;
					}
					ucPeerRxNumSupport = i - 1;
					pEntry->rStaRecBf.nc_bw160 = ucPeerRxNumSupport;
				}
				pEntry->rStaRecBf.nc_bw160 = min(pEntry->rStaRecBf.nc_bw160, pEntry->rStaRecBf.nr_bw160);
			}
		}

		break;
#endif /*DOT11_HE_AX*/
#endif
#ifdef DOT11_VHT_AC

	case MODE_VHT:

		if (wlan_config_get_etxbf(pEntry->wdev) == SUBF_ALL || wlan_config_get_etxbf(pEntry->wdev) == SUBF_BFER)
			fgETxBfCap = mt_WrapClientSupportsVhtETxBF(pAd, &pEntry->vht_cap_ie.vht_cap);
		else
			fgETxBfCap = FALSE;

		if (fgETxBfCap)
			pEntry->rStaRecBf.u1TxBfCap      = TXBF_PFMU_STA_ETXBF_SUP;
		else
			pEntry->rStaRecBf.u1TxBfCap      = 0;

		fgITxBfCap                           = wlan_config_get_itxbf(pEntry->wdev);
		if (fgITxBfCap)
			pEntry->rStaRecBf.u1TxBfCap      |= TXBF_PFMU_STA_ITXBF_SUP;

		pEntry->rStaRecBf.fgSU_MU            = FALSE;
		pEntry->rStaRecBf.ucTxMode           = 4; /* VHT mode */
		pEntry->rStaRecBf.ucCBW              = pEntry->MaxHTPhyMode.field.BW;

		if (fgETxBfCap) {
			mt_WrapSetVHTETxBFCap(pAd, pEntry->wdev, &vht_cap);
			pEntry->rStaRecBf.ucSoundingPhy  = 1; /* OFDM */
			pEntry->rStaRecBf.ucNdpaRate     = 73; /* OFDM 24M */
			pEntry->rStaRecBf.ucNdpRate      = 0; /* 0 : MCS0 */
			pEntry->rStaRecBf.ucReptPollRate = 73; /* OFDM 24M */
			ucBFerMaxNr                      = vht_cap.num_snd_dimension;
			ucBFeeMaxNr                      = pEntry->vht_cap_ie.vht_cap.bfee_sts_cap;
			ucBFerCurrNr                     = ucTxPath - 1;
			pEntry->rStaRecBf.ucNr           = (ucBFerMaxNr < ucBFeeMaxNr) ? ucBFerMaxNr : ucBFeeMaxNr;
			pEntry->rStaRecBf.ucNr           = (pEntry->rStaRecBf.ucNr < ucBFerCurrNr) ? pEntry->rStaRecBf.ucNr : ucBFerCurrNr;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 != 3) ? 1 : 0;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 != 3) ? 2 : ucPeerRxNumSupport;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 != 3) ? 3 : ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = (pEntry->rStaRecBf.ucNc > pEntry->rStaRecBf.ucNr) ? pEntry->rStaRecBf.ucNr : ucPeerRxNumSupport;
			pEntry->rStaRecBf.uciBfDBW       = pEntry->MaxHTPhyMode.field.BW;
			pEntry->rStaRecBf.uciBfNcol      = pEntry->rStaRecBf.ucNc;
			pEntry->rStaRecBf.uciBfNrow      = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfTimeOut   = 0x18;

			/* Force eBF Nr from 4 to 2 for AP_CBW160c/nc + STA_DBW160c/nc case
			 * For BW8080 and BW160, MaxHTPhyMode.field.BW=BW_160 (Ref: vht_mode_adjust)
			 */
			if (!IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_DUALPHY) &&
			    pEntry->MaxHTPhyMode.field.BW == BW_160)
				pEntry->rStaRecBf.ucNr = 1;
		} else {
			pEntry->rStaRecBf.ucNr           = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfDBW       = pEntry->MaxHTPhyMode.field.BW;
			pEntry->rStaRecBf.uciBfNrow      = ucTxPath - 1;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 != 3) ? 1 : 0;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 != 3) ? 2 : ucPeerRxNumSupport;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 != 3) ? 3 : ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = (pEntry->rStaRecBf.ucNc > pEntry->rStaRecBf.ucNr) ? pEntry->rStaRecBf.ucNr : ucPeerRxNumSupport;
			pEntry->rStaRecBf.uciBfNcol      = ucPeerRxNumSupport;

			if ((pEntry->MaxHTPhyMode.field.BW <= BW_40) && (pEntry->rStaRecBf.ucNc == 0))
				pEntry->rStaRecBf.uciBfTimeOut   = 0x48;
			else
				pEntry->rStaRecBf.uciBfTimeOut   = 0x18;

			/* Force iBF Nr from 4 to 2 for AP_CBW160c/nc + STA_DBW160c/nc case */
			if (!IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_DUALPHY) &&
			    pEntry->MaxHTPhyMode.field.BW == BW_160)
				pEntry->rStaRecBf.uciBfNrow = 1;
		}

		break;
#endif /* DOT11_VHT_AC */

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:

		if (wlan_config_get_etxbf(pEntry->wdev) == SUBF_ALL || wlan_config_get_etxbf(pEntry->wdev) == SUBF_BFER)
			fgETxBfCap = mt_WrapClientSupportsETxBF(pAd, &pEntry->HTCapability.TxBFCap);
		else
			fgETxBfCap = FALSE;

		if (fgETxBfCap)
			pEntry->rStaRecBf.u1TxBfCap      = TXBF_PFMU_STA_ETXBF_SUP;
		else
			pEntry->rStaRecBf.u1TxBfCap      = 0;

		fgITxBfCap                           = wlan_config_get_itxbf(pEntry->wdev);
		if (fgITxBfCap)
			pEntry->rStaRecBf.u1TxBfCap      |= TXBF_PFMU_STA_ITXBF_SUP;

		pEntry->rStaRecBf.ucTxMode           = 2; /* HT mode */
		pEntry->rStaRecBf.ucCBW              = pEntry->MaxHTPhyMode.field.BW;

		if (fgETxBfCap) {
			pEntry->rStaRecBf.fgSU_MU        = 0;
			ht_cap                           = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(pEntry->wdev);
			ucBFerMaxNr                      = ht_cap->TxBFCap.ChanEstimation;
			ucBFeeMaxNr                      = pEntry->HTCapability.TxBFCap.ComSteerBFAntSup;
			ucBFerCurrNr                     = ucTxPath - 1;
			pEntry->rStaRecBf.ucNr           = (ucBFerMaxNr < ucBFeeMaxNr) ? ucBFerMaxNr : ucBFeeMaxNr;
			pEntry->rStaRecBf.ucNr           = (pEntry->rStaRecBf.ucNr < ucBFerCurrNr) ? pEntry->rStaRecBf.ucNr : ucBFerCurrNr;
			pEntry->rStaRecBf.ucNdpRate      = pEntry->rStaRecBf.ucNr * 8;
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[1] > 0) ? 1 : 0;
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[2] > 0) ? 2 : ucPeerRxNumSupport;
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[3] > 0) ? 3 : ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = (pEntry->rStaRecBf.ucNc > pEntry->rStaRecBf.ucNr) ? pEntry->rStaRecBf.ucNr : ucPeerRxNumSupport;
			pEntry->rStaRecBf.uciBfDBW       = pEntry->MaxHTPhyMode.field.BW;
			pEntry->rStaRecBf.uciBfNcol      = pEntry->rStaRecBf.ucNc;
			pEntry->rStaRecBf.uciBfNrow      = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfTimeOut   = 0x18;
		} else {
			pEntry->rStaRecBf.ucNr           = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfDBW       = pEntry->MaxHTPhyMode.field.BW;
			pEntry->rStaRecBf.uciBfNrow      = ucTxPath - 1;
			/* __________________________________________________________ */
			/* |                |Tx MCS Set |Tx Rx MCS|Tx Max Num Spatial| */
			/* |   Condition    |Defined    |Set N EQL|Streams Supported | */
			/* |________________|___________|_________|__________________| */
			/* |No Tx MCS set is|           |         |                  | */
			/* |defined         |     0     |    0    |        0         | */
			/* |________________|___________|_________|__________________| */
			/* |The Tx MCS set  |           |         |                  | */
			/* |defined to be   |           |         |                  | */
			/* |equal to the Rx |     1     |    0    |        0         | */
			/* |MCS set         |           |         |                  | */
			/* |________________|___________|_________|__________________| */
			/* |The Tx MCS set  |           |         |Set to N for N+1  | */
			/* |may differ from |           |         |spatial stream    | */
			/* |The Rx MCS set  |     1     |    1    |                  | */
			/* |________________|___________|_________|__________________| */
			ucTxMCSSetdefined                = ((pEntry->HTCapability.MCSSet[12] &
												 TX_MCS_SET_DEFINED) >>
												TX_MCS_SET_DEFINED_OFFSET);
			ucTxRxMCSSetNotEqual             = ((pEntry->HTCapability.MCSSet[12] &
												 TX_RX_MCS_SET_N_EQUAL) >>
												TX_RX_MCS_SET_N_EQUAL_OFFSET);
			ucTxMaxNumSpatilStream           = ((pEntry->HTCapability.MCSSet[12] &
												 TX_MAX_NUM_SPATIAL_STREAMS_SUPPORTED) >>
												TX_MAX_NUM_SPATIAL_STREAMS_SUPPORTED_OFFSET);
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[1] > 0) ? 1 : 0;
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[2] > 0) ? 2 : ucPeerRxNumSupport;
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[3] > 0) ? 3 : ucPeerRxNumSupport;

			if ((ucTxMCSSetdefined == 1) && (ucTxRxMCSSetNotEqual == 1))
				ucPeerRxNumSupport           = ucTxMaxNumSpatilStream;

			pEntry->rStaRecBf.ucNc           = ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = (pEntry->rStaRecBf.ucNc > pEntry->rStaRecBf.ucNr) ? pEntry->rStaRecBf.ucNr : ucPeerRxNumSupport;
			pEntry->rStaRecBf.uciBfNcol      = ucPeerRxNumSupport;

			if ((pEntry->MaxHTPhyMode.field.BW <= BW_40) && (pEntry->rStaRecBf.ucNc == 0))
				pEntry->rStaRecBf.uciBfTimeOut   = 0x48;
			else
				pEntry->rStaRecBf.uciBfTimeOut   = 0x18;
		}

#ifdef VHT_TXBF_2G_EPIGRAM_IE
		if (pEntry->rStaBfRecVendorUpdate.fgIsBrcm2GeTxBFIe
				&& (pEntry->rStaBfRecVendorUpdate.Nrow == 3)) {
			fgETxBfCap = TRUE;
			fgETxBfCap &= wlan_config_get_etxbf(pEntry->wdev);
			pEntry->rStaRecBf.u1TxBfCap = 0;
			if (fgETxBfCap)
				pEntry->rStaRecBf.u1TxBfCap |= TXBF_PFMU_STA_ETXBF_SUP;
			pEntry->rStaRecBf.fgSU_MU = FALSE;
			/* VHT Mode */
			pEntry->rStaRecBf.ucTxMode = 4;
			pEntry->rStaRecBf.ucCBW = pEntry->MaxHTPhyMode.field.BW;
			/* OFDM */
			pEntry->rStaRecBf.ucSoundingPhy = 1;
			/* OFDM 24M */
			pEntry->rStaRecBf.ucNdpaRate = 9;
			/* MCS 0 */
			pEntry->rStaRecBf.ucNdpRate = 0;
			/* OFDM 24M */
			pEntry->rStaRecBf.ucReptPollRate = 9;
			ucBFerCurrNr = ucTxPath - 1;
			pEntry->rStaRecBf.ucNr
				= (pEntry->rStaBfRecVendorUpdate.Nrow < ucBFerCurrNr) ?
					pEntry->rStaBfRecVendorUpdate.Nrow : ucBFerCurrNr;
			pEntry->rStaRecBf.ucNc = pEntry->rStaRecBf.ucNr;
			pEntry->rStaRecBf.uciBfDBW = pEntry->MaxHTPhyMode.field.BW;
			pEntry->rStaRecBf.uciBfNcol = pEntry->rStaRecBf.ucNc;
			pEntry->rStaRecBf.uciBfNrow = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfTimeOut = 0x18;
		}
#endif /* VHT_TXBF_2G_EPIGRAM_IE */
		break;

	case MODE_OFDM:
	case MODE_CCK:
		pEntry->rStaRecBf.u1TxBfCap     = 0;
		pEntry->rStaRecBf.ucTxMode      = 1; /* legacy mode */
		pEntry->rStaRecBf.ucNc          = 0;
		pEntry->rStaRecBf.ucNr          = ucTxPath - 1;
		pEntry->rStaRecBf.ucCBW         = pEntry->MaxHTPhyMode.field.BW;
		pEntry->rStaRecBf.uciBfTimeOut  = 0x18;
		pEntry->rStaRecBf.uciBfDBW      = pEntry->MaxHTPhyMode.field.BW;
		pEntry->rStaRecBf.uciBfNcol     = 0;
		pEntry->rStaRecBf.uciBfNrow     = ucTxPath - 1;
		break;

	default:
		ucStatus = FALSE;
		break;
	}

#ifdef DOT11_HE_AX
#ifdef HE_TXBF_SUPPORT
	if (man_bf_sta_rec->conf & BIT(MANUAL_HE_SU_MU)) {
		pEntry->rStaRecBf.fgSU_MU = man_bf_sta_rec->conf_su_mu;
	}
#endif /*HE_TXBF_SUPPORT*/
#endif /* DOT11_HE_AX*/
	/* Once find the phy mode is incorrect, return FALSE alarm */
	if (ucStatus == FALSE)
		return ucStatus;

	if (IS_ETXBF_SUP(pEntry->rStaRecBf.u1TxBfCap) == TRUE) {
		if (pEntry->rStaRecBf.ucNr == (ucTxPath - 1)) {
			index1 = pEntry->rStaRecBf.ucNr;
			index2 = pEntry->rStaRecBf.ucNc;
		} else {
			index1 = ucTxPath - 1;
			index2 = pEntry->rStaRecBf.ucNc;
		}
	} else {
		index1 = pEntry->rStaRecBf.ucNr;
		index2 = pEntry->rStaRecBf.ucNc;
	}

	if ((index1 < 4) && (index2 < 4))
		pEntry->rStaRecBf.ucMemRequire20M = g_ru2PfmuMemReq[index1][index2];
	else
		return FALSE;

	pEntry->rStaRecBf.ucTotMemRequire        = pEntry->rStaRecBf.ucMemRequire20M * g_aPfmuTimeOfMem20M[pEntry->rStaRecBf.ucCBW];
	pEntry->rStaRecBf.ucMemRow0              = 0;
	pEntry->rStaRecBf.ucMemCol0              = 0;
	pEntry->rStaRecBf.ucMemRow1              = 0;
	pEntry->rStaRecBf.ucMemCol1              = 0;
	pEntry->rStaRecBf.ucMemRow2              = 0;
	pEntry->rStaRecBf.ucMemCol2              = 0;
	pEntry->rStaRecBf.ucMemRow3              = 0;
	pEntry->rStaRecBf.ucMemCol3              = 0;
	pEntry->rStaRecBf.u2SmartAnt             = 0;
	/* pEntry->rStaRecBf.ucSEIdx                = 24; */
#ifdef TXBF_DYNAMIC_DISABLE
	pEntry->rStaRecBf.ucAutoSoundingCtrl     = pAd->CommonCfg.ucAutoSoundingCtrl;
#endif /* TXBF_DYNAMIC_DISABLE */
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"====================== BF StaRec Info =====================\n");
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"fgSU_MU        =%d\n", pEntry->rStaRecBf.fgSU_MU);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"BandIdx        =%d\n", band_idx);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"fgETxBfCap     =%d\n", IS_ETXBF_SUP(pEntry->rStaRecBf.u1TxBfCap));
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"fgITxBfCap     =%d\n", IS_ITXBF_SUP(pEntry->rStaRecBf.u1TxBfCap));
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucNdpaRate     =%d\n", pEntry->rStaRecBf.ucNdpaRate);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucNdpRate      =%d\n", pEntry->rStaRecBf.ucNdpRate);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucReptPollRate =%d\n", pEntry->rStaRecBf.ucReptPollRate);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucTxMode       =%d\n", pEntry->rStaRecBf.ucTxMode);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucNc           =%d\n", pEntry->rStaRecBf.ucNc);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucNr           =%d\n", pEntry->rStaRecBf.ucNr);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucCBW          =%d\n", pEntry->rStaRecBf.ucCBW);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucTotMemRequire=%d\n", pEntry->rStaRecBf.ucTotMemRequire);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucMemRequire20M=%d\n", pEntry->rStaRecBf.ucMemRequire20M);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucMemRow0      =%d\n", pEntry->rStaRecBf.ucMemRow0);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucMemCol0      =%d\n", pEntry->rStaRecBf.ucMemCol0);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucMemRow1      =%d\n", pEntry->rStaRecBf.ucMemRow1);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucMemCol1      =%d\n", pEntry->rStaRecBf.ucMemCol1);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucMemRow2      =%d\n", pEntry->rStaRecBf.ucMemRow2);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucMemCol2      =%d\n", pEntry->rStaRecBf.ucMemCol2);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucMemRow3      =%d\n", pEntry->rStaRecBf.ucMemRow3);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucMemCol3      =%d\n", pEntry->rStaRecBf.ucMemCol3);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucSEIdx        =%d\n", pEntry->rStaRecBf.ucSEIdx);
#ifdef TXBF_DYNAMIC_DISABLE
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ucAutoSoundingCtrl=%d\n", pEntry->rStaRecBf.ucAutoSoundingCtrl);
#endif /* TXBF_DYNAMIC_DISABLE */

	if (fgITxBfCap == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
			"uciBfTimeOut   =0x%x\n", pEntry->rStaRecBf.uciBfTimeOut);
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
			"uciBfDBW       =%d\n",   pEntry->rStaRecBf.uciBfDBW);
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
			"uciBfNcol      =%d\n",   pEntry->rStaRecBf.uciBfNcol);
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
			"uciBfNrow      =%d\n",   pEntry->rStaRecBf.uciBfNrow);
	}

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"nr_bw160       =0x%2X\n", pEntry->rStaRecBf.nr_bw160);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"nc_bw160       =0x%2X\n", pEntry->rStaRecBf.nc_bw160);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ru_start_idx   =%d\n", pEntry->rStaRecBf.ru_start_idx);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ru_end_idx     =%d\n", pEntry->rStaRecBf.ru_end_idx);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"trigger_su     =%d\n", pEntry->rStaRecBf.trigger_su);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"trigger_mu     =%d\n", pEntry->rStaRecBf.trigger_mu);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ng16_su       =%d\n", pEntry->rStaRecBf.ng16_su);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"ng16_mu       =%d\n", pEntry->rStaRecBf.ng16_mu);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"codebook42_su  =%d\n", pEntry->rStaRecBf.codebook42_su);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"codebook75_mu  =%d\n", pEntry->rStaRecBf.codebook75_mu);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"he_ltf         =%d\n", pEntry->rStaRecBf.he_ltf);

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"===========================================================\n");

	return ucStatus;
}

INT32 mt_AsicBfeeStaRecUpdate(
	RTMP_ADAPTER *pAd,
	UCHAR        u1PhyMode,
	UCHAR        u1BssIdx,
	UINT16       u2WlanIdx)
{
	PMAC_TABLE_ENTRY pEntry;
	UCHAR ucStatus = TRUE;
	UCHAR u1StaNr;
	BOOLEAN fgStaBfer;
	UINT8 u1TxPath;
#ifdef DBDC_MODE
	UINT8 u1BandIdx = 0;
#endif
	pEntry = &pAd->MacTab.Content[u2WlanIdx];
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_ERROR, "Fail. pEntry null\n");
		return FALSE;
	}

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		u1BandIdx = HcGetBandByWdev(pEntry->wdev);

		if (u1BandIdx == DBDC_BAND0)
			u1TxPath = pAd->dbdc_band0_tx_path;
		else
			u1TxPath = pAd->dbdc_band1_tx_path;
	} else
#endif
	{
		u1TxPath = pAd->Antenna.field.TxPath;
	}

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(pEntry->wdev);
		if (pAd->bAntennaSetAPEnable[BandIdx])
			u1TxPath = pAd->TxStream[BandIdx];
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"dbdc: %u, u1TxPath: %u\n", pAd->CommonCfg.dbdc_mode, u1TxPath);

	os_zero_mem(&pEntry->rStaRecBfee, sizeof(BFEE_STA_REC));
	if (wlan_config_get_etxbf(pEntry->wdev) == SUBF_ALL || wlan_config_get_etxbf(pEntry->wdev) == SUBF_BFEE) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "txbf_get_oui: %u\n", txbf_get_oui());

		if (txbf_get_oui() & BIT(ENUM_BF_OUI_METALINK)) {
			if (get_sta_bfer_nr(pEntry, &fgStaBfer, &u1StaNr)) {
				/* If STA is Bfer and Nr=1(2T) and We Bfee Tx Path=2 (2T), */
				/* set Bfee reply Feedback with Identity Matrix to avoid 2x2 BF negative gain */
				if (fgStaBfer && u1StaNr == 1 && u1TxPath == 2) {
					pEntry->rStaRecBfee.fgFbIdentityMatrix = TRUE;
				}
				MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"u2WlanIdx: %u fgStaBfer:%u, u1StaNr=%u\n", u2WlanIdx, fgStaBfer, u1StaNr);
			}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"====================== BFee StaRec Info =====================\n");
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"fgFbIdentityMatrix: %u\n", pEntry->rStaRecBfee.fgFbIdentityMatrix);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_DEBUG,
		"fgIgnFbk: %u\n", pEntry->rStaRecBfee.fgIgnFbk);

	return ucStatus;
}


INT32 mt_AsicBfStaRecRelease(
	RTMP_ADAPTER * pAd,
	UCHAR        ucBssIdx,
	UINT16       u2WlanIdx)
{
	PMAC_TABLE_ENTRY pEntry;
	UCHAR ucStatus = TRUE;

	pEntry = &pAd->MacTab.Content[u2WlanIdx];
	/* Clear BF StaRec */
	os_zero_mem(&pEntry->rStaRecBf, sizeof(TXBF_PFMU_STA_INFO));
	pEntry->rStaRecBf.u2PfmuId = 0xFFFF;
	{
		STA_REC_CFG_T StaCfg;

		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));

		if (!pEntry->wdev) {
			ASSERT(pEntry->wdev);
			return -1;
		}

		StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
		StaCfg.ConnectionState = TRUE;
		StaCfg.ConnectionType = 0;
		StaCfg.u4EnableFeature = (1 << STA_REC_BF);
		StaCfg.ucBssIndex = ucBssIdx;
		StaCfg.u2WlanIdx = u2WlanIdx;
		StaCfg.pEntry = pEntry;

		if (CmdExtStaRecUpdate(pAd, &StaCfg) != STATUS_TRUE) {
			ucStatus = FALSE;
			MTWF_DBG(pAd, DBG_CAT_BF, CATBF_ASSOC, DBG_LVL_WARN,
				"Something wrong in the BF STA Rec update!!\n");
		}
	}
	return ucStatus;
}


VOID mt_AsicClientBfCap(
	RTMP_ADAPTER * pAd,
	PMAC_TABLE_ENTRY pEntry)
{
#ifdef APCLI_SUPPORT

	if (pAd->fgApClientMode == TRUE) {
		/* Force the Pfmu ID of the other repeater cli to be the same with ApCli */
		AsicTxBfApClientCluster(pAd, pEntry->wcid, pAd->ApCli_CmmWlanId);
	}

#endif /* APCLI_SUPPORT */
}

#if defined(MT_MAC)
VOID TxBfProfileTag_PfmuIdx(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucProfileID)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->pfmu_idx = ucProfileID;
#else
	prPfmuTag1->rField.ucProfileID = ucProfileID;
#endif
}

VOID TxBfProfileTag_TxBfType(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucTxBf)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->ebf = ucTxBf;
#else
	prPfmuTag1->rField.ucTxBf = ucTxBf;
#endif
}

VOID TxBfProfileTag_DBW(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucBw)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->dbw = ucBw;
#else
	prPfmuTag1->rField.ucDBW = ucBw;
#endif
}

VOID TxBfProfileTag_SuMu(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucSuMu)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->su_mu = ucSuMu;
#else
	prPfmuTag1->rField.ucSU_MU = ucSuMu;
#endif
}

VOID TxBfProfileTag_InValid(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR InvalidFlg)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->invalid_prof = InvalidFlg;
#else
	prPfmuTag1->rField.ucInvalidProf = InvalidFlg;
#endif
}

VOID TxBfProfileTag_Mem(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN PUCHAR pMemAddrColIdx,
	IN PUCHAR pMemAddrRowIdx)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->mem_col0 = pMemAddrColIdx[0];
	pfmu_tag_info->mem_row0 = pMemAddrRowIdx[0];
	pfmu_tag_info->mem_col1 = pMemAddrColIdx[1];
	pfmu_tag_info->mem_row1 = pMemAddrRowIdx[1];
	pfmu_tag_info->mem_col2 = pMemAddrColIdx[2];
	pfmu_tag_info->mem_row2 = pMemAddrRowIdx[2];
	pfmu_tag_info->mem_col3 = pMemAddrColIdx[3];
	pfmu_tag_info->mem_row3 = pMemAddrRowIdx[3];
#else
	prPfmuTag1->rField.ucMemAddr1ColIdx = pMemAddrColIdx[0];
	prPfmuTag1->rField.ucMemAddr1RowIdx = pMemAddrRowIdx[0];
	prPfmuTag1->rField.ucMemAddr2ColIdx = pMemAddrColIdx[1];
	prPfmuTag1->rField.ucMemAddr2RowIdx = pMemAddrRowIdx[1] & 0x1F;
	prPfmuTag1->rField.ucMemAddr2RowIdxMsb = pMemAddrRowIdx[1] >> 5;
	prPfmuTag1->rField.ucMemAddr3ColIdx = pMemAddrColIdx[2];
	prPfmuTag1->rField.ucMemAddr3RowIdx = pMemAddrRowIdx[2];
	prPfmuTag1->rField.ucMemAddr4ColIdx = pMemAddrColIdx[3];
	prPfmuTag1->rField.ucMemAddr4RowIdx = pMemAddrRowIdx[3];
#endif
}


VOID TxBfProfileTag_Matrix(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucNrow,
	IN UCHAR ucNcol,
	IN UCHAR ucNgroup,
	IN UCHAR ucLM,
	IN UCHAR ucCodeBook,
	IN UCHAR ucHtcExist)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->lm = ucLM;
	pfmu_tag_info->nr = ucNrow;
	pfmu_tag_info->nc = ucNcol;
	pfmu_tag_info->codebook = ucCodeBook;
	pfmu_tag_info->ng = ucNgroup;
#else
	prPfmuTag1->rField.ucNrow    = ucNrow;
	prPfmuTag1->rField.ucNcol    = ucNcol;
	prPfmuTag1->rField.ucNgroup  = ucNgroup;
	prPfmuTag1->rField.ucLM      = ucLM;
	prPfmuTag1->rField.ucCodeBook = ucCodeBook;
	prPfmuTag1->rField.ucHtcExist = ucHtcExist;
#endif
}


VOID TxBfProfileTag_SNR(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucSNR_STS0,
	IN UCHAR ucSNR_STS1,
	IN UCHAR ucSNR_STS2,
	IN UCHAR ucSNR_STS3)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->snr_sts0 = ucSNR_STS0;
	pfmu_tag_info->snr_sts1 = ucSNR_STS1;
	pfmu_tag_info->snr_sts2 = ucSNR_STS2;
	pfmu_tag_info->snr_sts3 = ucSNR_STS3;
#else
	prPfmuTag1->rField.ucSNR_STS0 = ucSNR_STS0;
	prPfmuTag1->rField.ucSNR_STS1 = ucSNR_STS1;
	prPfmuTag1->rField.ucSNR_STS2 = ucSNR_STS2;
	prPfmuTag1->rField.ucSNR_STS3 = ucSNR_STS3;
#endif
}


VOID TxBfProfileTag_SmtAnt(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN USHORT u2SmartAnt)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->smart_ant = u2SmartAnt;
#else
	prPfmuTag2->rField.u2SmartAnt = u2SmartAnt;
#endif
}



VOID TxBfProfileTag_SeIdx(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR ucSEIdx)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->se_idx = ucSEIdx;
#else
	prPfmuTag2->rField.ucSEIdx = ucSEIdx;
#endif
}


VOID TxBfProfileTag_RmsdThd(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR ucRMSDThd)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->rmsd = ucRMSDThd;
#else
	prPfmuTag2->rField.ucRMSDThd = ucRMSDThd;
#endif
}


VOID TxBfProfileTag_McsThd(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN PUCHAR pMCSThLSS,
	IN PUCHAR pMCSThSSS)
{
#if defined(DOT11_HE_AX)

#else
	prPfmuTag2->rField.ucMCSThL1SS = pMCSThLSS[0];
	prPfmuTag2->rField.ucMCSThS1SS = pMCSThSSS[0];
	prPfmuTag2->rField.ucMCSThL2SS = pMCSThLSS[1];
	prPfmuTag2->rField.ucMCSThS2SS = pMCSThSSS[1];
	prPfmuTag2->rField.ucMCSThL3SS = pMCSThLSS[2];
	prPfmuTag2->rField.ucMCSThS3SS = pMCSThSSS[2];
#endif
}


VOID TxBfProfileTag_TimeOut(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif

	IN UCHAR uciBfTimeOut)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->ibf_timeout = uciBfTimeOut;
#else
	prPfmuTag2->rField.uciBfTimeOut = uciBfTimeOut;
#endif
}


VOID TxBfProfileTag_DesiredBW(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR uciBfDBW)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->ibf_desired_dbw = uciBfDBW;
#else
	prPfmuTag2->rField.uciBfDBW = uciBfDBW;
#endif
}


VOID TxBfProfileTag_DesiredNc(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR uciBfNcol)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->ibf_desired_ncol = uciBfNcol;
#else
	prPfmuTag2->rField.uciBfNcol = uciBfNcol;
#endif
}


VOID TxBfProfileTag_DesiredNr(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR uciBfNrow)
{
#if defined(DOT11_HE_AX)
	pfmu_tag_info->ibf_desired_nrow = uciBfNrow;
#else
	prPfmuTag2->rField.uciBfNrow = uciBfNrow;
#endif
}

INT TxBfProfileTagRead(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR		PfmuIdx,
	IN BOOLEAN              fgBFer)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPfmuProfileTagRead(pAd, PfmuIdx, fgBFer) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


INT TxBfProfileTagWrite(
	IN PRTMP_ADAPTER	pAd,
	IN P_PFMU_PROFILE_TAG1  prPfmuTag1,
	IN P_PFMU_PROFILE_TAG2  prPfmuTag2,
	IN UCHAR		profileIdx)
{
#if defined(DOT11_HE_AX)
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	BOOLEAN  fgStatus = FALSE;


	if (ops->write_txbf_pfmu_tag)
		fgStatus = ops->write_txbf_pfmu_tag(pAd->hdev_ctrl, profileIdx);
	else
		fgStatus = FALSE;
#else
	BOOLEAN  fgStatus = FALSE;


	if (CmdETxBfPfmuProfileTagWrite(pAd,
									(PUCHAR) (prPfmuTag1),
									(PUCHAR) (prPfmuTag2),
									sizeof(*prPfmuTag1),
									sizeof(*prPfmuTag2),
									profileIdx) == STATUS_TRUE)
		fgStatus = TRUE;
#endif

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG,
				"============================= TxBf profile Tage1 Info ========================================\n"
				"Row data0 = 0x%x, Row data1 = 0x%x, Row data2 = 0x%x, Row data3 = 0x%x\n\n"
				"PFMU ID = %d        Invalid status = %d\n"
				"iBf/eBf = %d\n\n"
				"DBW   = %d\n"
				"SU/MU = %d\n"
				"RMSD  = %d\n"
				"nrow=%d, ncol=%d, ng=%d, LM=%d, CodeBook=%d, HtcExist=%d\n\n"
				"Mem Col1 = %d, Mem Row1 = %d, Mem Col2 = %d, Mem Row2 = %d\n"
				"Mem Col3 = %d, Mem Row3 = %d, Mem Col4 = %d, Mem Row4 = %d\n\n"
				"STS0_SNR =0x%x, STS1_SNR=0x%x, STS2_SNR=0x%x, STS3_SNR=0x%x\n\n"
				"iBf LNA Idx=0x%x\n"
				"==============================================================================================\n",
				prPfmuTag1->au4RawData[0], prPfmuTag1->au4RawData[1], prPfmuTag1->au4RawData[2], prPfmuTag1->au4RawData[3],
				prPfmuTag1->rField.ucProfileID,      prPfmuTag1->rField.ucInvalidProf,
				prPfmuTag1->rField.ucTxBf,
				prPfmuTag1->rField.ucDBW,
				prPfmuTag1->rField.ucSU_MU,
				prPfmuTag1->rField.ucRMSD,
				prPfmuTag1->rField.ucNrow,           prPfmuTag1->rField.ucNcol,          prPfmuTag1->rField.ucNgroup, prPfmuTag1->rField.ucLM,
				prPfmuTag1->rField.ucCodeBook,       prPfmuTag1->rField.ucHtcExist,
				prPfmuTag1->rField.ucMemAddr1ColIdx, prPfmuTag1->rField.ucMemAddr1RowIdx,
				prPfmuTag1->rField.ucMemAddr2ColIdx, (prPfmuTag1->rField.ucMemAddr2RowIdx | (prPfmuTag1->rField.ucMemAddr2RowIdxMsb << 5)),
				prPfmuTag1->rField.ucMemAddr3ColIdx, prPfmuTag1->rField.ucMemAddr3RowIdx,
				prPfmuTag1->rField.ucMemAddr4ColIdx, prPfmuTag1->rField.ucMemAddr4RowIdx,
				prPfmuTag1->rField.ucSNR_STS0,       prPfmuTag1->rField.ucSNR_STS1,
				prPfmuTag1->rField.ucSNR_STS2,       prPfmuTag1->rField.ucSNR_STS3,
				prPfmuTag1->rField.ucIBfLnaIdx);
	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG,
				"============================= TxBf profile Tage2 Info ========================================\n"
				"Row data0 = 0x%x, Row data1 = 0x%x, Row data2 = 0x%x\n\n"
				"Smart antenna ID = 0x%x,  SE index = %d\n"
				"RMSD threshold = %d\n"
				"MCS L1SS thd = %d, S1SS thd = %d, L2SS thd = %d, S2SS thd = %d, L3SS thd = %d, S3SS thd = %d\n"
				"Time out = 0x%x\n"
				"Desired BW = %d, Desired Ncol = %d, Desired Nrow = %d\n"
				"==============================================================================================\n",
				prPfmuTag2->au4RawData[0], prPfmuTag2->au4RawData[1], prPfmuTag2->au4RawData[2],
				prPfmuTag2->rField.u2SmartAnt,   prPfmuTag2->rField.ucSEIdx,
				prPfmuTag2->rField.ucRMSDThd,
				prPfmuTag2->rField.ucMCSThL1SS,  prPfmuTag2->rField.ucMCSThS1SS,
				prPfmuTag2->rField.ucMCSThL2SS, prPfmuTag2->rField.ucMCSThS2SS,
				prPfmuTag2->rField.ucMCSThL3SS,  prPfmuTag2->rField.ucMCSThS3SS,
				prPfmuTag2->rField.uciBfTimeOut, prPfmuTag2->rField.uciBfDBW,
				prPfmuTag2->rField.uciBfNcol, prPfmuTag2->rField.uciBfNrow);
	return fgStatus;
}


VOID TxBfProfileTagPrint(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN       fgBFer,
	IN PUCHAR        pBuf)
{
	P_PFMU_PROFILE_TAG1 prPfmuTag1;
	P_PFMU_PROFILE_TAG2 prPfmuTag2;

	prPfmuTag1 = (P_PFMU_PROFILE_TAG1) pBuf;
	prPfmuTag2 = (P_PFMU_PROFILE_TAG2) (pBuf + sizeof(PFMU_PROFILE_TAG1));
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange((char *)prPfmuTag1, sizeof(PFMU_PROFILE_TAG1));
	RTMPEndianChange((char *)prPfmuTag2, sizeof(PFMU_PROFILE_TAG2));
#endif
	NdisCopyMemory(&pAd->rPfmuTag1, prPfmuTag1, sizeof(PFMU_PROFILE_TAG1));
	NdisCopyMemory(&pAd->rPfmuTag2, prPfmuTag2, sizeof(PFMU_PROFILE_TAG2));
	MTWF_PRINT("============================= TxBf profile Tage1 Info ========================================\n"
				"Row data0 = 0x%x, Row data1 = 0x%x, Row data2 = 0x%x, Row data3 = 0x%x\n\n"
				"PFMU ID = %d        Invalid status = %d\n"
				"iBf/eBf = %d\n\n"
				"DBW   = %d\n"
				"SU/MU = %d\n"
				"RMSD  = %d\n"
				"nrow=%d, ncol=%d, ng=%d, LM=%d, CodeBook=%d, HtcExist=%d\n\n"
				"Mem Col1 = %d, Mem Row1 = %d, Mem Col2 = %d, Mem Row2 = %d\n"
				"Mem Col3 = %d, Mem Row3 = %d, Mem Col4 = %d, Mem Row4 = %d\n\n"
				"STS0_SNR =0x%x, STS1_SNR=0x%x, STS2_SNR=0x%x, STS3_SNR=0x%x\n\n"
				"iBf LNA Idx=0x%x\n"
				"==============================================================================================\n",
				prPfmuTag1->au4RawData[0], prPfmuTag1->au4RawData[1], prPfmuTag1->au4RawData[2], prPfmuTag1->au4RawData[3],
				prPfmuTag1->rField.ucProfileID,      prPfmuTag1->rField.ucInvalidProf,
				prPfmuTag1->rField.ucTxBf,
				prPfmuTag1->rField.ucDBW,
				prPfmuTag1->rField.ucSU_MU,
				prPfmuTag1->rField.ucRMSD,
				prPfmuTag1->rField.ucNrow,           prPfmuTag1->rField.ucNcol,          prPfmuTag1->rField.ucNgroup, prPfmuTag1->rField.ucLM,
				prPfmuTag1->rField.ucCodeBook,       prPfmuTag1->rField.ucHtcExist,
				prPfmuTag1->rField.ucMemAddr1ColIdx, prPfmuTag1->rField.ucMemAddr1RowIdx,
				prPfmuTag1->rField.ucMemAddr2ColIdx, (prPfmuTag1->rField.ucMemAddr2RowIdx | (prPfmuTag1->rField.ucMemAddr2RowIdxMsb << 5)),
				prPfmuTag1->rField.ucMemAddr3ColIdx, prPfmuTag1->rField.ucMemAddr3RowIdx,
				prPfmuTag1->rField.ucMemAddr4ColIdx, prPfmuTag1->rField.ucMemAddr4RowIdx,
				prPfmuTag1->rField.ucSNR_STS0,       prPfmuTag1->rField.ucSNR_STS1,
				prPfmuTag1->rField.ucSNR_STS2,       prPfmuTag1->rField.ucSNR_STS3,
				prPfmuTag1->rField.ucIBfLnaIdx);

	if (fgBFer == TRUE) {
		MTWF_PRINT("============================= TxBf profile Tage2 Info ========================================\n"
					"Row data0 = 0x%x, Row data1 = 0x%x, Row data2 = 0x%x\n\n"
					"Smart antenna ID = 0x%x,  SE index = %d\n"
					"RMSD threshold = %d\n"
					"MCS L1SS thd = %d, S1SS thd = %d, L2SS thd = %d, S2SS thd = %d, L3SS thd = %d, S3SS thd = %d\n"
					"Time out = 0x%x\n"
					"Desired BW = %d, Desired Ncol = %d, Desired Nrow = %d\n"
					"==============================================================================================\n",
					prPfmuTag2->au4RawData[0], prPfmuTag2->au4RawData[1], prPfmuTag2->au4RawData[2],
					prPfmuTag2->rField.u2SmartAnt, prPfmuTag2->rField.ucSEIdx,
					prPfmuTag2->rField.ucRMSDThd,
					prPfmuTag2->rField.ucMCSThL1SS, prPfmuTag2->rField.ucMCSThS1SS, prPfmuTag2->rField.ucMCSThL2SS,
					prPfmuTag2->rField.ucMCSThS2SS, prPfmuTag2->rField.ucMCSThL3SS,  prPfmuTag2->rField.ucMCSThS3SS,
					prPfmuTag2->rField.uciBfTimeOut, prPfmuTag2->rField.uciBfDBW, prPfmuTag2->rField.uciBfNcol,
					prPfmuTag2->rField.uciBfNrow);
	}
}


INT TxBfProfilePnRead(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         profileIdx)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPfmuProfilePnRead(pAd, profileIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


INT TxBfProfilePnWrite(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         PfmuIdx,
	IN UCHAR         ucBw,
	IN PUCHAR        pProfileData)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPfmuProfilePnWrite(pAd, PfmuIdx, ucBw, pProfileData) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


VOID TxBfProfilePnPrint(
	IN UCHAR  ucBw,
	IN PUCHAR pBuf)
{
	P_PFMU_PN_DBW20M    prPfmuPn20M;
	P_PFMU_PN_DBW40M    prPfmuPn40M;
	P_PFMU_PN_DBW80M    prPfmuPn80M;
	P_PFMU_PN_DBW80_80M prPfmuPn160M;

	switch (ucBw) {
	case P_DBW20M:
		prPfmuPn20M = (P_PFMU_PN_DBW20M) pBuf;
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange((UCHAR *)prPfmuPn20M, sizeof(PFMU_PN_DBW20M));
#endif
		MTWF_PRINT("============================= TxBf profile PN Info 20M ========================================\n"
					"1STS_TX0 = 0x%x, 1STS_TX1 = 0x%x, 1STS_TX2 = 0x%x, 1STS_TX3 = 0x%x\n"
					"2STS_TX0 = 0x%x, 2STS_TX1 = 0x%x, 2STS_TX2 = 0x%x, 2STS_TX3 = 0x%x\n"
					"3STS_TX0 = 0x%x, 3STS_TX1 = 0x%x, 3STS_TX2 = 0x%x, 3STS_TX3 = 0x%x\n"
					"===============================================================================================\n",
					prPfmuPn20M->rField.u2DBW20_1STS_Tx0, prPfmuPn20M->rField.u2DBW20_1STS_Tx1,
					prPfmuPn20M->rField.u2DBW20_1STS_Tx2 | (prPfmuPn20M->rField.u2DBW20_1STS_Tx2Msb << 11),
					prPfmuPn20M->rField.u2DBW20_1STS_Tx3,
					prPfmuPn20M->rField.u2DBW20_2STS_Tx0,
					prPfmuPn20M->rField.u2DBW20_2STS_Tx1 | (prPfmuPn20M->rField.u2DBW20_2STS_Tx1Msb << 11),
					prPfmuPn20M->rField.u2DBW20_2STS_Tx2, prPfmuPn20M->rField.u2DBW20_2STS_Tx3,
					prPfmuPn20M->rField.u2DBW20_3STS_Tx0, prPfmuPn20M->rField.u2DBW20_3STS_Tx1,
					prPfmuPn20M->rField.u2DBW20_3STS_Tx2,
					prPfmuPn20M->rField.u2DBW20_3STS_Tx3 | (prPfmuPn20M->rField.u2DBW20_3STS_Tx3Msb << 11));
		break;

	case P_DBW40M:
		prPfmuPn40M = (P_PFMU_PN_DBW40M) pBuf;
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange((UCHAR *)prPfmuPn40M, sizeof(PFMU_PN_DBW40M));
#endif
		MTWF_PRINT("============================= TxBf profile PN Info 40M ========================================\n"
					"1STS_TX0 = 0x%x, 1STS_TX1 = 0x%x, 1STS_TX2 = 0x%x, 1STS_TX3 = 0x%x\n"
					"2STS_TX0 = 0x%x, 2STS_TX1 = 0x%x, 2STS_TX2 = 0x%x, 2STS_TX3 = 0x%x\n"
					"3STS_TX0 = 0x%x, 3STS_TX1 = 0x%x, 3STS_TX2 = 0x%x, 3STS_TX3 = 0x%x\n"
					"===============================================================================================\n",
					prPfmuPn40M->rField.u2DBW40_1STS_Tx0, prPfmuPn40M->rField.u2DBW40_1STS_Tx1,
					prPfmuPn40M->rField.u2DBW40_1STS_Tx2 | (prPfmuPn40M->rField.u2DBW40_1STS_Tx2Msb << 11),
					prPfmuPn40M->rField.u2DBW40_1STS_Tx3,
					prPfmuPn40M->rField.u2DBW40_2STS_Tx0,
					prPfmuPn40M->rField.u2DBW40_2STS_Tx1 | (prPfmuPn40M->rField.u2DBW40_2STS_Tx1Msb << 11),
					prPfmuPn40M->rField.u2DBW40_2STS_Tx2, prPfmuPn40M->rField.u2DBW40_2STS_Tx3,
					prPfmuPn40M->rField.u2DBW40_3STS_Tx0, prPfmuPn40M->rField.u2DBW40_3STS_Tx1,
					prPfmuPn40M->rField.u2DBW40_3STS_Tx2,
					prPfmuPn40M->rField.u2DBW40_3STS_Tx3 | (prPfmuPn40M->rField.u2DBW40_3STS_Tx3Msb << 11));
		break;

	case P_DBW80M:
		prPfmuPn80M = (P_PFMU_PN_DBW80M) pBuf;
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange((UCHAR *)prPfmuPn80M, sizeof(PFMU_PN_DBW80M));
#endif
		MTWF_PRINT("============================= TxBf profile PN Info 80M ========================================\n"
					"1STS_TX0 = 0x%x, 1STS_TX1 = 0x%x, 1STS_TX2 = 0x%x, 1STS_TX3 = 0x%x\n"
					"2STS_TX0 = 0x%x, 2STS_TX1 = 0x%x, 2STS_TX2 = 0x%x, 2STS_TX3 = 0x%x\n"
					"3STS_TX0 = 0x%x, 3STS_TX1 = 0x%x, 3STS_TX2 = 0x%x, 3STS_TX3 = 0x%x\n"
					"===============================================================================================\n",
					prPfmuPn80M->rField.u2DBW80_1STS_Tx0, prPfmuPn80M->rField.u2DBW80_1STS_Tx1,
					prPfmuPn80M->rField.u2DBW80_1STS_Tx2 | (prPfmuPn80M->rField.u2DBW80_1STS_Tx2Msb << 11),
					prPfmuPn80M->rField.u2DBW80_1STS_Tx3,
					prPfmuPn80M->rField.u2DBW80_2STS_Tx0,
					prPfmuPn80M->rField.u2DBW80_2STS_Tx1 | (prPfmuPn80M->rField.u2DBW80_2STS_Tx1Msb << 11),
					prPfmuPn80M->rField.u2DBW80_2STS_Tx2, prPfmuPn80M->rField.u2DBW80_2STS_Tx3,
					prPfmuPn80M->rField.u2DBW80_3STS_Tx0, prPfmuPn80M->rField.u2DBW80_3STS_Tx1,
					prPfmuPn80M->rField.u2DBW80_3STS_Tx2,
					prPfmuPn80M->rField.u2DBW80_3STS_Tx3 | (prPfmuPn80M->rField.u2DBW80_3STS_Tx3Msb << 11));
		break;

	case P_DBW160M:
		prPfmuPn160M = (P_PFMU_PN_DBW80_80M) pBuf;
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange((UCHAR *)prPfmuPn160M, sizeof(PFMU_PN_DBW80_80M));
#endif
		MTWF_PRINT("============================= TxBf profile PN Info 80M ========================================\n"
					"1STS_TX0 = 0x%x, 1STS_TX1 = 0x%x\n"
					"2STS_TX0 = 0x%x, 2STS_TX1 = 0x%x\n"
					"===============================================================================================\n",
					prPfmuPn160M->rField.u2DBW160_1STS_Tx0, prPfmuPn160M->rField.u2DBW160_1STS_Tx1,
					prPfmuPn160M->rField.u2DBW160_2STS_Tx0 | (prPfmuPn160M->rField.u2DBW160_2STS_Tx0Msb << 11),
					prPfmuPn160M->rField.u2DBW160_2STS_Tx1);
		break;

	default:
		break;
	}
}

/* SC_TABLE_ENTRY g_arSubCarIdxBwTbl[4]={{228, 255, 1, 28}, {198, 254, 2, 58}, {134, 254, 2, 122}, {134, 254, 2, 122}}; */

INT TxBfProfileDataRead(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         profileIdx,
	IN BOOLEAN       fgBFer,
	IN USHORT        subCarrIdx)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPfmuProfileDataRead(pAd, profileIdx, fgBFer, subCarrIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


INT TxBfProfileDataWrite(
	IN PRTMP_ADAPTER pAd,
	IN PUSHORT Input
)
{
	BOOLEAN fgStatus = FALSE;
	UCHAR profileIdx;
	USHORT subcarrierIdx;
	PFMU_DATA rPfmuProfileData;
	PUCHAR pProfile;

	profileIdx = Input[0];
	subcarrierIdx = Input[1];
	rPfmuProfileData.rField.u2Phi11  = Input[2];
	rPfmuProfileData.rField.ucPsi21  = Input[3];
	rPfmuProfileData.rField.u2Phi21  = Input[4];
	rPfmuProfileData.rField.ucPsi31  = Input[5];
	rPfmuProfileData.rField.u2Phi31  = Input[6];
	rPfmuProfileData.rField.ucPsi41  = Input[7];
	rPfmuProfileData.rField.u2Phi22  = Input[8];
	rPfmuProfileData.rField.ucPsi32  = Input[9];
	rPfmuProfileData.rField.u2Phi32  = Input[10];
	rPfmuProfileData.rField.ucPsi42  = Input[11];
	rPfmuProfileData.rField.u2Phi33  = Input[12];
	rPfmuProfileData.rField.ucPsi43  = Input[13];
	rPfmuProfileData.rField.u2dSNR00 = Input[14];
	rPfmuProfileData.rField.u2dSNR01 = Input[15];
	rPfmuProfileData.rField.u2dSNR02 = Input[16];
	rPfmuProfileData.rField.u2dSNR03 = Input[17];

	pProfile = (PUCHAR)&rPfmuProfileData;
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange(pProfile, sizeof(PFMU_DATA));
#endif

	if (CmdETxBfPfmuProfileDataWrite(pAd, profileIdx, subcarrierIdx, pProfile)
			== STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxBfQdRead(
	IN PRTMP_ADAPTER pAd,
	IN INT8          subCarrIdx)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfQdRead(pAd, subCarrIdx) == STATUS_TRUE) {
		fgStatus = TRUE;
	}

	return fgStatus;
}

INT TxBfFbRptDbgInfo(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucData)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfFbRptDbgInfo(pAd, pucData) == STATUS_TRUE) {
		fgStatus = TRUE;
	}
	return fgStatus;
}

INT TxBfTxSndInfo(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucData)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfTxSndInfo(pAd, pucData) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxBfPlyInfo(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucData)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPlyInfo(pAd, pucData) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxBfTxCmd(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucData)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfTxCmd(pAd, pucData) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxBfSndCnt(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucData)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfSndCnt(pAd, pucData) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT HeRaMuMetricInfo(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucData)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdHeRaMuMetricInfo(pAd, pucData) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

VOID TxBfFbRptDbgInfoPrint(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucBuf)
{
	P_BF_SND_FBRPT_STATISTICS_T pBfFbRptInfo = NULL;
	UINT32 u4Total, i, u4BufOffset = 0;
	UINT32 u4PollPFMUIntrStatTimeOut, u4FbRptDeQInterval;
	UINT32 u4PktCntInFbRptTimeOutQ, u4PktCntInFbRptQ;

	memcpy((PUINT8)&u4FbRptDeQInterval, pucBuf, sizeof(UINT32));
	u4BufOffset += sizeof(UINT32);
	memcpy((PUINT8)&u4PollPFMUIntrStatTimeOut, (pucBuf + u4BufOffset), sizeof(UINT32));
	u4BufOffset += sizeof(UINT32);
	memcpy((PUINT8)&u4PktCntInFbRptTimeOutQ, (pucBuf + u4BufOffset), sizeof(UINT32));
	u4BufOffset += sizeof(UINT32);
	memcpy((PUINT8)&u4PktCntInFbRptQ, (pucBuf + u4BufOffset), sizeof(UINT32));
	u4BufOffset += sizeof(UINT32);
	pBfFbRptInfo = (P_BF_SND_FBRPT_STATISTICS_T)(pucBuf + u4BufOffset);

	u4Total = pBfFbRptInfo->u4PFMUWRDoneCnt + pBfFbRptInfo->u4PFMUWRFailCnt;
	u4Total += pBfFbRptInfo->u4PFMUWRTimeoutFreeCnt + pBfFbRptInfo->u4FbRptPktDropCnt;

	MTWF_PRINT("\n");
	MTWF_PRINT("\x1b[32m ===========================================================\x1b[m\n");
	MTWF_PRINT("\x1b[32m PFMUWRDoneCnt              = %u\x1b[m\n", pBfFbRptInfo->u4PFMUWRDoneCnt);
	MTWF_PRINT("\x1b[32m PFMUWRFailCnt              = %u\x1b[m\n", pBfFbRptInfo->u4PFMUWRFailCnt);
	MTWF_PRINT("\x1b[32m PFMUWRTimeOutCnt           = %u\x1b[m\n", pBfFbRptInfo->u4PFMUWRTimeOutCnt);
	MTWF_PRINT("\x1b[32m PFMUWRTimeoutFreeCnt       = %u\x1b[m\n", pBfFbRptInfo->u4PFMUWRTimeoutFreeCnt);
	MTWF_PRINT("\x1b[32m FbRptPktDropCnt            = %u\x1b[m\n", pBfFbRptInfo->u4FbRptPktDropCnt);
	MTWF_PRINT("\x1b[32m TotalFbRptPkt              = %u\x1b[m\n", u4Total);
	MTWF_PRINT("\x1b[32m PollPFMUIntrStatTimeOut    = %u(micro-sec)\x1b[m\n", u4PollPFMUIntrStatTimeOut);
	MTWF_PRINT("\x1b[32m FbRptDeQInterval           = %u(milli-sec)\x1b[m\n", u4FbRptDeQInterval);
	MTWF_PRINT("\x1b[32m PktCntInFbRptTimeOutQ      = %u\x1b[m\n", u4PktCntInFbRptTimeOutQ);
	MTWF_PRINT("\x1b[32m PktCntInFbRptQ             = %u\x1b[m\n", u4PktCntInFbRptQ);

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (pEntry->EntryType == ENTRY_NONE)
			continue;

		MTWF_PRINT("\x1b[32m AID%u  RxFbRptCnt           = %u\x1b[m\n"
			, pEntry->Aid, pBfFbRptInfo->u4RxPerStaFbRptCnt[i]);
	}

	MTWF_PRINT("\x1b[32m ===========================================================\x1b[m\n");
	MTWF_PRINT("\n");
}

VOID TxBfTxSndInfoPrint(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucBuf)
{
	P_BF_SND_CTRL_EVT_T pSndCtrlEvt = (P_BF_SND_CTRL_EVT_T)pucBuf;
	P_BF_SND_CFG_T prSndCfg = &pSndCtrlEvt->rSndCfg;
	P_BF_SND_TRG_INFO_T prSndTrgInfo = &pSndCtrlEvt->rSndTrgInfo;
	P_BF_SND_STA_INFO_T prSndStaInfo;
	UINT16 Idx;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	MTWF_PRINT("============================= Global Setting ========================================\n");

	MTWF_PRINT("VhtOpt = 0x%02X, HeOpt = 0x%02X, GloOpt = 0x%02X\n",
		prSndCfg->u1VhtOpt, prSndCfg->u1HeOpt, prSndCfg->u1GloOpt);

	for (Idx = 0; Idx < (wtbl_max_num / 32); Idx += 4) {
		MTWF_PRINT("SuSta[%d] = 0x%08X, SuSta[%d] = 0x%08X, SuSta[%d] = 0x%08X, SuSta[%d] = 0x%08X\n",
			Idx, pSndCtrlEvt->au4SndRecSuSta[Idx],
			(Idx+1), pSndCtrlEvt->au4SndRecSuSta[Idx+1],
			(Idx+2), pSndCtrlEvt->au4SndRecSuSta[Idx+2],
			(Idx+3), pSndCtrlEvt->au4SndRecSuSta[Idx+3]);
	}

	for (Idx = 0; Idx < (wtbl_max_num / 32); Idx += 4) {
		MTWF_PRINT("VhtMuSta[%d] = 0x%08X, VhtMuSta[%d] = 0x%08X, VhtMuSta[%d] = 0x%08X, VhtMuSta[%d] = 0x%08X\n",
			Idx, pSndCtrlEvt->au4SndRecVhtMuSta[Idx],
			(Idx+1), pSndCtrlEvt->au4SndRecVhtMuSta[Idx+1],
			(Idx+2), pSndCtrlEvt->au4SndRecVhtMuSta[Idx+2],
			(Idx+3), pSndCtrlEvt->au4SndRecVhtMuSta[Idx+3]);
	}

	for (Idx = 0; Idx < (wtbl_max_num / 32); Idx += 4) {
		MTWF_PRINT("HeTBSta[%d] = 0x%08X, HeTBSta[%d] = 0x%08X, HeTBSta[%d] = 0x%08X, HeTBSta[%d] = 0x%08X\n",
			Idx, pSndCtrlEvt->au4SndRecHeTBSta[Idx],
			(Idx+1), pSndCtrlEvt->au4SndRecHeTBSta[Idx+1],
			(Idx+2), pSndCtrlEvt->au4SndRecHeTBSta[Idx+2],
			(Idx+3), pSndCtrlEvt->au4SndRecHeTBSta[Idx+3]);
	}

	for (Idx = 0; Idx < DBDC_BAND_NUM; Idx++) {
		MTWF_PRINT("Band%u:\n", Idx);
		MTWF_PRINT("    Wlan Idx For VHT MC Sounding = %u\n", pSndCtrlEvt->u2WlanIdxForMcSnd[Idx]);
		MTWF_PRINT("    Wlan Idx For TB Sounding = %u\n", pSndCtrlEvt->u2WlanIdxForTbSnd[Idx]);
	}

	MTWF_PRINT("ULLen = %d, ULMcs = %d, ULLDCP = %d\n",
		prSndTrgInfo->u2ULLength, prSndTrgInfo->u1Mcs, prSndTrgInfo->u1LDPC);

	MTWF_PRINT("============================= STA Info ========================================\n");

	for (Idx = 1; Idx < 5; Idx++) {
		prSndStaInfo = &pSndCtrlEvt->arSndStaInfo[Idx];
		MTWF_PRINT("Idx%2u Interval = %d, CountDown = %d, TxCnt = %d, StopReason = 0x%02X\n",
			Idx,
			prSndStaInfo->u1SndIntv,
			prSndStaInfo->u1SndCntDn,
			prSndStaInfo->u1SndTxCnt,
			prSndStaInfo->u1SndStopReason);
	}

	MTWF_PRINT("============================= STA Info Connected ==============================\n");

	for (Idx = 1; VALID_UCAST_ENTRY_WCID(pAd, Idx); Idx++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[Idx];

		if (pEntry->EntryType == ENTRY_NONE)
			continue;

		prSndStaInfo = &pSndCtrlEvt->arSndStaInfo[Idx];
		MTWF_PRINT("AID%2u Interval = %d (%u ms), CountDown = %d (%u ms), TxCnt = %d, StopReason = 0x%02X\n",
			pEntry->Aid,
			prSndStaInfo->u1SndIntv,
			prSndStaInfo->u1SndIntv * 10,
			prSndStaInfo->u1SndCntDn,
			prSndStaInfo->u1SndCntDn * 10,
			prSndStaInfo->u1SndTxCnt,
			prSndStaInfo->u1SndStopReason);
	}

	MTWF_PRINT("=====================================================================================\n");

}

VOID TxBFPlyGetGrpStr(
	IN UINT8 ucPly,
	OUT PCHAR pcStr)
{
	if (pcStr) {
		switch (ucPly) {
		case BF_PLY_NON:
			os_move_mem(pcStr, "NON\0", 4);
			break;
		case BF_PLY_SSS:
			os_move_mem(pcStr, "SSS\0", 4);
			break;
		case BF_PLY_SSE:
			os_move_mem(pcStr, "SSE\0", 4);
			break;
		case BF_PLY_SSL:
			os_move_mem(pcStr, "SSL\0", 4);
			break;
		case BF_PLY_SEE:
			os_move_mem(pcStr, "SEE\0", 4);
			break;
		case BF_PLY_SEL:
			os_move_mem(pcStr, "SEL\0", 4);
			break;
		case BF_PLY_SLL:
			os_move_mem(pcStr, "SLL\0", 4);
			break;
		case BF_PLY_ESE:
			os_move_mem(pcStr, "ESE\0", 4);
			break;
		case BF_PLY_ESL:
			os_move_mem(pcStr, "ESL\0", 4);
			break;
		case BF_PLY_EEE:
			os_move_mem(pcStr, "EEE\0", 4);
			break;
		case BF_PLY_EEL:
			os_move_mem(pcStr, "EEL\0", 4);
			break;
		case BF_PLY_ELL:
			os_move_mem(pcStr, "ELL\0", 4);
			break;
		case BF_PLY_ZERO_SNR:
			os_move_mem(pcStr, "ZERO SNR\0", 9);
			break;
		case BF_PLY_MAN:
			os_move_mem(pcStr, "MAN\0", 4);
			break;
		case BF_PLY_ERR:
			os_move_mem(pcStr, "ERR\0", 4);
			break;
		default:
			break;
		}
	}
}

VOID TxBFPlyGetPlyStr(
	IN UINT8 ucPly,
	OUT PCHAR pcStr)
{
	if (pcStr) {
		switch (ucPly) {
		case BF_PLY_RULE_NONBF:
			os_move_mem(pcStr, "NBF\0", 4);
			break;
		case BF_PLY_RULE_EBF:
			os_move_mem(pcStr, "EBF\0", 4);
			break;
		case BF_PLY_RULE_IBF:
			os_move_mem(pcStr, "IBF\0", 4);
			break;
		case BF_PLY_RULE_EBF_THEN_IBF:
			os_move_mem(pcStr, "E>I\0", 4);
			break;
		case BF_PLY_RULE_IBF_THEN_EBF:
			os_move_mem(pcStr, "I>E\0", 4);
			break;
		default:
			break;
		}
	}
}

VOID TxBfPlyInfoPrint(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucBuf)
{
	P_BF_PLY_MGMT_EVT_T pPlyMgmtEvt = (P_BF_PLY_MGMT_EVT_T)pucBuf;
	P_BF_PLY_CFG_T prPlyCfg = &pPlyMgmtEvt->rPlyOpt;
	P_BF_PLY_NSS_T prPlyStaNss;
	P_BF_PLY_RLT_T prPlyStaRlt;
	UINT16 Idx, u1SS;
	CHAR cStrGrp[9] = {0};
	CHAR cStrPly[4] = {0};
	CHAR cStrRlt[4] = {0};

	MTWF_PRINT("============================= Global Setting ========================================\n");

	MTWF_PRINT("GloOpt = 0x%02X, GrpIBfOpt = 0x%02X, GrpEBfOp = 0x%02X\n",
		prPlyCfg->u1GloOpt, prPlyCfg->u1GrpIBfOpt, prPlyCfg->u1GrpEBfOpt);

	MTWF_PRINT("============================= STA Info ========================================\n");
	for (Idx = 1; Idx < 5; Idx++) {
		MTWF_PRINT("AID%2u ", Idx);

		prPlyStaRlt = &pPlyMgmtEvt->arStaRlt[Idx];
		for (u1SS = 0; u1SS < 4; u1SS++) {
			prPlyStaNss = &pPlyMgmtEvt->arStaSS[Idx][u1SS];
			TxBFPlyGetGrpStr(prPlyStaNss->u1SSGrp, cStrGrp);
			TxBFPlyGetPlyStr(prPlyStaNss->u1SSPly, cStrPly);

			MTWF_PRINT("Nss%2u Grp=%s, Ply=%s ", (u1SS+1), cStrGrp, cStrPly);
		}

		TxBFPlyGetPlyStr(prPlyStaRlt->u1CurrRlt, cStrRlt);
		MTWF_PRINT("Rlt=%s ", cStrRlt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("============================= STA Info Connected ==============================\n");

	for (Idx = 1; VALID_UCAST_ENTRY_WCID(pAd, Idx); Idx++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[Idx];

		if (pEntry->EntryType == ENTRY_NONE)
			continue;

		MTWF_PRINT("AID%2u ", pEntry->Aid);

		prPlyStaRlt = &pPlyMgmtEvt->arStaRlt[Idx];
		for (u1SS = 0; u1SS < 4; u1SS++) {
			prPlyStaNss = &pPlyMgmtEvt->arStaSS[Idx][u1SS];
			TxBFPlyGetGrpStr(prPlyStaNss->u1SSGrp, cStrGrp);
			TxBFPlyGetPlyStr(prPlyStaNss->u1SSPly, cStrPly);

			MTWF_PRINT("Nss%2u Grp=%s, Ply=%s ", (u1SS+1), cStrGrp, cStrPly);
		}

		TxBFPlyGetPlyStr(prPlyStaRlt->u1CurrRlt, cStrRlt);
		MTWF_PRINT("Rlt=%s ", cStrRlt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("=====================================================================================\n");

}

VOID TxBfSndCntInfoPrint(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucBuf)
{
	P_BF_SND_CNT_CFG_T pSndCntEvt = (P_BF_SND_CNT_CFG_T)pucBuf;
	UINT8 u1BandIdx;
	MTWF_PRINT("\n======= BF Sounding Count Info (per 1280ms) =======\n");

	MTWF_PRINT("Current Snd Count:\n");

	for (u1BandIdx = 0; u1BandIdx < DBDC_BAND_NUM; u1BandIdx++) {
		MTWF_PRINT("    Band%u: %u\n", u1BandIdx, pSndCntEvt->u2SndCnt[u1BandIdx]);
	}

	MTWF_PRINT("Snd Count Limit        : %u\n", pSndCntEvt->u2SndCntLmt);
	MTWF_PRINT("Manual Snd Count Limit : %u\n", pSndCntEvt->u2SndCntLmtMan);
	MTWF_PRINT("Current Condition      : 0x%x ", pSndCntEvt->u1SndCndCondi);

	if (pSndCntEvt->u1SndCndCondi & SND_CNT_CONDI_MANUAL) {
		MTWF_PRINT(" Manual Mode,");
	}

	if (pSndCntEvt->u1SndCndCondi & SND_CNT_CONDI_8RU) {
		MTWF_PRINT(" 8 RU Mode");
	}

	if (pSndCntEvt->u1SndCndCondi == SND_CNT_CONDI_DEFAULT) {
		MTWF_PRINT(" Default Mode");
	}

	MTWF_PRINT("\n");

	MTWF_PRINT("===================================================\n");

}

VOID HeRaMuMetricInfoPrint(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucBuf)
{
	P_HERA_MU_METRIC_CMD_RPT pMuMetRpt = (P_HERA_MU_METRIC_CMD_RPT)pucBuf;
	P_WH_HERA_METRIC_RPT pQdRpt;
	UINT8 useridx;

	MTWF_PRINT("============================= Global ===============================\n");
	MTWF_PRINT("u1CurState=0x%02X\n", pMuMetRpt->u1CurState);
	MTWF_PRINT("u1RunningFailCnt=0x%02X\n", pMuMetRpt->u1RunningFailCnt);
	MTWF_PRINT("u1ErrRptCnt=0x%02X\n", pMuMetRpt->u1ErrRptCnt);
	MTWF_PRINT("u1FreeReqCnt=0x%02X\n", pMuMetRpt->u1FreeReqCnt);
	MTWF_PRINT("u1PendingReqCnt=0x%02X\n", pMuMetRpt->u1PendingReqCnt);
	MTWF_PRINT("u1PollingTime=0x%02X\n", pMuMetRpt->u1PollingTime);
	MTWF_PRINT("u1NUser=0x%02X\n", pMuMetRpt->u1NUser);
	MTWF_PRINT("fgIsLQErr=0x%02X\n", pMuMetRpt->fgIsLQErr);
	MTWF_PRINT("u2LQErr=0x%02X\n\n", pMuMetRpt->u2LQErr);

	for (useridx = 0; useridx < 4; useridx++) {
		MTWF_PRINT("============================= User %d ==============================\n", useridx);
		pQdRpt = &pMuMetRpt->rMetricRpt[useridx];
		MTWF_PRINT("BPSK=0x%02X, QPSK=0x%02X, 16QAM=0x%02X, 64QAM=0x%02X\n",
			pQdRpt->u1BPSK, pQdRpt->u1QPSK, pQdRpt->u116QAM, pQdRpt->u164QAM);
		MTWF_PRINT("u1256QAM=0x%02X, u11024QAM=0x%02X, u1Capacity=0x%02X, InitMCS=0x%02X\n\n",
			pQdRpt->u1256QAM, pQdRpt->u11024QAM, pQdRpt->u1Capacity, pMuMetRpt->u1InitMCSUser[useridx]);
	}

}

VOID TxBfTxCmdCfgInfoPrint(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pucBuf)
{
	P_BF_TXCMD_CFG_EVT_T pBfTxCmdCfgEvt = (P_BF_TXCMD_CFG_EVT_T)pucBuf;
	BOOLEAN fgTxCmdBfManual = pBfTxCmdCfgEvt->fgTxCmdBfManual;
	UINT_8 ucTxCmdBfBit = pBfTxCmdCfgEvt->ucTxCmdBfBit;

	MTWF_PRINT("============================= Global Setting ========================================\n");
	MTWF_PRINT("TxCmdBfManual = 0x%02X, TxCmdBfBit = 0x%02X\n", fgTxCmdBfManual, ucTxCmdBfBit);
	MTWF_PRINT("=====================================================================================\n");
}

#ifdef CONFIG_ATE
BOOLEAN TxBfProfileDataFormatTranslate(
	PRTMP_ADAPTER pAd,
	PUCHAR pucDataIn,
	P_PFMU_HALF_DATA pPfmuHalfData)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR   control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32  u4SubCarrId, u4AnglePh11, u4AnglePh21, u4AnglePh31, u4AnglePh41;
	INT16   i2Phi11,     i2Phi21,     i2Phi31;
	UINT16  u2InIdx;
	UCHAR	ucLoop;
	UINT8   ucTxPath = pAd->Antenna.field.TxPath;

	MTWF_PRINT("%s:: Band index = %d, Is dual phy = %d\n", __func__, control_band_idx, IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_DUALPHY));

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		if (control_band_idx == DBDC_BAND0)
			ucTxPath = pAd->dbdc_band0_tx_path;
		else
			ucTxPath = pAd->dbdc_band1_tx_path;
	}

	MTWF_PRINT("%s:: Tx num = %d\n", __func__, ucTxPath);
#endif

	for (ucLoop = 0; ucLoop < 64; ucLoop++) {
		u2InIdx  = ucLoop * 20;
		/* Subcarrier */
		NdisMoveMemory(&u4SubCarrId, &pucDataIn[u2InIdx], 4);
		u4SubCarrId = PKTL_TRAN_TO_HOST(u4SubCarrId);

		if (u4SubCarrId < 32)
			u4SubCarrId += 224;
		else
			u4SubCarrId -= 32;

		/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, */
		/* "%s:: val: %u\n",__FUNCTION__, u4SubCarrId)); */
		/* Angle Ph11 */
		NdisMoveMemory(&u4AnglePh11, &pucDataIn[u2InIdx + 4], 4);
		u4AnglePh11 = PKTL_TRAN_TO_HOST(u4AnglePh11);
		/* Angle Ph21 */
		NdisMoveMemory(&u4AnglePh21, &pucDataIn[u2InIdx + 8], 4);
		u4AnglePh21 = PKTL_TRAN_TO_HOST(u4AnglePh21);
		/* Angle Ph31 */
		NdisMoveMemory(&u4AnglePh31, &pucDataIn[u2InIdx + 12], 4);
		u4AnglePh31 = PKTL_TRAN_TO_HOST(u4AnglePh31);
		/* Angle Ph41 */
		NdisMoveMemory(&u4AnglePh41, &pucDataIn[u2InIdx + 16], 4);
		u4AnglePh41 = PKTL_TRAN_TO_HOST(u4AnglePh41);
		switch (ucTxPath) {
		case 2:
			i2Phi11    = (INT16)(u4AnglePh21 - u4AnglePh11);
			i2Phi21    = 0;
			i2Phi31    = 0;
			break;
		case 3:
			i2Phi11    = (INT16)(u4AnglePh31 - u4AnglePh11);
			i2Phi21    = (INT16)(u4AnglePh31 - u4AnglePh21);
			i2Phi31    = 0;
			break;

		case 4:
		default:
#ifdef DBDC_MODE
			if ((pAd->CommonCfg.dbdc_mode) && (!IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_DUALPHY))) {
				i2Phi11    = (INT16)(u4AnglePh21 - u4AnglePh11);
				i2Phi21    = 0;
				i2Phi31    = 0;
			} else
#endif
			{
				i2Phi11    = (INT16)(u4AnglePh41 - u4AnglePh11);
				i2Phi21    = (INT16)(u4AnglePh41 - u4AnglePh21);
				i2Phi31    = (INT16)(u4AnglePh41 - u4AnglePh31);
			}

			break;
		}

		pPfmuHalfData[ucLoop].u2SubCarrIdx = (UINT16)u4SubCarrId;
#ifdef RT_BIG_ENDIAN
		pPfmuHalfData[ucLoop].u2SubCarrIdx = cpu2le16(pPfmuHalfData[ucLoop].u2SubCarrIdx);
#endif
		pPfmuHalfData[ucLoop].i2Phi11      = cpu2le16(i2Phi11);
		pPfmuHalfData[ucLoop].i2Phi21      = cpu2le16(i2Phi21);
		pPfmuHalfData[ucLoop].i2Phi31      = cpu2le16(i2Phi31);
	}

	return TRUE;
}


#if defined(DOT11_HE_AX)
VOID TxBf_Status_Update(RTMP_ADAPTER *pAd, UCHAR *data, UINT32 len)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UCHAR *txbf_info = TESTMODE_GET_PARAM(pAd, control_band_idx, txbf_info);
	UINT32 op_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
	struct _EXT_EVENT_BF_STATUS_T *bf_info =
					(struct _EXT_EVENT_BF_STATUS_T *)data;
	struct _EXT_EVENT_IBF_STATUS_T *ibf_info =
					(struct _EXT_EVENT_IBF_STATUS_T *)data;
	UINT32 status = 0;
	UINT32 data_len = 0;
	UCHAR *bf_data = bf_info->aucBuffer;

	if (!(op_mode & fATE_IN_BF))
		return;

	TESTMODE_SET_PARAM(pAd, control_band_idx, txbf_info_len, 0);
	os_alloc_mem(pAd, (UCHAR **)&txbf_info, sizeof(UCHAR)*len);

	if (!txbf_info) {
		status = NDIS_STATUS_RESOURCES;
		goto err0;
	}

	os_zero_mem(txbf_info, sizeof(UCHAR)*len);
	TESTMODE_SET_PARAM(pAd, control_band_idx, txbf_info_len, len);

	switch (bf_info->ucBfDataFormatID) {
	case BF_PFMU_TAG:
		if (bf_info->fgBFer)
			data_len = sizeof(PFMU_PROFILE_TAG1) +
					sizeof(PFMU_PROFILE_TAG2);
		else
			data_len = sizeof(PFMU_PROFILE_TAG1);

		NdisMoveMemory(txbf_info, bf_data, data_len);
		TESTMODE_SET_PARAM(pAd,
				control_band_idx,
				txbf_info_len,
				data_len);
		break;

	case BF_PFMU_DATA:
		NdisMoveMemory(txbf_info, bf_data, sizeof(PFMU_DATA));
		data_len = sizeof(PFMU_DATA);
		TESTMODE_SET_PARAM(pAd,
				control_band_idx,
				txbf_info_len,
				data_len);
		break;

	case BF_CAL_PHASE:
		TESTMODE_SET_PARAM(pAd,
				control_band_idx,
				iBFCalStatus,
				ibf_info->ucStatus);
		break;

	case BF_QD_DATA:
		NdisMoveMemory(txbf_info, bf_data, sizeof(BF_QD));
		data_len = sizeof(BF_QD);
		TESTMODE_SET_PARAM(pAd, control_band_idx, txbf_info_len, data_len);
		break;

	default:
		break;
	}

err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, "(%x)\n", status);
}
#endif /* defined(DOT11_HE_AX) */
#endif /* CONFIG_ATE */


INT TxBfProfileDataWrite20MAll(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         profileIdx,
	IN PUCHAR        pData
)
{
	PFMU_HALF_DATA arProfileData[64];
	BOOLEAN  fgStatus = FALSE;
#ifdef CONFIG_ATE

	if (TxBfProfileDataFormatTranslate(pAd, pData, arProfileData) == FALSE)
		return fgStatus;

#endif /* CONFIG_ATE */

	if (CmdETxBfPfmuProfileDataWrite20MAll(pAd,
										   profileIdx,
										   (PUCHAR)&arProfileData[0]) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

VOID TxBfProfileDataPrint(
	IN PRTMP_ADAPTER pAd,
	IN USHORT        subCarrIdx,
	IN PUCHAR        pBuf)
{
	P_PFMU_DATA prPfmuDataStart;
	PFMU_DATA   rPfmuData;

	prPfmuDataStart = (P_PFMU_DATA) pBuf;
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange((UCHAR *)prPfmuDataStart, sizeof(PFMU_DATA));
#endif
	NdisCopyMemory(&rPfmuData, prPfmuDataStart, sizeof(PFMU_DATA));
	NdisCopyMemory(&pAd->prof, prPfmuDataStart, sizeof(PFMU_DATA));
	MTWF_PRINT("============================= TxBf profile Data - Subcarrier Idx = %d =================\n"
				"Psi41 = 0x%x, Phi31 = 0x%x, Psi31 = 0x%x, Phi21 = 0x%x, Psi21 = 0x%x, Phi11 = 0x%x\n"
				"Psi43 = 0x%x, Phi33 = 0x%x, Psi42 = 0x%x, Phi32 = 0x%x, Psi32 = 0x%x, Phi22 = 0x%x\n"
				"SNR00 = 0x%x, SNR01 = 0x%x, SNR02 = 0x%x, SNR03 = 0x%x\n"
				"=======================================================================================\n",
				subCarrIdx,
				rPfmuData.rField.ucPsi41, rPfmuData.rField.u2Phi31,
				rPfmuData.rField.ucPsi31, rPfmuData.rField.u2Phi21,
				rPfmuData.rField.ucPsi21, rPfmuData.rField.u2Phi11,
				rPfmuData.rField.ucPsi43, rPfmuData.rField.u2Phi33,
				rPfmuData.rField.ucPsi42, rPfmuData.rField.u2Phi32,
				rPfmuData.rField.ucPsi32, rPfmuData.rField.u2Phi22,
				rPfmuData.rField.u2dSNR00, rPfmuData.rField.u2dSNR01,
				rPfmuData.rField.u2dSNR02, rPfmuData.rField.u2dSNR03);
}

VOID StaRecBfUpdate(
	IN MAC_TABLE_ENTRY *pEntry,
	IN P_CMD_STAREC_BF pCmdStaRecBf)
{
	pCmdStaRecBf->u2Tag      = STA_REC_BF;
	pCmdStaRecBf->u2Length   = sizeof(CMD_STAREC_BF);
#ifdef RT_BIG_ENDIAN
	pCmdStaRecBf->u2Tag      = cpu2le16(pCmdStaRecBf->u2Tag);
	pCmdStaRecBf->u2Length   = cpu2le16(pCmdStaRecBf->u2Length);
#endif
	pCmdStaRecBf->rTxBfPfmuInfo.u2PfmuId      = cpu2le16(pEntry->rStaRecBf.u2PfmuId);
	pCmdStaRecBf->rTxBfPfmuInfo.fgSU_MU       = pEntry->rStaRecBf.fgSU_MU;
	pCmdStaRecBf->rTxBfPfmuInfo.u1TxBfCap     = pEntry->rStaRecBf.u1TxBfCap;
	pCmdStaRecBf->rTxBfPfmuInfo.ucSoundingPhy = pEntry->rStaRecBf.ucSoundingPhy;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNdpaRate    = pEntry->rStaRecBf.ucNdpaRate;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNdpRate     = pEntry->rStaRecBf.ucNdpRate;
	pCmdStaRecBf->rTxBfPfmuInfo.ucReptPollRate  = pEntry->rStaRecBf.ucReptPollRate;
	pCmdStaRecBf->rTxBfPfmuInfo.ucTxMode      = pEntry->rStaRecBf.ucTxMode;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNc          = pEntry->rStaRecBf.ucNc;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNr          = pEntry->rStaRecBf.ucNr;
	pCmdStaRecBf->rTxBfPfmuInfo.ucCBW         = pEntry->rStaRecBf.ucCBW;
	pCmdStaRecBf->rTxBfPfmuInfo.ucTotMemRequire = pEntry->rStaRecBf.ucTotMemRequire;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRequire20M = pEntry->rStaRecBf.ucMemRequire20M;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow0     = pEntry->rStaRecBf.ucMemRow0;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol0     = pEntry->rStaRecBf.ucMemCol0;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow1     = pEntry->rStaRecBf.ucMemRow1;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol1     = pEntry->rStaRecBf.ucMemCol1;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow2     = pEntry->rStaRecBf.ucMemRow2;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol2     = pEntry->rStaRecBf.ucMemCol2;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow3     = pEntry->rStaRecBf.ucMemRow3;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol3     = pEntry->rStaRecBf.ucMemCol3;
	pCmdStaRecBf->rTxBfPfmuInfo.u2SmartAnt    = cpu2le16(pEntry->rStaRecBf.u2SmartAnt);
	pCmdStaRecBf->rTxBfPfmuInfo.ucSEIdx       = pEntry->rStaRecBf.ucSEIdx;
#ifdef TXBF_DYNAMIC_DISABLE
	pCmdStaRecBf->rTxBfPfmuInfo.ucAutoSoundingCtrl = pEntry->rStaRecBf.ucAutoSoundingCtrl;
#endif /* TXBF_DYNAMIC_DISABLE */
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfTimeOut  = pEntry->rStaRecBf.uciBfTimeOut;
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfDBW      = pEntry->rStaRecBf.uciBfDBW;
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfNcol     = pEntry->rStaRecBf.uciBfNcol;
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfNrow     = pEntry->rStaRecBf.uciBfNrow;
	pCmdStaRecBf->rTxBfPfmuInfo.nr_bw160     = pEntry->rStaRecBf.nr_bw160;
	pCmdStaRecBf->rTxBfPfmuInfo.nc_bw160     = pEntry->rStaRecBf.nc_bw160;
	pCmdStaRecBf->rTxBfPfmuInfo.ru_start_idx     = pEntry->rStaRecBf.ru_start_idx;
	pCmdStaRecBf->rTxBfPfmuInfo.ru_end_idx     = pEntry->rStaRecBf.ru_end_idx;
	pCmdStaRecBf->rTxBfPfmuInfo.trigger_su     = pEntry->rStaRecBf.trigger_su;
	pCmdStaRecBf->rTxBfPfmuInfo.trigger_mu     = pEntry->rStaRecBf.trigger_mu;
	pCmdStaRecBf->rTxBfPfmuInfo.ng16_su     = pEntry->rStaRecBf.ng16_su;
	pCmdStaRecBf->rTxBfPfmuInfo.ng16_mu     = pEntry->rStaRecBf.ng16_mu;
	pCmdStaRecBf->rTxBfPfmuInfo.codebook42_su     = pEntry->rStaRecBf.codebook42_su;
	pCmdStaRecBf->rTxBfPfmuInfo.codebook75_mu     = pEntry->rStaRecBf.codebook75_mu;
	pCmdStaRecBf->rTxBfPfmuInfo.he_ltf     = pEntry->rStaRecBf.he_ltf;
}

#ifdef WIFI_UNIFIED_COMMAND
VOID UniCmdStaRecBfUpdate(
	IN MAC_TABLE_ENTRY *pEntry,
	IN P_UNI_CMD_STAREC_BF_T pCmdStaRecBf)
{
	pCmdStaRecBf->u2Tag      = UNI_CMD_STAREC_BF;
	pCmdStaRecBf->u2Length   = sizeof(UNI_CMD_STAREC_BF_T);
#ifdef RT_BIG_ENDIAN
	pCmdStaRecBf->u2Tag      = cpu2le16(pCmdStaRecBf->u2Tag);
	pCmdStaRecBf->u2Length   = cpu2le16(pCmdStaRecBf->u2Length);
#endif
	pCmdStaRecBf->rTxBfPfmuInfo.u2PfmuId      = cpu2le16(pEntry->rStaRecBf.u2PfmuId);
	pCmdStaRecBf->rTxBfPfmuInfo.fgSU_MU       = pEntry->rStaRecBf.fgSU_MU;
	pCmdStaRecBf->rTxBfPfmuInfo.u1TxBfCap     = pEntry->rStaRecBf.u1TxBfCap;
	pCmdStaRecBf->rTxBfPfmuInfo.ucSoundingPhy = pEntry->rStaRecBf.ucSoundingPhy;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNdpaRate    = pEntry->rStaRecBf.ucNdpaRate;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNdpRate     = pEntry->rStaRecBf.ucNdpRate;
	pCmdStaRecBf->rTxBfPfmuInfo.ucReptPollRate  = pEntry->rStaRecBf.ucReptPollRate;
	pCmdStaRecBf->rTxBfPfmuInfo.ucTxMode      = pEntry->rStaRecBf.ucTxMode;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNc          = pEntry->rStaRecBf.ucNc;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNr          = pEntry->rStaRecBf.ucNr;
	pCmdStaRecBf->rTxBfPfmuInfo.ucCBW         = pEntry->rStaRecBf.ucCBW;
	pCmdStaRecBf->rTxBfPfmuInfo.ucTotMemRequire = pEntry->rStaRecBf.ucTotMemRequire;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRequire20M = pEntry->rStaRecBf.ucMemRequire20M;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow0     = pEntry->rStaRecBf.ucMemRow0;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol0     = pEntry->rStaRecBf.ucMemCol0;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow1     = pEntry->rStaRecBf.ucMemRow1;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol1     = pEntry->rStaRecBf.ucMemCol1;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow2     = pEntry->rStaRecBf.ucMemRow2;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol2     = pEntry->rStaRecBf.ucMemCol2;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow3     = pEntry->rStaRecBf.ucMemRow3;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol3     = pEntry->rStaRecBf.ucMemCol3;
	pCmdStaRecBf->rTxBfPfmuInfo.u2SmartAnt    = cpu2le16(pEntry->rStaRecBf.u2SmartAnt);
	pCmdStaRecBf->rTxBfPfmuInfo.ucSEIdx       = pEntry->rStaRecBf.ucSEIdx;
#ifdef TXBF_DYNAMIC_DISABLE
	pCmdStaRecBf->rTxBfPfmuInfo.ucAutoSoundingCtrl = pEntry->rStaRecBf.ucAutoSoundingCtrl;
#endif /* TXBF_DYNAMIC_DISABLE */
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfTimeOut  = pEntry->rStaRecBf.uciBfTimeOut;
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfDBW      = pEntry->rStaRecBf.uciBfDBW;
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfNcol     = pEntry->rStaRecBf.uciBfNcol;
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfNrow     = pEntry->rStaRecBf.uciBfNrow;
	pCmdStaRecBf->rTxBfPfmuInfo.nr_bw160     = pEntry->rStaRecBf.nr_bw160;
	pCmdStaRecBf->rTxBfPfmuInfo.nc_bw160     = pEntry->rStaRecBf.nc_bw160;
	pCmdStaRecBf->rTxBfPfmuInfo.ru_start_idx     = pEntry->rStaRecBf.ru_start_idx;
	pCmdStaRecBf->rTxBfPfmuInfo.ru_end_idx     = pEntry->rStaRecBf.ru_end_idx;
	pCmdStaRecBf->rTxBfPfmuInfo.trigger_su     = pEntry->rStaRecBf.trigger_su;
	pCmdStaRecBf->rTxBfPfmuInfo.trigger_mu     = pEntry->rStaRecBf.trigger_mu;
	pCmdStaRecBf->rTxBfPfmuInfo.ng16_su     = pEntry->rStaRecBf.ng16_su;
	pCmdStaRecBf->rTxBfPfmuInfo.ng16_mu     = pEntry->rStaRecBf.ng16_mu;
	pCmdStaRecBf->rTxBfPfmuInfo.codebook42_su     = pEntry->rStaRecBf.codebook42_su;
	pCmdStaRecBf->rTxBfPfmuInfo.codebook75_mu     = pEntry->rStaRecBf.codebook75_mu;
	pCmdStaRecBf->rTxBfPfmuInfo.he_ltf     = pEntry->rStaRecBf.he_ltf;
}
#endif /* WIFI_UNIFIED_COMMAND */

VOID StaRecBfeeUpdate(
	IN MAC_TABLE_ENTRY *pEntry,
	IN P_CMD_STAREC_BFEE pCmdStaRecBfee)
{
	pCmdStaRecBfee->u2Tag      = STA_REC_BFEE;
	pCmdStaRecBfee->u2Length   = sizeof(CMD_STAREC_BFEE);
#ifdef RT_BIG_ENDIAN
	pCmdStaRecBfee->u2Tag      = cpu2le16(pCmdStaRecBfee->u2Tag);
	pCmdStaRecBfee->u2Length   = cpu2le16(pCmdStaRecBfee->u2Length);
#endif
	pCmdStaRecBfee->rBfeeStaRec.fgFbIdentityMatrix	= pEntry->rStaRecBfee.fgFbIdentityMatrix;
	pCmdStaRecBfee->rBfeeStaRec.fgIgnFbk	= pEntry->rStaRecBfee.fgIgnFbk;

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"fgFbIdentityMatrix: %u\n", pCmdStaRecBfee->rBfeeStaRec.fgFbIdentityMatrix);
	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"fgIgnFbk: %u\n", pCmdStaRecBfee->rBfeeStaRec.fgIgnFbk);
}

VOID StaRecBfRead(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBuf)
{
	NdisZeroMemory(&pAd->rStaRecBf, sizeof(TXBF_PFMU_STA_INFO));
	NdisCopyMemory(&pAd->rStaRecBf, pBuf, sizeof(TXBF_PFMU_STA_INFO));
#ifdef RT_BIG_ENDIAN
	pAd->rStaRecBf.u2PfmuId = le2cpu16(pAd->rStaRecBf.u2PfmuId);
	pAd->rStaRecBf.u2SmartAnt = le2cpu16(pAd->rStaRecBf.u2SmartAnt);
#endif
	MTWF_PRINT("====================================== BF StaRec ========================================\n"
				"rStaRecBf.u2PfmuId      = %d\n"
				"rStaRecBf.fgSU_MU       = %d\n"
				"rStaRecBf.u1TxBfCap     = %d\n"
				"rStaRecBf.ucSoundingPhy = %d\n"
				"rStaRecBf.ucNdpaRate    = %d\n"
				"rStaRecBf.ucNdpRate     = %d\n"
				"rStaRecBf.ucReptPollRate= %d\n"
				"rStaRecBf.ucTxMode      = %d\n"
				"rStaRecBf.ucNc          = %d\n"
				"rStaRecBf.ucNr          = %d\n"
				"rStaRecBf.ucCBW         = %d\n"
				"rStaRecBf.ucTotMemRequire = %d\n"
				"rStaRecBf.ucMemRequire20M = %d\n"
				"rStaRecBf.ucMemRow0     = %d\n"
				"rStaRecBf.ucMemCol0     = %d\n"
				"rStaRecBf.ucMemRow1     = %d\n"
				"rStaRecBf.ucMemCol1     = %d\n"
				"rStaRecBf.ucMemRow2     = %d\n"
				"rStaRecBf.ucMemCol2     = %d\n"
				"rStaRecBf.ucMemRow3     = %d\n"
				"rStaRecBf.ucMemCol3     = %d\n",
				pAd->rStaRecBf.u2PfmuId,
				pAd->rStaRecBf.fgSU_MU,
				pAd->rStaRecBf.u1TxBfCap,
				pAd->rStaRecBf.ucSoundingPhy,
				pAd->rStaRecBf.ucNdpaRate,
				pAd->rStaRecBf.ucNdpRate,
				pAd->rStaRecBf.ucReptPollRate,
				pAd->rStaRecBf.ucTxMode,
				pAd->rStaRecBf.ucNc,
				pAd->rStaRecBf.ucNr,
				pAd->rStaRecBf.ucCBW,
				pAd->rStaRecBf.ucTotMemRequire,
				pAd->rStaRecBf.ucMemRequire20M,
				pAd->rStaRecBf.ucMemRow0,
				pAd->rStaRecBf.ucMemCol0,
				pAd->rStaRecBf.ucMemRow1,
				pAd->rStaRecBf.ucMemCol1,
				pAd->rStaRecBf.ucMemRow2,
				pAd->rStaRecBf.ucMemCol2,
				pAd->rStaRecBf.ucMemRow3,
				pAd->rStaRecBf.ucMemCol3);

	MTWF_PRINT("rStaRecBf.u2SmartAnt    = 0x%x\n"
				"rStaRecBf.ucSEIdx       = %d\n"
#ifdef TXBF_DYNAMIC_DISABLE
				"rStaRecBf.ucAutoSoundingCtrl = %d\n"
#endif /* TXBF_DYNAMIC_DISABLE */
				"rStaRecBf.uciBfTimeOut  = 0x%x\n"
				"rStaRecBf.uciBfDBW      = %d\n"
				"rStaRecBf.uciBfNcol     = %d\n"
				"rStaRecBf.uciBfNrow     = %d\n"
				"rStaRecBf.nr_bw160      = %d\n"
				"rStaRecBf.nc_bw160 	  = %d\n"
				"rStaRecBf.ru_start_idx  = %d\n"
				"rStaRecBf.ru_end_idx 	  = %d\n"
				"rStaRecBf.trigger_su 	  = %d\n"
				"rStaRecBf.trigger_mu 	  = %d\n"
				"rStaRecBf.ng16_su 	  = %d\n"
				"rStaRecBf.ng16_mu 	  = %d\n"
				"rStaRecBf.codebook42_su = %d\n"
				"rStaRecBf.codebook75_mu = %d\n"
				"rStaRecBf.he_ltf 	      = %d\n"
				"=======================================================================================\n",
				pAd->rStaRecBf.u2SmartAnt,
				pAd->rStaRecBf.ucSEIdx,
#ifdef TXBF_DYNAMIC_DISABLE
				pAd->rStaRecBf.ucAutoSoundingCtrl,
#endif /* TXBF_DYNAMIC_DISABLE */
				pAd->rStaRecBf.uciBfTimeOut,
				pAd->rStaRecBf.uciBfDBW,
				pAd->rStaRecBf.uciBfNcol,
				pAd->rStaRecBf.uciBfNrow,
				pAd->rStaRecBf.nr_bw160,
				pAd->rStaRecBf.nc_bw160,
				pAd->rStaRecBf.ru_start_idx,
				pAd->rStaRecBf.ru_end_idx,
				pAd->rStaRecBf.trigger_su,
				pAd->rStaRecBf.trigger_mu,
				pAd->rStaRecBf.ng16_su,
				pAd->rStaRecBf.ng16_mu,
				pAd->rStaRecBf.codebook42_su,
				pAd->rStaRecBf.codebook75_mu,
				pAd->rStaRecBf.he_ltf);
}

VOID TxBfProfileMemAllocMap(
	IN PUCHAR  pBuf)
{
	UINT_16 *au2PfmuMemAllocMap;
	UINT_8  ucLoop, ucBit;
	UINT_16 len = sizeof(UINT_16) * TXBF_PFMU_ARRAY_SIZE * MAX_PFMU_MEM_LEN_PER_ROW;

	os_alloc_mem(NULL, (UCHAR **)&au2PfmuMemAllocMap, len);
	if (!au2PfmuMemAllocMap)
		return;
	NdisCopyMemory(au2PfmuMemAllocMap, pBuf, len);

	for (ucLoop = 0; ucLoop < TXBF_PFMU_ARRAY_SIZE; ucLoop++) {
		MTWF_PRINT("========= PFMU memory allocation map =========\n");
		MTWF_PRINT("%3d :", ucLoop);

		for (ucBit = 0; ucBit < MAX_PFMU_MEM_LEN_PER_ROW; ucBit++) {
			UINT_16 idx = ucLoop * MAX_PFMU_MEM_LEN_PER_ROW + ucBit;
			MTWF_PRINT("%4d |", le2cpu16(au2PfmuMemAllocMap[idx]));

			if (ucBit == 5) {
				MTWF_PRINT("\n");
			}
		}

		MTWF_PRINT("==============================================\n");
	}
	os_free_mem(au2PfmuMemAllocMap);
}

BOOLEAN TxBfModuleEnCtrl(
	IN PRTMP_ADAPTER pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct _RTMP_CHIP_CAP *cap;
	RADIO_CTRL *pRadioCtrl = NULL;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UINT8 i;
	UINT8 u1BfNum = 0;
	UINT8 u1isBfBfBandNum = 0;
	UINT8 u1BfBitmap = 0;
	UINT8 u1BfSelBand[8];

	if (pHwResource->concurrent_bands < 2) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"It's not DBDC mode\n");

		return FALSE;
	}
	os_zero_mem(&u1BfSelBand[0], sizeof(u1BfSelBand));
	cap = hc_get_chip_cap(ctrl);

	u1BfNum = (cap->FlgHwTxBfCap & TXBF_HW_2BF) ? 2 : 1;
	if (u1BfNum > 1) {
		/* For 2 BF DBDC mode */
		for (i = 0; i < pHwResource->concurrent_bands; i++) {
			pPhyCtrl =  &pHwResource->PhyCtrl[i];
			pRadioCtrl =  &pPhyCtrl->RadioCtrl;

			if (pRadioCtrl->IsBfBand) {
				u1isBfBfBandNum++;
				u1BfBitmap |= 1 << i;
				u1BfSelBand[i] = 0;
			}
		}
	} else {
		/* For 1 BF DBDC mode */
		for (i = 0; i < pHwResource->concurrent_bands; i++) {
			pPhyCtrl =  &pHwResource->PhyCtrl[i];
			pRadioCtrl =  &pPhyCtrl->RadioCtrl;

			if (pRadioCtrl->IsBfBand) {
				u1isBfBfBandNum++;
				u1BfBitmap |= 1;
				u1BfSelBand[0] = i;
			}

			MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"i = %d, pRadioCtrl->IsBfBand = %d\n",
				i, pRadioCtrl->IsBfBand);
		}
	}

	if (u1isBfBfBandNum > u1BfNum) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Error - isBfBfBandNum > hardware capability\n");

		return FALSE;
	}
	MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"u1BfNum = %d, u1BfBitmap = %d, u1BfSelBand[0] = %d\n",
					u1BfNum, u1BfBitmap, u1BfSelBand[0]);

	AsicTxBfModuleEnCtrl(pAd, u1BfNum, u1BfBitmap, &u1BfSelBand[0]);

	return TRUE;
}

BOOLEAN TxBfCfgBfPhy(
	IN PRTMP_ADAPTER pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	RADIO_CTRL *pRadioCtrl = NULL;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;
	EXT_CMD_TXBF_CFG_BF_PHY_T rTxBfCfgBfPhy;
	UINT8 ucBandIdx;
	UINT8 ucBypass;
	UINT8 i;

	memset(&rTxBfCfgBfPhy, 0, sizeof(EXT_CMD_TXBF_CFG_BF_PHY_T));

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		pPhyCtrl =  &pHwResource->PhyCtrl[i];
		pRadioCtrl =  &pPhyCtrl->RadioCtrl;

		if (pRadioCtrl->IsBfBand) {
			rTxBfCfgBfPhy.ucAction = BF_PHY_SMTH_INTL_BYPASS;
			ucBandIdx = rTxBfCfgBfPhy.ucBandIdx = pRadioCtrl->BandIdx;
			ucBypass = rTxBfCfgBfPhy.ucSmthIntlBypass = pAd->CommonCfg.BfSmthIntlBypass[ucBandIdx];
			pRadioCtrl->BfSmthIntlBypass = pAd->CommonCfg.BfSmthIntlBypass[ucBandIdx];
			if (ucBypass)
				AsicTxBfCfgBfPhy(pAd, (PUINT8)&rTxBfCfgBfPhy);
		}

		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"ucAction = %d, ucBandIdx = %d, ucSmthIntlBypass = %d\n",
						rTxBfCfgBfPhy.ucAction,
						rTxBfCfgBfPhy.ucBandIdx,
						rTxBfCfgBfPhy.ucSmthIntlBypass);
	}

	return TRUE;
}

/*
 * ==========================================================================
 * Description:
 * Tx BF customized configuration
 *
 * param[in] UINT8  u1ConfigType  Configuration Type
 *                UINT8  au1ConfigPara[]  Configuration Parameters (at most 6)
 * Return:
 * TRUE if all parameters are OK, FALSE otherwise
 * ==========================================================================
 */
INT txbf_config(
	IN    PRTMP_ADAPTER        pAd,
	IN    UINT8                config_type,
	IN    UINT8                config_para[])
{
	UINT8 i;
	BOOLEAN status = FALSE;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "config_type=%d\n", config_type);

	/* Check TxBF config type */
	if (config_type >= BF_CONFIG_TYPE_MAX) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Wrong Tx BF Config Type: config_type=%d > %d\n",
			config_type, BF_CONFIG_TYPE_MAX);
		return FALSE;
	}

	/* Check TxBF config parameter number, at most 6 */
	if (strlen(config_para) > 6) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Wrong Tx BF Config parameter number (> 6)!\n");
		return FALSE;
	}

	/* Show TxBF config parameters */
	for (i = 0 ; i < 6 ; i++) {
		MTWF_PRINT("config_para[%d]=%d\n", i, config_para[i]);
	}

	/* Tx BF Config in FW */
	if (cmd_txbf_config(pAd, config_type, &config_para[0]) == STATUS_TRUE) {
	    status = TRUE;
	}

	return status;

}


/*
 * ==========================================================================
 * Description:
 * Tx BF dynamic mechanism function, called when every STA associated or disconnected
 *
 * Return:
 * VOID
 * ==========================================================================
 */
VOID txbf_dyn_mech(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN status = TRUE;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "\n");
	if (pAd->bfdm.bfdm_bitmap & BFDM_BFEE_ADAPTION_BITMAP)
	    status = asic_txbf_bfee_adaption(pAd);
}


/*
 * ==========================================================================
 * Description:
 * BFee dynamic enable/ disable judgement
 *
 * Return:
 * TRUE if setting BFee enable/disable successfully, FALSE otherwise
 * ==========================================================================
 */
BOOLEAN txbf_bfee_adaption(
    IN PRTMP_ADAPTER pAd)
{
    BOOLEAN status = TRUE;
	BOOLEAN rtk_sta_bfer = FALSE;

	/* MT7663 has BFee IOT with DWA-192 Realtek 8814AU
	 *  BFee will be disabled if DWA-192 is associated
	 */
	rtk_sta_bfer = has_rtk_sta_bfer(pAd);

    if (rtk_sta_bfer && pAd->bfdm.bfdm_bfee_enabled) { /* Disable BFee */
		status = AsicTxBfeeHwCtrl(pAd, FALSE);
		pAd->bfdm.bfdm_bfee_enabled = FALSE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Disable BFee\n");
    } else if (!rtk_sta_bfer && !pAd->bfdm.bfdm_bfee_enabled) { /* Enable BFee */
		status = AsicTxBfeeHwCtrl(pAd, TRUE);
		pAd->bfdm.bfdm_bfee_enabled = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Enable BFee\n");
    }

    return status;

}

/*
 * ==========================================================================
 * Description:
 * Judge if there is Realtek station associated and it is a BFer with Nr=2
 *
 * Return:
 * TRUE if STA DWA-192 (Realtek) with BFer Nr=2 is associated, FALSE otherwise
 * ==========================================================================
 */
BOOLEAN has_rtk_sta_bfer(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN found_rtk_sta_bfer = FALSE;
	UINT16 i;
	UCHAR nr;
	BOOLEAN bfer;

	/*
	 * Scan every associated STAs to see if there is DWA-192 Realtek STA BFer
	 */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (found_rtk_sta_bfer)
			break;

		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		/* DWA-192 (Ball Card) has no OUI in assoc req.
		 *  If the STA has no OUI, it is a suspect of DWA-192.
		 *  It is known that Broadcom, Qualcomm, MTK all have OUI,
		 *  But Intel may also has no OUI in assoc req.
		 */
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"STA[%d] has_oui:%d\n", i, pEntry->has_oui);
		if (pEntry->has_oui > 0) /* skip STA with OUI */
			continue;

		/*
		 *  DWA-192 (Ball Card) has TxBF Cap of BFer with Nr=2 (3T)
		 *  Check STA's HT and VHT TxBF Cap of BFer and Nr
		 */
		if (get_sta_bfer_nr(pEntry, &bfer, &nr)) {
			if (bfer && nr == 2) {
				found_rtk_sta_bfer = TRUE;
			}
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"sta %d BFer:%d, Nr=%d\n", i, bfer, nr);
		}
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"found_rtk_sta_bfer=%d\n", found_rtk_sta_bfer);

	return found_rtk_sta_bfer;
}

#ifdef TXBF_DYNAMIC_DISABLE


/*
	==========================================================================
	Description:
		(1) Dynamically disable BF for current assoc STAs by setting pEntry->rStaRecBf.ucAutoSoundingCtrl[DYNAMIC_BF_DISABLE]
		(2) Set pAd->CommonCfg.ucAutoSoundingCtrl[DYNAMIC_BF_DISABLE] so that STA in the future is also with BF disabled
		(3) Also updates WTBL BF flags

	==========================================================================
 */
INT DynamicTxBfDisable(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN		 fgDisable)
{
	BOOLEAN fgStatus = FALSE, fgETxBf, fgITxBf;
	INT i;

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG, "\n");

	if (fgDisable) {
		fgETxBf = FALSE;
		fgITxBf = FALSE;
		/* Update ucAutoSoundingCtrol with DYNAMIC_BF_DISABLE
		  * to bypass WTBL BF flag setting and sounding packet Tx
		 */
		pAd->CommonCfg.ucAutoSoundingCtrl |= DYNAMIC_TXBF_DISABLE;
		/* Disable BFee by setting HW */
		AsicTxBfeeHwCtrl(pAd, FALSE);
	} else {
		fgETxBf = pAd->CommonCfg.ETxBfEnCond;
		fgITxBf = pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn;
		pAd->CommonCfg.ucAutoSoundingCtrl = 0;
		/* Enable BFee by setting HW */
		AsicTxBfeeHwCtrl(pAd, TRUE);
	}

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG,
		"fgETxBf=%d, fgITxBf=%d, ucAutoSoundingCtrl=%d\n", fgETxBf, fgITxBf, pAd->CommonCfg.ucAutoSoundingCtrl);

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {/* For every STA */
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) && (pEntry->Sst == SST_ASSOC)) {
			STA_REC_CFG_T StaCfg;

			MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG,
				"Enable/Disable BF for wlanid %d\n", i);

			os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));
			pEntry->rStaRecBf.ucAutoSoundingCtrl = pAd->CommonCfg.ucAutoSoundingCtrl;

			StaCfg.MuarIdx = 0;
			StaCfg.ConnectionState = TRUE;
			StaCfg.ConnectionType = 0;
			StaCfg.u4EnableFeature = (1 << STA_REC_BF);
			StaCfg.ucBssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
			StaCfg.u2WlanIdx = i;
			StaCfg.pEntry = pEntry;
			/* update ucAutoSoundingCtrol and WTBL BF flag */
			if (CmdExtStaRecUpdate(pAd, &StaCfg) == STATUS_TRUE) {
			    fgStatus = TRUE;
			}

			CmdTxBfTxApplyCtrl(pAd, i, fgETxBf, fgITxBf, FALSE, FALSE); /* Set WTBL IBF EBF */
		}
	}

	return fgStatus;
}
#endif /* TXBF_DYNAMIC_DISABLE */

#endif /* MT_MAC */

/*sta rec txbf feature decision*/
UINT32 starec_txbf_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UINT32 features = 0;
	UINT8 u1BandIdx = HcGetBandByWdev(wdev);

	if (entry && (!IS_ENTRY_NONE(entry))) {
		if (HcIsBfCapSupport(wdev)) {
			if (wlan_config_get_etxbf(wdev) == SUBF_ALL
				|| wlan_config_get_etxbf(wdev) == SUBF_BFER
				|| wlan_config_get_itxbf(wdev) == TRUE) {
					switch (wdev->wdev_type) {
					case WDEV_TYPE_AP:
						if (IS_ENTRY_CLIENT(entry)) {
							ad->fgApClientMode = FALSE;
							features |= STA_REC_BF_FEATURE;
						}
						break;
					case WDEV_TYPE_STA:
						if (IF_COMBO_HAVE_AP_STA(ad)) {
							if ((IS_ENTRY_PEER_AP(entry) || IS_ENTRY_REPEATER(entry))) {
								ad->fgApClientMode = TRUE;

								if (ad->fgApCliBfStaRecRegister[u1BandIdx] == FALSE) {
									ad->fgApCliBfStaRecRegister[u1BandIdx] = TRUE;
									ad->ApCli_CmmWlanId = entry->wcid;
									features |= STA_REC_BF_FEATURE;
								}
							}
						} else {
							/* we don't check IS_ENTRY_AP(entry) because there are some problems in the setting */
							ad->fgApClientMode = FALSE;
							features |= STA_REC_BF_FEATURE;
						}
						break;
					default:
						break;
					}
			}

			if (wlan_config_get_etxbf(wdev) == SUBF_ALL || wlan_config_get_etxbf(wdev) == SUBF_BFEE) {
				features |= STA_REC_BFEE_FEATURE;
			}
		}
	}
	/*return value, must use or operation*/
	*feature |= features;
	return TRUE;
}

INT TxBfPseudoTagUpdate(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         lm,
	IN UCHAR         nr,
	IN UCHAR         nc,
	IN UCHAR         bw,
	IN UCHAR         codebook,
	IN UCHAR         group
)
{
	BOOLEAN  fgStatus = FALSE;
	EXT_CMD_ETXBF_PFMU_SW_TAG_T rEBfPfmuSwTag;
	UCHAR band = BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG, "=>\n");

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_ERROR, "wdev is NULL\n");
	} else {
		band = HcGetBandByWdev(wdev);
	}

	memset(&rEBfPfmuSwTag, 0, sizeof(EXT_CMD_ETXBF_PFMU_SW_TAG_T));
	rEBfPfmuSwTag.ucPfmuProfileFormatId = BF_PFMU_SW_TAG_WRITE; /* for Bfee Pseudo PFMU TAG update */
	rEBfPfmuSwTag.ucLm = lm;
	rEBfPfmuSwTag.ucNr = nr;
	rEBfPfmuSwTag.ucNc = nc;
	rEBfPfmuSwTag.ucBw = bw;
	rEBfPfmuSwTag.ucCodebook = codebook;
	rEBfPfmuSwTag.ucgroup = group;
	rEBfPfmuSwTag.ucTxBf = band;

	if (CmdETxBfPseudoTagWrite(pAd, rEBfPfmuSwTag) == STATUS_TRUE) {
		fgStatus = TRUE;
	}

	return fgStatus;
}

#endif /* TXBF_SUPPORT */
