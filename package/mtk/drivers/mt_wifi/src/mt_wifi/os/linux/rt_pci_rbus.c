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
    rt_pci_rbus.c

    Abstract:
    Create and register network interface.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#define RTMP_MODULE_OS

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <linux/pci.h>


IRQ_HANDLE_TYPE
#if (KERNEL_VERSION(2, 6, 19) <= LINUX_VERSION_CODE)
rt2860_interrupt(int irq, void *dev_instance);
#else
rt2860_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
#endif

IRQ_HANDLE_TYPE
#if (KERNEL_VERSION(2, 6, 19) <= LINUX_VERSION_CODE)
rt2860_interrupt(int irq, void *dev_instance)
#else
rt2860_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
#ifdef CONFIG_WIFI_MSI_SUPPORT
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)dev_instance;
	if (pci_hif_chip->is_msi) {
		pci_handle_msi_irq(irq, dev_instance);
	} else
#endif
	pci_handle_irq(dev_instance);
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	return  IRQ_HANDLED;
#endif
}

#ifdef MULTI_INTR_SUPPORT
IRQ_HANDLE_TYPE
#if (KERNEL_VERSION(2, 6, 19) <= LINUX_VERSION_CODE)
rt2860_multi_interrupt(int irq, void *dev_instance);
#else
rt2860_multi_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
#endif

IRQ_HANDLE_TYPE
#if (KERNEL_VERSION(2, 6, 19) <= LINUX_VERSION_CODE)
rt2860_multi_interrupt(int irq, void *dev_instance)
#else
rt2860_multi_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	pci_handle_multi_irq(dev_instance);
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	return  IRQ_HANDLED;
#endif
}

IRQ_HANDLE_TYPE
#if (KERNEL_VERSION(2, 6, 19) <= LINUX_VERSION_CODE)
rt2860_multi_interrupt_2nd(int irq, void *dev_instance);
#else
rt2860_multi_interrupt_2nd(int irq, void *dev_instance, struct pt_regs *regs);
#endif

IRQ_HANDLE_TYPE
#if (KERNEL_VERSION(2, 6, 19) <= LINUX_VERSION_CODE)
rt2860_multi_interrupt_2nd(int irq, void *dev_instance)
#else
rt2860_multi_interrupt_2nd(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	pci_handle_multi_irq_2nd(dev_instance);
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	return  IRQ_HANDLED;
#endif
}

IRQ_HANDLE_TYPE
#if (KERNEL_VERSION(2, 6, 19) <= LINUX_VERSION_CODE)
rt2860_multi_interrupt_3rd(int irq, void *dev_instance);
#else
rt2860_multi_interrupt_3rd(int irq, void *dev_instance, struct pt_regs *regs);
#endif

IRQ_HANDLE_TYPE
#if (KERNEL_VERSION(2, 6, 19) <= LINUX_VERSION_CODE)
rt2860_multi_interrupt_3rd(int irq, void *dev_instance)
#else
rt2860_multi_interrupt_3rd(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	pci_handle_multi_irq_3rd(dev_instance);
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	return  IRQ_HANDLED;
#endif
}

IRQ_HANDLE_TYPE
#if (KERNEL_VERSION(2, 6, 19) <= LINUX_VERSION_CODE)
rt2860_multi_interrupt_4th(int irq, void *dev_instance);
#else
rt2860_multi_interrupt_4th(int irq, void *dev_instance, struct pt_regs *regs);
#endif

IRQ_HANDLE_TYPE
#if (KERNEL_VERSION(2, 6, 19) <= LINUX_VERSION_CODE)
rt2860_multi_interrupt_4th(int irq, void *dev_instance)
#else
rt2860_multi_interrupt_4th(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	pci_handle_multi_irq_4th(dev_instance);
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	return  IRQ_HANDLED;
#endif
}
#endif

