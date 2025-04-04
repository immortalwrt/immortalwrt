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
    pci_main_dev.c

    Abstract:
    Create and register network interface for PCI based chipsets in Linux platform.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#define RTMP_MODULE_OS

#include "rtmp_comm.h"
#include "rtmp_chip.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include "rt_config.h"
#include <linux/pci.h>

#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
#define DEVEXIT
#define DEVEXIT_P
#define DEVINIT
#define DEVINITDATA
#else
#define DEVEXIT __devexit
#define DEVEXIT_P __devexit_p
#define DEVINIT __devinit
#define DEVINITDATA __devinitdata
#endif

#ifdef MEM_ALLOC_INFO_SUPPORT
extern MEM_INFO_LIST MemInfoList;
extern MEM_INFO_LIST PktInfoList;
#endif /*MEM_ALLOC_INFO_SUPPORT*/
/**
 * @addtogroup hif
 * @{
 * @code PCI Device ID
 */

static struct pci_device_id rt_pci_tbl[] DEVINITDATA = {




#ifdef MT7986
	{PCI_DEVICE(MT7986_PCI_VENDOR_ID, NIC7986_PCIe_DEVICE_ID)},
	{PCI_DEVICE(MT7986_PCI_VENDOR_ID, NIC7986_FPGA_PCIe_DEVICE_ID)},
#endif
#ifdef MT7916
	{PCI_DEVICE(MT7906_PCI_VENDOR_ID, NIC7906_PCIe_DEVICE_ID)},
#endif
#ifdef MT7981
	{PCI_DEVICE(MT7981_PCI_VENDOR_ID, NIC7981_PCIe_DEVICE_ID)},
	{PCI_DEVICE(MT7981_PCI_VENDOR_ID, NIC7981_FPGA_PCIe_DEVICE_ID)},
#endif
	{} /* terminate list */
};

MODULE_DEVICE_TABLE(pci, rt_pci_tbl);

static struct pci_device_id mt_pci_slave_tbl[] DEVINITDATA = {
#ifdef MT7916
	{PCI_DEVICE(MT790A_PCI_VENDOR_ID, NIC790A_PCIe_DEVICE_ID)},
#endif
	{} /*temimate list*/
};

MODULE_DEVICE_TABLE(pci, mt_pci_slave_tbl);

/** @endcode */
/** @} */

/**
 * @addtogroup hif HIF operation
 * @{
 * @name PCIe initialzation
 * @{
 */

/***************************************************************************
 *
 *	PCI device initialization related procedures.
 *
 ***************************************************************************/
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
#ifdef CONFIG_PM

#if (KERNEL_VERSION(2, 6, 10) >= LINUX_VERSION_CODE)
#define pm_message_t u32
#endif

static int rt_pci_suspend(struct pci_dev *pci_dev, pm_message_t state)
{
	struct pci_hif_chip *chip_hif = pci_get_drvdata(pci_dev);
	struct net_device *net_dev = chip_hif->hif->net_dev;
	VOID *pAd = NULL;
	INT32 retval = 0;

	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "===>%s()\n", __func__);

	if (net_dev == NULL)
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "net_dev == NULL!\n");
	else {
		ULONG IfNum = 0;

		GET_PAD_FROM_NET_DEV(pAd, net_dev);
		/* we can not use IFF_UP because ra0 down but ra1 up */
		/* and 1 suspend/resume function for 1 module, not for each interface */
		/* so Linux will call suspend/resume function once */
		RTMP_DRIVER_VIRTUAL_INF_NUM_GET(pAd, &IfNum);

		if (IfNum > 0) {
			/* avoid users do suspend after interface is down */
			/* stop interface */
			netif_carrier_off(net_dev);
			netif_stop_queue(net_dev);
			/* mark device as removed from system and therefore no longer available */
			netif_device_detach(net_dev);
			RTMP_DRIVER_PCI_SUSPEND(pAd);
			RT_MOD_HNAT_DEREG(net_dev);
			RT_MOD_DEC_USE_COUNT();
		}
	}

