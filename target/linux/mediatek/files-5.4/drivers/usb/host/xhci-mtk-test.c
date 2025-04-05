// SPDX-License-Identifier: GPL-2.0
/*
 * xHCI host controller toolkit driver
 *
 * Copyright (C) 2021  MediaTek Inc.
 *
 *  Author: Zhanyong Wang <zhanyong.wang@mediatek.com>
 *          Shaocheng.Wang <shaocheng.wang@mediatek.com>
 *          Chunfeng.Yun <chunfeng.yun@mediatek.com>
 */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/kobject.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <dt-bindings/phy/phy.h>
#include "../core/usb.h"
#include "xhci-mtk.h"
#include "xhci-mtk-test.h"
#include "xhci-mtk-unusual.h"

static int t_test_j(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_k(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_se0(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_packet(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_suspend(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_resume(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_get_device_descriptor(struct xhci_hcd_mtk *mtk,
				int argc, char **argv);
static int t_test_enumerate_bus(struct xhci_hcd_mtk *mtk,
				int argc, char **argv);
static int t_debug_port(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_power_u1u2(struct xhci_hcd_mtk *mtk, int argc, char **argv);

#define PORT_PLS_VALUE(p) ((p >> 5) & 0xf)
/* ip_xhci_cap register */
#define CAP_U3_PORT_NUM(p)	((p) & 0xff)
#define CAP_U2_PORT_NUM(p)	(((p) >> 8) & 0xff)

#define MAX_NAME_SIZE 32
#define MAX_ARG_SIZE 4

struct class_info {
        int class;
        char *class_name;
};

static const struct class_info clas_info[] = {
        /* max. 5 chars. per name string */
        {USB_CLASS_PER_INTERFACE,       ">ifc"},
        {USB_CLASS_AUDIO,               "audio"},
        {USB_CLASS_COMM,                "comm."},
        {USB_CLASS_HID,                 "HID"},
        {USB_CLASS_PHYSICAL,            "PID"},
        {USB_CLASS_STILL_IMAGE,         "still"},
        {USB_CLASS_PRINTER,             "print"},
        {USB_CLASS_MASS_STORAGE,        "stor."},
        {USB_CLASS_HUB,                 "hub"},
        {USB_CLASS_CDC_DATA,            "data"},
        {USB_CLASS_CSCID,               "scard"},
        {USB_CLASS_CONTENT_SEC,         "c-sec"},
        {USB_CLASS_VIDEO,               "video"},
        {USB_CLASS_WIRELESS_CONTROLLER, "wlcon"},
        {USB_CLASS_MISC,                "misc"},
        {USB_CLASS_APP_SPEC,            "app."},
        {USB_CLASS_VENDOR_SPEC,         "vend."},
        {-1,                            "unk."}         /* leave as last */
};

struct hqa_test_cmd {
	char name[MAX_NAME_SIZE];
	int (*cb_func)(struct xhci_hcd_mtk *mtk, int argc, char **argv);
	char *discription;
};

struct hqa_test_cmd xhci_mtk_hqa_cmds[] = {
	{"test.j", &t_test_j, "Test_J"},
	{"test.k", &t_test_k, "Test_K"},
	{"test.se0", &t_test_se0, "Test_SE0_NAK"},
	{"test.packet", &t_test_packet, "Test_PACKET"},
	{"test.suspend", &t_test_suspend, "Port Suspend"},
	{"test.resume", &t_test_resume, "Port Resume"},
	{"test.enumbus", &t_test_enumerate_bus, "Enumerate Bus"},
	{"test.getdesc", &t_test_get_device_descriptor,
				"Get Device Discriptor"},
	{"test.debug", &t_debug_port, "debug Port infor"},
	{"pm.u1u2", &t_power_u1u2, "Port U1,U2"},
	{"", NULL, ""},
};

static const char *class_decode(const int class)
{
        int i;

        for (i = 0; clas_info[i].class != -1; i++)
                if (clas_info[i].class == class)
                        break;
        return clas_info[i].class_name;
}

int call_hqa_func(struct xhci_hcd_mtk *mtk, char *buf)
{
	struct hqa_test_cmd *hqa;
	struct usb_hcd *hcd = mtk->hcd;
	char *argv[MAX_ARG_SIZE];
	int argc;
	int i;

	argc = 0;
	do {
		argv[argc] = strsep(&buf, " ");
		xhci_err(hcd_to_xhci(hcd), "[%d] %s\r\n", argc, argv[argc]);
		argc++;
	} while (buf);

	for (i = 0; i < ARRAY_SIZE(xhci_mtk_hqa_cmds); i++) {
		hqa = &xhci_mtk_hqa_cmds[i];
		if ((!strcmp(hqa->name, argv[0])) && (hqa->cb_func != NULL))
			return hqa->cb_func(mtk, argc, argv);
	}

	return -1;
}

static int test_mode_enter(struct xhci_hcd_mtk *mtk,
				u32 port_id, u32 test_value)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 __iomem *addr;
	u32 temp;

	if (mtk->test_mode == 0) {
		xhci_stop(hcd);
		xhci_halt(xhci);
	}

	addr = &xhci->op_regs->port_power_base +
			NUM_PORT_REGS * ((port_id - 1) & 0xff);
	temp = readl(addr);
	temp &= ~(0xf << 28);
	temp |= (test_value << 28);
	writel(temp, addr);
	mtk->test_mode = 1;

	return 0;
}

static int test_mode_exit(struct xhci_hcd_mtk *mtk)
{
	if (mtk->test_mode == 1)
		mtk->test_mode = 0;

	return 0;
}

static int t_test_j(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	long port_id;
	u32 test_value;

	port_id = 2;
	test_value = 1;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);
	test_mode_enter(mtk, port_id, test_value);

	return 0;
}

static int t_test_k(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	long port_id;
	u32 test_value;

	port_id = 2;
	test_value = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);
	test_mode_enter(mtk, port_id, test_value);

	return 0;
}

static int t_test_se0(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	long port_id;
	u32 test_value;

	port_id = 2;
	test_value = 3;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%ld\n", __func__, port_id);
	test_mode_enter(mtk, port_id, test_value);

	return 0;
}

static int t_test_packet(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	long port_id;
	u32 test_value;

	port_id = 2;
	test_value = 4;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%ld\n", __func__, port_id);
	test_mode_enter(mtk, port_id, test_value);

	return 0;
}

/* only for u3 ports, valid values are 1, 2, ...*/
static int t_power_u1u2(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 __iomem *addr;
	u32 temp;
	int port_id;
	int retval = 0;
	int u_num = 1;
	int u1_val = 1;
	int u2_val = 0;

	port_id = 1; /* first u3port by default */

	if (argc > 1 && kstrtoint(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	if (argc > 2 && kstrtoint(argv[2], 10, &u_num))
		xhci_err(xhci, "mu3h %s get u_num failed\n", __func__);

	if (argc > 3 && kstrtoint(argv[3], 10, &u1_val))
		xhci_err(xhci, "mu3h %s get u1_val failed\n", __func__);

	if (argc > 4 && kstrtoint(argv[4], 10, &u2_val))
		xhci_err(xhci, "mu3h %s get u2_val failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d, u_num%d, u1_val%d, u2_val%d\n",
		__func__, (int)port_id, u_num, u1_val, u2_val);

	if (mtk->test_mode == 1) {
		xhci_err(xhci, "please suspend port first\n");
		return -1;
	}

	xhci_err(xhci, "%s: stop port polling\n", __func__);
	clear_bit(HCD_FLAG_POLL_RH, &hcd->flags);
	del_timer_sync(&hcd->rh_timer);
	clear_bit(HCD_FLAG_POLL_RH, &xhci->shared_hcd->flags);
	del_timer_sync(&xhci->shared_hcd->rh_timer);
	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &xhci->shared_hcd->flags);

	addr = &xhci->op_regs->port_power_base +
			NUM_PORT_REGS * ((port_id - 1) & 0xff);

	temp = readl(addr);
	if (u_num == 1) {
		temp &= ~PORT_U1_TIMEOUT_MASK;
		temp |= PORT_U1_TIMEOUT(u1_val);
	} else if (u_num == 2) {
		temp &= ~PORT_U2_TIMEOUT_MASK;
		temp |= PORT_U2_TIMEOUT(u2_val);
	} else if (u_num == 3) {
		temp &= ~(PORT_U1_TIMEOUT_MASK | PORT_U2_TIMEOUT_MASK);
		temp |= PORT_U1_TIMEOUT(u1_val) | PORT_U2_TIMEOUT(u2_val);
	}

	writel(temp, addr);

	return retval;
}

static void show_string(struct usb_device *udev, char *id, char *string)
{
	if (!string)
		return;
	dev_info(&udev->dev, "%s: %s\n", id, string);
}

static void announce_device(struct usb_device *udev)
{
	u16 bcdDevice = le16_to_cpu(udev->descriptor.bcdDevice);

	dev_info(&udev->dev,
		"New USB device found, idVendor=%04x, idProduct=%04x, bcdDevice=%2x.%02x\n",
		le16_to_cpu(udev->descriptor.idVendor),
		le16_to_cpu(udev->descriptor.idProduct),
		bcdDevice >> 8, bcdDevice & 0xff);
	dev_info(&udev->dev,
		"New USB device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
		udev->descriptor.iManufacturer,
		udev->descriptor.iProduct,
		udev->descriptor.iSerialNumber);
	show_string(udev, "Product", udev->product);
	show_string(udev, "Manufacturer", udev->manufacturer);
	show_string(udev, "SerialNumber", udev->serial);
}

static int t_debug_port(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct usb_device *usb2_rh;
	struct usb_device *udev;
	long port_id;
        const struct usb_device_descriptor *desc;
        u16 bcdUSB;
        u16 bcdDevice;

	port_id = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);


	usb2_rh = hcd->self.root_hub;
	udev = usb_hub_find_child(usb2_rh, port_id - 1);
	if (udev == NULL) {
		xhci_err(xhci, "mu3h %s usb_hub_find_child(..., %i) failed\n", __func__, (int)port_id);
		return -EPERM;
	}

	dev_info(&udev->dev, "%s\n", usb_state_string(udev->state));
	if (udev && udev->state == USB_STATE_CONFIGURED) {
		announce_device(udev);
		desc = (const struct usb_device_descriptor *)&udev->descriptor;
                bcdUSB    = le16_to_cpu(desc->bcdUSB);
                bcdDevice = le16_to_cpu(desc->bcdDevice);

                dev_info(&udev->dev, "D:  Ver=%2x.%02x Cls=%02x(%-5s) Sub=%02x Prot=%02x MxPS=%2d #Cfgs=%3d\n",
                        bcdUSB >> 8, bcdUSB & 0xff,
                        desc->bDeviceClass,
                        class_decode(desc->bDeviceClass),
                        desc->bDeviceSubClass,
                        desc->bDeviceProtocol,
                        desc->bMaxPacketSize0,
                        desc->bNumConfigurations);

                dev_info(&udev->dev, "P:  Vendor=%04x ProdID=%04x Rev=%2x.%02x\n",
                        le16_to_cpu(desc->idVendor),
                        le16_to_cpu(desc->idProduct),
                        bcdDevice >> 8, bcdDevice & 0xff);
	}

	return 0;
}

static int t_test_suspend(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 __iomem *addr;
	u32 temp;
	long port_id;

	port_id = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);

	xhci_err(xhci, "%s: stop port polling\n", __func__);
	clear_bit(HCD_FLAG_POLL_RH, &hcd->flags);
	del_timer_sync(&hcd->rh_timer);
	clear_bit(HCD_FLAG_POLL_RH, &xhci->shared_hcd->flags);
	del_timer_sync(&xhci->shared_hcd->rh_timer);
	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &xhci->shared_hcd->flags);

	temp = readl(&xhci->ir_set->irq_pending);
	writel(ER_IRQ_DISABLE(temp), &xhci->ir_set->irq_pending);

	if (mtk->test_mode == 1)
		test_mode_exit(mtk);

	/* set PLS = 3 */
	addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS*((port_id - 1) & 0xff);

	temp = readl(addr);
	temp = xhci_port_state_to_neutral(temp);
	temp = (temp & ~(0xf << 5));
	temp = (temp | (3 << 5) | PORT_LINK_STROBE);
	writel(temp, addr);
	xhci_handshake(addr, (0xf << 5), (3 << 5), 30*1000);

	temp = readl(addr);
	if (PORT_PLS_VALUE(temp) != 3)
		xhci_err(xhci, "port not enter suspend state\n");
	else
		xhci_err(xhci, "port enter suspend state\n");

	return 0;
}

static int t_test_resume(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 __iomem *addr;
	u32 temp;
	long port_id;
	int retval = 0;

	port_id = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);

	if (mtk->test_mode == 1) {
		xhci_err(xhci, "please suspend port first\n");
		return -1;
	}
	addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS * ((port_id - 1) & 0xff);

	temp = readl(addr);
	if (PORT_PLS_VALUE(temp) != 3) {
		xhci_err(xhci, "port not in suspend state, please suspend port first\n");
		retval = -1;
	} else {
		temp = xhci_port_state_to_neutral(temp);
		temp = (temp & ~(0xf << 5));
		temp = (temp | (15 << 5) | PORT_LINK_STROBE);
		writel(temp, addr);
		mdelay(20);

		temp = readl(addr);
		temp = xhci_port_state_to_neutral(temp);
		temp = (temp & ~(0xf << 5));
		temp = (temp | PORT_LINK_STROBE);
		writel(temp, addr);

		xhci_handshake(addr, (0xf << 5), (0 << 5), 100*1000);
		temp = readl(addr);
		if (PORT_PLS_VALUE(temp) != 0) {
			xhci_err(xhci, "rusume fail,%x\n",
				PORT_PLS_VALUE(temp));
			retval = -1;
		} else {
			xhci_err(xhci, "port resume ok\n");
		}
	}

	return retval;
}

static int t_test_enumerate_bus(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct usb_device *usb2_rh;
	struct usb_device *udev;
	long port_id;
	u32 retval;

	port_id = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);

	if (mtk->test_mode == 1) {
		test_mode_exit(mtk);
		return 0;
	}

	usb2_rh = hcd->self.root_hub;
	udev = usb_hub_find_child(usb2_rh, port_id - 1);

	if (udev != NULL) {
		retval = usb_reset_device(udev);
		if (retval) {
			xhci_err(xhci, "ERROR: enumerate bus fail!\n");
			return -1;
		}
	} else {
		xhci_err(xhci, "ERROR: Device does not exist!\n");
		return -1;
	}

	return 0;
}
static int t_test_get_device_descriptor(struct xhci_hcd_mtk *mtk,
				int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct usb_device *usb2_rh;
	struct usb_device *udev;
	long port_id;
	u32 retval = 0;

	port_id = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);

	if (mtk->test_mode == 1) {
		test_mode_exit(mtk);
		msleep(2000);
	}

	usb2_rh = hcd->self.root_hub;

	udev = usb_hub_find_child(usb2_rh, port_id - 1);

	if (udev != NULL) {
		retval = usb_get_device_descriptor(udev, USB_DT_DEVICE_SIZE);
		if (retval != sizeof(udev->descriptor)) {
			xhci_err(xhci, "ERROR: get device descriptor fail!\n");
			return -1;
		}
	} else {
		xhci_err(xhci, "ERROR: Device does not exist!\n");
		return -1;
	}

	return 0;
}

