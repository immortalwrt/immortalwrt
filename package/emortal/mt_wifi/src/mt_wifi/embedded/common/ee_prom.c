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
	ee_prom.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"



/* IRQL = PASSIVE_LEVEL*/
static inline VOID RaiseClock(RTMP_ADAPTER *pAd, UINT32 *x)
{
	*x = *x | EESK;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, E2PROM_CSR, *x);
	RtmpusecDelay(1);				/* Max frequency = 1MHz in Spec. definition */
}


/* IRQL = PASSIVE_LEVEL*/
static inline VOID LowerClock(RTMP_ADAPTER *pAd, UINT32 *x)
{
	*x = *x & ~EESK;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, E2PROM_CSR, *x);
	RtmpusecDelay(1);
}


/* IRQL = PASSIVE_LEVEL*/
static inline USHORT ShiftInBits(RTMP_ADAPTER *pAd)
{
	UINT32 x = 0, i;
	UINT16 data = 0;
	RTMP_IO_READ32(pAd->hdev_ctrl, E2PROM_CSR, &x);
	x &= ~(EEDO | EEDI);

	for (i = 0; i < 16; i++) {
		data = data << 1;
		RaiseClock(pAd, &x);
		RTMP_IO_READ32(pAd->hdev_ctrl, E2PROM_CSR, &x);
		LowerClock(pAd, &x); /*prevent read failed*/
		x &= ~(EEDI);

		if (x & EEDO)
			data |= 1;
	}

	return data;
}


/* IRQL = PASSIVE_LEVEL*/
static inline VOID ShiftOutBits(RTMP_ADAPTER *pAd, USHORT data, USHORT count)
{
	UINT32 x = 0, mask;
	mask = 0x01 << (count - 1);
	RTMP_IO_READ32(pAd->hdev_ctrl, E2PROM_CSR, &x);
	x &= ~(EEDO | EEDI);

	do {
		x &= ~EEDI;

		if (data & mask)
			x |= EEDI;

		RTMP_IO_WRITE32(pAd->hdev_ctrl, E2PROM_CSR, x);
		RaiseClock(pAd, &x);
		LowerClock(pAd, &x);
		mask = mask >> 1;
	} while (mask);

	x &= ~EEDI;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, E2PROM_CSR, x);
}


/* IRQL = PASSIVE_LEVEL*/
static inline VOID EEpromCleanup(RTMP_ADAPTER *pAd)
{
	UINT32 x = 0;
	RTMP_IO_READ32(pAd->hdev_ctrl, E2PROM_CSR, &x);
	x &= ~(EECS | EEDI);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, E2PROM_CSR, x);
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);
}


static inline VOID EWEN(RTMP_ADAPTER *pAd)
{
	UINT32	x = 0;
	/* reset bits and set EECS*/
	RTMP_IO_READ32(pAd->hdev_ctrl, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, E2PROM_CSR, x);
	/* kick a pulse*/
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);
	/* output the read_opcode and six pulse in that order    */
	ShiftOutBits(pAd, EEPROM_EWEN_OPCODE, 5);
	ShiftOutBits(pAd, 0, 6);
	EEpromCleanup(pAd);
}


static inline VOID EWDS(RTMP_ADAPTER *pAd)
{
	UINT32	x = 0;
	/* reset bits and set EECS*/
	RTMP_IO_READ32(pAd->hdev_ctrl, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, E2PROM_CSR, x);
	/* kick a pulse*/
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);
	/* output the read_opcode and six pulse in that order    */
	ShiftOutBits(pAd, EEPROM_EWDS_OPCODE, 5);
	ShiftOutBits(pAd, 0, 6);
	EEpromCleanup(pAd);
}


/* IRQL = PASSIVE_LEVEL*/
BOOLEAN rtmp_ee_prom_read16(RTMP_ADAPTER *pAd, UINT32 Offset, UINT16 *pValue)
{
	UINT32 x = 0;
	UINT16 data;
	BOOLEAN IsEmpty = 0;
	Offset /= 2;
	/* reset bits and set EECS*/
	RTMP_IO_READ32(pAd->hdev_ctrl, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, E2PROM_CSR, x);
	/* output the read_opcode and register number in that order    */
	ShiftOutBits(pAd, EEPROM_READ_OPCODE, 3);
	ShiftOutBits(pAd, Offset, pAd->EEPROMAddressNum);
	/* Now read the data (16 bits) in from the selected EEPROM word*/
	data = ShiftInBits(pAd);
	EEpromCleanup(pAd);
	*pValue = data;

	if ((*pValue == 0xffff) || (*pValue == 0x0000))
		IsEmpty = 1;

	return IsEmpty;
}


int rtmp_ee_prom_write16(RTMP_ADAPTER *pAd, UINT32 Offset, UINT16 Data)
{
	UINT32 x = 0;
	Offset /= 2;
	EWEN(pAd);
	/* reset bits and set EECS*/
	RTMP_IO_READ32(pAd->hdev_ctrl, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, E2PROM_CSR, x);
	/* output the read_opcode ,register number and data in that order */
	ShiftOutBits(pAd, EEPROM_WRITE_OPCODE, 3);
	ShiftOutBits(pAd, Offset, pAd->EEPROMAddressNum);
	ShiftOutBits(pAd, Data, 16);		/* 16-bit access*/
	/* read DO status*/
	RTMP_IO_READ32(pAd->hdev_ctrl, E2PROM_CSR, &x);
	EEpromCleanup(pAd);
	RtmpusecDelay(10000);	/*delay for twp(MAX)=10ms*/
	EWDS(pAd);
	EEpromCleanup(pAd);
	return NDIS_STATUS_SUCCESS;
}


INT rtmp_ee_write_to_prom(RTMP_ADAPTER *pAd)
{
	USHORT value, offset = 0;
	{
		for (offset = 0; offset < (EEPROM_SIZE >> 1);) {
			value = ((PUSHORT)pAd->EEPROMImage)[offset];
			RT28xx_EEPROM_WRITE16(pAd, (offset << 1), value);
			offset++;
		}
	}

	return TRUE;
}

