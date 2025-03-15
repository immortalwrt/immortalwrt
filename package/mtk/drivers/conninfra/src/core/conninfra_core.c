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

#define pr_fmt(fmt) KBUILD_MODNAME "@(%s:%d) " fmt, __func__, __LINE__

#include "consys_hw.h"
#include "conninfra_core.h"
#include "msg_thread.h"
#include "consys_reg_mng.h"

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define CONNINFRA_EVENT_TIMEOUT 3000
#define CONNINFRA_RESET_TIMEOUT 500

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include <linux/delay.h>

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

static int opfunc_power_on(struct msg_op_data *op);
static int opfunc_power_off(struct msg_op_data *op);
static int opfunc_chip_rst(struct msg_op_data *op);
static int opfunc_rfspi_read(struct msg_op_data *op);
static int opfunc_rfspi_write(struct msg_op_data *op);
static int opfunc_adie_top_ck_en_on(struct msg_op_data *op);
static int opfunc_adie_top_ck_en_off(struct msg_op_data *op);
static int opfunc_spi_clock_switch(struct msg_op_data *op);
static int opfunc_force_conninfra_wakeup(struct msg_op_data *op);
static int opfunc_force_conninfra_sleep(struct msg_op_data *op);
static int opfunc_dump_power_state(struct msg_op_data *op);
static int opfunc_subdrv_pre_reset(struct msg_op_data *op);
static int opfunc_subdrv_post_reset(struct msg_op_data *op);
static void _conninfra_core_update_rst_status(enum chip_rst_status status);

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

struct conninfra_ctx g_conninfra_ctx;

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
static const msg_opid_func conninfra_core_opfunc[] = {
	[CONNINFRA_OPID_PWR_ON] = opfunc_power_on,
	[CONNINFRA_OPID_PWR_OFF] = opfunc_power_off,
	[CONNINFRA_OPID_RFSPI_READ] = opfunc_rfspi_read,
	[CONNINFRA_OPID_RFSPI_WRITE] = opfunc_rfspi_write,
	[CONNINFRA_OPID_ADIE_TOP_CK_EN_ON] = opfunc_adie_top_ck_en_on,
	[CONNINFRA_OPID_ADIE_TOP_CK_EN_OFF] = opfunc_adie_top_ck_en_off,
	[CONNINFRA_OPID_SPI_CLOCK_SWITCH] = opfunc_spi_clock_switch,
	[CONNINFRA_OPID_FORCE_CONNINFRA_WAKUP] = opfunc_force_conninfra_wakeup,
	[CONNINFRA_OPID_FORCE_CONNINFRA_SLEEP] = opfunc_force_conninfra_sleep,
	[CONNINFRA_OPID_DUMP_POWER_STATE] = opfunc_dump_power_state,
};

static const msg_opid_func conninfra_core_cb_opfunc[] = {
	[CONNINFRA_CB_OPID_CHIP_RST] = opfunc_chip_rst,
};


/* subsys ops */
static char *drv_thread_name[] = {
	[CONNDRV_TYPE_BT] = "sub_bt_thrd",
	[CONNDRV_TYPE_FM] = "sub_fm_thrd",
	[CONNDRV_TYPE_GPS] = "sub_gps_thrd",
	[CONNDRV_TYPE_WIFI] = "sub_wifi_thrd",
	[CONNDRV_TYPE_CONNINFRA] = "sub_conninfra_thrd",
};

static char *drv_name[] = {
	[CONNDRV_TYPE_BT] = "BT",
	[CONNDRV_TYPE_FM] = "FM",
	[CONNDRV_TYPE_GPS] = "GPS",
	[CONNDRV_TYPE_WIFI] = "WIFI",
	[CONNDRV_TYPE_CONNINFRA] = "CONNINFRA",
};

typedef enum {
	INFRA_SUBDRV_OPID_PRE_RESET		= 0,
	INFRA_SUBDRV_OPID_POST_RESET	= 1,
	INFRA_SUBDRV_OPID_MAX
} infra_subdrv_op;


static const msg_opid_func infra_subdrv_opfunc[] = {
	[INFRA_SUBDRV_OPID_PRE_RESET] = opfunc_subdrv_pre_reset,
	[INFRA_SUBDRV_OPID_POST_RESET] = opfunc_subdrv_post_reset,
};

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

static void reset_chip_rst_trg_data(void)
{
	g_conninfra_ctx.trg_drv = CONNDRV_TYPE_MAX;
	memset(g_conninfra_ctx.trg_reason, '\0', CHIP_RST_REASON_MAX_LEN);
}

static unsigned long timeval_to_ms(struct timeval *begin, struct timeval *end)
{
	unsigned long time_diff;

	time_diff = (end->tv_sec - begin->tv_sec) * 1000;
	time_diff += (end->tv_usec - begin->tv_usec) / 1000;

	return time_diff;
}

static unsigned int opfunc_get_current_status(void)
{
	unsigned int ret = 0;
	unsigned int i;

	for (i = 0; i < CONNDRV_TYPE_MAX; i++) {
		ret |= (g_conninfra_ctx.drv_inst[i].drv_status << i);
	}

	return ret;
}

