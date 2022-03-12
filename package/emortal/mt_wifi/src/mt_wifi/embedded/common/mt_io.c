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
	mt_io.c
*/
#include	"rt_config.h"





#ifdef MT7622
UINT32 mt7622_mac_cr_range[] = {
	0x82060000, 0x8000, 0x450, /* WF_PLE */
	0x82068000, 0xc000, 0x450, /* WF_PSE */
	0x8206c000, 0xe000, 0x300, /* PP */
	0x820d0000, 0x20000, 0x200, /* WF_AON */
	0x820f0000, 0x20200, 0x400, /* WF_CFG */
	0x820f0800, 0x20600, 0x200, /* WF_CFGOFF */
	0x820f1000, 0x20800, 0x200, /* WF_TRB */
	0x820f2000, 0x20a00, 0x200, /* WF_AGG */
	0x820f3000, 0x20c00, 0x400, /* WF_ARB */
	0x820f4000, 0x21000, 0x200, /* WF_TMAC */
	0x820f5000, 0x21200, 0x400, /* WF_RMAC */
	0x820f6000, 0x21600, 0x200, /* WF_SEC */
	0x820f7000, 0x21800, 0x200, /* WF_DMA */

	0x820f8000, 0x22000, 0x1000, /* WF_PF */
	0x820f9000, 0x23000, 0x400, /* WF_WTBLON */
	0x820f9800, 0x23400, 0x200, /* WF_WTBLOFF */

	0x820fa000, 0x24000, 0x200, /* WF_ETBF */
	0x820fb000, 0x24200, 0x400, /* WF_LPON */
	0x820fc000, 0x24600, 0x200, /* WF_INT */
	0x820fd000, 0x24800, 0x400, /* WF_MIB */

	0x820fe000, 0x25000, 0x2000, /* WF_MU */

	0x820e0000, 0x30000, 0x10000, /* WF_WTBL */

	0x80020000, 0x00000, 0x2000, /* TOP_CFG */
	0x80000000, 0x02000, 0x2000, /* MCU_CFG */
	0x50000000, 0x04000, 0x4000, /* PDMA_CFG */
	0xA0000000, 0x08000, 0x8000, /* PSE_CFG */
	0x82070000, 0x10000, 0x10000, /* WF_PHY */

	0x0, 0x0, 0x40000, /* fixed remap range */
	0x0, 0x0, 0x0, /* imply end of search */
};
#endif /* MT7622 */





#ifdef MT7915
UINT32 mt7915_mac_cr_range[] = {
	0x54000000, 0x02000, 0x1000, /* WFDMA PCIE0 MCU DMA0 */
	0x55000000, 0x03000, 0x1000, /* WFDMA PCIE0 MCU DMA1 */
	0x56000000, 0x04000, 0x1000, /* WFDMA reserved */
	0x57000000, 0x05000, 0x1000, /* WFDMA MCU wrap CR */
	0x58000000, 0x06000, 0x1000, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	0x59000000, 0x07000, 0x1000, /* WFDMA PCIE1 MCU DMA1 */
	0x820c0000, 0x08000, 0x4000, /* WF_UMAC_TOP (PLE) */
	0x820c8000, 0x0c000, 0x2000, /* WF_UMAC_TOP (PSE) */
	0x820cc000, 0x0e000, 0x2000, /* WF_UMAC_TOP (PP) */
	0x820e0000, 0x20000, 0x0400, /* WF_LMAC_TOP BN0 (WF_CFG) */
	0x820e1000, 0x20400, 0x0200, /* WF_LMAC_TOP BN0 (WF_TRB) */
	0x820e2000, 0x20800, 0x0400, /* WF_LMAC_TOP BN0 (WF_AGG) */
	0x820e3000, 0x20c00, 0x0400, /* WF_LMAC_TOP BN0 (WF_ARB) */
	0x820e4000, 0x21000, 0x0400, /* WF_LMAC_TOP BN0 (WF_TMAC) */
	0x820e5000, 0x21400, 0x0800, /* WF_LMAC_TOP BN0 (WF_RMAC) */
	0x820ce000, 0x21c00, 0x0200, /* WF_LMAC_TOP (WF_SEC) */
	0x820e7000, 0x21e00, 0x0200, /* WF_LMAC_TOP BN0 (WF_DMA) */
	0x820cf000, 0x22000, 0x1000, /* WF_LMAC_TOP (WF_PF) */
	0x820e9000, 0x23400, 0x0200, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	0x820ea000, 0x24000, 0x0200, /* WF_LMAC_TOP BN0 (WF_ETBF) */
	0x820eb000, 0x24200, 0x0400, /* WF_LMAC_TOP BN0 (WF_LPON) */
	0x820ec000, 0x24600, 0x0200, /* WF_LMAC_TOP BN0 (WF_INT) */
	0x820ed000, 0x24800, 0x0800, /* WF_LMAC_TOP BN0 (WF_MIB) */
	0x820ca000, 0x26000, 0x2000, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	0x820d0000, 0x30000, 0x10000, /* WF_LMAC_TOP (WF_WTBLON) */
	0x40000000, 0x70000, 0x10000, /* WF_UMAC_SYSRAM */
	0x00400000, 0x80000, 0x10000, /* WF_MCU_SYSRAM */
	0x00410000, 0x90000, 0x10000, /* WF_MCU_SYSRAM (configure register) */
	0x820f0000, 0xa0000, 0x0400, /* WF_LMAC_TOP BN1 (WF_CFG) */
	0x820f1000, 0xa0600, 0x0200, /* WF_LMAC_TOP BN1 (WF_TRB) */
	0x820f2000, 0xa0800, 0x0400, /* WF_LMAC_TOP BN1 (WF_AGG) */
	0x820f3000, 0xa0c00, 0x0400, /* WF_LMAC_TOP BN1 (WF_ARB) */
	0x820f4000, 0xa1000, 0x0400, /* WF_LMAC_TOP BN1 (WF_TMAC) */
	0x820f5000, 0xa1400, 0x0800, /* WF_LMAC_TOP BN1 (WF_RMAC) */
	0x820f7000, 0xa1e00, 0x0200, /* WF_LMAC_TOP BN1 (WF_DMA) */
	0x820f9000, 0xa3400, 0x0200, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	0x820fa000, 0xa4000, 0x0200, /* WF_LMAC_TOP BN1 (WF_ETBF) */
	0x820fb000, 0xa4200, 0x0400, /* WF_LMAC_TOP BN1 (WF_LPON) */
	0x820fc000, 0xa4600, 0x0200, /* WF_LMAC_TOP BN1 (WF_INT) */
	0x820fd000, 0xa4800, 0x0800, /* WF_LMAC_TOP BN1 (WF_MIB) */
	0x820cc000, 0xa5000, 0x2000, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	0x820c4000, 0xa8000, 0x4000, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	0x820b0000, 0xae000, 0x1000, /* [APB2] WFSYS_ON */
	0x80020000, 0xb0000, 0x10000, /* WF_TOP_MISC_OFF */
	0x81020000, 0xc0000, 0x10000, /* WF_TOP_MISC_ON */
	0x7c020000, 0xd0000, 0x10000, /* CONN_INFRA, wfdma */
	0x7c000000, 0xf0000, 0x10000, /* CONN_INFRA */

	0x0, 0x0, 0x100000, /* fixed remap range */
	0x0, 0x0, 0x0, /* imply end of search */
};
#endif