static ssize_t hqa_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 __iomem *addr;
	u32 val;
	u32 ports;
	int len = 0;
	struct hqa_test_cmd *hqa;
	int i;

	len += sprintf(buf+len, "info:\n");
	len += sprintf(buf+len,
			"\techo -n item port-id > hqa\n");
	len += sprintf(buf+len,
			"\tport-id : based on number of usb3-port, e.g.\n");
	len += sprintf(buf+len,
			"\t\txHCI with 1 u3p, 2 u2p: 1st u2p-id is 2(1+1), 2nd is 3\n");
	len += sprintf(buf+len, "items:\n");

	for (i = 0; i < ARRAY_SIZE(xhci_mtk_hqa_cmds); i++) {
		hqa = &xhci_mtk_hqa_cmds[i];
		len += sprintf(buf+len,
				"\t%s: %s\n", hqa->name, hqa->discription);
	}

	ports = mtk->num_u3_ports + mtk->num_u2_ports;
	for (i = 1; i <= ports; i++) {
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS * ((i - 1) & 0xff);
		val = readl(addr);
		if (i <= mtk->num_u3_ports)
			len += sprintf(buf + len,
				"USB30 Port%i: 0x%08X\n", i, val);
		else {
			len += sprintf(buf + len,
				"USB20 Port%i: 0x%08X\n", i, val);

			addr = &xhci->op_regs->port_power_base +
				NUM_PORT_REGS * ((i - 1) & 0xff);
			val = readl(addr);
			len += sprintf(buf+len,
				"USB20 Port%i PORTMSC[31,28] 4b'0000: 0x%08X\n",
				i, val);
		}
	}

	return len;
}