#if (KERNEL_VERSION(2, 6, 10) < LINUX_VERSION_CODE)
	/*
	 *	reference to http://vovo2000.com/type-lab/linux/kernel-api/linux-kernel-api.html
	 *	enable device to generate PME# when suspended
	 *	pci_choose_state(): Choose the power state of a PCI device to be suspended
	 */
	retval = pci_enable_wake(pci_dev, pci_choose_state(pci_dev, state), 1);
	/* save the PCI configuration space of a device before suspending */
	pci_save_state(pci_dev);
	/* disable PCI device after use */
	pci_disable_device(pci_dev);
	retval = pci_set_power_state(pci_dev, pci_choose_state(pci_dev, state));
#endif
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "<===%s()\n", __func__);
	return retval;
}


static int rt_pci_resume(struct pci_dev *pci_dev)
{
	struct pci_hif_chip *chip_hif = pci_get_drvdata(pci_dev);
	struct net_device *net_dev = chip_hif->hif->net_dev;
	VOID *pAd = NULL;

#if (KERNEL_VERSION(2, 6, 10) < LINUX_VERSION_CODE)
	INT32 retval;
	/*  Set the power state of a PCI device
	 *
	 *	PCI has 4 power states, DO (normal) ~ D3(less power)
	 *	you can find that in include/linux/pci.h
	 *	#define PCI_D0          ((pci_power_t __force) 0)
	 *	#define PCI_D1          ((pci_power_t __force) 1)
	 *	#define PCI_D2          ((pci_power_t __force) 2)
	 *	#define PCI_D3hot       ((pci_power_t __force) 3)
	 *	#define PCI_D3cold      ((pci_power_t __force) 4)
	 *	#define PCI_UNKNOWN     ((pci_power_t __force) 5)
	 *	#define PCI_POWER_ERROR ((pci_power_t __force) -1)
	 */
	retval = pci_set_power_state(pci_dev, PCI_D0);
	/* restore the saved state of a PCI device */
	pci_restore_state(pci_dev);

	/* initialize device before it's used by a driver */
	if (pci_enable_device(pci_dev)) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "pci enable fail!\n");
		return 0;
	}

#endif
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "===>%s()\n", __func__);

	if (net_dev == NULL)
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "net_dev == NULL!\n");
	else
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd != NULL) {
		ULONG IfNum = 0;
		/*
		 *	we can not use IFF_UP because ra0 down but ra1 up
		 *	and 1 suspend/resume function for 1 module, not for each interface
		 *	so Linux will call suspend/resume function once
		 */
		RTMP_DRIVER_VIRTUAL_INF_NUM_GET(pAd, &IfNum);

		if (IfNum > 0)
			/*		if (VIRTUAL_IF_NUM(pAd) > 0) */
		{
			/* mark device as attached from system and restart if needed */
			netif_device_attach(net_dev);
			/* increase MODULE use count */
			RT_MOD_INC_USE_COUNT();
			RT_MOD_HNAT_REG(net_dev);
			/*			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS); */
			/*			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF); */
			RTMP_DRIVER_PCI_RESUME(pAd);
			netif_start_queue(net_dev);
			netif_carrier_on(net_dev);
			netif_wake_queue(net_dev);
		}
	}

	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "<=== %s()\n", __func__);
	return 0;
}
#endif /* CONFIG_PM */
#endif

/*
*
*/
static VOID mt_pci_dev_free(struct pci_dev *pdev, unsigned long csr_addr)
{
	iounmap((void *)(csr_addr));
	pci_release_regions(pdev);
	pci_disable_device(pdev);
}

/*
* pcie device related initial setting
*/
static int mt_pci_dev_set(struct pci_dev *pdev, unsigned long *csr_addr)
{
	int rv = 0;
	char *print_name;
#ifdef CONFIG_WIFI_MSI_SUPPORT
	int vector;
#endif

	/* wake up and enable device */
	rv = pci_enable_device(pdev);

	if (rv != 0) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 "Enable PCI device failed, errno=%d!\n", rv);
		return rv;
	}

#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	print_name = (char *)pci_name(pdev);
#else
	print_name = pdev->slot_name;
#endif /* LINUX_VERSION_CODE */

#if defined(PLATFORM_M_STB)
	if (IS_ENABLED(CONFIG_ARM64) && (!IS_ENABLED(CONFIG_ZONE_DMA))) {
		if ((rv = pci_set_dma_mask(pdev, DMA_BIT_MASK(64))) != 0) {
			MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					 "set DMA mask failed!errno=%d\n", rv);
			return rv;
		}

		if ((rv = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64))) != 0) {
			MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					 "set DMA consistent mask failed!errno=%d\n", rv);
			return rv;
		}
	}
	else