static void opfunc_vcn_control_internal(unsigned int drv_type, bool on)
{
	/* VCNx enable */
	switch (drv_type) {
	case CONNDRV_TYPE_BT:
		consys_hw_bt_power_ctl(on);
		break;
	case CONNDRV_TYPE_FM:
		consys_hw_fm_power_ctl(on);
		break;
	case CONNDRV_TYPE_GPS:
		consys_hw_gps_power_ctl(on);
		break;
	case CONNDRV_TYPE_WIFI:
		consys_hw_wifi_power_ctl(on);
		break;
	case CONNDRV_TYPE_CONNINFRA:
		break;
	default:
		pr_err("Wrong parameter: drv_type(%d)\n", drv_type);
		break;
	}
}

static int opfunc_power_on_internal(unsigned int drv_type)
{
	int ret;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	/* Check abnormal type */
	if (drv_type >= CONNDRV_TYPE_MAX) {
		pr_err("abnormal Fun(%d)\n", drv_type);
		return -EINVAL;
	}

	/* Check abnormal state */
	if ((g_conninfra_ctx.drv_inst[drv_type].drv_status < DRV_STS_POWER_OFF)
	    || (g_conninfra_ctx.drv_inst[drv_type].drv_status >= DRV_STS_MAX)) {
		pr_err("func(%d) status[0x%x] abnormal\n", drv_type,
				g_conninfra_ctx.drv_inst[drv_type].drv_status);
		return -EINVAL;
	}

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return ret;
	}

	/* check if func already on */
	if (g_conninfra_ctx.drv_inst[drv_type].drv_status == DRV_STS_POWER_ON) {
		pr_warn("func(%d) already on\n", drv_type);
		osal_unlock_sleepable_lock(&infra_ctx->core_lock);
		return 0;
	}

	ret = consys_hw_pwr_on(opfunc_get_current_status(), drv_type);
	if (ret) {
		pr_err("Conninfra power on fail. drv(%d) ret=(%d)\n",
			drv_type, ret);
		osal_unlock_sleepable_lock(&infra_ctx->core_lock);
		return -3;
	}

	/* POWER ON SEQUENCE */
	g_conninfra_ctx.infra_drv_status = DRV_STS_POWER_ON;
	g_conninfra_ctx.drv_inst[drv_type].drv_status = DRV_STS_POWER_ON;

	/* VCNx enable */
	opfunc_vcn_control_internal(drv_type, true);

	pr_info("[Conninfra Pwr On] BT=[%d] FM=[%d] GPS=[%d] WF=[%d] CONNINFRA=[%d]\n",
			infra_ctx->drv_inst[CONNDRV_TYPE_BT].drv_status,
			infra_ctx->drv_inst[CONNDRV_TYPE_FM].drv_status,
			infra_ctx->drv_inst[CONNDRV_TYPE_GPS].drv_status,
			infra_ctx->drv_inst[CONNDRV_TYPE_WIFI].drv_status,
			infra_ctx->drv_inst[CONNDRV_TYPE_CONNINFRA].drv_status);

	osal_unlock_sleepable_lock(&infra_ctx->core_lock);

	return 0;
}

static int opfunc_power_on(struct msg_op_data *op)
{
	unsigned int drv_type = op->op_data[0];

	return opfunc_power_on_internal(drv_type);
}