static ssize_t hqa_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	int retval;

	retval = call_hqa_func(mtk, (char *)buf);
	if (retval < 0) {
		xhci_err(xhci, "mu3h cli fail\n");
		return -1;
	}

	return count;
}

static DEVICE_ATTR_RW(hqa);

static ssize_t usb3hqa_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	ssize_t cnt = 0;
	u32 __iomem *addr;
	u32 val;
	u32 i;
	int ports;

	cnt += sprintf(buf + cnt, "usb3hqa usage:\n");
	cnt += sprintf(buf + cnt, "	echo [u3port] >usb3hqa\n");

	ports = mtk->num_u3_ports + mtk->num_u2_ports;
	for (i = 1; i <= ports; i++) {
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS * ((i - 1) & 0xff);
		val = readl(addr);
		if (i <= mtk->num_u3_ports)
			cnt += sprintf(buf + cnt,
				"USB30 Port%i: 0x%08X\n", i, val);
		else
			cnt += sprintf(buf + cnt,
				"USB20 Port%i: 0x%08X\n", i, val);
	}

	if (mtk->hqa_pos) {
		cnt += sprintf(buf + cnt, "%s", mtk->hqa_buf);
		mtk->hqa_pos = 0;
	}

	return cnt;
}

static ssize_t
usb3hqa_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t n)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 __iomem *addr;
	u32 val;
	int port;
	int words;

	mtk->hqa_pos = 0;
	memset(mtk->hqa_buf, 0, mtk->hqa_size);

	hqa_info(mtk, "usb3hqa: %s\n", buf);

	words = sscanf(buf, "%d", &port);
	if ((words != 1) ||
	    (port < 1 || port > mtk->num_u3_ports)) {
		hqa_info(mtk, "usb3hqa: param number:%i, port:%i (%i) failure\n",
			words, port, mtk->num_u3_ports);
		return -EINVAL;
	}

	addr = &xhci->op_regs->port_status_base +
		NUM_PORT_REGS * ((port - 1) & 0xff);
	val  = readl(addr);
	val &= ~(PORT_PLS_MASK);
	val |= (PORT_LINK_STROBE | XDEV_COMP_MODE);
	writel(val, addr);
	hqa_info(mtk, "usb3hqa: port%i: 0x%08X but 0x%08X\n",
		port, val, readl(addr));

	return n;
}
static DEVICE_ATTR_RW(usb3hqa);