#endif
	if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(32))) {
		/*
		 * pci_set_consistent_dma_mask() will always be able to set the same
		 * or a smaller mask as pci_set_dma_mask()
		 */
		pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
	} else {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 "set DMA mask failed!errno=%d\n", rv);
		goto err_out;
	}

	/*msi request*/
#ifdef CONFIG_WIFI_MSI_SUPPORT
#ifndef MT7916
	if (RtmpOsPciMsiEnable(pdev) != 0) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
		 "Request MSI resource failed, use INTx!\n");
	}
#else
#if (KERNEL_VERSION(4, 8, 0) <= LINUX_VERSION_CODE)
	vector = pci_alloc_irq_vectors(pdev, 8, 8, PCI_IRQ_MSI);
#else
	vector = pci_enable_msi_range(pdev, 8, 8);
#endif
	if (vector < 8)
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
		 "Request MSI resource failed, use INTx!\n");
	else
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO,
		 "MSI vector = %d, pdev->msi_enabled = %d, pdev->irq = %d\n",
		 vector,  pdev->msi_enabled, pdev->irq);
#endif
#endif /*CONFIG_WIFI_MSI_SUPPORT*/

	rv = pci_request_regions(pdev, print_name);

	if (rv != 0) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 "Request PCI resource failed, errno=%d!\n", rv);
		goto err_out;
	}

	/* map physical address to virtual address for accessing register */
	*csr_addr = (unsigned long) ioremap(pci_resource_start(pdev, 0), pci_resource_len(pdev, 0));

	if (!*csr_addr) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 "ioremap failed for device %s, region 0x%lX @ 0x%lX\n",
				  print_name, (ULONG)pci_resource_len(pdev, 0),
				  (ULONG)pci_resource_start(pdev, 0));
		goto err_out_free_res;
	} else {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO,
				 "%s: at 0x%lx, VA 0x%lx, IRQ %d.\n",	print_name,
				  (ULONG)pci_resource_start(pdev, 0), (ULONG)*csr_addr, pdev->irq);
	}

	/* Set DMA master */
	pci_set_master(pdev);

	return rv;

err_out_free_res:
	pci_release_regions(pdev);
err_out:
	pci_disable_device(pdev);
	return -ENODEV;
}
#ifdef INTERFACE_SPEED_DETECT
/* TODO: Need to update if we supports higher version PCIE than PCIE V6 */
const UINT32 pci_base_speed[] = {2000, 4000, 7876, 15753, 31507, 63015};
#define PCI_BASE_SPEED_NUM ARRAY_SIZE(pci_base_speed)

UINT pci_iface_spd_detect(struct pci_dev *pdev)
{
	UINT i = 0, ret = 0;
	UINT speed = 0, pci_width = 0, pci_linkspd = 0;
	UINT16 linksta = 0;

	ret = pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &linksta);
	if (ret != 0)
		return 0;

	pci_linkspd = linksta&PCI_EXP_LNKSTA_CLS;
	pci_width = (linksta&PCI_EXP_LNKSTA_NLW)>>PCI_EXP_LNKSTA_NLW_SHIFT;

	for (i = 0; i < PCI_BASE_SPEED_NUM; i++) {
		if ((pci_linkspd & (1<<i)) != 0) {
			speed = pci_width * pci_base_speed[i];
			break;
		}
	}
	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_DEBUG,
		"%s(): pci_width: %08x, pci_linkspd: %08x, speed = %d mb/s\n",
		__func__, pci_width, pci_linkspd, speed);
	return speed;
}
#endif /* INTERFACE_SPEED_DETECT */


/*
 *	 PCI device probe & initialization function
 */