static int opfunc_power_off_internal(unsigned int drv_type)
{
	int i, ret;
	bool try_power_off = true;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;
	unsigned int curr_status = opfunc_get_current_status();

	/* Check abnormal type */
	if (drv_type >= CONNDRV_TYPE_MAX) {
		pr_err("abnormal Fun(%d)\n", drv_type);
		return -EINVAL;
	}

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return ret;
	}

	/* Check abnormal state */
	if ((g_conninfra_ctx.drv_inst[drv_type].drv_status < DRV_STS_POWER_OFF)
	    || (g_conninfra_ctx.drv_inst[drv_type].drv_status >= DRV_STS_MAX)) {
		pr_err("func(%d) status[0x%x] abnormal\n", drv_type,
			g_conninfra_ctx.drv_inst[drv_type].drv_status);
		osal_unlock_sleepable_lock(&infra_ctx->core_lock);
		return -2;
	}

	/* Special case for force power off */
	if (drv_type == CONNDRV_TYPE_CONNINFRA) {
		if (g_conninfra_ctx.infra_drv_status == DRV_STS_POWER_OFF) {
			pr_warn("Connsys already off, do nothing for force off\n");
			return 0;
		}
		/* Turn off subsys VCN and update record */
		for (i = 0; i < CONNDRV_TYPE_MAX; i++) {
			if (g_conninfra_ctx.drv_inst[i].drv_status == DRV_STS_POWER_ON) {
				opfunc_vcn_control_internal(i, false);
				g_conninfra_ctx.drv_inst[i].drv_status = DRV_STS_POWER_OFF;
			}
		}
		/* POWER OFF SEQUENCE */
		ret = consys_hw_pwr_off(0, drv_type);
		/* For force power off operation, ignore err code */
		if (ret)
			pr_err("Force power off fail. ret=%d\n", ret);
		try_power_off = true;
	} else {
		/* check if func already off */
		if (g_conninfra_ctx.drv_inst[drv_type].drv_status
					== DRV_STS_POWER_OFF) {
			pr_warn("func(%d) already off\n", drv_type);
			osal_unlock_sleepable_lock(&infra_ctx->core_lock);
			return 0;
		}
		/* VCNx disable */
		opfunc_vcn_control_internal(drv_type, false);
		g_conninfra_ctx.drv_inst[drv_type].drv_status = DRV_STS_POWER_OFF;
		/* is there subsys on ? */
		for (i = 0; i < CONNDRV_TYPE_MAX; i++)
			if (g_conninfra_ctx.drv_inst[i].drv_status == DRV_STS_POWER_ON)
				try_power_off = false;

		/* POWER OFF SEQUENCE */
		ret = consys_hw_pwr_off(curr_status, drv_type);
		if (ret) {
			pr_err("Conninfra power on fail. drv(%d) ret=(%d)\n",
				drv_type, ret);
			osal_unlock_sleepable_lock(&infra_ctx->core_lock);
			return -3;
		}
	}

	if (try_power_off)
		g_conninfra_ctx.infra_drv_status = DRV_STS_POWER_OFF;

	pr_info("[Conninfra Pwr Off] Conninfra=[%d] BT=[%d] FM=[%d] GPS=[%d] WF=[%d]\n",
			infra_ctx->infra_drv_status,
			infra_ctx->drv_inst[CONNDRV_TYPE_BT].drv_status,
			infra_ctx->drv_inst[CONNDRV_TYPE_FM].drv_status,
			infra_ctx->drv_inst[CONNDRV_TYPE_GPS].drv_status,
			infra_ctx->drv_inst[CONNDRV_TYPE_WIFI].drv_status);

	osal_unlock_sleepable_lock(&infra_ctx->core_lock);
	return 0;
}

static int opfunc_power_off(struct msg_op_data *op)
{
	unsigned int drv_type = op->op_data[0];

	return opfunc_power_off_internal(drv_type);
}

static int opfunc_chip_rst(struct msg_op_data *op)
{
	int i, ret, cur_rst_state;
	struct subsys_drv_inst *drv_inst;
	unsigned int drv_pwr_state[CONNDRV_TYPE_MAX];
	const unsigned int subdrv_all_done = (0x1 << CONNDRV_TYPE_MAX) - 1;
	struct timeval pre_begin, pre_end, reset_end, done_end;

	if (g_conninfra_ctx.infra_drv_status == DRV_STS_POWER_OFF) {
		pr_info("No subsys on, just return\n");
		_conninfra_core_update_rst_status(CHIP_RST_NONE);
		return 0;
	}

	osal_gettimeofday2(&pre_begin);

	atomic_set(&g_conninfra_ctx.rst_state, 0);
	sema_init(&g_conninfra_ctx.rst_sema, 1);

	_conninfra_core_update_rst_status(CHIP_RST_PRE_CB);

	/* pre */
	for (i = 0; i < CONNDRV_TYPE_MAX; i++) {
		drv_inst = &g_conninfra_ctx.drv_inst[i];
		drv_pwr_state[i] = drv_inst->drv_status;
		pr_info("subsys %d is %d\n", i, drv_inst->drv_status);
		ret = msg_thread_send_1(&drv_inst->msg_ctx, INFRA_SUBDRV_OPID_PRE_RESET, i);
	}

	pr_info("[chip_rst] pre vvvvvvvvvvvvv\n");
	while (atomic_read(&g_conninfra_ctx.rst_state) != subdrv_all_done) {
		ret = down_timeout(&g_conninfra_ctx.rst_sema, msecs_to_jiffies(CONNINFRA_RESET_TIMEOUT));
		pr_info("sema ret=[%d]\n", ret);
		if (ret == 0)
			continue;
		cur_rst_state = atomic_read(&g_conninfra_ctx.rst_state);
		pr_info("cur_rst state =[%d]\n", cur_rst_state);
		for (i = 0; i < CONNDRV_TYPE_MAX; i++) {
			if ((cur_rst_state & (0x1 << i)) == 0) {
				pr_info("[chip_rst] [%s] pre-callback is not back\n", drv_thread_name[i]);
			}
		}
	}

	_conninfra_core_update_rst_status(CHIP_RST_RESET);

	osal_gettimeofday2(&pre_end);

	pr_info("[chip_rst] reset ++++++++++++\n");
	/*******************************************************/
	/* reset */
	/* call consys_hw */
	/*******************************************************/
	/* Special power-off function, turn off connsys directly */
	ret = opfunc_power_off_internal(CONNDRV_TYPE_CONNINFRA);
	pr_info("Force conninfra power off, ret=%d\n", ret);
	pr_info("conninfra status should be power off. Status=%d\n", g_conninfra_ctx.infra_drv_status);

	/* Turn on subsys */
	for (i = 0; i < CONNDRV_TYPE_MAX; i++) {
		if (drv_pwr_state[i]) {
			ret = opfunc_power_on_internal(i);
			pr_info("Call subsys(%d) power on ret=%d\n", i, ret);
		}
	}
	pr_info("conninfra status should be power on. Status=%d\n", g_conninfra_ctx.infra_drv_status);

	pr_info("[chip_rst] reset --------------\n");

	_conninfra_core_update_rst_status(CHIP_RST_POST_CB);

	osal_gettimeofday2(&reset_end);

	/* post */
	atomic_set(&g_conninfra_ctx.rst_state, 0);
	sema_init(&g_conninfra_ctx.rst_sema, 1);
	for (i = 0; i < CONNDRV_TYPE_MAX; i++) {
		drv_inst = &g_conninfra_ctx.drv_inst[i];
		ret = msg_thread_send_1(&drv_inst->msg_ctx, INFRA_SUBDRV_OPID_POST_RESET, i);
	}

	while (atomic_read(&g_conninfra_ctx.rst_state) != subdrv_all_done) {
		ret = down_timeout(&g_conninfra_ctx.rst_sema, msecs_to_jiffies(CONNINFRA_RESET_TIMEOUT));
		if (ret == 0)
			continue;
		cur_rst_state = atomic_read(&g_conninfra_ctx.rst_state);
		for (i = 0; i < CONNDRV_TYPE_MAX; i++) {
			if ((cur_rst_state & (0x1 << i)) == 0) {
				pr_info("[chip_rst] [%s] post-callback is not back\n", drv_thread_name[i]);
			}
		}
	}
	pr_info("[chip_rst] post ^^^^^^^^^^^^^^\n");

	reset_chip_rst_trg_data();
	//_conninfra_core_update_rst_status(CHIP_RST_DONE);
	_conninfra_core_update_rst_status(CHIP_RST_NONE);
	osal_gettimeofday2(&done_end);

	pr_info("[chip_rst] summary pre=[%lu] reset=[%lu] post=[%lu]\n",
				timeval_to_ms(&pre_begin, &pre_end),
				timeval_to_ms(&pre_end, &reset_end),
				timeval_to_ms(&reset_end, &done_end));

	return 0;
}

