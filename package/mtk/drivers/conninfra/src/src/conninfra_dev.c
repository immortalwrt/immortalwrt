/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */
/*! \file
*    \brief  Declaration of library functions
*
*    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/

#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include "conninfra.h"
#include "conninfra_core.h"
#include "consys_hw.h"
#include "emi_mng.h"

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

#define CONNINFRA_DEV_MAJOR 164
#define CONNINFRA_DEV_NUM 1
#define CONNINFRA_DRVIER_NAME "conninfra_drv"
#define CONNINFRA_DEVICE_NAME "conninfra_dev"

#define CONNINFRA_DEV_IOC_MAGIC 0xc2
#define CONNINFRA_IOCTL_GET_CHIP_ID _IOR(CONNINFRA_DEV_IOC_MAGIC, 0, int)

#define CONNINFRA_DEV_INIT_TO_MS (2 * 1000)

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

enum conninfra_init_status {
	CONNINFRA_INIT_NOT_START,
	CONNINFRA_INIT_START,
	CONNINFRA_INIT_DONE,
};
static int g_conninfra_init_status = CONNINFRA_INIT_NOT_START;
static wait_queue_head_t g_conninfra_init_wq;

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static int conninfra_dev_open(struct inode *inode, struct file *file);
static int conninfra_dev_close(struct inode *inode, struct file *file);
static ssize_t conninfra_dev_read(struct file *filp, char __user *buf,
				size_t count, loff_t *f_pos);
static ssize_t conninfra_dev_write(struct file *filp,
				const char __user *buf, size_t count,
				loff_t *f_pos);
static long conninfra_dev_unlocked_ioctl(
		struct file *filp, unsigned int cmd, unsigned long arg);
#ifdef CONFIG_COMPAT
static long conninfra_dev_compat_ioctl(
		struct file *filp, unsigned int cmd, unsigned long arg);
#endif /* CONFIG_COMPAT */

static int conninfra_dev_suspend_cb(void);
static int conninfra_dev_resume_cb(void);
static int conninfra_dev_pmic_event_cb(unsigned int, unsigned int);
/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

struct class *pConninfraClass;
struct device *pConninfraDev;
static struct cdev gConninfraCdev;

const struct file_operations gConninfraDevFops = {
	.open = conninfra_dev_open,
	.release = conninfra_dev_close,
	.read = conninfra_dev_read,
	.write = conninfra_dev_write,
	.unlocked_ioctl = conninfra_dev_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = conninfra_dev_compat_ioctl,
#endif /* CONFIG_COMPAT */
};

static int gConnInfraMajor = CONNINFRA_DEV_MAJOR;

/* screen on/off notification */

static struct conninfra_dev_cb gConninfraDevCb = {
	.conninfra_suspend_cb = conninfra_dev_suspend_cb,
	.conninfra_resume_cb = conninfra_dev_resume_cb,
	.conninfra_pmic_event_notifier = conninfra_dev_pmic_event_cb,
};

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

int conninfra_dev_open(struct inode *inode, struct file *file)
{
	static DEFINE_RATELIMIT_STATE(_rs, HZ, 1);

	if (!wait_event_timeout(g_conninfra_init_wq, g_conninfra_init_status == CONNINFRA_INIT_DONE,
							msecs_to_jiffies(CONNINFRA_DEV_INIT_TO_MS))) {
		if (__ratelimit(&_rs)) {
			pr_warn("wait_event_timeout (%d)ms,(%lu)jiffies,return -EIO\n",
			        CONNINFRA_DEV_INIT_TO_MS, msecs_to_jiffies(CONNINFRA_DEV_INIT_TO_MS));
		}
		return -EIO;
	}

	pr_info("open major %d minor %d (pid %d)\n",
			imajor(inode), iminor(inode), current->pid);

	return 0;
}

int conninfra_dev_close(struct inode *inode, struct file *file)
{
	pr_info("close major %d minor %d (pid %d)\n",
			imajor(inode), iminor(inode), current->pid);

	return 0;
}

ssize_t conninfra_dev_read(struct file *filp, char __user *buf,
					size_t count, loff_t *f_pos)
{
	return 0;
}

ssize_t conninfra_dev_write(struct file *filp,
			const char __user *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

static long conninfra_dev_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int retval = 0;

	pr_info("[%s] cmd (%d),arg(%ld)\n", __func__, cmd, arg);
	switch (cmd) {
	case CONNINFRA_IOCTL_GET_CHIP_ID:
		retval = consys_hw_chipid_get();
		break;
	}
	return retval;

}

#ifdef CONFIG_COMPAT
static long conninfra_dev_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret;

	pr_info("[%s] cmd (%d)\n", __func__, cmd);
	ret = conninfra_dev_unlocked_ioctl(filp, cmd, arg);
	return ret;
}
#endif /* CONFIG_COMPAT */

