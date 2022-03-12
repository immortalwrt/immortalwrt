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
	client.h
*/

#ifndef __CLIENT_H__
#define __CLIENT_H__

#define CLIENT_BASE		0x800C0000
#define RXINF (CLIENT_BASE + 0x0068)
#define RXSH_GROUP1_EN (1 << 0)
#define RXSH_GROUP2_EN (1 << 1)
#define RXSH_GROUP3_EN (1 << 2)

#define RST (CLIENT_BASE + 0x0070)
#define TX_R_E_1 (1 << 16)
#define TX_R_E_2 (1 << 17)
#define TX_R_E_1_S (1 << 20)
#define TX_R_E_2_S (1 << 21)

#endif