static int opfunc_rfspi_read(struct msg_op_data *op)
{
	int ret = 0;
	unsigned int data = 0;
	unsigned int* data_pt = (unsigned int*)op->op_data[2];

	ret = osal_lock_sleepable_lock(&g_conninfra_ctx.core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return CONNINFRA_SPI_OP_FAIL;
	}

	if (g_conninfra_ctx.infra_drv_status != DRV_STS_POWER_ON) {
		pr_err("Connsys didn't power on\n");
		ret = CONNINFRA_SPI_OP_FAIL;
		goto err;
	}

	if (consys_hw_reg_readable() == 0) {
		pr_err("connsys reg not readable\n");
		ret = CONNINFRA_SPI_OP_FAIL;
		goto err;
	}

	/* DO read spi */
	ret = consys_hw_spi_read(op->op_data[0], op->op_data[1], &data);
	if (data_pt)
		*(data_pt) = data;
err:
	osal_unlock_sleepable_lock(&g_conninfra_ctx.core_lock);
	return ret;
}

static int opfunc_rfspi_write(struct msg_op_data *op)
{
	int ret = 0;

	ret = osal_lock_sleepable_lock(&g_conninfra_ctx.core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return CONNINFRA_SPI_OP_FAIL;
	}

	if (g_conninfra_ctx.infra_drv_status != DRV_STS_POWER_ON) {
		pr_err("Connsys didn't power on\n");
		ret = CONNINFRA_SPI_OP_FAIL;
		goto err;
	}

	if (consys_hw_reg_readable() == 0) {
		pr_err("connsys reg not readable\n");
		ret = CONNINFRA_SPI_OP_FAIL;
		goto err;
	}

	/* DO spi write */
	ret = consys_hw_spi_write(op->op_data[0], op->op_data[1], op->op_data[2]);
err:
	osal_unlock_sleepable_lock(&g_conninfra_ctx.core_lock);
	return ret;
}

static int opfunc_adie_top_ck_en_on(struct msg_op_data *op)
{
	int ret = 0;
	unsigned int type = op->op_data[0];

	if (type >= CONNDRV_TYPE_MAX) {
		pr_err("wrong parameter %d\n", type);
		return -EINVAL;
	}

	ret = osal_lock_sleepable_lock(&g_conninfra_ctx.core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		ret = -1;
		goto err;
	}

	if (g_conninfra_ctx.infra_drv_status != DRV_STS_POWER_ON) {
		pr_err("Connsys didn't power on\n");
		ret = -2;
		goto err;
	}

	ret = consys_hw_adie_top_ck_en_on(type);

err:
	osal_unlock_sleepable_lock(&g_conninfra_ctx.core_lock);
	return ret;
}