static struct device_attribute *mu3h_hqa_attr_list[] = {
	&dev_attr_hqa,
	&dev_attr_usb3hqa,
#include "unusual-statement.h"
};

int hqa_create_attr(struct device *dev)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct mu3c_ippc_regs __iomem *ippc = mtk->ippc_regs;
	struct platform_device *device = to_platform_device(dev);
	int num = ARRAY_SIZE(mu3h_hqa_attr_list);
	int idx;
	int err = 0;
	u32 value;
	u32 addr = hcd->rsrc_start;
	u32 length;

	if (dev == NULL || mtk == NULL)
		return -EINVAL;

	mtk->hqa_size = HQA_PREFIX_SIZE;
	mtk->hqa_pos  = 0;
	mtk->hqa_buf = kzalloc(mtk->hqa_size, GFP_KERNEL);
	if (!mtk->hqa_buf)
		return -ENOMEM;

	if (!mtk->has_ippc) {
		err = query_reg_addr(device, &addr, &length, "ippc");
		if (err)
			return -EINVAL;

		mtk->ippc_regs = ioremap(addr, length);
	}

	ippc  = mtk->ippc_regs;
	value = readl(&ippc->ip_xhci_cap);
	mtk->num_u3_ports = CAP_U3_PORT_NUM(value);
	mtk->num_u2_ports = CAP_U2_PORT_NUM(value);

	for (idx = 0; idx < num; idx++) {
		err = device_create_file(dev, mu3h_hqa_attr_list[idx]);
		if (err)
			break;
	}

	return err;
}

void hqa_remove_attr(struct device *dev)
{
	int idx;
	int num = ARRAY_SIZE(mu3h_hqa_attr_list);
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);

	for (idx = 0; idx < num; idx++)
		device_remove_file(dev, mu3h_hqa_attr_list[idx]);

	kfree(mtk->hqa_buf);
	mtk->hqa_size = 0;
	mtk->hqa_pos  = 0;
	if (!mtk->has_ippc) {
		iounmap(mtk->ippc_regs);
		mtk->ippc_regs = NULL;
	}
}
