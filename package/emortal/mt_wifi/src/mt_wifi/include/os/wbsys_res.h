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

	Module Name: mt_wifi
	wbsys_of.h
*/

#ifndef _WBSYS_RES_H
#define _WBSYS_RES_H

#include "rtmp_chip.h"

/*platform device & platform driver match name*/
static char wbsys_string[] = "wb_sys";

#if defined(CONFIG_OF)
#include <linux/of_irq.h>
#include <linux/of_address.h>

static const struct of_device_id wbsys_of_ids[] = {
	{   .compatible = OF_WBSYS_NAME, },
	{ },
};

#define wbsys_dev_alloc(res)
#define wbsys_dev_release(res)

#else
static struct resource wbsys_res[] = {
	[0] = {
		.start = (RTMP_MAC_CSR_ADDR),
		.end = (RTMP_MAC_CSR_ADDR + RTMP_MAC_CSR_LEN-1),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = RTMP_IRQ_NUM,
		.end = RTMP_IRQ_NUM,
		.flags = IORESOURCE_IRQ,
	}
};

static void platform_wbsys_release(struct device *dev)
{
	return;
}

struct platform_device wbsys_dev = {
	.name = wbsys_string,
	.id = -1,
	.num_resources	= ARRAY_SIZE(wbsys_res),
	.resource		= wbsys_res,
	.dev = {
		.release = platform_wbsys_release,
	}
};
#define wbsys_dev_alloc(dev)\
	{\
		platform_device_register(dev);\
	}

#define wbsys_dev_release(dev)\
	{\
		platform_device_unregister(dev);\
	}
#endif /*CONFIG_OF*/

#endif /*_WBSYS_RES_H*/
