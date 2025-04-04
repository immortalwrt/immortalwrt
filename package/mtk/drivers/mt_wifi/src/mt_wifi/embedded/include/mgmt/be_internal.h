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

#ifndef __BE_INTERNAL_H__
#define __BE_INTERNAL_H__

#include "mgmt/be_export.h"

/*
* private structure definition to prevent direct access
*/
struct wlan_config {
	struct phy_cfg phy_conf;
	struct ht_cfg ht_conf;
	struct vht_cfg vht_conf;
	struct he_cfg he_conf;
};


struct wlan_operate {
	struct phy_op phy_oper;
	struct ht_op ht_oper;
	struct ht_op_status ht_status;
	struct vht_op vht_oper;
	struct he_op he_oper;
	UCHAR state;
};

/*
* Utility
*/
BOOLEAN phy_get_freq_adjust(struct wifi_dev *wdev, struct freq_cfg *cfg, struct freq_oper *op);

VOID phy_freq_get_cfg(struct wifi_dev *wdev, struct freq_cfg *cfg);


/*
* internal export configure loader
*/
VOID phy_oper_init(struct wifi_dev *wdev, struct phy_op *obj);
VOID phy_oper_exit(struct phy_op *obj);

/*
*ht related
*/
VOID ht_oper_init(struct wifi_dev *wdev, struct ht_op *obj);
VOID ht_oper_exit(struct ht_op *obj);


/*
*vht related
*/
VOID vht_oper_init(struct wifi_dev *wdev, struct vht_op *obj);
VOID vht_oper_exit(struct vht_op *obj);

/*
*he related
*/
VOID he_oper_init(struct wifi_dev *wdev, struct he_op *obj);
VOID he_oper_exit(struct he_op *obj);

/*
* ht operate related
*/
VOID ht_op_status_init(struct wifi_dev *wdev, struct ht_op_status *obj);
VOID ht_op_status_exit(struct ht_op_status *obj);

/*
* be_phy module
*/
VOID operate_loader_prim_ch(struct wlan_operate *op);
VOID operate_loader_phy(struct wifi_dev *wdev, struct freq_cfg *fcfg);

/*
* be_ht module
*/
VOID operate_loader_ht_bw(struct wlan_operate *op);
VOID operate_loader_ext_cha(struct wlan_operate *op);
VOID operate_loader_frag_thld(struct wlan_operate *op, UINT32 frag_thld);
VOID operate_loader_rts_len_thld(struct wlan_operate *op, UINT32 len_thld);
VOID operate_loader_rts_pkt_thld(struct wlan_operate *op, UCHAR pkt_num);
VOID operate_loader_trx_stream(struct wifi_dev *wdev, struct wlan_operate *op, UINT8 tx_stream, UINT8 rx_stream);
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
VOID operate_loader_eap_trx_stream(struct wifi_dev *wdev, struct wlan_operate *op, UINT8 tx_stream, UINT8 rx_stream);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


/*
* be_vht module
*/
VOID operate_loader_vht_bw(struct wlan_operate *op);
VOID operate_loader_vht_ldpc(struct wlan_operate *op, UCHAR vht_ldpc);


#endif /*__BE_INTERNAL_H__*/
