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
/****************************************************************************
 ****************************************************************************

    Abstract:
	IEEE P802.11w

 */
#ifndef __PMF_H
#define __PMF_H
#ifdef DOT11W_PMF_SUPPORT

#define SAQ_IDLE	0
#define SAQ_RETRY	1
#define SAQ_SENDING	2

#define DEFAULT_SAQUERY_TIMEOUT		1000
#define DEFAULT_SAQUERY_CONFIRM_TIMEOUT	200

VOID PMF_PeerAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID PMF_MlmeSAQueryReq(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY * pEntry);

VOID PMF_PeerSAQueryReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID PMF_PeerSAQueryRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID PMF_DeriveIGTK(
	IN PRTMP_ADAPTER pAd,
	OUT UCHAR * output);

VOID PMF_InsertIGTKKDE(
	IN PRTMP_ADAPTER pAd,
	IN INT apidx,
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen);

BOOLEAN PMF_ExtractIGTKKDE(
	IN PUCHAR pBuf,
	IN INT buf_len,
	OUT PUCHAR IGTK,
	OUT UCHAR * IGTKLEN,
	OUT PUCHAR IPN,
	OUT UINT8 * IGTK_KeyIdx);

BOOLEAN PMF_MakeRsnIeGMgmtCipher(
	IN SECURITY_CONFIG * pSecConfig,
	IN UCHAR ie_idx,
	OUT UCHAR * rsn_len);

UINT PMF_RsnCapableValidation(
	IN PUINT8 pRsnie,
	IN UINT rsnie_len,
	IN BOOLEAN self_MFPC,
	IN BOOLEAN self_MFPR,
	IN UINT32 self_igtk_cipher,
	IN UCHAR end_field,
	IN struct _SECURITY_CONFIG *pSecConfigEntry);

BOOLEAN PMF_PerformTxFrameAction(
	IN PRTMP_ADAPTER pAd,
	IN PHEADER_802_11 pHeader_802_11,
	IN UINT SrcBufLen,
	IN UINT8 tx_hw_hdr_len,
	OUT UCHAR * prot);

BOOLEAN	PMF_PerformRxFrameAction(
	IN PRTMP_ADAPTER pAd,
	IN RX_BLK * pRxBlk);

int PMF_RobustFrameClassify(
	IN PHEADER_802_11 pHdr,
	IN PUCHAR pFrame,
	IN UINT	frame_len,
	IN PUCHAR pData,
	IN BOOLEAN IsRx);

#ifdef SOFT_ENCRYPT
int PMF_EncapBIPAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMgmtFrame,
	IN UINT	mgmt_len);
#endif /* SOFT_ENCRYPT */

void PMF_AddMMIE(
	IN PMF_CFG * pPmfCfg,
	IN PUCHAR pMgmtFrame,
	IN UINT	mgmt_len);

void rtmp_read_pmf_parameters_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer);

INT Set_PMFMFPC_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_PMFMFPR_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_PMFSHA256_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_PMFSA_Q_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

#ifdef BCN_PROTECTION_SUPPORT
VOID read_bcn_prot_parma_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer);

INT build_bcn_mmie(
	IN struct bcn_protection_cfg *bcn_prot_cfg,
	IN UCHAR *buf);

VOID insert_bigtk_kde(
	IN PRTMP_ADAPTER pAd,
	IN INT apidx,
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen);


UCHAR get_bigtk_table_idx(
	struct bcn_protection_cfg *bcn_prot_cfg);

VOID bcn_prot_update_bipn(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev);
#endif

#endif /* DOT11W_PMF_SUPPORT */

#endif /* __PMF_H */