static int opfunc_adie_top_ck_en_off(struct msg_op_data *op)
{
	int ret = 0;
	unsigned int type = op->op_data[0];

	if (type >= CONNDRV_TYPE_MAX) {
		pr_err("wrong parameter %d\n", type);
		return -EINVAL;
	}

	ret = osal_lock_sleepable_lock(&g_conninfra_ctx.core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		ret = -1;
		goto err;
	}
	if (g_conninfra_ctx.infra_drv_status != DRV_STS_POWER_ON) {
		pr_err("Connsys didn't power on\n");
		ret = -2;
		goto err;
	}

	ret = consys_hw_adie_top_ck_en_off(type);
err:
	osal_unlock_sleepable_lock(&g_conninfra_ctx.core_lock);
	return ret;
}

static int opfunc_spi_clock_switch(struct msg_op_data *op)
{
	int ret = 0;
	unsigned int type = op->op_data[0];

	if (type >= CONNSYS_SPI_SPEED_MAX) {
		pr_err("wrong parameter %d\n", type);
		return -EINVAL;
	}

	ret = osal_lock_sleepable_lock(&g_conninfra_ctx.core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		ret = -2;
		goto err;
	}
	if (g_conninfra_ctx.infra_drv_status != DRV_STS_POWER_ON) {
		pr_err("Connsys didn't power on\n");
		ret = -2;
		goto err;
	}

	ret = consys_hw_spi_clock_switch(type);
err:
	osal_unlock_sleepable_lock(&g_conninfra_ctx.core_lock);
	return ret;
}

static int opfunc_force_conninfra_wakeup(struct msg_op_data *op)
{
	int ret;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return ret;
	}

	/* check if conninfra already on */
	if (g_conninfra_ctx.infra_drv_status != DRV_STS_POWER_ON) {
		ret = -1;
		goto err;
	}

	ret = consys_hw_force_conninfra_wakeup();
	if (ret)
		pr_err("force conninfra wakeup fail\n");

err:
	osal_unlock_sleepable_lock(&infra_ctx->core_lock);
	return ret;
}

static int opfunc_force_conninfra_sleep(struct msg_op_data *op)
{
	int ret;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return ret;
	}

	/* check if conninfra already on */
	if (g_conninfra_ctx.infra_drv_status != DRV_STS_POWER_ON) {
		ret = -1;
		goto err;
	}

	ret = consys_hw_force_conninfra_sleep();
	if (ret)
		pr_err("force conninfra sleep fail\n");

err:
	osal_unlock_sleepable_lock(&infra_ctx->core_lock);
	return ret;
}


static int opfunc_dump_power_state(struct msg_op_data *op)
{
	int ret;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return ret;
	}

	/* check if conninfra already on */
	if (g_conninfra_ctx.infra_drv_status != DRV_STS_POWER_ON) {
		ret = -1;
		goto err;
	}

	ret = consys_hw_dump_power_state();
	if (ret)
		pr_err("dump power state fail\n");

err:
	osal_unlock_sleepable_lock(&infra_ctx->core_lock);
	return ret;

}

static int opfunc_subdrv_pre_reset(struct msg_op_data *op)
{
	int ret, cur_rst_state;
	unsigned int drv_type = op->op_data[0];
	struct subsys_drv_inst *drv_inst;


	/* TODO: should be locked, to avoid cb was reset */
	drv_inst = &g_conninfra_ctx.drv_inst[drv_type];
	if (/*drv_inst->drv_status == DRV_ST_POWER_ON &&*/
			drv_inst->ops_cb.rst_cb.pre_whole_chip_rst) {

		ret = drv_inst->ops_cb.rst_cb.pre_whole_chip_rst(g_conninfra_ctx.trg_drv, g_conninfra_ctx.trg_reason);
		if (ret)
			pr_err("[%s] fail [%d]\n", __func__, ret);
	}

	atomic_add(0x1 << drv_type, &g_conninfra_ctx.rst_state);
	cur_rst_state = atomic_read(&g_conninfra_ctx.rst_state);

	pr_info("[%s] rst_state=[%d]\n", drv_thread_name[drv_type], cur_rst_state);

	up(&g_conninfra_ctx.rst_sema);
	return 0;
}

static int opfunc_subdrv_post_reset(struct msg_op_data *op)
{
	int ret;
	unsigned int drv_type = op->op_data[0];
	struct subsys_drv_inst *drv_inst;

	/* TODO: should be locked, to avoid cb was reset */
	drv_inst = &g_conninfra_ctx.drv_inst[drv_type];
	if (/*drv_inst->drv_status == DRV_ST_POWER_ON &&*/
			drv_inst->ops_cb.rst_cb.post_whole_chip_rst) {
		ret = drv_inst->ops_cb.rst_cb.post_whole_chip_rst();
		if (ret)
			pr_warn("[%s] fail [%d]\n", __func__, ret);
	}

	atomic_add(0x1 << drv_type, &g_conninfra_ctx.rst_state);
	up(&g_conninfra_ctx.rst_sema);
	return 0;
}

/*
 * CONNINFRA API
 */
