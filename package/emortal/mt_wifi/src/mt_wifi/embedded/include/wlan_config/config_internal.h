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

#ifndef __CONFIG_INTERNAL_H__
#define __CONFIG_INTERNAL_H__

#include "mgmt/be_internal.h"

/*
* define struct constructor & deconstructor
*/

VOID phy_cfg_init(struct phy_cfg *obj);
VOID phy_cfg_exit(struct phy_cfg *obj);

/*
*ht phy info related
*/
VOID ht_cfg_init(struct ht_cfg *obj);
VOID ht_cfg_exit(struct ht_cfg *obj);

/*
*vht info related
*/
VOID vht_cfg_init(struct vht_cfg *obj);
VOID vht_cfg_exit(struct vht_cfg *obj);

/*
 * he related
 */
VOID he_cfg_init(struct he_cfg *obj);
VOID he_cfg_exit(struct he_cfg *obj);

/*
* internal export configure loader
*/

#endif
