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
	aironet.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/

#ifndef	__ACTION_H__
#define	__ACTION_H__


struct _RTMP_ADAPTER;



VOID MlmeQOSAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID MlmeDLSAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID MlmeInvalidAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID PeerRMAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID PeerQOSAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID peer_addba_req_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem);

VOID peer_addba_rsp_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem);

VOID peer_delba_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem);

VOID PeerBAAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerHTAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

#ifdef DOT11_VHT_AC
VOID PeerVHTAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);
#endif /* DOT11_VHT_AC */

VOID PeerPublicAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

#ifdef CONFIG_STA_SUPPORT
VOID StaPublicAction(
	IN PRTMP_ADAPTER pAd,
	IN BSS_2040_COEXIST_IE * pBss2040CoexIE,
	IN struct wifi_dev *wdev);
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID ApPublicAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_HE_AX
void sta_send_event_report(
	IN struct _RTMP_ADAPTER *pad,
	IN struct _MLME_QUEUE_ELEM *elem);
#endif

#ifdef CONFIG_6G_SUPPORT
ULONG build_fils_discovery_action(struct wifi_dev *wdev, UCHAR *frm_buf);
#endif

#endif /* __ACTION_H__ */
