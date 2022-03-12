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
 ***************************************************************************/

/****************************************************************************

	Abstract:
	Management Frame Protection Required is defined in IEEE802.11w

	Define all structures, data types that rtmp.h needed in this file. Don't
	put any sturctures and functions definition which refer to RTMP_ADAPTER
	Here.

***************************************************************************/


#ifdef DOT11W_PMF_SUPPORT

#ifndef __PMF_CMM_H__
#define __PMF_CMM_H__

#include "rtmp_type.h"
#include "security/dot11w_pmf.h"

#define	NORMAL_FRAME                    0
#define	ERROR_FRAME                     1 /* no used */
#define	NOT_ROBUST_GROUP_FRAME		2
#define	NOT_ROBUST_UNICAST_FRAME	3
#define	UNICAST_ROBUST_FRAME		4
#define	GROUP_ROBUST_FRAME		5

typedef enum _PMF_STATUS_NUM {
	PMF_STATUS_SUCCESS,
	PMF_POLICY_VIOLATION,
	PMF_UNICAST_ENCRYPT_FAILURE,
	PMF_ENCAP_BIP_FAILURE,
	PMF_UNICAST_DECRYPT_FAILURE,
	PMF_EXTRACT_BIP_FAILURE,
	PMF_STATUS_RESV
} PMF_STATUS_NUM;

/*
 * Management Frame Protection Required is defined in IEEE802.11w
 */
typedef struct GNU_PACKED _PMF_CFG {
	/*
	 * A STA sets this bit to 1 to advertise that protection of Robust
	 * Management Frames is enabled.
	 */
	BOOLEAN	MFPC;           /* This is actual active */
	BOOLEAN	Desired_MFPC;	/* This is user desired */

	/*
	 * A STA sets this bit to 1 to advertise that protection of
	 * Robust Management Frames is mandatory.
	 * If a STA sets this bit to 1, then that STA only allows RSNAs
	 * from STAs which provide Management Frame Protection.
	 */
	BOOLEAN	MFPR;           /* This is actual active */
	BOOLEAN	Desired_MFPR;	/* This is user desired */

	BOOLEAN	PMFSHA256;         /* This is actual active */
	BOOLEAN	Desired_PMFSHA256; /* This is user desired */

	/* Connect State */
	BOOLEAN	UsePMFConnect;

	UINT32 igtk_cipher;

	UINT8	IGTK_KeyIdx;			/* It shall be 4 or 5 */
	UCHAR	IGTK[2][LEN_MAX_IGTK];
	UCHAR	IPN[2][LEN_WPA_TSC];

	UCHAR PmfTxTsc[LEN_WPA_TSC];
	UCHAR PmfRxTsc[LEN_WPA_TSC];
	RALINK_TIMER_STRUCT SAQueryTimer;
	RALINK_TIMER_STRUCT SAQueryConfirmTimer;
	UCHAR SAQueryStatus;
	USHORT TransactionID;
} PMF_CFG, *PPMF_CFG;

#ifdef BCN_PROTECTION_SUPPORT
struct bcn_protection_cfg {
	UCHAR desired_bcn_prot_en;
	UCHAR bcn_prot_en;
	UINT32 bigtk_cipher;         /* redundant? it should align with igtk_cipher */
	UINT8 bigtk_key_idx;			/* It shall be 6 or 7*/
	UCHAR bigtk[2][LEN_MAX_BIGTK];
	UCHAR bipn[2][LEN_WPA_TSC]; /* firmware maintain, request from firmware */
};

enum {
	BCN_PROT_EN_OFF = 0,
	BCN_PROT_EN_SW_MODE = 1,
	BCN_PROT_EN_HW_MODE = 2,
};

struct bcn_prot_test_ctrl {
	UCHAR omn_ie_en;
	UCHAR csa_ie_en;
	UCHAR mm_ie_en;
	UCHAR ht_opt_ie_en;
	UCHAR channel;
	UCHAR bw;
	UCHAR switch_cnt;
	UCHAR omn_nss;
	UCHAR ht_opt_bw;
	UCHAR ht_opt_nss;
};
#endif

#endif /* __PMF_CMM_H__ */
#endif /* DOT11W_PMF_SUPPORT */