BOOLEAN mt_mac_cr_range_mapping(RTMP_ADAPTER *pAd, UINT32 *mac_addr)
{
	UINT32 mac_addr_hif = *mac_addr;
	INT idx = 0;
	BOOLEAN IsFound = 0;
	UINT32 *mac_cr_range = NULL;


#ifdef MT7622

	if (IS_MT7622(pAd))
		mac_cr_range = &mt7622_mac_cr_range[0];

#endif /* MT7622 */
#ifdef MT7915

	if (IS_MT7915(pAd))
		mac_cr_range = &mt7915_mac_cr_range[0];

#endif

	if (!mac_cr_range) {
		MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NotSupported Chip for this function!, CHIP_ID=%x\n", __func__, pAd->ChipID));
		return IsFound;
	}

	do {
		if (mac_addr_hif >= mac_cr_range[idx] &&
			mac_addr_hif < (mac_cr_range[idx] + mac_cr_range[idx + 2])) {
			mac_addr_hif -= mac_cr_range[idx];
			mac_addr_hif += mac_cr_range[idx + 1];
			IsFound = 1;
			break;
		}

		idx += 3;
	} while (mac_cr_range[idx + 2] != 0);

#ifdef RTMP_MAC_PCI
	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		/* PCIe address space is 20-bit wide */
		if (IsFound && mac_addr_hif > 0xfffff) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CR Addr[0x%x] out of range\n", mac_addr_hif));
				return 0;
		}
	}
#endif /* MT_MAC */

	*mac_addr = mac_addr_hif;
	return IsFound;
}


