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
	icap.c
*/
#include "rt_config.h"

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
#ifdef WIFI_SPECTRUM_SUPPORT
/* MT7622 wifi-spectrum descriptor */
RBIST_DESC_T MT7622_SPECTRUM_DESC[] = {
	/* AHB Start Addr, AddrOffset, Bank, BankSize, BitWidth, ADCRes, IQCRes, LBank, MBank, HBank */
	{0x00110004, 0x10,  5, 16, 32, 10, 12, NULL, NULL, NULL},
};
UINT8 MT7622_SpectrumBankNum  = sizeof(MT7622_SPECTRUM_DESC) / sizeof(RBIST_DESC_T);
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
/* MT7622 icap descriptor */
UINT8 MT7622_ICAP_LBANK[] = {1, 5};
UINT8 MT7622_ICAP_MBANK[] = {2, 6};
UINT8 MT7622_ICAP_HBANK[] = {3, 7};
RBIST_DESC_T MT7622_ICAP_DESC[] = {
	/* AHB Start Addr, AddrOffset, Bank, BankSize, BitWidth, ADCRes, IQCRes, LBank, MBank, HBank */
	{0x00100004, 0x10, 1, 16, 96, 4, 12, MT7622_ICAP_LBANK, MT7622_ICAP_MBANK, MT7622_ICAP_HBANK},
	{0x00100008, 0x10, 2, 16, 96, 4, 12, MT7622_ICAP_LBANK, MT7622_ICAP_MBANK, MT7622_ICAP_HBANK},
	{0x0010000C, 0x10, 3, 16, 96, 4, 12, MT7622_ICAP_LBANK, MT7622_ICAP_MBANK, MT7622_ICAP_HBANK},
	{0x00110004, 0x10, 5, 16, 96, 4, 12, MT7622_ICAP_LBANK, MT7622_ICAP_MBANK, MT7622_ICAP_HBANK},
	{0x00110008, 0x10, 6, 16, 96, 4, 12, MT7622_ICAP_LBANK, MT7622_ICAP_MBANK, MT7622_ICAP_HBANK},
	{0x0011000C, 0x10, 7, 16, 96, 4, 12, MT7622_ICAP_LBANK, MT7622_ICAP_MBANK, MT7622_ICAP_HBANK}
};
UINT8 MT7622_ICapBankNum  = sizeof(MT7622_ICAP_DESC) / sizeof(RBIST_DESC_T);
#endif /* INTERNAL_CAPTURE_SUPPORT */
/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

