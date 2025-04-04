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
	mt_io.h
*/

#ifndef __MT_WIFI_IO_H__
#define __MT_WIFI_IO_H__

struct _RTMP_ADAPTER;

UINT32 mt_physical_addr_map(struct _RTMP_ADAPTER *pAd, UINT32 addr);
BOOLEAN mt_mac_cr_range_mapping(struct _RTMP_ADAPTER *pAd, UINT32 *mac_addr);

VOID hif_io_force_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID hif_io_force_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);
VOID hif_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID hif_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);

VOID phy_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID phy_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);

VOID mcu_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID mcu_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);

VOID sys_io_read32(ULONG reg, UINT32 *val);
VOID sys_io_write32(ULONG reg, UINT32 val);

VOID mac_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);
VOID mac_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);

VOID hw_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val);

VOID hw_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val);

VOID hif_core_ops_register(void *hdev_ctrl, INT infType);

VOID hif_core_ops_unregister(void *hdev_ctrl, INT infType);

VOID hif_ctrl_exit(void *chip_hif);
NDIS_STATUS hif_ctrl_init(void **chip_hif, INT infType);
VOID hif_chip_init(VOID *hif_ctrl, UINT32 device_id);



/***********************************************************************************
 * Device Register I/O Access related definitions and data structures.
 **********************************************************************************/

#define MAC_IO_READ32(_hdc, _R, _pV) mac_io_read32(_hdc, _R, _pV)
#define MAC_IO_WRITE32(_hdc, _R, _V) mac_io_write32(_hdc, _R, _V)

#define HIF_IO_READ32(_hdc, _R, _pV) hif_io_read32(_hdc, _R, _pV)
#define HIF_IO_WRITE32(_hdc, _R, _V) hif_io_write32(_hdc, _R, _V)

#define PHY_IO_READ32(_hdc, _R, _pV) phy_io_read32(_hdc, _R, _pV)
#define PHY_IO_WRITE32(_hdc, _R, _V) phy_io_write32(_hdc, _R, _V)

#define HW_IO_READ32(_hdc, _R, _pV) hw_io_read32(_hdc, _R, _pV)
#define HW_IO_WRITE32(_hdc, _R, _V) hw_io_write32(_hdc, _R, _V)

#define MCU_IO_READ32(_hdc, _R, _pV) mcu_io_read32(_hdc, _R, _pV)
#define MCU_IO_WRITE32(_hdc, _R, _V) mcu_io_write32(_hdc, _R, _V)

#define RTMP_IO_READ32(_hdc, _R, _pV) mac_io_read32(_hdc, _R, _pV)
#define RTMP_IO_WRITE32(_hdc, _R, _V) mac_io_write32(_hdc, _R, _V)

#define RTMP_SYS_IO_READ32(_R, _pV) sys_io_read32(_R, _pV)
#define RTMP_SYS_IO_WRITE32(_R, _V) sys_io_write32(_R, _V)

#define RTMP_IO_FORCE_READ32(_hdc, _R, _pV) hif_io_force_read32(_hdc, _R, _pV)
#define RTMP_IO_FORCE_WRITE32(_hdc, _R, _V) hif_io_force_write32(_hdc, _R, _V)

#endif