int conninfra_core_power_on(enum consys_drv_type type)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = msg_thread_send_wait_1(&infra_ctx->msg_ctx,
				CONNINFRA_OPID_PWR_ON, 0, type);
	if (ret) {
		pr_err("[%s] fail, ret = %d\n", __func__, ret);
		return -1;
	}
	return 0;
}

int conninfra_core_power_off(enum consys_drv_type type)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = msg_thread_send_wait_1(&infra_ctx->msg_ctx,
				CONNINFRA_OPID_PWR_OFF, 0, type);
	if (ret) {
		pr_err("[%s] send msg fail, ret = %d\n", __func__, ret);
		return -1;
	}
	return 0;
}

int conninfra_core_reg_readable(void)
{
	int ret = 0, rst_status;
	unsigned long flag;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;


	/* check if in reseting, can not read */
	spin_lock_irqsave(&g_conninfra_ctx.rst_lock, flag);
	rst_status = g_conninfra_ctx.rst_status;
	spin_unlock_irqrestore(&g_conninfra_ctx.rst_lock, flag);

	if (rst_status >= CHIP_RST_RESET &&
		rst_status < CHIP_RST_POST_CB)
		return 0;

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return 0;
	}

	if (infra_ctx->infra_drv_status == DRV_STS_POWER_ON)
		ret = consys_hw_reg_readable();
	osal_unlock_sleepable_lock(&infra_ctx->core_lock);

	return ret;
}

int conninfra_core_reg_readable_no_lock(void)
{
	int rst_status;
	unsigned long flag;

	/* check if in reseting, can not read */
	spin_lock_irqsave(&g_conninfra_ctx.rst_lock, flag);
	rst_status = g_conninfra_ctx.rst_status;
	spin_unlock_irqrestore(&g_conninfra_ctx.rst_lock, flag);

	if (rst_status >= CHIP_RST_RESET &&
		rst_status < CHIP_RST_POST_CB)
		return 0;

	return consys_hw_reg_readable();
}

int conninfra_core_is_bus_hang(void)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return 0;
	}

	if (infra_ctx->infra_drv_status == DRV_STS_POWER_ON)
		ret = consys_hw_is_bus_hang();
	osal_unlock_sleepable_lock(&infra_ctx->core_lock);

	return ret;

}

int conninfra_core_is_consys_reg(phys_addr_t addr)
{
	return consys_hw_is_connsys_reg(addr);
}

int conninfra_core_reg_read(unsigned long address, unsigned int *value, unsigned int mask)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return 0;
	}

	if (infra_ctx->infra_drv_status == DRV_STS_POWER_ON) {
		if (consys_reg_mng_is_host_csr(address))
			ret = consys_reg_mng_reg_read(address, value, mask);
		else if (consys_hw_reg_readable())
			ret = consys_reg_mng_reg_read(address, value, mask);
		else
			pr_info("CR (%lx) is not readable\n", address);
	} else
		pr_info("CR (%lx) cannot read. conninfra is off\n", address);

	osal_unlock_sleepable_lock(&infra_ctx->core_lock);
	return ret;
}

int conninfra_core_reg_write(unsigned long address, unsigned int value, unsigned int mask)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return 0;
	}

	if (infra_ctx->infra_drv_status == DRV_STS_POWER_ON) {
		if (consys_reg_mng_is_host_csr(address))
			ret = consys_reg_mng_reg_write(address, value, mask);
		else if (consys_hw_reg_readable())
			ret = consys_reg_mng_reg_write(address, value, mask);
		else
			pr_info("CR (%p) is not readable\n", (void*)address);
	} else
		pr_info("CR (%p) cannot read. conninfra is off\n", (void*)address);

	osal_unlock_sleepable_lock(&infra_ctx->core_lock);
	return ret;

}

int conninfra_core_lock_rst(void)
{
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;
	int ret = 0;
	unsigned long flag;

	spin_lock_irqsave(&infra_ctx->rst_lock, flag);

	ret = infra_ctx->rst_status;
	if (infra_ctx->rst_status > CHIP_RST_NONE &&
		infra_ctx->rst_status < CHIP_RST_DONE) {
		/* do nothing */
	} else {
		infra_ctx->rst_status = CHIP_RST_START;
	}
	spin_unlock_irqrestore(&infra_ctx->rst_lock, flag);

	pr_info("[%s] ret=[%d]\n", __func__, ret);
	return ret;
}

int conninfra_core_unlock_rst(void)
{
	unsigned long flag;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	spin_lock_irqsave(&infra_ctx->rst_lock, flag);
	infra_ctx->rst_status = CHIP_RST_NONE;
	spin_unlock_irqrestore(&infra_ctx->rst_lock, flag);
	return 0;
}

int conninfra_core_trg_chip_rst(enum consys_drv_type drv, char *reason)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	infra_ctx->trg_drv = drv;
	snprintf(infra_ctx->trg_reason, CHIP_RST_REASON_MAX_LEN, "%s", reason);
	ret = msg_thread_send_1(&infra_ctx->cb_ctx,
				CONNINFRA_CB_OPID_CHIP_RST, drv);
	if (ret) {
		pr_err("send msg fail, ret = %d\n", ret);
		return -1;
	}
	pr_info("trg_reset DONE!\n");
	return 0;
}