static int conninfra_dev_suspend_cb(void)
{
	return 0;
}

static int conninfra_dev_resume_cb(void)
{
	conninfra_core_dump_power_state();
	return 0;
}

static int conninfra_dev_pmic_event_cb(unsigned int id, unsigned int event)
{
	conninfra_core_pmic_event_cb(id, event);

	return 0;
}

/************************************************************************/

static int conninfra_dev_init(void)
{
	dev_t devID = MKDEV(gConnInfraMajor, 0);
	int cdevErr = -1;
	int iret = 0;

	g_conninfra_init_status = CONNINFRA_INIT_START;
	init_waitqueue_head((wait_queue_head_t *)&g_conninfra_init_wq);

	iret = register_chrdev_region(devID, CONNINFRA_DEV_NUM, CONNINFRA_DRVIER_NAME);
	if (iret) {
		pr_err("fail to register chrdev\n");
		g_conninfra_init_status = CONNINFRA_INIT_NOT_START;
		return -1;
	}

	cdev_init(&gConninfraCdev, &gConninfraDevFops);
	gConninfraCdev.owner = THIS_MODULE;

	cdevErr = cdev_add(&gConninfraCdev, devID, CONNINFRA_DEV_NUM);
	if (cdevErr) {
		pr_err("cdev_add() fails (%d)\n", cdevErr);
		goto err1;
	}

	pConninfraClass = class_create(THIS_MODULE, CONNINFRA_DEVICE_NAME);
	if (IS_ERR(pConninfraClass)) {
		pr_err("class create fail, error code(%ld)\n", PTR_ERR(pConninfraClass));
		goto err1;
	}

	pConninfraDev = device_create(pConninfraClass, NULL, devID, NULL, CONNINFRA_DEVICE_NAME);
	if (IS_ERR(pConninfraDev)) {
		pr_err("device create fail, error code(%ld)\n", PTR_ERR(pConninfraDev));
		goto err2;
	}

	iret = mtk_conninfra_drv_init(&gConninfraDevCb);
	if (iret) {
		pr_err("init consys_hw fail, ret = %d\n", iret);
		g_conninfra_init_status = CONNINFRA_INIT_NOT_START;
		return -2;
	}

	iret = conninfra_core_init();
	if (iret) {
		pr_err("conninfra init fail\n");
		g_conninfra_init_status = CONNINFRA_INIT_NOT_START;
		return -3;
	}

	pr_info("ConnInfra Dev: init (%d)\n", iret);
	g_conninfra_init_status = CONNINFRA_INIT_DONE;

#ifdef CONFIG_CONNINFRA_AUTO_UP
	iret = conninfra_core_power_on(CONNDRV_TYPE_CONNINFRA);
	if (iret) {
		pr_err("conninfra auto load power on fail\n");
		return -4;
	}
#endif /* CONFIG_CONNINFRA_AUTO_UP */

	return 0;

err2:

	pr_err("[conninfra_dev_init] err2\n");
	if (pConninfraClass) {
		class_destroy(pConninfraClass);
		pConninfraClass = NULL;
	}
err1:
	pr_err("[conninfra_dev_init] err1\n");
	if (cdevErr == 0)
		cdev_del(&gConninfraCdev);

	if (iret == 0) {
		unregister_chrdev_region(devID, CONNINFRA_DEV_NUM);
		gConnInfraMajor = -1;
	}

	g_conninfra_init_status = CONNINFRA_INIT_NOT_START;
	return -2;
}

static void conninfra_dev_deinit(void)
{
	dev_t dev = MKDEV(gConnInfraMajor, 0);
	int iret = 0;

#ifdef CONFIG_CONNINFRA_AUTO_UP
	iret = conninfra_core_power_off(CONNDRV_TYPE_CONNINFRA);
	if (iret) {
		pr_err("conninfra auto load power off fail\n");
	}
#endif /* CONFIG_CONNINFRA_AUTO_UP */

	g_conninfra_init_status = CONNINFRA_INIT_NOT_START;

	iret = conninfra_core_deinit();

	iret = mtk_conninfra_drv_deinit();

	if (pConninfraDev) {
		device_destroy(pConninfraClass, dev);
		pConninfraDev = NULL;
	}

	if (pConninfraClass) {
		class_destroy(pConninfraClass);
		pConninfraClass = NULL;
	}

	cdev_del(&gConninfraCdev);
	unregister_chrdev_region(dev, CONNINFRA_DEV_NUM);

	pr_info("ConnInfra: platform init (%d)\n", iret);
}

module_init(conninfra_dev_init);
module_exit(conninfra_dev_deinit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Willy.Yu @ CTD/SE5/CS5");

module_param(gConnInfraMajor, uint, 0644);