#ifdef WIFI_MD_COEX_SUPPORT
static UCHAR active_pci_num;
UCHAR mt_get_active_pci_num(void)
{
	return active_pci_num;
}
#endif
static int DEVINIT rt_pci_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id)
{
	struct _RTMP_ADAPTER *pAd = NULL, *handle;
	struct net_device *net_dev;
	unsigned long csr_addr;
	int rv = 0;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	UINT32 OpMode = 0;
	RT_CMD_PCIE_INIT pci_config;
	struct pci_hif_chip *hif_chip = NULL;
	struct pci_hif_chip_cfg cfg;
	struct _PCI_HIF_T *pci_hif;

	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "===> %s(): device: %x\n", __func__, pci_id->device);
	rv = mt_pci_dev_set(pdev, &csr_addr);
	if (rv != 0) {
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "failed with rv = %d!\n", rv);
		return rv;
	}

	/* Allocate RTMP_ADAPTER adapter structure */
	os_alloc_mem(NULL, (UCHAR **)&handle, sizeof(struct os_cookie));

	if (handle == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 "Allocate memory for os handle failed!\n");
		goto err_out_iounmap;
	}

	memset(handle, 0, sizeof(struct os_cookie));
	((POS_COOKIE)handle)->pci_dev = pdev;
	((POS_COOKIE)handle)->pDev = &(pdev->dev);

	/* we may need the pdev for allocate structure of "RTMP_ADAPTER" */
	rv = RTMPAllocAdapterBlock(handle, (VOID **)&pAd, RTMP_DEV_INF_PCIE);

	if (rv != NDIS_STATUS_SUCCESS)
		goto err_out_iounmap;

#ifdef WIFI_MD_COEX_SUPPORT
	active_pci_num++;
#endif

	RTMP_DRIVER_PCI_CSR_SET(pAd, csr_addr);
	RTMP_DRIVER_PCIE_INIT(pAd, &pci_config, pdev);

	if (pci_config.pci_init_succeed != TRUE)
		goto err_out_free_radev;

	/*NetDevInit============================================== */
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);

	if (net_dev == NULL)
		goto err_out_free_radev;

	/* Here are the net_device structure with pci-bus specific parameters. */
	net_dev->irq = pdev->irq;
	net_dev->base_addr = csr_addr;
	/* +++Add by shiang for jeffrey debug */
#ifdef LINUX
	net_dev->features &= (~NETIF_F_HIGHDMA);
#endif /* LINUX */
	/* ---Add by shiang for jeffrey debug */
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	/* for supporting Network Manager */
	/*
	 *	Set the sysfs physical device reference for the network logical device
	 *	if set prior to registration will cause a symlink during initialization.
	 */
#if (KERNEL_VERSION(2, 6, 0) <= LINUX_VERSION_CODE)
	SET_NETDEV_DEV(net_dev, &(pdev->dev));
#endif
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

	/*prepare pci_hif_chip*/
	cfg.csr_addr = csr_addr;
#ifdef CONFIG_WIFI_MSI_SUPPORT
	cfg.msi_en = pdev->msi_enabled;
#endif /*CONFIG_WIFI_MSI_SUPPORT*/
	cfg.device = &pdev->dev;
	cfg.device_id = pci_id->device;
	cfg.irq = pdev->irq;
#ifdef INTERFACE_SPEED_DETECT
	cfg.IfaceSpeed = pci_iface_spd_detect(pdev);
#endif
	pci_hif_chip_init((VOID **)&hif_chip, &cfg);
	pci_set_drvdata(pdev, hif_chip);
	pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	pci_hif->main_hif_chip = hif_chip;
	pci_hif->net_dev = net_dev;
	hif_chip->hif = pci_hif;
#ifdef CONFIG_WIFI_MSI_SUPPORT
	hif_chip->is_main = TRUE;
#endif /*CONFIG_WIFI_MSI_SUPPORT*/

	RTMP_DRIVER_CHIP_PREPARE(pAd);
	/*All done, it's time to register the net device to linux kernel. */
	/* Register this device */
	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
	rv = RtmpOSNetDevAttach(OpMode, net_dev, &netDevHook);

	if (rv)
		goto err_out_free_netdev;

#ifdef CONFIG_STA_SUPPORT
	/*	pAd->StaCfg[0].OriDevType = net_dev->type; */
	/* RTMP_DRIVER_STA_DEV_TYPE_SET(pAd, net_dev->type); */