static inline char* conninfra_core_spi_subsys_string(enum sys_spi_subsystem subsystem)
{
	static char* subsys_name[] = {
		"SYS_SPI_WF1",
		"SYS_SPI_WF",
		"SYS_SPI_BT",
		"SYS_SPI_FM",
		"SYS_SPI_GPS",
		"SYS_SPI_TOP",
		"SYS_SPI_WF2",
		"SYS_SPI_WF3",
		"SYS_SPI_2ND_ADIE_WF1",
		"SYS_SPI_2ND_ADIE_WF",
		"SYS_SPI_2ND_ADIE_BT",
		"SYS_SPI_2ND_ADIE_FM",
		"SYS_SPI_2ND_ADIE_GPS",
		"SYS_SPI_2ND_ADIE_TOP",
		"SYS_SPI_2ND_ADIE_WF2",
		"SYS_SPI_2ND_ADIE_WF3",
		"SYS_SPI_MAX"
	};
	return subsys_name[subsystem];
}

int conninfra_core_spi_read(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int *data)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;
	size_t data_ptr = (size_t)data;

	ret = msg_thread_send_wait_3(&infra_ctx->msg_ctx, CONNINFRA_OPID_RFSPI_READ, 0,
								subsystem, addr, data_ptr);
	if (ret) {
		pr_err("failed (ret = %d). subsystem=%s addr=%x\n",
				ret, conninfra_core_spi_subsys_string(subsystem), addr);
		return CONNINFRA_SPI_OP_FAIL;
	}
	return 0;
}

int conninfra_core_spi_write(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int data)
{
	int ret;
	ret = msg_thread_send_wait_3(&(g_conninfra_ctx.msg_ctx), CONNINFRA_OPID_RFSPI_WRITE, 0,
								subsystem, addr, data);
	if (ret) {
		pr_err("failed (ret = %d). subsystem=%s addr=0x%x data=%d\n",
				ret, conninfra_core_spi_subsys_string(subsystem), addr, data);
		return CONNINFRA_SPI_OP_FAIL;
	}
	return 0;
}

int conninfra_core_adie_top_ck_en_on(enum consys_drv_type type)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = msg_thread_send_wait_1(&infra_ctx->msg_ctx,
				CONNINFRA_OPID_ADIE_TOP_CK_EN_ON, 0, type);
	if (ret) {
		pr_err("fail, ret = %d\n", ret);
		return -1;
	}
	return 0;
}

int conninfra_core_adie_top_ck_en_off(enum consys_drv_type type)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = msg_thread_send_wait_1(&infra_ctx->msg_ctx,
				CONNINFRA_OPID_ADIE_TOP_CK_EN_OFF, 0, type);
	if (ret) {
		pr_err("fail, ret = %d\n", ret);
		return -1;
	}
	return 0;
}

int conninfra_core_force_conninfra_wakeup(void)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	/* if in conninfra_cored thread */
	if (current == infra_ctx->msg_ctx.thread.pThread)
		return opfunc_force_conninfra_wakeup(NULL);

	ret = msg_thread_send_wait(&infra_ctx->msg_ctx,
				CONNINFRA_OPID_FORCE_CONNINFRA_WAKUP, 0);
	if (ret) {
		pr_err("fail, ret = %d\n", ret);
		return -1;
	}
	return 0;
}

int conninfra_core_force_conninfra_sleep(void)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	/* if in conninfra_cored thread */
	if (current == infra_ctx->msg_ctx.thread.pThread)
		return opfunc_force_conninfra_sleep(NULL);

	ret = msg_thread_send_wait(&infra_ctx->msg_ctx,
				CONNINFRA_OPID_FORCE_CONNINFRA_SLEEP, 0);
	if (ret) {
		pr_err("fail, ret = %d\n", ret);
		return -1;
	}
	return 0;
}

int conninfra_core_spi_clock_switch(enum connsys_spi_speed_type type)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = msg_thread_send_wait_1(&infra_ctx->msg_ctx,
				CONNINFRA_OPID_SPI_CLOCK_SWITCH, 0, type);
	if (ret) {
		pr_err("fail, ret = %d\n", ret);
		return -1;
	}
	return 0;
}

int conninfra_core_subsys_ops_reg(enum consys_drv_type type,
					struct sub_drv_ops_cb *cb)
{
	unsigned long flag;
	struct subsys_drv_inst *drv_inst;
	int ret = 0;

	spin_lock_irqsave(&g_conninfra_ctx.infra_lock, flag);
	drv_inst = &g_conninfra_ctx.drv_inst[type];
	memcpy(&g_conninfra_ctx.drv_inst[type].ops_cb, cb, sizeof(struct sub_drv_ops_cb));
	spin_unlock_irqrestore(&g_conninfra_ctx.infra_lock, flag);

