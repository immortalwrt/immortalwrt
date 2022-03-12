/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	test_engine.h
*/
#ifndef __TEST_ENGINE_H__
#define __TEST_ENGINE_H__

#include "operation.h"

/*****************************************************************************
 *	Macro
 *****************************************************************************/
#define engine_min(_a, _b) ((_a > _b) ? _b : _a)

#define engine_max(_a, _b) ((_a > _b) ? _a : _b)

#define engine_ceil(_a, _b) (((_a%_b) > 0) ? ((_a/_b)+1) : (_a/_b))

#define TEST_ANT_USER_DEF 0x80000000

/*****************************************************************************
 *	Enum value definition
 *****************************************************************************/
enum TEST_HETB_CTRL {
	OP_HETB_TX_CFG = 0,
	OP_HETB_TX_START = 1,
	OP_HETB_TX_STOP = 2,
	OP_HETB_RX_CFG = 3,
};

/*****************************************************************************
 *	Data struct definition
 *****************************************************************************/
struct test_he_ru_const {
	u_int8	max_index;
	u_int16	sd;		/* data subcarriers */
	u_int16	sd_d;		/* data subcarriers for DCM */
	u_int16	sd_s;		/* data subcarriers short */
	u_int16	sd_s_d;		/* data subcarriers short for DCM*/
};

/*****************************************************************************
 *	Function declaration
 *****************************************************************************/
s_int32 mt_engine_search_stack(
	struct test_configuration *configs,
	u_int8 wcid,
	u_int8 *stack_idx,
	void **virtual_wtbl);
s_int32 mt_engine_subscribe_tx(
	struct test_operation *ops,
	struct test_wlan_info *winfos,
	void *virtual_device,
	struct test_configuration *configs);
s_int32 mt_engine_unsubscribe_tx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs);
s_int32 mt_engine_start(
	struct test_wlan_info *winfos,
	struct test_backup_params *bak,
	struct test_configuration *configs,
	struct test_operation *ops,
	struct test_bk_cr *bks,
	struct test_rx_stat *rx_stat,
	u_int32 en_log);
s_int32 mt_engine_stop(
	struct test_wlan_info *winfos,
	struct test_backup_params *bak,
	struct test_configuration *configs,
	struct test_operation *ops,
	struct test_bk_cr *bks,
	struct test_log_dump_cb *log_dump);
s_int32 mt_engine_calc_ipg_param_by_ipg(
	struct test_configuration *configs);
s_int32 mt_engine_set_auto_resp(
	struct test_wlan_info *winfos,
	struct test_operation *ops,
	struct test_configuration *configs,
	u_char band_idx, u_char mode);
s_int32 mt_engine_start_tx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_operation *ops,
	u_char band_idx);
s_int32 mt_engine_stop_tx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_operation *ops,
	u_char band_idx);
s_int32 mt_engine_start_rx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_operation *ops,
	u_char band_idx);
s_int32 mt_engine_stop_rx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_operation *ops,
	u_char band_idx);

#endif /* __TEST_ENGINE_H__ */