#endif /* CONFIG_STA_SUPPORT */
#ifdef PRE_ASSIGN_MAC_ADDR
	{
		UCHAR mac_addr[MAC_ADDR_LEN];

		RTMP_DRIVER_MAC_ADDR_GET(pAd, &mac_addr[0]);
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO,
				 "@%s MAC address: "MACSTR"\n",
				  __func__, MAC2STR(mac_addr));
		/* Set up the Mac address */
		RtmpOSNetDevAddrSet(OpMode, net_dev, &mac_addr[0], NULL);
	}
#endif /* PRE_ASSIGN_MAC_ADDR */
#ifdef WIFI_DIAG
	diag_ctrl_alloc(pAd);
#endif
#ifdef COEX_DIRECT_PATH
	pAd->fw2apccci_msg.pci_base_addr = (unsigned int)pci_resource_start(pdev, 0);
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "pci_resource_start = 0x%x \n",
			pAd->fw2apccci_msg.pci_base_addr);
#endif
#ifdef MTK_FE_RESET_RECOVER
	mtk_fe_reset_notifier_init(pAd);
#endif
#ifdef WF_RESET_SUPPORT
	rv = wf_reset_init(pAd);
	if (!rv)
		goto err_out_free_netdev;
#endif /* WF_RESET_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "<=%s()\n", __func__);
	return 0; /* probe ok */
	/* --------------------------- ERROR HANDLE --------------------------- */
err_out_free_netdev:
	RtmpOSNetDevFree(net_dev);
err_out_free_radev:
	/* free RTMP_ADAPTER strcuture and os_cookie*/
	RTMPFreeAdapter(pAd);
err_out_iounmap:
	mt_pci_dev_free(pdev, csr_addr);
	return -ENODEV; /* probe fail */
}


static VOID DEVEXIT rt_pci_remove(struct pci_dev *pci_dev)
{
	struct pci_hif_chip *chip_hif = pci_get_drvdata(pci_dev);
	PNET_DEV net_dev = chip_hif->hif->net_dev;
	RTMP_ADAPTER *pAd = NULL;
	ULONG csr_addr = net_dev->base_addr;
#ifdef DBDC_ONE_BAND1_SUPPORT
	struct _RTMP_CHIP_CAP *cap;
	BOOLEAN DbdcOneBand1Support = FALSE;
#endif
	GET_PAD_FROM_NET_DEV(pAd, net_dev);
	MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "===> %s()\n", __func__);

#ifdef DBDC_ONE_BAND1_SUPPORT
	if (pAd != NULL) {
		cap = hc_get_chip_cap(pAd->hdev_ctrl);
		DbdcOneBand1Support = cap->DbdcOneBand1Support;
	}
#endif
	if (pAd != NULL) {
#ifdef WF_RESET_SUPPORT
		wf_reset_exit(pAd);
#endif /* WF_RESET_SUPPORT */
#ifdef MTK_FE_RESET_RECOVER
		mtk_fe_reset_notifier_exit(pAd);
#endif
#ifdef WIFI_MD_COEX_SUPPORT
		active_pci_num--;
#endif
#ifdef WIFI_DIAG
		diag_ctrl_free(pAd);
#endif
		/* Unregister/Free all allocated net_device. */
		RtmpPhyNetDevExit(pAd, net_dev);
		/* Free RTMP_ADAPTER related structures. */
		RtmpRaDevCtrlExit(pAd);
		/* Unmap CSR base address */
		iounmap((char *)(csr_addr));
		/* release memory region */
		pci_release_regions(pci_dev);
	} else {
		/* Unregister network device */
		RtmpOSNetDevDetach(net_dev);
		/* Unmap CSR base address */
		iounmap((char *)(net_dev->base_addr));
		/* release memory region */
		pci_release_regions(pci_dev);
	}
#ifdef CONFIG_WIFI_MSI_SUPPORT
	if (pci_dev->msi_enabled)
		RtmpOsPciMsiDisable(pci_dev);
#endif /*CONFIG_WIFI_MSI_SUPPORT*/
#ifdef DBDC_ONE_BAND1_SUPPORT
	/* main netdev freed in MBSS_REMOVE */
	if (!DbdcOneBand1Support)
#endif
		/* Free the root net_device */
		RtmpOSNetDevFree(net_dev);
	/*chip_hif free*/
	pci_hif_chip_exit(chip_hif);
}

/*
 *	 PCI device probe & initialization function
 */