	pr_info("[pre_cal] type=[%s] cb rst=[%p][%p]\n",
			drv_name[type], cb->rst_cb.pre_whole_chip_rst, cb->rst_cb.post_whole_chip_rst);

	return ret;
}

int conninfra_core_subsys_ops_unreg(enum consys_drv_type type)
{
	unsigned long flag;

	spin_lock_irqsave(&g_conninfra_ctx.infra_lock, flag);
	memset(&g_conninfra_ctx.drv_inst[type].ops_cb, 0,
					sizeof(struct sub_drv_ops_cb));
	spin_unlock_irqrestore(&g_conninfra_ctx.infra_lock, flag);

	return 0;
}

static void _conninfra_core_update_rst_status(enum chip_rst_status status)
{
	unsigned long flag;

	spin_lock_irqsave(&g_conninfra_ctx.rst_lock, flag);
	g_conninfra_ctx.rst_status = status;
	spin_unlock_irqrestore(&g_conninfra_ctx.rst_lock, flag);
}


int conninfra_core_is_rst_locking(void)
{
	unsigned long flag;
	int ret = 0;

	spin_lock_irqsave(&g_conninfra_ctx.rst_lock, flag);

	if (g_conninfra_ctx.rst_status > CHIP_RST_NONE &&
		g_conninfra_ctx.rst_status < CHIP_RST_POST_CB)
		ret = 1;
	spin_unlock_irqrestore(&g_conninfra_ctx.rst_lock, flag);
	return ret;
}

int conninfra_core_dump_power_state(void)
{
	int ret = 0;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	ret = msg_thread_send(&infra_ctx->msg_ctx,
				CONNINFRA_OPID_DUMP_POWER_STATE);
	if (ret) {
		pr_err("fail, ret = %d\n", ret);
		return -1;
	}

	return 0;

}

int conninfra_core_pmic_event_cb(unsigned int id, unsigned int event)
{
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;
	int ret;

	if (conninfra_core_is_rst_locking()) {
		return 0;
	}

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail!!\n");
		return 0;
	}

	if (infra_ctx->infra_drv_status == DRV_STS_POWER_ON)
		consys_hw_pmic_event_cb(id, event);

	osal_unlock_sleepable_lock(&infra_ctx->core_lock);

	return 0;
}

int conninfra_core_debug_dump(void)
{
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;
	int ret = -1;
	unsigned int i;

	ret = osal_lock_sleepable_lock(&infra_ctx->core_lock);
	if (ret) {
		pr_err("core_lock fail, ret=%d\n", ret);
		return -1;
	}

	msg_thread_dump(&infra_ctx->msg_ctx);
	msg_thread_dump(&infra_ctx->cb_ctx);
	for (i = 0; i < CONNDRV_TYPE_MAX; i++) {
		msg_thread_dump(&(infra_ctx->drv_inst[i].msg_ctx));
	}

	osal_unlock_sleepable_lock(&infra_ctx->core_lock);

	return ret;
}

int conninfra_core_init(void)
{
	int ret = 0, i;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	osal_memset(&g_conninfra_ctx, 0, sizeof(g_conninfra_ctx));

	reset_chip_rst_trg_data();

	spin_lock_init(&infra_ctx->infra_lock);
	osal_sleepable_lock_init(&infra_ctx->core_lock);
	spin_lock_init(&infra_ctx->rst_lock);


	ret = msg_thread_init(&infra_ctx->msg_ctx, "conninfra_cored",
				conninfra_core_opfunc, CONNINFRA_OPID_MAX);
	if (ret) {
		pr_err("msg_thread init fail(%d)\n", ret);
		return -1;
	}

	ret = msg_thread_init(&infra_ctx->cb_ctx, "conninfra_cb",
                               conninfra_core_cb_opfunc, CONNINFRA_CB_OPID_MAX);
	if (ret) {
		pr_err("callback msg thread init fail(%d)\n", ret);
		return -1;
	}

	/* init subsys drv state */
	for (i = 0; i < CONNDRV_TYPE_MAX; i++) {
		ret += msg_thread_init(&infra_ctx->drv_inst[i].msg_ctx,
				drv_thread_name[i], infra_subdrv_opfunc,
				INFRA_SUBDRV_OPID_MAX);
	}

	if (ret) {
		pr_err("subsys callback thread init fail.\n");
		return -1;
	}

	return ret;
}

int conninfra_core_deinit(void)
{
	int ret, i;
	struct conninfra_ctx *infra_ctx = &g_conninfra_ctx;

	for (i = 0; i < CONNDRV_TYPE_MAX; i++) {
		ret = msg_thread_deinit(&infra_ctx->drv_inst[i].msg_ctx);
		if (ret)
			pr_warn("subdrv [%d] msg_thread deinit fail (%d)\n",
						i, ret);
	}

	ret = msg_thread_deinit(&infra_ctx->msg_ctx);
	if (ret) {
		pr_err("msg_thread_deinit fail(%d)\n", ret);
		return -1;
	}

	osal_sleepable_lock_deinit(&infra_ctx->core_lock);

	return 0;
}

