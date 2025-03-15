#ifndef __RTL_ADAPTER_LOAD__
#define __RTL_ADAPTER_LOAD__

#ifdef __KERNEL__
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/perf_event.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/ioport.h>
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


#define uint32 uint32_t
#define uint16 uint16_t
#define uint8 uint8_t
#ifdef __KERNEL__
#define osal_time_udelay(aa) msleep((aa)/1000)
#define phy_osal_printf printk
#define osal_printf printk
#else
#define osal_time_udelay(aa) usleep(aa)
#define phy_osal_printf printf
#define osal_printf printf
#endif
#endif