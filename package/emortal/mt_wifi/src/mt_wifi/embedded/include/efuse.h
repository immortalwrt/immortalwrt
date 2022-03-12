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
	efuse.h
*/
#ifndef __EFUSE_H__
#define __EFUSE_H__

/* eFuse registers */
#define EFUSE_CTRL				0x0580
#define EFUSE_DATA0				0x0590
#define EFUSE_DATA1				0x0594
#define EFUSE_DATA2				0x0598
#define EFUSE_DATA3				0x059c

#define EFUSE_CTRL_3290			0x24
#define EFUSE_DATA0_3290		0x28
#define EFUSE_DATA1_3290		0x2c
#define EFUSE_DATA2_3290		0x30
#define EFUSE_DATA3_3290		0x34

#define EFUSE_TAG				0x2fe

#if defined(MT_FMAC)
#define MT_EEF_BASE 0x70011000
#define MT_EE_CTRL (MT_EEF_BASE)
#define MT_EFUSE_CTRL (MT_EEF_BASE + 0x8)
#define MT_EFUSE_WDATA0 (MT_EEF_BASE + 0x10)
#define MT_EFUSE_WDATA1 (MT_EEF_BASE + 0x14)
#define MT_EFUSE_WDATA2 (MT_EEF_BASE + 0x18)
#define MT_EFUSE_WDATA3 (MT_EEF_BASE + 0x1C)
#define MT_EFUSE_RDATA0 (MT_EEF_BASE + 0x30)
#define MT_EFUSE_RDATA1 (MT_EEF_BASE + 0x34)
#define MT_EFUSE_RDATA2 (MT_EEF_BASE + 0x38)
#define MT_EFUSE_RDATA3 (MT_EEF_BASE + 0x3C)
#else
#define MT_EEF_BASE 0x81070000
#define MT_EE_CTRL (MT_EEF_BASE)
#define MT_EFUSE_CTRL (MT_EEF_BASE + 0x8)
#define MT_EFUSE_WDATA0 (MT_EEF_BASE + 0x10)
#define MT_EFUSE_WDATA1 (MT_EEF_BASE + 0x14)
#define MT_EFUSE_WDATA2 (MT_EEF_BASE + 0x18)
#define MT_EFUSE_WDATA3 (MT_EEF_BASE + 0x1C)
#define MT_EFUSE_RDATA0 (MT_EEF_BASE + 0x30)
#define MT_EFUSE_RDATA1 (MT_EEF_BASE + 0x34)
#define MT_EFUSE_RDATA2 (MT_EEF_BASE + 0x38)
#define MT_EFUSE_RDATA3 (MT_EEF_BASE + 0x3C)
#endif

#ifdef RT_BIG_ENDIAN
typedef	union	_EFUSE_CTRL_STRUC {
	struct	{
		UINT32            SEL_EFUSE:1;
		UINT32            EFSROM_KICK:1;
		UINT32            EFSROM_DOUT_VLD:1;
		UINT32            RESERVED:3;
		UINT32            EFSROM_AIN:10;
		UINT32            EFSROM_LDO_ON_TIME:2;
		UINT32            EFSROM_LDO_OFF_TIME:6;
		UINT32            EFSROM_MODE:2;
		UINT32            EFSROM_AOUT:6;
	}	field;
	UINT32			word;
}	EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#else
typedef	union	_EFUSE_CTRL_STRUC {
	struct	{
		UINT32            EFSROM_AOUT:6;
		UINT32            EFSROM_MODE:2;
		UINT32            EFSROM_LDO_OFF_TIME:6;
		UINT32            EFSROM_LDO_ON_TIME:2;
		UINT32            EFSROM_AIN:10;
		UINT32            RESERVED:3;
		UINT32            EFSROM_DOUT_VLD:1;
		UINT32            EFSROM_KICK:1;
		UINT32            SEL_EFUSE:1;
	}	field;
	UINT32			word;
}	EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */

VOID eFuseReadPhysical(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUSHORT lpInBuffer,
	IN	ULONG nInBufferSize,
	OUT	PUSHORT lpOutBuffer,
	IN	ULONG nOutBufferSize);

#endif /* __EFUSE_H__ */