static int DEVINIT rt_pci_slave_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id)
{
	int rv;
	unsigned long csr_addr;
	struct pci_hif_chip_cfg cfg;
	struct pci_hif_chip *hif_chip = NULL;

	rv = mt_pci_dev_set(pdev, &csr_addr);
	if (rv != 0) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "failed with rv = %d!\n", rv);
		return rv;
	}
		/*prepare pci_hif_chip*/
	cfg.csr_addr = csr_addr;
#ifdef CONFIG_WIFI_MSI_SUPPORT
	cfg.msi_en = pdev->msi_enabled;
#endif /*CONFIG_WIFI_MSI_SUPPORT*/
	cfg.device = &pdev->dev;
	cfg.device_id = pci_id->device;
	cfg.irq = pdev->irq;
#ifdef INTERFACE_SPEED_DETECT
	cfg.IfaceSpeed = pci_iface_spd_detect(pdev);
#endif
	pci_hif_chip_init((VOID **)&hif_chip, &cfg);
	pci_set_drvdata(pdev, hif_chip);
	return 0;
}

/*
*
*/
static VOID DEVEXIT mt_pci_slave_remove(struct pci_dev *pci_dev)
{
	struct pci_hif_chip *chip_hif = pci_get_drvdata(pci_dev);

#ifdef CONFIG_WIFI_MSI_SUPPORT
	if (pci_dev->msi_enabled)
		RtmpOsPciMsiDisable(pci_dev);
#endif /*CONFIG_WIFI_MSI_SUPPORT*/
	mt_pci_dev_free(pci_dev, (unsigned long) chip_hif->CSRBaseAddress);
	pci_hif_chip_exit(chip_hif);
}


/*
 *	Our PCI driver structure
 */
static struct pci_driver rt_pci_driver = {
name:
	RTMP_DRV_NAME,
id_table :
	rt_pci_tbl,
probe :
	rt_pci_probe,
#if LINUX_VERSION_CODE >= 0x20412
remove :
	DEVEXIT_P(rt_pci_remove),
#else
remove :
	__devexit(rt_pci_remove),
#endif

#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
#ifdef CONFIG_PM
suspend :
	rt_pci_suspend,
resume :
	rt_pci_resume,
#endif
#endif
};

/*
* Slave PCIE driver
*/
static struct pci_driver mt_pci_slave_driver = {
name:
	"mt_pci_slave",
id_table :
	mt_pci_slave_tbl,
probe :
	rt_pci_slave_probe,
#if LINUX_VERSION_CODE >= 0x20412
remove :
	DEVEXIT_P(mt_pci_slave_remove),
#else
remove :
	__devexit(mt_pci_slave_remove),
#endif

#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
#ifdef CONFIG_PM
suspend :
	rt_pci_suspend,
resume :
	rt_pci_resume,
#endif
#endif
};

/*
 *	Driver module load/unload function
 */
int __init rt_pci_init_module(void)
{
	int ret;
	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "register %s\n", RTMP_DRV_NAME);
	/*
	 *	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_DEBUG, "DriverVersion: 2.7.0.2-Beta-121007\n"
	 *								"\tBBP:120824\n"
	 *								"\tRF :120813\n");
	 */
#ifndef MULTI_INF_SUPPORT
	os_module_init();
#endif
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	ret = pci_register_driver(&mt_pci_slave_driver);
	return pci_register_driver(&rt_pci_driver);
#else
	ret = pci_module_init(&mt_pci_slave_driver);
	return pci_module_init(&rt_pci_driver);
#endif
}

void __exit rt_pci_cleanup_module(void)
{
	pci_unregister_driver(&mt_pci_slave_driver);
	pci_unregister_driver(&rt_pci_driver);
 #ifndef MULTI_INF_SUPPORT
	os_module_exit();
#endif
}

#ifdef MT7622_FPGA
late_initcall(rt_pci_init_module);
#endif /*MT7622_FPGA*/

/** @} */
/** @} */
#ifndef MULTI_INF_SUPPORT
module_init(rt_pci_init_module);
module_exit(rt_pci_cleanup_module);

#ifdef CONFIG_STA_SUPPORT
#ifdef MODULE_VERSION
MODULE_VERSION(STA_DRIVER_VERSION);
#endif
#endif /* CONFIG_STA_SUPPORT */
#endif /* MULTI_INF_SUPPORT */