UINT32 mt_physical_addr_map(RTMP_ADAPTER *pAd, UINT32 addr)
{
	UINT32 global_addr = 0x0, idx = 1;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 wtbl_2_base = cap->WtblPseAddr;

	if (addr < 0x2000)
		global_addr = 0x80020000 + addr;
	else if ((addr >= 0x2000) && (addr < 0x4000))
		global_addr = 0x80000000 + addr - 0x2000;
	else if ((addr >= 0x4000) && (addr < 0x8000))
		global_addr = 0x50000000 + addr - 0x4000;
	else if ((addr >= 0x8000) && (addr < 0x10000))
		global_addr = 0xa0000000 + addr - 0x8000;
	else if ((addr >= 0x10000) && (addr < 0x20000))
		global_addr = 0x60200000 + addr - 0x10000;
	else if ((addr >= 0x20000) && (addr < 0x40000)) {
		UINT32 *mac_cr_range = NULL;
#ifdef MT7622

		if (IS_MT7622(pAd))
			mac_cr_range = &mt7622_mac_cr_range[0];

#endif /* MT7622 */
#ifdef MT7915

		if (IS_MT7915(pAd))
			mac_cr_range = &mt7915_mac_cr_range[0];

#endif

		if (!mac_cr_range) {
			MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NotSupported Chip for this function!\n", __func__));
			return global_addr;
		}

		do {
			if ((addr >= mac_cr_range[idx]) && (addr < (mac_cr_range[idx] + mac_cr_range[idx + 1]))) {
				global_addr = mac_cr_range[idx - 1] + (addr - mac_cr_range[idx]);
				break;
			}

			idx += 3;
		} while (mac_cr_range[idx] != 0);

		if (mac_cr_range[idx] == 0)
			MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("unknow addr range = %x\n", addr));
	} else if ((addr >= 0x40000) && (addr < 0x80000)) { /* WTBL Address */
		global_addr = wtbl_2_base + addr - 0x40000;
		MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr1=0x%x\n", global_addr));
	} else if ((addr >= 0xc0000) && (addr < 0xc0100)) { /* PSE Client */
		global_addr = 0x800c0000 + addr - 0xc0000;
		MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr2=0x%x\n", global_addr));
	} else {
		global_addr = addr;
		MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr3=0x%x\n", global_addr));
	}

	return global_addr;
}


/*export io func.*/
VOID hif_io_force_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hif_io_forec_read32)
		ops->hif_io_forec_read32(hdev_ctrl, reg, val);
}

VOID hif_io_force_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hif_io_forec_write32)
		ops->hif_io_forec_write32(hdev_ctrl, reg, val);
}

VOID hif_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hif_io_read32)
		ops->hif_io_read32(hdev_ctrl, reg, val);
}

VOID hif_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hif_io_write32)
		ops->hif_io_write32(hdev_ctrl, reg, val);
}

VOID sys_io_read32(ULONG reg, UINT32 *val)
{
	*val = readl((void *) reg);
}

VOID sys_io_write32(ULONG reg, UINT32 val)
{
	writel(val, (void *) reg);
}

VOID mac_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->mac_io_read32)
		ops->mac_io_read32(hdev_ctrl, reg, val);
}

VOID mac_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->mac_io_write32)
		ops->mac_io_write32(hdev_ctrl, reg, val);
}

VOID phy_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->phy_io_read32)
		ops->phy_io_read32(hdev_ctrl, reg, val);
}

VOID phy_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->phy_io_write32)
		ops->phy_io_write32(hdev_ctrl, reg, val);
}

VOID mcu_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->mcu_io_read32)
		ops->mcu_io_read32(hdev_ctrl, reg, val);
}

VOID mcu_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->mcu_io_write32)
		ops->mcu_io_write32(hdev_ctrl, reg, val);
}

VOID hw_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hw_io_read32)
		ops->hw_io_read32(hdev_ctrl, reg, val);
}

VOID hw_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hw_io_write32)
		ops->hw_io_write32(hdev_ctrl, reg, val);
}

VOID hif_core_ops_register(void *hdev_ctrl, INT infType)
{
#ifdef RTMP_MAC_PCI
	if (infType == RTMP_DEV_INF_PCIE || infType == RTMP_DEV_INF_PCI || infType == RTMP_DEV_INF_RBUS)
		pci_core_ops_register(hdev_ctrl);
#endif


}

VOID hif_core_ops_unregister(void *hdev_ctrl, INT infType)
{
#ifdef RTMP_MAC_PCI
	if (infType == RTMP_DEV_INF_PCIE || infType == RTMP_DEV_INF_PCI || infType == RTMP_DEV_INF_RBUS)
		pci_core_ops_unregister(hdev_ctrl);
#endif


}

NDIS_STATUS hif_ctrl_init(void **chip_hif, INT infType)
{
	UINT32 hif_size = 0;
#ifdef RTMP_MAC_PCI
	if (infType == RTMP_DEV_INF_PCIE || infType == RTMP_DEV_INF_PCI || infType == RTMP_DEV_INF_RBUS)
		hif_size = sizeof(struct pci_hif_chip);
#endif



	if (hif_size == 0)
		return NDIS_STATUS_INVALID_DATA;

	if (multi_hif_entry_alloc(chip_hif, hif_size) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	return NDIS_STATUS_SUCCESS;
}

VOID hif_ctrl_exit(void *chip_hif)
{
	multi_hif_entry_free(chip_hif);
}

VOID hif_chip_init(VOID *chip_hif, UINT32 device_id)
{
#ifdef MT7915
	if ((device_id & 0x0000FFFF) == 0x7916)
		mt7916_hif_init(chip_hif);
	if ((device_id & 0x0000FFFF) == 0x7915)
		mt7915_hif_ctrl_chip_init(chip_hif);
#endif /*MT7915*/
#ifdef MT7622
	if ((device_id & 0x0000FFFF) == 0x7622)
		mt7622_hif_ctrl_chip_init(chip_hif);
#endif /*MT7622*/

}
