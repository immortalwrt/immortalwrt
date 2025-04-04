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
	ap_vow.c
*/

#include "rt_config.h"
#include "mcu/mt_cmd.h"

#define UMAC_DRR_TABLE_CTRL0            (0x00008388)

#define UMAC_DRR_TABLE_WDATA0           (0x00008340)
#define UMAC_DRR_TABLE_WDATA1           (0x00008344)
#define UMAC_DRR_TABLE_WDATA2           (0x00008348)
#define UMAC_DRR_TABLE_WDATA3           (0x0000834C)

/* Charge mode control control operation (0x8x : charge tx time & length) */

#define UMAC_CHARGE_BW_TOKEN_BIT_MASK                       BIT(0)
#define UMAC_CHARGE_BW_DRR_BIT_MASK                         BIT(1)
#define UMAC_CHARGE_AIRTIME_DRR_BIT_MASK                    BIT(2)
#define UMAC_CHARGE_ADD_MODE_BIT_MASK                       BIT(3)

#define UMAC_CHARGE_OP_BASE                                 0x80
#define UMAC_CHARGE_BW_TOKEN_OP_MASK                        (UMAC_CHARGE_OP_BASE | UMAC_CHARGE_BW_TOKEN_BIT_MASK)
#define UMAC_CHARGE_BW_DRR_OP_MASK                          (UMAC_CHARGE_OP_BASE | UMAC_CHARGE_BW_DRR_BIT_MASK)
#define UMAC_CHARGE_AIRTIME_DRR_OP_MASK                     (UMAC_CHARGE_OP_BASE | UMAC_CHARGE_AIRTIME_DRR_BIT_MASK)

#define UMAC_CHARGE_MODE_STA_ID_MASK                        BITS(0, 7)
#define UMAC_CHARGE_MODE_STA_ID_OFFSET                      0
#define UMAC_CHARGE_MODE_QUEUE_ID_MASK                      BITS(8, 11)
#define UMAC_CHARGE_MODE_QUEUE_ID_OFFSET                    8

#define UMAC_CHARGE_MODE_BSS_GROUP_MASK                     BITS(0, 3)
#define UMAC_CHARGE_MODE_BSS_GROUP_OFFSET                   0


#define UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_MASK           BITS(0, 15)
#define UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_OFFSET         0

#define UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_MASK             BITS(16, 31)
#define UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_OFFSET           16


/* Change MODE Ctrl operation */
#define UMAC_DRR_TABLE_CTRL0_CHANGE_MODE_MASK               BIT(23)
#define UMAC_DRR_TABLE_CTRL0_CHANGE_MODE_OFFSET             23


/* 00000340 DRR_Table_WData DRR table Wdata register register   00000000 */

#define UMAC_DRR_TABLE_WDATA0_STA_MODE_MASK                  BITS(0, 15)
#define UMAC_DRR_TABLE_WDATA0_STA_MODE_OFFSET                0

/* 00000350 DRR_Table_Rdata DRR table control register read data    00000000 */
#define UMAC_DRR_TABLE_RDATA0_STA_MODE_MASK                  BITS(0, 15)
#define UMAC_DRR_TABLE_RDATA0_STA_MODE_OFFSET                0

/* 00000388 DRR_Table_ctrl0     DRR table control register register 0   00000000 */

#define UMAC_DRR_TABLE_CTRL0_EXEC_MASK                      BIT(31)
#define UMAC_DRR_TABLE_CTRL0_EXEC_OFFSET                    31
#define UMAC_DRR_TABLE_CTRL0_MODE_OP_OFFSET                 16

#define UMAC_BSS_GROUP_NUMBER               16


#ifndef _LINUX_BITOPS_H
#define BIT(n)                          ((UINT32) 1 << (n))
#endif /* BIT */
#define BITS(m, n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))




#define VOW_DEF_AVA_AIRTIME (1000000)  /* us */

#define VOW_BSS_SETTING_BEGIN   16
#define VOW_BSS_SETTING_END     (VOW_BSS_SETTING_BEGIN + 16)

#define WF_WTBLON	0x29000
#define LPON_FREE_RUN	0x2b07c

/* global variables */
PRTMP_ADAPTER pvow_pad;
UINT32 vow_tx_time[MAX_LEN_OF_MAC_TABLE];
UINT32 vow_rx_time[MAX_LEN_OF_MAC_TABLE];
UINT32 vow_tx_ok[MAX_LEN_OF_MAC_TABLE];
UINT32 vow_tx_fail[MAX_LEN_OF_MAC_TABLE];
UINT32 vow_sum_tx_rx_time;
UINT32 vow_avg_sum_time;
UINT32 vow_last_tx_time[MAX_LEN_OF_MAC_TABLE];
UINT32 vow_last_rx_time[MAX_LEN_OF_MAC_TABLE];
UINT16 vow_idx;
UINT32 vow_tx_bss_byte[WMM_NUM_OF_AC];
UINT32 vow_rx_bss_byte[WMM_NUM_OF_AC];
UINT32 vow_tx_mbss_byte[VOW_MAX_GROUP_NUM];
UINT32 vow_rx_mbss_byte[VOW_MAX_GROUP_NUM];
UINT32 vow_ampdu_cnt;
UINT32 vow_interval;
UINT32 vow_last_free_cnt;

/* VOW internal commands */
/***********************************************************/
/*      EXT_CMD_ID_DRR_CTRL = 0x36                         */
/***********************************************************/
/* for station DWRR configration */
INT32 vow_set_sta(PRTMP_ADAPTER pad, UINT16 sta_id, UINT32 subcmd)
{
	EXT_CMD_VOW_DRR_CTRL_T sta_ctrl;
	UINT32 Setting = 0;
	INT32 ret;

	NdisZeroMemory(&sta_ctrl, sizeof(sta_ctrl));
	sta_ctrl.u4CtrlFieldID = subcmd;
	WCID_SET_H_L(sta_ctrl.ucStaIDHnVer, sta_ctrl.ucStaIDL, sta_id);

	switch (subcmd) {
	case ENUM_VOW_DRR_CTRL_FIELD_STA_ALL:
	case ENUM_VOW_DRR_CTRL_FIELD_STA_EXCLUDE_GROUP:
		/* station configration */
		Setting |= pad->vow_sta_cfg[sta_id].group;
		Setting |= (pad->vow_sta_cfg[sta_id].ac_change_rule << pad->vow_gen.VOW_STA_AC_PRIORITY_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BK] << pad->vow_gen.VOW_STA_WMM_AC0_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BE] << pad->vow_gen.VOW_STA_WMM_AC1_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VI] << pad->vow_gen.VOW_STA_WMM_AC2_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VO] << pad->vow_gen.VOW_STA_WMM_AC3_OFFSET);
		if (pad->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG)
			Setting |= ((pad->vow_sta_cfg[sta_id].group + UMAC_BWC_GROUP_MIN) << pad->vow_gen.VOW_STA_BWC_GROUP_OFFSET);
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(Setting);
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(SubCmd %x, Value = 0x%x)\n", subcmd, Setting);
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].group;
#ifdef RT_BIG_ENDIAN
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
		break;
	case ENUM_VOW_DRR_CTRL_FIELD_STA_BWC_GROUP:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].group + UMAC_BWC_GROUP_MIN;
#ifdef RT_BIG_ENDIAN
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
		break;
	case ENUM_VOW_DRR_CTRL_FIELD_STA_WMM_ID:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].wmm_idx;
#ifdef RT_BIG_ENDIAN
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
		break;
	case ENUM_VOW_DRR_CTRL_FIELD_STA_PRIORITY:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].ac_change_rule;
#ifdef RT_BIG_ENDIAN
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC0_QUA_ID:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BK];
#ifdef RT_BIG_ENDIAN
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC1_QUA_ID:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BE];
#ifdef RT_BIG_ENDIAN
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC2_QUA_ID:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VI];
#ifdef RT_BIG_ENDIAN
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC3_QUA_ID:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VO];
#ifdef RT_BIG_ENDIAN
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
		break;
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L1:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L2:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L3:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L4:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L5:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L6:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L7:
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_cfg.vow_sta_dwrr_quantum[subcmd - ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0];
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(SubCmd %x, Value = 0x%x)\n",
				  subcmd, pad->vow_cfg.vow_sta_dwrr_quantum[subcmd - ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0]);
#ifdef RT_BIG_ENDIAN
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL: {
		UINT32 i;

		/* station quantum configruation */
		for (i = 0; i < VOW_MAX_STA_DWRR_NUM; i++) {
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(vow_sta_dwrr_quantum[%d] = 0x%x)\n", i, pad->vow_cfg.vow_sta_dwrr_quantum[i]);
			sta_ctrl.rAirTimeCtrlValue.rAirTimeQuantumAllField.ucAirTimeQuantum[i] = pad->vow_cfg.vow_sta_dwrr_quantum[i];
		}

		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, Setting);
	}
	break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING: {
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_sta_cfg[sta_id].paused;
#ifdef RT_BIG_ENDIAN
		sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pad->vow_sta_cfg[sta_id].paused);
	}
	break;

	default:
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(No such command = 0x%x)\n", subcmd);
		break;
	}

	ret = MtCmdSetVoWDRRCtrl(pad, &sta_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_VOW_DRR_CTRL_T));
	return ret;
}

/* for DWRR max wait time configuration */
INT vow_set_sta_DWRR_max_time(PRTMP_ADAPTER pad)
{
	EXT_CMD_VOW_DRR_CTRL_T sta_ctrl;
	INT32 ret;

	NdisZeroMemory(&sta_ctrl, sizeof(sta_ctrl));
	sta_ctrl.u4CtrlFieldID = ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_DEFICIT_BOUND;
	sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_cfg.sta_max_wait_time;
#ifdef RT_BIG_ENDIAN
	sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(sta_max_wait_time = 0x%x)\n", pad->vow_cfg.sta_max_wait_time);
	ret = MtCmdSetVoWDRRCtrl(pad, &sta_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_VOW_DRR_CTRL_T));
	return ret;
}
/***********************************************************/
/*      EXT_CMD_ID_BSSGROUP_CTRL = 0x37                    */
/***********************************************************/
VOID vow_fill_group_all(PRTMP_ADAPTER pad, UINT8 group_id, EXT_CMD_BSS_CTRL_T *group_ctrl)
{
	/* DW0 */
	group_ctrl->arAllBssGroupMultiField[group_id].u2MinRateToken = pad->vow_bss_cfg[group_id].min_rate_token;
	group_ctrl->arAllBssGroupMultiField[group_id].u2MaxRateToken = pad->vow_bss_cfg[group_id].max_rate_token;
	/* DW1 */
	group_ctrl->arAllBssGroupMultiField[group_id].u4MinTokenBucketTimeSize = pad->vow_bss_cfg[group_id].min_airtimebucket_size;
	group_ctrl->arAllBssGroupMultiField[group_id].u4MinAirTimeToken = pad->vow_bss_cfg[group_id].min_airtime_token;
	group_ctrl->arAllBssGroupMultiField[group_id].u4MinTokenBucketLengSize = pad->vow_bss_cfg[group_id].min_ratebucket_size;
	/* DW2 */
	group_ctrl->arAllBssGroupMultiField[group_id].u4MaxTokenBucketTimeSize = pad->vow_bss_cfg[group_id].max_airtimebucket_size;
	group_ctrl->arAllBssGroupMultiField[group_id].u4MaxAirTimeToken = pad->vow_bss_cfg[group_id].max_airtime_token;
	group_ctrl->arAllBssGroupMultiField[group_id].u4MaxTokenBucketLengSize = pad->vow_bss_cfg[group_id].max_ratebucket_size;
	/* DW3 */
	group_ctrl->arAllBssGroupMultiField[group_id].u4MaxWaitTime = pad->vow_bss_cfg[group_id].max_wait_time;
	group_ctrl->arAllBssGroupMultiField[group_id].u4MaxBacklogSize = pad->vow_bss_cfg[group_id].max_backlog_size;
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(Group id = 0x%x, min_rate %d, max_rate %d, min_ratio %d, max_ratio %d)\n",
			 group_id,
			 pad->vow_bss_cfg[group_id].min_rate,
			 pad->vow_bss_cfg[group_id].max_rate,
			 pad->vow_bss_cfg[group_id].min_airtime_ratio,
			 pad->vow_bss_cfg[group_id].max_airtime_ratio);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min rate token = 0x%x)\n", pad->vow_bss_cfg[group_id].min_rate_token);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max rate token = 0x%x)\n", pad->vow_bss_cfg[group_id].max_rate_token);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min airtime token = 0x%x)\n", pad->vow_bss_cfg[group_id].min_airtime_token);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max airtime token = 0x%x)\n", pad->vow_bss_cfg[group_id].max_airtime_token);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min rate bucket = 0x%x)\n", pad->vow_bss_cfg[group_id].min_ratebucket_size);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max rate bucket = 0x%x)\n", pad->vow_bss_cfg[group_id].max_ratebucket_size);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min airtime bucket = 0x%x)\n", pad->vow_bss_cfg[group_id].min_airtimebucket_size);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max airtime bucket = 0x%x)\n", pad->vow_bss_cfg[group_id].max_airtimebucket_size);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max baclog size = 0x%x)\n", pad->vow_bss_cfg[group_id].max_backlog_size);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max wait time = 0x%x)\n", pad->vow_bss_cfg[group_id].max_wait_time);
}
/* for group configuration */
INT vow_set_group(PRTMP_ADAPTER pad, UINT8 group_id, UINT32 subcmd)
{
	EXT_CMD_BSS_CTRL_T group_ctrl;
	INT32 ret;

	NdisZeroMemory(&group_ctrl, sizeof(group_ctrl));
	group_ctrl.u4CtrlFieldID = subcmd;
	group_ctrl.ucBssGroupID = group_id;

	switch (subcmd) {
	/* group configuration */
	case ENUM_BSSGROUP_CTRL_ALL_ITEM_FOR_1_GROUP:
		vow_fill_group_all(pad, group_id, &group_ctrl);
		break;

	case ENUM_BSSGROUP_CTRL_MIN_RATE_TOKEN_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_rate_token;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, group_ctrl.u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_RATE_TOKEN_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_rate_token;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, group_ctrl.u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_airtimebucket_size;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, group_ctrl.u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MIN_AIRTIME_TOKEN_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_airtime_token;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, group_ctrl.u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_ratebucket_size;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, group_ctrl.u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_airtimebucket_size;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, group_ctrl.u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_AIRTIME_TOKEN_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_airtime_token;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, group_ctrl.u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_ratebucket_size;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, group_ctrl.u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_WAIT_TIME_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_wait_time;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, group_ctrl.u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_BACKLOG_SIZE_CFG_ITEM:
		group_ctrl.u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_backlog_size;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, group_ctrl.u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_ALL_ITEM_FOR_ALL_GROUP: {
		UINT32 i;

		for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
			vow_fill_group_all(pad, i, &group_ctrl);
	}
	break;

	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_00:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_01:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_02:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_03:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_04:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_05:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_06:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_07:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_08:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_09:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0A:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0B:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0C:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0D:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0E:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0F:
		/* Group DWRR quantum */
		group_ctrl.ucBssGroupQuantumTime[group_id] = pad->vow_bss_cfg[group_id].dwrr_quantum;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group %d DWRR quantum = 0x%x)\n", group_id, pad->vow_bss_cfg[group_id].dwrr_quantum);
		break;

	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_ALL: {
		UINT32 i;

		for (i = 0; i < VOW_MAX_GROUP_NUM; i++) {
			group_ctrl.ucBssGroupQuantumTime[i] = pad->vow_bss_cfg[i].dwrr_quantum;
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group %d DWRR quantum = 0x%x)\n", i, pad->vow_bss_cfg[i].dwrr_quantum);
		}
	}
	break;

	default:
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(No such command = 0x%x)\n", subcmd);
		break;
	}

	ret = MtCmdSetVoWGroupCtrl(pad, &group_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_BSS_CTRL_T));
	return ret;
}

/* for DWRR max wait time configuration */
INT vow_set_group_DWRR_max_time(PRTMP_ADAPTER pad)
{
	EXT_CMD_VOW_DRR_CTRL_T sta_ctrl;
	INT32 ret;

	if (!(pad->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCTRL))
		return 0;

	NdisZeroMemory(&sta_ctrl, sizeof(sta_ctrl));
	sta_ctrl.u4CtrlFieldID = ENUM_VOW_DRR_CTRL_FIELD_BW_DEFICIT_BOUND;
	sta_ctrl.rAirTimeCtrlValue.u4ComValue = pad->vow_cfg.group_max_wait_time;
#ifdef RT_BIG_ENDIAN
	sta_ctrl.rAirTimeCtrlValue.u4ComValue = cpu2le32(sta_ctrl.rAirTimeCtrlValue.u4ComValue);
#endif
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group_max_wait_time = 0x%x)\n", pad->vow_cfg.group_max_wait_time);
	ret = MtCmdSetVoWDRRCtrl(pad, &sta_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_VOW_DRR_CTRL_T));
	return ret;
}

/***********************************************************/
/*      EXT_CMD_ID_VOW_FEATURE_CTRL = 0x38                 */
/***********************************************************/
/* for group configuration */
INT vow_set_feature_all(PRTMP_ADAPTER pad)
{
	EXT_CMD_VOW_FEATURE_CTRL_T feature_ctrl;
	INT32 ret, i;

	NdisZeroMemory(&feature_ctrl, sizeof(feature_ctrl));
	/* DW0 - flags */
	feature_ctrl.u2IfApplyBss_0_to_16_CtrlFlag = 0xFFFF; /* 16'b */
	feature_ctrl.u2IfApplyRefillPerildFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyDbdc1SearchRuleFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyDbdc0SearchRuleFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyEnTxopNoChangeBssFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyAirTimeFairnessFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyWeightedAirTimeFairnessFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyEnbwrefillFlag = TRUE; /* 1'b */
	feature_ctrl.u2IfApplyEnbwCtrlFlag = TRUE; /* 1'b */
	feature_ctrl.u4IfApplyKeepQuantumFlag = TRUE; /* 1'b */
	/* DW1 - flags */
	feature_ctrl.u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag = 0xFFFF;
	/* DW2 - flags */
	feature_ctrl.u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag = 0xFFFF;
	/* DW5 - ctrl values */
	feature_ctrl.u2Bss_0_to_16_CtrlValue = pad->vow_cfg.per_bss_enable; /* 16'b */
	feature_ctrl.u2RefillPerildValue = pad->vow_cfg.refill_period; /* 8'b */
	feature_ctrl.u2Dbdc1SearchRuleValue = pad->vow_cfg.dbdc1_search_rule; /* 1'b */
	feature_ctrl.u2Dbdc0SearchRuleValue = pad->vow_cfg.dbdc0_search_rule; /* 1'b */
	feature_ctrl.u2WeightedAirTimeFairnessValue = pad->vow_watf_en; /* 1'b */
	feature_ctrl.u2EnTxopNoChangeBssValue = pad->vow_cfg.en_txop_no_change_bss; /* 1'b */
	feature_ctrl.u2AirTimeFairnessValue = pad->vow_cfg.en_airtime_fairness; /* 1'b */
	feature_ctrl.u2EnbwrefillValue = pad->vow_cfg.en_bw_refill; /* 1'b */
	feature_ctrl.u2EnbwCtrlValue = pad->vow_cfg.en_bw_ctrl; /* 1'b */
	feature_ctrl.u4KeepQuantumValue = pad->vow_misc_cfg.keep_quantum; /* 1'b */

	/* DW6 - ctrl values */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
		feature_ctrl.u2BssCheckTimeToken_0_to_16_CtrlValue |= (pad->vow_bss_cfg[i].at_on << i);

	/* DW7 - ctrl values */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
		feature_ctrl.u2BssCheckLengthToken_0_to_16_CtrlValue |= (pad->vow_bss_cfg[i].bw_on << i);

	if ((pad->vow_gen.VOW_GEN == VOW_GEN_2) ||
	    (pad->vow_gen.VOW_GEN == VOW_GEN_TALOS) ||
	    (pad->vow_gen.VOW_GEN == VOW_GEN_FALCON)) {
		/* DW8 - misc */
		feature_ctrl.u4IfApplyStaLockForRtsFlag = TRUE; /* 1'b */
		feature_ctrl.u4RtsStaLockValue = pad->vow_misc_cfg.rts_sta_lock; /* 1'b */
		/* VOW is disabled, skip all setting */
		if (vow_is_enabled(pad)) {
			feature_ctrl.u4IfApplyTxCountModeFlag = TRUE; /* 1'b */
			feature_ctrl.u4TxCountValue = pad->vow_misc_cfg.tx_rr_count; /* 4'b */
			feature_ctrl.u4IfApplyTxMeasurementModeFlag = TRUE; /* 1'b */
			feature_ctrl.u4TxMeasurementModeValue = pad->vow_misc_cfg.measurement_mode; /* 1'b */
			feature_ctrl.u4IfApplyTxBackOffBoundFlag = TRUE; /* 1'b */
			feature_ctrl.u4TxBackOffBoundEnable = pad->vow_misc_cfg.max_backoff_bound_en; /* 1'b */
			feature_ctrl.u4TxBackOffBoundValue = pad->vow_misc_cfg.max_backoff_bound; /* 4'b */
			feature_ctrl.u4IfApplyRtsFailedChargeDisFlag = TRUE; /* 1'b */
			feature_ctrl.u4RtsFailedChargeDisValue = pad->vow_misc_cfg.rts_failed_charge_time_en; /* 1'b */
			feature_ctrl.u4IfApplyRxEifsToZeroFlag = TRUE; /* 1'b */
			feature_ctrl.u4ApplyRxEifsToZeroValue =	pad->vow_misc_cfg.zero_eifs_time; /* 1'b */
			feature_ctrl.u4IfApplyRxRifsModeforCckCtsFlag = TRUE; /* 1'b */
			feature_ctrl.u4RxRifsModeforCckCtsValue = pad->vow_misc_cfg.rx_rifs_mode; /* 1'b */
			feature_ctrl.u4IfApplyKeepVoWSettingForSerFlag = TRUE; /* 1'b */
			feature_ctrl.u4VowKeepSettingValue = pad->vow_misc_cfg.keep_vow_sram_setting; /* 1'b */
			feature_ctrl.u4VowKeepSettingBit = pad->vow_misc_cfg.keep_vow_sram_setting_bit; /* 1'b */
			if (pad->vow_gen.VOW_GEN < VOW_GEN_FALCON) {
				feature_ctrl.u4IfApplySplFlag = TRUE; /* 1'b */
				feature_ctrl.u4SplStaNumValue = pad->vow_misc_cfg.spl_sta_count; /* 3'b */
			}
		}
	}

	feature_ctrl.u4DbgPrnLvl = (pad->vow_show_en == 0) ? 0 : (pad->vow_show_en - 1 );

	/* DW9 - schedule */
	feature_ctrl.u4IfApplyVowSchCtrl = pad->vow_sch_cfg.apply_sch_ctrl;
	feature_ctrl.u4VowScheduleType = pad->vow_sch_cfg.sch_type;
	feature_ctrl.u4VowSchedulePolicy = pad->vow_sch_cfg.sch_policy;

	if (pad->vow_gen.VOW_GEN == VOW_GEN_FALCON) {
		if (vow_is_enabled(pad)) {
			feature_ctrl.u4IfApplyRxEifsToZeroFlag = TRUE; /* 1'b */
			feature_ctrl.u4ApplyRxEifsToZeroValue =	pad->vow_misc_cfg.zero_eifs_time; /* 1'b */
		}
	}

	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2Bss_0_to_16_CtrlValue  = 0x%x)\n", pad->vow_cfg.per_bss_enable);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2RefillPerildValue = 0x%x)\n", pad->vow_cfg.refill_period);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2Dbdc1SearchRuleValue = 0x%x)\n", pad->vow_cfg.dbdc1_search_rule);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2Dbdc0SearchRuleValue = 0x%x)\n", pad->vow_cfg.dbdc0_search_rule);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2EnTxopNoChangeBssValue = 0x%x)\n", pad->vow_cfg.en_txop_no_change_bss);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2AirTimeFairnessValue = 0x%x)\n", pad->vow_cfg.en_airtime_fairness);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2EnbwrefillValue = 0x%x)\n", pad->vow_cfg.en_bw_refill);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2EnbwCtrlValue = 0x%x)\n", pad->vow_cfg.en_bw_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2WeightedAirTimeFairnessValue = 0x%x)\n", pad->vow_watf_en);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2BssCheckTimeToken_0_to_16_CtrlValue = 0x%x)\n", feature_ctrl.u2BssCheckTimeToken_0_to_16_CtrlValue);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2BssCheckLengthToken_0_to_16_CtrlValue = 0x%x)\n", feature_ctrl.u2BssCheckLengthToken_0_to_16_CtrlValue);
	ret = MtCmdSetVoWFeatureCtrl(pad, &feature_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_VOW_FEATURE_CTRL_T));
	return ret;
}

/***********************************************************/
/*      EXT_CMD_ID_RX_AIRTIME_CTRL = 0x4a                  */
/***********************************************************/
/* for RX airtime configuration */
INT vow_set_rx_airtime(PRTMP_ADAPTER pad, UINT8 cmd, UINT32 subcmd)
{
	EXT_CMD_RX_AT_CTRL_T                rx_at_ctrl;
	INT32 ret;
	/* init structure to zero */
	NdisZeroMemory(&rx_at_ctrl, sizeof(rx_at_ctrl));
	/* assign cmd and subcmd */
	rx_at_ctrl.u4CtrlFieldID = cmd;
	rx_at_ctrl.u4CtrlSubFieldID = subcmd;

	switch (cmd) {
	/* RX airtime feature control */
	case ENUM_RX_AT_FEATURE_CTRL:
		switch (subcmd) {
		case ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN:
			rx_at_ctrl.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.fgRxAirTimeEn = pad->vow_rx_time_cfg.rx_time_en;
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(cmd = 0x%x, subcmd = 0x%x, value = 0x%x)\n",
					 cmd, subcmd, pad->vow_rx_time_cfg.rx_time_en);
			break;

		case ENUM_RX_AT_FEATURE_SUB_TYPE_MIBTIME_EN:
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(Not implemented yet = 0x%x)\n", subcmd);
			break;

		case ENUM_RX_AT_FEATURE_SUB_TYPE_EARLYEND_EN:
			rx_at_ctrl.rRxAtGeneralCtrl.rRxAtFeatureSubCtrl.fgRxEarlyEndEn = pad->vow_rx_time_cfg.rx_early_end_en;
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(cmd = 0x%x, subcmd = 0x%x, value = 0x%x)\n",
					 cmd, subcmd, pad->vow_rx_time_cfg.rx_early_end_en);
			break;

		default:
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such sub command = 0x%x)\n", subcmd);
		}

		break;

	case ENUM_RX_AT_BITWISE_CTRL:
		switch (subcmd) {
		case ENUM_RX_AT_BITWISE_SUB_TYPE_AIRTIME_CLR: /* clear all RX airtime counters */
			rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.fgRxAirTimeClrEn = TRUE;
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(cmd = 0x%x, subcmd = 0x%x, value = 0x%x)\n",
					 cmd, subcmd, rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.fgRxAirTimeClrEn);
			break;

		case ENUM_RX_AT_BITWISE_SUB_TYPE_MIBTIME_CLR:
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(Not implemented yet = 0x%x)\n", subcmd);
			break;

		default:
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such sub command = 0x%x)\n", subcmd);
		}

		break;

	case ENUM_RX_AT_TIMER_VALUE_CTRL:
		switch (subcmd) {
		case ENUM_RX_AT_TIME_VALUE_SUB_TYPE_ED_OFFSET_CTRL:
			rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.ucEdOffsetValue = pad->vow_rx_time_cfg.ed_offset;
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(cmd = 0x%x, subcmd =  0x%x, value = 0x%x)\n",
					 cmd, subcmd, pad->vow_rx_time_cfg.ed_offset);
			break;

		default:
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such sub command = 0x%x)\n", subcmd);
		}

		break;

	case EMUM_RX_AT_REPORT_CTRL:
		switch (subcmd) {

		default:
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such sub command = 0x%x)\n", subcmd);
		}

		break;

	default:
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(No such command = 0x%x)\n", subcmd);
		break;
	}

	ret = MtCmdSetVoWRxAirtimeCtrl(pad, &rx_at_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_RX_AT_CTRL_T));
	return ret;
}

/* select RX WMM backoff time for 4 OM */
INT vow_set_wmm_selection(PRTMP_ADAPTER pad, UINT8 om)
{
	EXT_CMD_RX_AT_CTRL_T                rx_at_ctrl;
	INT32 ret;
	/* init structure to zero */
	NdisZeroMemory(&rx_at_ctrl, sizeof(rx_at_ctrl));
	/* assign cmd and subcmd */
	rx_at_ctrl.u4CtrlFieldID = ENUM_RX_AT_BITWISE_CTRL;
	rx_at_ctrl.u4CtrlSubFieldID = ENUM_RX_AT_BITWISE_SUB_TYPE_STA_WMM_CTRL;
	rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.ucOwnMacID = om;
	rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.fgtoApplyWm00to03MibCfg = pad->vow_rx_time_cfg.wmm_backoff_sel[om];
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(cmd = 0x%x, subcmd = 0x%x, OM = 0x%x, Map = 0x%x)\n",
			 rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.ucOwnMacID,
			 ENUM_RX_AT_BITWISE_SUB_TYPE_STA_WMM_CTRL, om,
			 pad->vow_rx_time_cfg.wmm_backoff_sel[om]);
	ret = MtCmdSetVoWRxAirtimeCtrl(pad, &rx_at_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n",
			ret, sizeof(EXT_CMD_RX_AT_CTRL_T));
	return ret;
}

/* set 16 MBSS  mapping to 4 RX backoff time configurations */
INT vow_set_mbss2wmm_map(PRTMP_ADAPTER pad, UINT8 bss_idx)
{
	EXT_CMD_RX_AT_CTRL_T                rx_at_ctrl;
	INT32 ret;
	/* init structure to zero */
	NdisZeroMemory(&rx_at_ctrl, sizeof(rx_at_ctrl));
	/* assign cmd and subcmd */
	rx_at_ctrl.u4CtrlFieldID = ENUM_RX_AT_BITWISE_CTRL;
	rx_at_ctrl.u4CtrlSubFieldID = ENUM_RX_AT_BITWISE_SUB_TYPE_MBSS_WMM_CTRL;
	rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.ucMbssGroup = bss_idx;
	rx_at_ctrl.rRxAtGeneralCtrl.rRxAtBitWiseSubCtrl.ucWmmGroup = pad->vow_rx_time_cfg.bssid2wmm_set[bss_idx];
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(cmd = 0x%x, subcmd = 0x%x, bss_idx = 0x%x, Map = 0x%x)\n",
			 rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, bss_idx,
			 pad->vow_rx_time_cfg.bssid2wmm_set[bss_idx]);
	ret = MtCmdSetVoWRxAirtimeCtrl(pad, &rx_at_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n",
			ret, sizeof(EXT_CMD_RX_AT_CTRL_T));
	return ret;
}

/* set backoff time for RX*/
INT vow_set_backoff_time(PRTMP_ADAPTER pad, UINT8 target)
{
	EXT_CMD_RX_AT_CTRL_T                rx_at_ctrl;
	INT32 ret;
	/* init structure to zero */
	NdisZeroMemory(&rx_at_ctrl, sizeof(rx_at_ctrl));
	/* assign cmd and subcmd */
	rx_at_ctrl.u4CtrlFieldID = ENUM_RX_AT_TIMER_VALUE_CTRL;
	rx_at_ctrl.u4CtrlSubFieldID = ENUM_RX_AT_TIME_VALUE_SUB_TYPE_BACKOFF_TIMER;
	rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackoffWmmGroupIdx = target;

	switch (target) {
	case ENUM_RX_AT_WMM_GROUP_0:
	case ENUM_RX_AT_WMM_GROUP_1:
	case ENUM_RX_AT_WMM_GROUP_2:
	case ENUM_RX_AT_WMM_GROUP_3:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BK];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC1Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BE];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC2Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VI];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC3Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VO];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxAtBackoffAcQMask =
			(ENUM_RX_AT_AC_Q0_MASK_T | ENUM_RX_AT_AC_Q1_MASK_T | ENUM_RX_AT_AC_Q2_MASK_T | ENUM_RX_AT_AC_Q3_MASK_T);
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, group = 0x%x, BK = 0x%x, BE = 0x%x, VI = 0x%x, VO = 0x%x)\n",
				 rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target,
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BK],
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BE],
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VI],
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VO]);
		break;

	case ENUM_RX_AT_WMM_GROUP_PEPEATER:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BK];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC1Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BE];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC2Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VI];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC3Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VO];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxAtBackoffAcQMask =
			(ENUM_RX_AT_AC_Q0_MASK_T | ENUM_RX_AT_AC_Q1_MASK_T | ENUM_RX_AT_AC_Q2_MASK_T | ENUM_RX_AT_AC_Q3_MASK_T);
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, group = 0x%x, BK = 0x%x, BE = 0x%x, VI = 0x%x, VO = 0x%x)\n",
				 rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target,
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BK],
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BE],
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VI],
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VO]);
		break;

	case ENUM_RX_AT_WMM_GROUP_STA:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BK];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC1Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BE];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC2Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VI];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC3Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VO];
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxAtBackoffAcQMask =
			(ENUM_RX_AT_AC_Q0_MASK_T | ENUM_RX_AT_AC_Q1_MASK_T | ENUM_RX_AT_AC_Q2_MASK_T | ENUM_RX_AT_AC_Q3_MASK_T);
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, group = 0x%x, BK = 0x%x, BE = 0x%x, VI = 0x%x, VO = 0x%x)\n",
				 rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target,
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BK],
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BE],
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VI],
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VO]);
		break;

	case ENUM_RX_AT_NON_QOS:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff =
			pad->vow_rx_time_cfg.non_qos_backoff;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, group = 0x%x, backoff time = 0x%x)\n",
				 rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target,
				 pad->vow_rx_time_cfg.non_qos_backoff);
		break;

	case ENUM_RX_AT_OBSS:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtTimeValueSubCtrl.rRxATBackOffCfg.u2AC0Backoff =
			pad->vow_rx_time_cfg.obss_backoff;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, group = 0x%x, backoff time = 0x%x)\n",
				 rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target,
				 pad->vow_rx_time_cfg.obss_backoff);
		break;

	default:
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(No such command = 0x%x)\n", target);
		break;
	}

	ret = MtCmdSetVoWRxAirtimeCtrl(pad, &rx_at_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_RX_AT_CTRL_T));
	return ret;
}

/* set backoff time for RX*/
INT vow_get_rx_time_counter(PRTMP_ADAPTER pad, UINT8 target, UINT8 band_idx)
{
	EXT_CMD_RX_AT_CTRL_T                rx_at_ctrl;
	INT32 ret;
	/* init structure to zero */
	NdisZeroMemory(&rx_at_ctrl, sizeof(rx_at_ctrl));
	/* assign cmd and subcmd */
	rx_at_ctrl.u4CtrlFieldID = EMUM_RX_AT_REPORT_CTRL;
	rx_at_ctrl.u4CtrlSubFieldID = target;

	switch (target) {
	case ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtReportSubCtrl.ucRxNonWiFiBandIdx = band_idx;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, target = 0x%x, band_idx = 0x%x)\n",
				 rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target, band_idx);
		break;

	case ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME:
		rx_at_ctrl.rRxAtGeneralCtrl.rRxAtReportSubCtrl.ucRxObssBandIdx = band_idx;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, target = 0x%x, band_idx = 0x%x)\n",
				 rx_at_ctrl.u4CtrlFieldID, rx_at_ctrl.u4CtrlSubFieldID, target, band_idx);
		break;

	case ENUM_RX_AT_REPORT_SUB_TYPE_MIB_OBSS_TIME:
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(Not implemented yet = 0x%x)\n", target);
		break;

	default:
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(No such command = 0x%x)\n", target);
	}

	ret = MtCmdGetVoWRxAirtimeCtrl(pad, &rx_at_ctrl);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_RX_AT_CTRL_T));

	if (target == ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME)
		return rx_at_ctrl.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxNonWiFiBandTimer;
	else if (target == ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME)
		return rx_at_ctrl.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxObssBandTimer;
	else if (target == ENUM_RX_AT_REPORT_SUB_TYPE_MIB_OBSS_TIME)
		return rx_at_ctrl.rRxAtGeneralCtrl.rRxAtReportSubCtrl.u4RxMibObssBandTimer;
	else
		return -1;
}
/***********************************************************/
/*      EXT_CMD_ID_AT_PROC_MODULE = 0x4b                   */
/***********************************************************/

/* for airtime estimator module */
INT vow_set_at_estimator(PRTMP_ADAPTER pad, UINT32 subcmd)
{
	EXT_CMD_AT_PROC_MODULE_CTRL_T   at_proc;
	INT32   ret;
	UINT16	u4CtrlFieldID;
	/* init structure to zero */
	NdisZeroMemory(&at_proc, sizeof(at_proc));
	/* assign cmd and subcmd */
	u4CtrlFieldID = ENUM_AT_RPOCESS_ESTIMATE_MODULE_CTRL;
	at_proc.u4CtrlFieldID = u4CtrlFieldID;
	at_proc.u4CtrlSubFieldID = cpu2le16(subcmd);
#ifdef RT_BIG_ENDIAN
	at_proc.u4CtrlFieldID = cpu2le16(at_proc.u4CtrlFieldID);
#endif

	switch (subcmd) {
	case ENUM_AT_PROC_EST_FEATURE_CTRL:
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.fgAtEstimateOnOff = pad->vow_at_est.at_estimator_en;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 u4CtrlFieldID, subcmd, pad->vow_at_est.at_estimator_en);
		break;

	case ENUM_AT_PROC_EST_MONITOR_PERIOD_CTRL:
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u2AtEstMonitorPeriod = cpu2le16(pad->vow_at_est.at_monitor_period);
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 u4CtrlFieldID, subcmd, pad->vow_at_est.at_monitor_period);
		break;

	default:
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(No such command = 0x%x)\n", subcmd);
	}

	ret = MtCmdSetVoWModuleCtrl(pad, &at_proc);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_AT_PROC_MODULE_CTRL_T));
	return ret;
}

INT vow_set_at_estimator_group(PRTMP_ADAPTER pad, UINT32 subcmd, UINT8 group_id)
{
	EXT_CMD_AT_PROC_MODULE_CTRL_T   at_proc;
	INT32   ret;
	UINT16	u4CtrlFieldID;
	/* init structure to zero */
	NdisZeroMemory(&at_proc, sizeof(at_proc));
	/* assign cmd and subcmd */
	u4CtrlFieldID = ENUM_AT_RPOCESS_ESTIMATE_MODULE_CTRL;
	at_proc.u4CtrlFieldID = ENUM_AT_RPOCESS_ESTIMATE_MODULE_CTRL;
	at_proc.u4CtrlSubFieldID = subcmd;
#ifdef RT_BIG_ENDIAN
	at_proc.u4CtrlFieldID = cpu2le16(at_proc.u4CtrlFieldID);
	at_proc.u4CtrlSubFieldID = cpu2le16(at_proc.u4CtrlSubFieldID);
#endif

	switch (subcmd) {
	case ENUM_AT_PROC_EST_GROUP_RATIO_CTRL:
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u4GroupRatioBitMask |= (1UL << group_id);
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u2GroupMaxRatioValue[group_id] = cpu2le16(pad->vow_bss_cfg[group_id].max_airtime_ratio);
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u2GroupMinRatioValue[group_id] = cpu2le16(pad->vow_bss_cfg[group_id].min_airtime_ratio);
#ifdef RT_BIG_ENDIAN
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u4GroupRatioBitMask =
			cpu2le32(at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.u4GroupRatioBitMask);
#endif
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, group %d, val = 0x%x/0x%x)\n",
				 u4CtrlFieldID, subcmd, group_id,
				 pad->vow_bss_cfg[group_id].max_airtime_ratio,
				 pad->vow_bss_cfg[group_id].min_airtime_ratio);
		break;

	case ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING:
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.ucGrouptoSelectBand = group_id;
		at_proc.rAtProcGeneralCtrl.rAtEstimateSubCtrl.ucBandSelectedfromGroup = pad->vow_bss_cfg[group_id].band_idx;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, group %d, val = 0x%x)\n",
				 u4CtrlFieldID, subcmd, group_id, pad->vow_bss_cfg[group_id].band_idx);
		break;

	default:
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(No such command = 0x%x)\n", subcmd);
	}

	ret = MtCmdSetVoWModuleCtrl(pad, &at_proc);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_AT_PROC_MODULE_CTRL_T));
	return ret;
}

/* for bad node detector */
INT vow_set_bad_node(PRTMP_ADAPTER pad, UINT32 subcmd)
{
	EXT_CMD_AT_PROC_MODULE_CTRL_T   at_proc;
	INT32   ret;
	UINT16	u4CtrlFieldID;
	/* init structure to zero */
	NdisZeroMemory(&at_proc, sizeof(at_proc));
	/* assign cmd and subcmd */
	u4CtrlFieldID = ENUM_AT_RPOCESS_BAD_NODE_MODULE_CTRL;
#ifdef RT_BIG_ENDIAN
	at_proc.u4CtrlFieldID = cpu2le16(u4CtrlFieldID);
#endif
	at_proc.u4CtrlSubFieldID = cpu2le16(subcmd);

	switch (subcmd) {
	case ENUM_AT_PROC_BAD_NODE_FEATURE_CTRL:
		at_proc.rAtProcGeneralCtrl.rAtBadNodeSubCtrl.fgAtBadNodeOnOff = pad->vow_badnode.bn_en;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 u4CtrlFieldID, subcmd, pad->vow_badnode.bn_en);
		break;

	case ENUM_AT_PROC_BAD_NODE_MONITOR_PERIOD_CTRL:
		at_proc.rAtProcGeneralCtrl.rAtBadNodeSubCtrl.u2AtBadNodeMonitorPeriod = cpu2le16(pad->vow_badnode.bn_monitor_period);
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 u4CtrlFieldID, subcmd, pad->vow_badnode.bn_monitor_period);
		break;

	case ENUM_AT_PROC_BAD_NODE_FALLBACK_THRESHOLD:
		at_proc.rAtProcGeneralCtrl.rAtBadNodeSubCtrl.ucFallbackThreshold = pad->vow_badnode.bn_fallback_threshold;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 u4CtrlFieldID, subcmd, pad->vow_badnode.bn_fallback_threshold);
		break;

	case ENUM_AT_PROC_BAD_NODE_PER_THRESHOLD:
		at_proc.rAtProcGeneralCtrl.rAtBadNodeSubCtrl.ucTxPERThreshold = pad->vow_badnode.bn_per_threshold;
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"(cmd = 0x%x, subcmd = 0x%x, val = 0x%x)\n",
				 u4CtrlFieldID, subcmd, pad->vow_badnode.bn_per_threshold);
		break;

	default:
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(No such command = 0x%x)\n", subcmd);
	}

	ret = MtCmdSetVoWModuleCtrl(pad, &at_proc);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_AT_PROC_MODULE_CTRL_T));
	return ret;
}

void vow_dump_umac_CRs(PRTMP_ADAPTER pad)
{
	int i;

	for (i = 0x8340; i < 0x83c0; i += 4) {
		UINT32 val = 0;

		RTMP_IO_READ32(pad->hdev_ctrl, i, &val);
		MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "0x%0x -> 0x%0x\n", i, val);
	}
}
/* ---------------------- end -------------------------------*/
BOOLEAN vow_is_supported(PRTMP_ADAPTER pad)
{
#ifdef VOW_SUPPORT
	return TRUE;
#else
	return FALSE;
#endif
}

BOOLEAN vow_is_enabled(PRTMP_ADAPTER pad)
{
#ifdef VOW_SUPPORT
	return (pad->vow_cfg.en_bw_ctrl || pad->vow_cfg.en_airtime_fairness);
#else
	return FALSE;
#endif
}

BOOLEAN vow_atf_is_enabled(PRTMP_ADAPTER pad)
{
#ifdef VOW_SUPPORT
	return pad->vow_cfg.en_airtime_fairness;
#else
	return FALSE;
#endif
}

BOOLEAN vow_watf_is_enabled(
	IN PRTMP_ADAPTER pad)
{
#ifdef VOW_SUPPORT
	return pad->vow_watf_en;
#else
	return FALSE;
#endif
}

void vow_variable_reset(PRTMP_ADAPTER pAd)
{
	int i;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	pAd->vow_gen.VOW_FEATURE = 0;
	pAd->vow_gen.VOW_FEATURE |= VOW_FEATURE_ATF;
	pAd->vow_gen.VOW_FEATURE |= VOW_FEATURE_BWCTRL;
	if ((!(IS_MT7915(pAd) || IS_MT7915_6E(pAd))) && (pAd->vow_gen.VOW_GEN >= VOW_GEN_FALCON))
		pAd->vow_gen.VOW_FEATURE |= VOW_FEATURE_BWCG;

	if (pAd->vow_gen.VOW_GEN == VOW_GEN_TALOS) {
		pAd->vow_gen.VOW_FEATURE |= VOW_FEATURE_SPL;
	}

	os_zero_mem(&(pAd->bss_group), sizeof(struct bss_group_rec));

	for (i = 0; i < VOW_GROUP_TABLE_MAX; i++)
		pAd->bss_group.group_idx[i] = 0;

	pAd->max_bssgroup_num = VOW_MAX_GROUP_NUM;
	if (pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG) {
		for (i = 0; i < VOW_GROUP_TABLE_MAX; i++) {
			pAd->bss_group.bw_group_idx[i] = VOW_MAX_GROUP_NUM;
		}
	}

	/* RX airtime */
	pAd->vow_rx_time_cfg.rx_time_en = TRUE;
	pAd->vow_rx_time_cfg.ed_offset = VOW_DEF_ED_OFFSET;
	pAd->vow_rx_time_cfg.obss_backoff = VOW_DEF_ABAND_OBSS_BACKOFF;
	pAd->vow_rx_time_cfg.non_qos_backoff = VOW_DEF_NON_QOS_BACKOFF;

	/*FIXME: depends on real WMM set mapping */
	for (i = 0; i < VOW_MAX_WMM_SET_NUM; i++) {
		pAd->vow_rx_time_cfg.wmm_backoff[i][WMM_AC_BK] = VOW_DEF_ABAND_BK_BACKOFF;
		pAd->vow_rx_time_cfg.wmm_backoff[i][WMM_AC_BE] = VOW_DEF_ABAND_BE_BACKOFF;
		pAd->vow_rx_time_cfg.wmm_backoff[i][WMM_AC_VI] = VOW_DEF_ABAND_VI_BACKOFF;
		pAd->vow_rx_time_cfg.wmm_backoff[i][WMM_AC_VO] = VOW_DEF_ABAND_VO_BACKOFF;
		pAd->vow_rx_time_cfg.wmm_backoff_sel[i] = VOW_WMM_ONE2ONE_MAPPING;
	}

	/*FIXME: depends on real WMM set mapping */
	pAd->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BK] = VOW_DEF_ABAND_BK_BACKOFF;
	pAd->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BE] = VOW_DEF_ABAND_BE_BACKOFF;
	pAd->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VI] = VOW_DEF_ABAND_VI_BACKOFF;
	pAd->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VO] = VOW_DEF_ABAND_VO_BACKOFF;
	/*FIXME: depends on real WMM set mapping */
	pAd->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BK] = VOW_DEF_ABAND_BK_BACKOFF;
	pAd->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BE] = VOW_DEF_ABAND_BE_BACKOFF;
	pAd->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VI] = VOW_DEF_ABAND_VI_BACKOFF;
	pAd->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VO] = VOW_DEF_ABAND_VO_BACKOFF;

	/*FIXME: depends on real BSS mapping */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
		pAd->vow_rx_time_cfg.bssid2wmm_set[i] = VOW_WMM_SET0;   /* default WMM SET0 */

	if (!vow_is_supported(pAd))
		return;

	/* VOW control */
	pAd->vow_cfg.en_bw_ctrl = FALSE;
	pAd->vow_cfg.en_bw_refill = TRUE;
	pAd->vow_cfg.en_airtime_fairness = TRUE;
	pAd->vow_cfg.en_txop_no_change_bss = TRUE;
	pAd->vow_cfg.dbdc0_search_rule = VOW_WMM_SET_FIRST;
	pAd->vow_cfg.dbdc1_search_rule = VOW_WMM_SET_FIRST;
	pAd->vow_cfg.refill_period = VOW_REFILL_PERIOD_32US; /* 32us */
	pAd->vow_cfg.per_bss_enable = 0xFFFF; /* enable all 16 group */
	pAd->vow_cfg.sta_max_wait_time = VOW_DEF_STA_MAX_WAIT_TIME;
	pAd->vow_cfg.group_max_wait_time = VOW_DEF_BSS_MAX_WAIT_TIME;
	pAd->vow_avg_num = 30; /* 20 packets */

	/* group(BSS) configuration */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++) {
		pAd->vow_bss_cfg[i].min_rate = 10; /* Mbps */
		pAd->vow_bss_cfg[i].max_rate = 30; /* Mbps */
		pAd->vow_bss_cfg[i].min_airtime_ratio = 5; /* % */
		pAd->vow_bss_cfg[i].max_airtime_ratio = 10; /* % */
		pAd->vow_bss_cfg[i].min_ratebucket_size = VOW_DEF_MIN_RATE_BUCKET_SIZE;
		pAd->vow_bss_cfg[i].max_ratebucket_size = VOW_DEF_MAX_RATE_BUCKET_SIZE;
		pAd->vow_bss_cfg[i].max_backlog_size = VOW_DEF_BACKLOG_SIZE;
		pAd->vow_bss_cfg[i].min_airtimebucket_size = VOW_DEF_MIN_AIRTIME_BUCKET_SIZE;
		pAd->vow_bss_cfg[i].max_airtimebucket_size = VOW_DEF_MAX_AIRTIME_BUCKET_SIZE;
		pAd->vow_bss_cfg[i].max_wait_time = VOW_DEF_MAX_WAIT_TIME;
		pAd->vow_bss_cfg[i].dwrr_quantum = VOW_DEF_BSS_DWRR_QUANTUM;
		pAd->vow_bss_cfg[i].min_rate_token = vow_convert_rate_token(pAd, VOW_MIN, i);
		pAd->vow_bss_cfg[i].max_rate_token = vow_convert_rate_token(pAd, VOW_MAX, i);
		pAd->vow_bss_cfg[i].min_airtime_token = vow_convert_airtime_token(pAd, VOW_MIN, i);
		pAd->vow_bss_cfg[i].max_airtime_token = vow_convert_airtime_token(pAd, VOW_MAX, i);
		pAd->vow_bss_cfg[i].bw_on = FALSE;
		pAd->vow_bss_cfg[i].at_on = TRUE;
		pAd->vow_bss_cfg[i].group_table_idx = VOW_GROUP_TABLE_MAX;
	}

	/* per station default configuration */
	for (i = 0; i < wtbl_max_num; i++) {
		pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BK] = VOW_STA_DWRR_IDX2; /* 4ms */
		pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BE] = VOW_STA_DWRR_IDX2; /* 4ms */
		pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VI] = VOW_STA_DWRR_IDX1; /* 3ms */
		pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VO] = VOW_STA_DWRR_IDX0; /* 1.5ms */
		pAd->vow_sta_cfg[i].group = 0;
		pAd->vow_sta_cfg[i].wmm_idx = VOW_WMM_SET0;
		pAd->vow_sta_cfg[i].ac_change_rule = VOW_DEFAULT_AC;
		pAd->vow_sta_cfg[i].paused = FALSE;
	}

	/* station DWRR quantum value */
	pAd->vow_cfg.vow_sta_dwrr_quantum[0] = VOW_STA_DWRR_QUANTUM0;
	pAd->vow_cfg.vow_sta_dwrr_quantum[1] = VOW_STA_DWRR_QUANTUM1;
	pAd->vow_cfg.vow_sta_dwrr_quantum[2] = VOW_STA_DWRR_QUANTUM2;
	pAd->vow_cfg.vow_sta_dwrr_quantum[3] = VOW_STA_DWRR_QUANTUM3;
	pAd->vow_cfg.vow_sta_dwrr_quantum[4] = VOW_STA_DWRR_QUANTUM4;
	pAd->vow_cfg.vow_sta_dwrr_quantum[5] = VOW_STA_DWRR_QUANTUM5;
	pAd->vow_cfg.vow_sta_dwrr_quantum[6] = VOW_STA_DWRR_QUANTUM6;
	pAd->vow_cfg.vow_sta_dwrr_quantum[7] = VOW_STA_DWRR_QUANTUM7;

	/* RX airtime */
	pAd->vow_rx_time_cfg.rx_early_end_en = TRUE;

	/* for fast round robin */
	pAd->vow_sta_frr_quantum = 2;	/* 0.5ms */
	pAd->vow_sta_frr_flag = FALSE;

	/* misc configuration */
	pAd->vow_misc_cfg.near_far_ctrl.adjust_en = TRUE;
	pAd->vow_misc_cfg.near_far_ctrl.slow_phy_th = NEAR_FAR_SLOW_PHY_RATE_TH;
	pAd->vow_misc_cfg.near_far_ctrl.fast_phy_th = NEAR_FAR_FAST_PHY_RATE_TH;
	pAd->vow_misc_cfg.keep_quantum = TRUE;

	if ((pAd->vow_gen.VOW_GEN == VOW_GEN_2) ||
	    (pAd->vow_gen.VOW_GEN == VOW_GEN_TALOS) ||
	    (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON)) {
		pAd->vow_misc_cfg.rts_sta_lock = TRUE;
		pAd->vow_misc_cfg.tx_rr_count = 0;	/* default: disable */
		pAd->vow_misc_cfg.measurement_mode = TRUE;
		pAd->vow_misc_cfg.max_backoff_bound_en = TRUE;
		pAd->vow_misc_cfg.max_backoff_bound = 20; /* ms, for CCK */
		pAd->vow_misc_cfg.rts_failed_charge_time_en = FALSE; /* FALSE: charge time when RTS failed */
		pAd->vow_misc_cfg.zero_eifs_time = TRUE;
		pAd->vow_misc_cfg.rx_rifs_mode = TRUE;
		pAd->vow_misc_cfg.keep_vow_sram_setting = TRUE;
		pAd->vow_misc_cfg.keep_vow_sram_setting_bit = 26; /* bit 26 */

		if (pAd->vow_gen.VOW_GEN == VOW_GEN_TALOS)
			pAd->vow_misc_cfg.spl_sta_count = VOW_SPL_STA_NUM;
		else
			pAd->vow_misc_cfg.spl_sta_count = 0;

		if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON)
			pAd->vow_rx_time_cfg.obss_backoff = 0;
	}

	pAd->vow_sch_cfg.apply_sch_ctrl = TRUE;
	pAd->vow_sch_cfg.sch_type = VOW_SCH_FOLLOW_ALGO;
	pAd->vow_sch_cfg.sch_policy = VOW_SCH_POL_SRR;

	return;

}

VOID vow_init(PRTMP_ADAPTER pad)
{
	BOOLEAN ret;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	/* for M2M test */
	pvow_pad = pad;
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[31m: start ...\x1b[m\n");

	pad->vow_gen.VOW_FEATURE = 0;
	pad->vow_gen.VOW_FEATURE |= VOW_FEATURE_ATF;
	pad->vow_gen.VOW_FEATURE |= VOW_FEATURE_BWCTRL;
	if ((!(IS_MT7915(pad) || IS_MT7915_6E(pad))) && (pad->vow_gen.VOW_GEN >= VOW_GEN_FALCON))
		pad->vow_gen.VOW_FEATURE |= VOW_FEATURE_BWCG;

	if (pad->vow_gen.VOW_GEN == VOW_GEN_TALOS) {
		pad->vow_gen.VOW_FEATURE |= VOW_FEATURE_SPL;
	}

	pad->max_bssgroup_num = VOW_MAX_GROUP_NUM;

	/* vow CR address init */
	vow_init_CR_offset(pad);
	/* vow_dump_umac_CRs(pad); */
	/* vow station init */
	vow_init_sta(pad);
	/* vow group init */
	vow_init_group(pad);
	/* vow rx init */
	vow_init_rx(pad);
	/* vow misc init */
	vow_init_misc(pad);
	/* feature control */
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			ret = uni_vmd_vow_set_feature_all(pad);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			ret = vow_set_feature_all(pad);

	/* configure badnode detector */
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[31m: end ...\x1b[m\n");
}

VOID vow_init_CR_offset(PRTMP_ADAPTER pad)
{
	if (pad->vow_gen.VOW_GEN == VOW_GEN_1) {
		/* VOW debug command 0x22/0x44 */
		pad->vow_gen.VOW_STA_SETTING_BEGIN = 0;
		pad->vow_gen.VOW_STA_SETTING_END = pad->vow_gen.VOW_STA_SETTING_BEGIN + 16;
		pad->vow_gen.VOW_STA_BITMAP_BEGIN = 0;
		pad->vow_gen.VOW_STA_BITMAP_END = pad->vow_gen.VOW_STA_BITMAP_BEGIN + 16;
		pad->vow_gen.VOW_BSS_TOKEN_OFFSET = 32;
		pad->vow_gen.VOW_STA_SETTING_FACTOR = 3;

		/* STA setting */
		pad->vow_gen.VOW_STA_AC_PRIORITY_OFFSET = 4;
		pad->vow_gen.VOW_STA_WMM_AC0_OFFSET = 6;
		pad->vow_gen.VOW_STA_WMM_AC1_OFFSET = 8;
		pad->vow_gen.VOW_STA_WMM_AC2_OFFSET = 10;
		pad->vow_gen.VOW_STA_WMM_AC3_OFFSET = 12;
		pad->vow_gen.VOW_STA_WMM_ID_OFFSET = 14;
	} else if (pad->vow_gen.VOW_GEN == VOW_GEN_2) {
		/* VOW debug command 0x22/0x44 */
		pad->vow_gen.VOW_STA_SETTING_BEGIN = 68;
		pad->vow_gen.VOW_STA_SETTING_END = pad->vow_gen.VOW_STA_SETTING_BEGIN + 32;
		pad->vow_gen.VOW_STA_BITMAP_BEGIN = 52;
		pad->vow_gen.VOW_STA_BITMAP_END = pad->vow_gen.VOW_STA_BITMAP_BEGIN + 16;
		pad->vow_gen.VOW_BSS_TOKEN_OFFSET = 0;
		pad->vow_gen.VOW_STA_SETTING_FACTOR = 2;

		/* STA setting */
		pad->vow_gen.VOW_STA_AC_PRIORITY_OFFSET = 4;
		pad->vow_gen.VOW_STA_WMM_AC0_OFFSET = 8;
		pad->vow_gen.VOW_STA_WMM_AC1_OFFSET = 12;
		pad->vow_gen.VOW_STA_WMM_AC2_OFFSET = 16;
		pad->vow_gen.VOW_STA_WMM_AC3_OFFSET = 20;
		pad->vow_gen.VOW_STA_WMM_ID_OFFSET = 6;
	} else {
		/* VOW_GEN_TALOS */
		/* VOW debug command 0x22/0x44 */
		pad->vow_gen.VOW_STA_SETTING_BEGIN = 28;
		pad->vow_gen.VOW_STA_SETTING_END = pad->vow_gen.VOW_STA_SETTING_BEGIN + 32;
		pad->vow_gen.VOW_STA_BITMAP_BEGIN = 12;
		pad->vow_gen.VOW_STA_BITMAP_END = pad->vow_gen.VOW_STA_BITMAP_BEGIN + 16;
		pad->vow_gen.VOW_BSS_TOKEN_OFFSET = 0;
		pad->vow_gen.VOW_STA_SETTING_FACTOR = 2;

		/* STA setting */
		pad->vow_gen.VOW_STA_AC_PRIORITY_OFFSET = 4;
		pad->vow_gen.VOW_STA_WMM_AC0_OFFSET = 8;
		pad->vow_gen.VOW_STA_WMM_AC1_OFFSET = 12;
		pad->vow_gen.VOW_STA_WMM_AC2_OFFSET = 16;
		pad->vow_gen.VOW_STA_WMM_AC3_OFFSET = 20;
		pad->vow_gen.VOW_STA_WMM_ID_OFFSET = 6;
		pad->vow_gen.VOW_STA_BWC_GROUP_OFFSET = 24;
	}
}


VOID vow_init_sta(PRTMP_ADAPTER pad)
{
	UINT16 i;
	BOOLEAN ret;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pad);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	/* if ATF is disabled, the default max DWRR wait time is configured as 256us to force STA round-robin */
	if (pad->vow_cfg.en_airtime_fairness == FALSE)
		pad->vow_cfg.sta_max_wait_time = 1; /* 256us */

	/* set max wait time for DWRR station */
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			ret = uni_vmd_vow_set_sta_DWRR_max_time(pad);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			ret = vow_set_sta_DWRR_max_time(pad);

	/* VOW is disabled, skip all setting */
	if (vow_is_enabled(pad) == FALSE)
		return;

	/* station DWRR quantum */
#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support) {
		ret = uni_cmd_vow_set_sta(pad, VOW_ALL_STA, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
		ret = vow_set_sta(pad, VOW_ALL_STA, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL);

	/* per station DWRR configuration */
	for (i = 0; i < wtbl_max_num; i++) {
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support) {
			ret = uni_cmd_vow_set_sta(pad, i, ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP);
			/* set station pause status */
			ret = uni_cmd_vow_set_sta(pad, i, ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING);
			ret = uni_cmd_vow_set_sta(pad, i, ENUM_VOW_DRR_CTRL_FIELD_STA_ALL);
		} else
#endif /* WIFI_UNIFIED_COMMAND */
		{
			ret = vow_set_sta(pad, i, ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP);
			/* set station pause status */
			ret = vow_set_sta(pad, i, ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING);
			ret = vow_set_sta(pad, i, ENUM_VOW_DRR_CTRL_FIELD_STA_ALL);
		}
	}
}

VOID vow_init_group(PRTMP_ADAPTER pad)
{
	BOOLEAN ret;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	/* VOW is disabled, skip all setting */
	if (vow_is_enabled(pad) == FALSE)
		return;

	/* group DWRR quantum */
	if (!(pad->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCTRL))
		return;

	ret = vow_set_group(pad, VOW_ALL_GROUP, ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_ALL);

	/* set group configuration */
	ret = vow_set_group(pad, VOW_ALL_GROUP, ENUM_BSSGROUP_CTRL_ALL_ITEM_FOR_ALL_GROUP);

	/* set max BSS wait time and sta wait time */
	/* RTMP_IO_WRITE32(pad->hdev_ctrl, 0x8374, 0x00200020); */
	vow_set_group_DWRR_max_time(pad);
	/*set Airtime estimator enable*/

	if (pad->vow_cfg.en_bw_ctrl)
		pad->vow_at_est.at_estimator_en = TRUE;
	else
		pad->vow_at_est.at_estimator_en = FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support)
		uni_cmd_vow_set_at_estimator(pad, ENUM_AT_PROC_EST_FEATURE_CTRL);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		vow_set_at_estimator(pad, ENUM_AT_PROC_EST_FEATURE_CTRL);
}

VOID vow_init_rx(PRTMP_ADAPTER pad)
{
	UINT8 i;
	BOOLEAN ret;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support) {
		/* reset all RX counters */
		ret = uni_cmd_vow_set_rx_airtime(pad, ENUM_RX_AT_BITWISE_CTRL, ENUM_RX_AT_BITWISE_SUB_TYPE_AIRTIME_CLR);
		/* RX airtime feature enable */
		ret = uni_cmd_vow_set_rx_airtime(pad, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN);
		/* RX early end enable */
		ret = uni_cmd_vow_set_rx_airtime(pad, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_EARLYEND_EN);
		/* set ED offset */
		ret = uni_cmd_vow_set_rx_airtime(pad, ENUM_RX_AT_TIMER_VALUE_CTRL, ENUM_RX_AT_TIME_VALUE_SUB_TYPE_ED_OFFSET_CTRL);

		/* set OBSS backoff time - 1 set*/
		ret = uni_cmd_vow_set_backoff_time(pad, ENUM_RX_AT_OBSS);
		/* set non QOS backoff time - 1 set */
		ret = uni_cmd_vow_set_backoff_time(pad, ENUM_RX_AT_NON_QOS);
		/* set repeater backoff time - 1 set */
		ret = uni_cmd_vow_set_backoff_time(pad, ENUM_RX_AT_WMM_GROUP_PEPEATER);
		/* set OM backoff time */
		ret = uni_cmd_vow_set_backoff_time(pad, ENUM_RX_AT_WMM_GROUP_STA);

		/* set WMM AC backoff time */
		for (i = 0; i < VOW_MAX_WMM_SET_NUM; i++)
			ret = uni_cmd_vow_set_backoff_time(pad, i);

	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		/* reset all RX counters */
		ret = vow_set_rx_airtime(pad, ENUM_RX_AT_BITWISE_CTRL, ENUM_RX_AT_BITWISE_SUB_TYPE_AIRTIME_CLR);
		/* RX airtime feature enable */
		ret = vow_set_rx_airtime(pad, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN);
		/* RX early end enable */
		ret = vow_set_rx_airtime(pad, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_EARLYEND_EN);
		/* set ED offset */
		ret = vow_set_rx_airtime(pad, ENUM_RX_AT_TIMER_VALUE_CTRL, ENUM_RX_AT_TIME_VALUE_SUB_TYPE_ED_OFFSET_CTRL);

		/* set OBSS backoff time - 1 set*/
		ret = vow_set_backoff_time(pad, ENUM_RX_AT_OBSS);
		/* set non QOS backoff time - 1 set */
		ret = vow_set_backoff_time(pad, ENUM_RX_AT_NON_QOS);
		/* set repeater backoff time - 1 set */
		ret = vow_set_backoff_time(pad, ENUM_RX_AT_WMM_GROUP_PEPEATER);
		/* set OM backoff time */
		ret = vow_set_backoff_time(pad, ENUM_RX_AT_WMM_GROUP_STA);

		/* set WMM AC backoff time */
		for (i = 0; i < VOW_MAX_WMM_SET_NUM; i++)
			ret = vow_set_backoff_time(pad, i);
	}

	/* set BSS belogs to which WMM set */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++) {
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			ret = uni_cmd_vow_set_mbss2wmm_map(pad, i);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			ret = vow_set_mbss2wmm_map(pad, i);
	}

	/* select RX WMM backoff time for 4 OM */
	for (i = 0; i < VOW_MAX_WMM_SET_NUM; i++) {
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			ret = uni_cmd_vow_set_wmm_selection(pad, i);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			ret = vow_set_wmm_selection(pad, i);
	}

	/* VOW is disabled, skip all setting */
	if (vow_is_enabled(pad) == FALSE) {
		pad->vow_rx_time_cfg.rx_early_end_en = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			uni_cmd_vow_set_rx_airtime(pad, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_EARLYEND_EN);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			vow_set_rx_airtime(pad, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_EARLYEND_EN);
		return;
	}

	/* configure airtime estimator */
	if (pad->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCTRL) {
		for (i = 0; i < VOW_MAX_GROUP_NUM; i++) {
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				uni_cmd_vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_RATIO_CTRL, i);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_RATIO_CTRL, i);
			/* vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING, i); */
		}
	}
}


VOID vow_init_misc(PRTMP_ADAPTER pad)
{
	UINT32 reg32 = 0, i;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (pad->vow_gen.VOW_GEN == VOW_GEN_1) {
		/* disable RTS failed airtime charge for RTS deadlock */
		HW_IO_READ32(pad->hdev_ctrl, AGG_SCR, &reg32);
		reg32 |= RTS_FAIL_CHARGE_DIS;
		HW_IO_WRITE32(pad->hdev_ctrl, AGG_SCR, reg32);
	}

	/* VOW is disabled, skip all setting */
	if (vow_is_enabled(pad) == FALSE)
		return;

	if (pad->vow_gen.VOW_GEN == VOW_GEN_1) {
		/* RX_RIFS_MODE enable, 820F4000 bit[23] set to 1 */
		/* detect ED signal down for CCK CTS */
		HW_IO_READ32(pad->hdev_ctrl, TMAC_TCR, &reg32);
		reg32 |= RX_RIFS_MODE;
		HW_IO_WRITE32(pad->hdev_ctrl, TMAC_TCR, reg32);

		/* Configure 1 to force rmac_cr_eifs_time=0 for VOW OBSS counter, 820f52e0 bit[21] set to 1 */
		HW_IO_READ32(pad->hdev_ctrl, RMAC_RSVD0, &reg32);
		reg32 |= RX_EIFS_TIME_ZERO;
		HW_IO_WRITE32(pad->hdev_ctrl, RMAC_RSVD0, reg32);

		/* for SER (0x82060370 bit[26]=1 --> keep all VOW setting) */
		HW_IO_READ32(pad->hdev_ctrl, VOW_CONTROL, &reg32);
		reg32 |= VOW_RESET_DISABLE;
		HW_IO_WRITE32(pad->hdev_ctrl, VOW_CONTROL, reg32);
	}

	if (vow_watf_is_enabled(pad)) {
		pad->vow_cfg.vow_sta_dwrr_quantum[0] = pad->vow_watf_q_lv0;
		pad->vow_cfg.vow_sta_dwrr_quantum[1] = pad->vow_watf_q_lv1;
		pad->vow_cfg.vow_sta_dwrr_quantum[2] = pad->vow_watf_q_lv2;
		pad->vow_cfg.vow_sta_dwrr_quantum[3] = pad->vow_watf_q_lv3;
	} else {
		pad->vow_cfg.vow_sta_dwrr_quantum[0] = VOW_STA_DWRR_QUANTUM0;
		pad->vow_cfg.vow_sta_dwrr_quantum[1] = VOW_STA_DWRR_QUANTUM1;
		pad->vow_cfg.vow_sta_dwrr_quantum[2] = VOW_STA_DWRR_QUANTUM2;
		pad->vow_cfg.vow_sta_dwrr_quantum[3] = VOW_STA_DWRR_QUANTUM3;
	}

	for (i = 0; i < VOW_WATF_LEVEL_NUM; i++) {
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			uni_cmd_vow_set_sta(pad, VOW_ALL_STA, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0 + i);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			vow_set_sta(pad, VOW_ALL_STA, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0 + i);
	}
}

VOID vow_update_om_wmm(PRTMP_ADAPTER pad, struct wifi_dev *wdev, UCHAR wmm_idx, PEDCA_PARM pApEdcaParm)
{
	UCHAR st;
	UINT16 cw;
	UCHAR ac_idx;
	UCHAR ac_map[] = {WMM_AC_BE, WMM_AC_BK, WMM_AC_VI, WMM_AC_VO};
	struct _EDCA_PARM *pBssEdca = wlan_config_get_ht_edca(wdev);
	UCHAR BandIdx = HcGetBandByWdev(wdev);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (WMODE_CAP_5G(wdev->PhyMode))
		st = SLOT_TIME_5G;
	else if (pad->CommonCfg.bUseShortSlotTime[BandIdx])
		st = SLOT_TIME_24G_SHORT;
	else
		st = SLOT_TIME_24G_LONG;

	if (pBssEdca) {
		/* invalid */
		if (pBssEdca->bValid == FALSE)
			return;

		if ((pApEdcaParm == NULL) || (pApEdcaParm->bValid == FALSE))
			return;

		for (ac_idx = 0; ac_idx < WMM_NUM_OF_AC; ac_idx++) {
			cw = (1 << pBssEdca->Cwmin[ac_map[ac_idx]]) - 1;
			pad->vow_rx_time_cfg.wmm_backoff[wmm_idx][ac_idx] =
				(WMODE_CAP_5G(wdev->PhyMode) ? SIFS_TIME_5G : SIFS_TIME_24G) +
				pBssEdca->Aifsn[ac_map[ac_idx]]*st + cw*st;
		}

#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			uni_cmd_vow_set_backoff_time(pad, wmm_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			vow_set_backoff_time(pad, wmm_idx);
	}
}

VOID vow_mbss_init(PRTMP_ADAPTER pad, struct wifi_dev *wdev)
{
	UCHAR wmm_idx;

	if (wdev && (wdev->wdev_type != WDEV_TYPE_REPEATER)) {
		vow_mbss_grp_band_map(pad, wdev);
		vow_mbss_wmm_map(pad, wdev);
		wmm_idx = HcGetWmmIdx(pad, wdev);

		/* configure BCMC entry */
		if (pad->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG) {
			if ((pad->vow_cfg.en_bw_ctrl) && (pad->bss_group.bw_group_idx[wdev->func_idx] < VOW_GROUP_TABLE_MAX))
					vow_set_client(pad, pad->bss_group.bw_group_idx[wdev->func_idx],
							wdev->tr_tb_idx, wmm_idx);
		} else {
		vow_set_client(pad, pad->bss_group.group_idx[wdev->func_idx], wdev->tr_tb_idx, wmm_idx);
	}
}
}

VOID vow_group_band_map(PRTMP_ADAPTER pad, UCHAR band_idx, UCHAR group_idx)
{
	UINT32 reg32 = 0;

	if (pad->vow_gen.VOW_GEN >= VOW_GEN_FALCON)
		return;

	HW_IO_READ32(pad->hdev_ctrl, VOW_DBDC_BW_GROUP_CTRL, &reg32);
	reg32 &= ~(1 << group_idx);
	reg32 |= (band_idx << group_idx);
	HW_IO_WRITE32(pad->hdev_ctrl, VOW_DBDC_BW_GROUP_CTRL, reg32);
}

/* do bss(group) and band mapping */
VOID vow_mbss_grp_band_map(PRTMP_ADAPTER pad, struct wifi_dev *wdev)
{
	UCHAR band_idx;
	UCHAR func_idx;
	UCHAR group_idx;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */


	if (wdev) {
		band_idx = HcGetBandByWdev(wdev);
		func_idx = wdev->func_idx;
		group_idx = pad->bss_group.group_idx[func_idx];
		/* MBSS <--> group 1 to 1 mapping, ex: SSID0 --> Group0 */
		vow_group_band_map(pad, band_idx, group_idx);
		pad->vow_bss_cfg[group_idx].band_idx = band_idx;
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			uni_cmd_vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING, group_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING, group_idx);
	}
}

/* do bss and wmm mapping for RX */
VOID vow_mbss_wmm_map(PRTMP_ADAPTER pad, struct wifi_dev *wdev)
{
	UCHAR wmm_idx;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (wdev) {
		wmm_idx = HcGetWmmIdx(pad, wdev);
		pad->vow_rx_time_cfg.bssid2wmm_set[wdev->func_idx] = wmm_idx;
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			uni_cmd_vow_set_mbss2wmm_map(pad, pad->bss_group.group_idx[wdev->func_idx]);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			vow_set_mbss2wmm_map(pad, pad->bss_group.group_idx[wdev->func_idx]);
	}
}

static UINT32 vow_get_availabe_airtime(VOID)
{
	return VOW_DEF_AVA_AIRTIME;
}

/* get rate token */
UINT16 vow_convert_rate_token(PRTMP_ADAPTER pad, UINT8 type, UINT8 group_id)
{
	UINT16 period, rate, token = 0;

	period = (1 << pad->vow_cfg.refill_period);

	if (type == VOW_MAX) {
		rate = pad->vow_bss_cfg[group_id].max_rate;
		token = (period * rate);
	} else {
		rate = pad->vow_bss_cfg[group_id].min_rate;
		token = (period * rate);
	}

	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "period %dus, rate %u, token %u\n",  period, rate, token);
	return token;
}

/* get airtime token */
UINT16 vow_convert_airtime_token(PRTMP_ADAPTER pad, UINT8 type, UINT8 group_id)
{
	UINT16 period, ratio, token = 0;
	UINT32 atime = vow_get_availabe_airtime();
	UINT64 tmp;

	period = (1 << pad->vow_cfg.refill_period);

	if (type == VOW_MAX)
		ratio = pad->vow_bss_cfg[group_id].max_airtime_ratio;
	else
		ratio = pad->vow_bss_cfg[group_id].min_airtime_ratio;

	/* shift 3 --> because unit is 1/8 us,
	   10^8 --> ratio needs to convert from integer to %, preiod needs to convert from us to s
	*/
	tmp = ((UINT64)period * atime * ratio) << 3;
	/* printk("%s: tmp %llu\n", __FUNCTION__, tmp); */
	token = div64_u64(tmp, 100000000);
	MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "period %dus, ratio %u, available time %u, token %u\n",
			 period, ratio, atime, token);
	return token;
}


/* add client(station) */

VOID vow_set_client(PRTMP_ADAPTER pad, UINT8 group, UINT16 sta_id, UINT8 wmm_id)
{
	BOOLEAN ret;
	BOOLEAN fgSkipGroupSetting = FALSE;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
	UINT_32 grp_opmode, set_all_opmode;

	grp_opmode = ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP;
	set_all_opmode = ENUM_VOW_DRR_CTRL_FIELD_STA_ALL;

	if (pad->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG) {
		fgSkipGroupSetting = TRUE;

		if ((pad->vow_cfg.en_bw_ctrl) && (group < VOW_MAX_GROUP_NUM) &&
			(pad->vow_bss_cfg[group].group_table_idx < VOW_GROUP_TABLE_MAX)) {
			grp_opmode = ENUM_VOW_DRR_CTRL_FIELD_STA_BWC_GROUP;
			fgSkipGroupSetting = FALSE;
		}
		set_all_opmode = ENUM_VOW_DRR_CTRL_FIELD_STA_EXCLUDE_GROUP;
	}

	/* set group for station */
	pad->vow_sta_cfg[sta_id].group = group;
	pad->vow_sta_cfg[sta_id].wmm_idx = wmm_id;
#ifdef WIFI_UNIFIED_COMMAND
	if (pChipCap->uni_cmd_support) {
		/* update station bitmap */
		/* don't change command sequence - STA_BSS_GROUP will refer to STA_ALL's old setting */
		if (!fgSkipGroupSetting)
			ret = uni_cmd_vow_set_sta(pad, sta_id, grp_opmode);
		ret = uni_cmd_vow_set_sta(pad, sta_id, set_all_opmode);
		/* set station pause status */
		ret = uni_cmd_vow_set_sta(pad, sta_id, ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		/* update station bitmap */
		/* don't change command sequence - STA_BSS_GROUP will refer to STA_ALL's old setting */
		if (!fgSkipGroupSetting)
			ret = vow_set_sta(pad, sta_id, grp_opmode);
		ret = vow_set_sta(pad, sta_id, set_all_opmode);
		/* set station pause status */
		ret = vow_set_sta(pad, sta_id, ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING);
	}
}

INT vow_set_schedule_ctrl(
	IN  PRTMP_ADAPTER pad,
	IN  UINT8 sch_type,
	IN  UINT8 sch_policy)
{
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	pad->vow_sch_cfg.apply_sch_ctrl = TRUE;
	pad->vow_sch_cfg.sch_type = sch_type;
	pad->vow_sch_cfg.sch_policy = sch_policy;
#ifdef WIFI_UNIFIED_COMMAND
		if (pChipCap->uni_cmd_support)
			return uni_vmd_vow_set_feature_all(pad);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			return vow_set_feature_all(pad);
}

#ifdef VOW_SUPPORT
VOID vow_reset(PRTMP_ADAPTER pad)
{
	vow_reset_watf(pad);
	vow_reset_dvt(pad);
}

INT set_vow_min_rate_token(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_rate_token = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_RATE_TOKEN_CFG_ITEM);
			MTWF_PRINT("%s: group %d set %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_rate_token(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_rate_token = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_RATE_TOKEN_CFG_ITEM);
			MTWF_PRINT("%s: group %d set %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_min_airtime_token(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_airtime_token = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_AIRTIME_TOKEN_CFG_ITEM);
			MTWF_PRINT("group %d set %u.\n", group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_airtime_token(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_airtime_token = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_AIRTIME_TOKEN_CFG_ITEM);
			MTWF_PRINT("%s: group %d set %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_min_rate_bucket(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_ratebucket_size = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM);
			MTWF_PRINT("group %d set %u.\n",  group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_rate_bucket(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_ratebucket_size = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM);
			MTWF_PRINT("%s: group %d set %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_min_airtime_bucket(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_airtimebucket_size = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM);
			MTWF_PRINT("group %d set %u.\n", group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_airtime_bucket(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_airtimebucket_size = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM);
			MTWF_PRINT("%s: group %d set %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_backlog_size(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_backlog_size = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_BACKLOG_SIZE_CFG_ITEM);
			MTWF_PRINT("%s: group %d set %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_wait_time(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_wait_time = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_WAIT_TIME_CFG_ITEM);
			MTWF_PRINT("group %d set %u.\n", group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_group_dwrr_max_wait_time(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) {
			INT ret;

			pad->vow_cfg.group_max_wait_time = val;
			ret = vow_set_group_DWRR_max_time(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_sta_pause(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &sta, &val);

		if ((rv > 1) && (IS_WCID_VALID(pad, sta))) {
			pad->vow_sta_cfg[sta].paused = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				uni_cmd_vow_set_sta(pad, sta, ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				vow_set_sta(pad, sta, ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING);
			MTWF_PRINT("%s: sta %d set %u.\n", __func__, sta, val);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_sta_group(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
	UINT_32 grp_opmode;

	grp_opmode = ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &sta, &val);

		if ((rv > 1) && (IS_WCID_VALID(pad, sta))) {
			INT ret;

			if ((pad->vow_cfg.en_bw_ctrl) && (val < VOW_MAX_GROUP_NUM) &&
				(pad->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG) &&
				(pad->vow_bss_cfg[val].group_table_idx < VOW_GROUP_TABLE_MAX))
					grp_opmode = ENUM_VOW_DRR_CTRL_FIELD_STA_BWC_GROUP;

			pad->vow_sta_cfg[sta].group = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_sta(pad, sta, grp_opmode);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_sta(pad, sta, grp_opmode);
			MTWF_PRINT("%s: sta %d group %u.\n", __func__, sta, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bw_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */


	if (!(pad->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCTRL)) {
		pad->vow_cfg.en_bw_ctrl = 0;
		return FALSE;
	}

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.en_bw_ctrl = val;
#ifdef RED_SUPPORT
			if (pad->red_mcu_offload)
				MtCmdCr4Set(pad,
					    CR4_SET_ID_RED_ENTER_LOW_FREE_PLE_MODE,
						pad->vow_cfg.en_bw_ctrl || (pad->vow_cfg.en_airtime_fairness &&
						pad->vow_watf_en), 0);
#endif
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_feature_all(pad);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_feature_all(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}

			pad->vow_at_est.at_estimator_en = (val > 0) ? TRUE : FALSE;
#ifdef WIFI_UNIFIED_COMMAND
				if (pChipCap->uni_cmd_support)
					uni_cmd_vow_set_at_estimator(pad, ENUM_AT_PROC_EST_FEATURE_CTRL);
				else
#endif /* WIFI_UNIFIED_COMMAND */
					vow_set_at_estimator(pad, ENUM_AT_PROC_EST_FEATURE_CTRL);

		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_refill_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.en_bw_refill = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_feature_all(pad);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_feature_all(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_airtime_fairness_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */


	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.en_airtime_fairness = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_feature_all(pad);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_feature_all(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);
#if defined (FQ_SCH_SUPPORT)
			if (pad->vow_cfg.en_airtime_fairness == 0)
				set_fq_enable(pad, "0-0");
#endif
			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_txop_switch_bss_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.en_txop_no_change_bss = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_feature_all(pad);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_feature_all(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_dbdc_search_rule(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, band;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &band, &val);

		if ((rv > 1)) {
			INT ret;

			if (band == 0)
				pad->vow_cfg.dbdc0_search_rule = val;
			else
				pad->vow_cfg.dbdc1_search_rule = val;

#ifdef WIFI_UNIFIED_COMMAND
				if (pChipCap->uni_cmd_support)
					ret = uni_vmd_vow_set_feature_all(pad);
				else
#endif /* WIFI_UNIFIED_COMMAND */
					ret = vow_set_feature_all(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_refill_period(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.refill_period = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_feature_all(pad);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_feature_all(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bss_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, group;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */


	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_cfg.per_bss_enable &= ~(1 << group);
			pad->vow_cfg.per_bss_enable |= (val << group);
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_feature_all(pad);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_feature_all(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_spl_sta_num(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 rv, sta_num;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &sta_num);

		if ((rv > 0) && (sta_num <= VOW_SPL_STA_NUM_LIMIT)) {
			INT ret;

			pad->vow_misc_cfg.spl_sta_count = (UINT8)sta_num;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_feature_all(pad);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_feature_all(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, sta_num);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_mcli_schedule_en(
	IN	PRTMP_ADAPTER pad,
	IN	RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			pad->vow_cfg.mcli_schedule_en = val;
			if (pad->vow_cfg.mcli_schedule_en)
				MTWF_PRINT("%s: mcli schedule code enable.\n", __func__);
			else
				MTWF_PRINT("%s: mcli schedule code disable.\n", __func__);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;

}

INT set_vow_mcli_schedule_parm(
	IN	PRTMP_ADAPTER pad,
	IN	RTMP_STRING * arg)
{
	UINT32 rv, cmd, op1, op2, op3;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type, bandidx = 0;
	POS_COOKIE pObj = (POS_COOKIE) pad->OS_Cookie;

	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;

	if (if_type == INT_MAIN || if_type == INT_MBSSID) {
		if (VALID_MBSS(pad, ifIndex))
			wdev = &pad->ApCfg.MBSSID[ifIndex].wdev;
		else
			MTWF_DBG(pad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid MBSS index %d\n", ifIndex);
	} else if (if_type == INT_APCLI) {
		if (ifIndex < MAX_MULTI_STA)
			wdev = &pad->StaCfg[ifIndex].wdev;
		else
			MTWF_DBG(pad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid APCLI index %d\n", ifIndex);
	} else if (if_type == INT_WDS) {
		if (ifIndex < MAX_WDS_ENTRY)
			wdev = &pad->WdsTab.WdsEntry[ifIndex].wdev;
		else
			MTWF_DBG(pad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid WDS index %d\n", ifIndex);
	} else {
		MTWF_DBG(pad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unexpected if_type\n");
		return FALSE;
	}

	if (!wdev) {
		MTWF_DBG(pad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid, wdev is NULL\n");
		return FALSE;
	}

	bandidx = HcGetBandByWdev(wdev);

	/* sanity check for Band index */
	if (bandidx >= DBDC_BAND_NUM)
		return FALSE;

	if (arg) {
		rv = sscanf(arg, "%u-%u-%u-%u", &cmd, &op1, &op2, &op3);

		if (rv <= 0)
			return FALSE;

		switch (cmd) {
		case MCLI_TCP_CNT_TH:
			pad->vow_cfg.mcli_sch_cfg.tcp_cnt_th = op1;
			MTWF_PRINT("%s: tcp_cnt_th=%u\n", __func__, pad->vow_cfg.mcli_sch_cfg.tcp_cnt_th);
			break;
		case MCLI_DL_WRR_ENABLE:
			pad->vow_cfg.mcli_sch_cfg.dl_wrr_en = (op1 > 0);
			MTWF_PRINT("%s:dl_wrr_en=%u\n", __func__, pad->vow_cfg.mcli_sch_cfg.dl_wrr_en);
			break;
		case MCLI_CWMIN_CWMAX_PARAM:
			if (op1 == VOW_MCLI_DL_MODE) {
				pad->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_DL_MODE][bandidx] = op2;
				pad->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_DL_MODE][bandidx] = op3;
				MTWF_PRINT("%s:band%u, DL mode, cwmin:cwmax=%u:%u\n", __func__, bandidx,
					pad->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_DL_MODE][bandidx],
					pad->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_DL_MODE][bandidx]);
			} else if (op1 == VOW_MCLI_UL_MODE) {
				pad->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_UL_MODE][bandidx] = op2;
				pad->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_UL_MODE][bandidx] = op3;
				MTWF_PRINT("%s:band%u, UL mode, cwmin:cwmax=%u:%u\n", __func__, bandidx,
					pad->vow_cfg.mcli_sch_cfg.cwmin[VOW_MCLI_UL_MODE][bandidx],
					pad->vow_cfg.mcli_sch_cfg.cwmax[VOW_MCLI_UL_MODE][bandidx]);
			} else
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"mode%u not support now\n", op1);
			break;
		default:
			MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Invaild parameter combination, cmd:%d, op:%d\n", cmd, op1);
			break;
		}
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_sta_dwrr_quantum(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, id;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &id, &val);

		if ((rv > 1) && (id < 8)) {
			INT ret;

			pad->vow_cfg.vow_sta_dwrr_quantum[id] = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_sta(pad, VOW_ALL_STA, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0 + id);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_sta(pad, VOW_ALL_STA, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0 + id);
			MTWF_PRINT("%s: set quantum id %u, val %d.\n", __func__, id, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_sta_frr_quantum(
		IN	PRTMP_ADAPTER pad,
		IN	RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);
		if ((rv > 0) && (val <= 0xff)) {
			pad->vow_sta_frr_quantum = val;
			pad->vow_sta_frr_flag = FALSE;
			MTWF_PRINT("%s: set FRR quantum %d.\n", __FUNCTION__, val);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_near_far_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv == 1) {
			pad->vow_misc_cfg.near_far_ctrl.adjust_en = val;
			MTWF_PRINT("%s: set %u.\n", __func__, val);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_near_far_th(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 rv, slow_th, fast_th;

	if (arg) {

		rv = sscanf(arg, "%d-%d", &slow_th, &fast_th);

		if (rv > 1) {
			pad->vow_misc_cfg.near_far_ctrl.slow_phy_th = slow_th;
			pad->vow_misc_cfg.near_far_ctrl.fast_phy_th = fast_th;
			MTWF_PRINT("%s: set slow_th %u, fast_th %d.\n",
					  __func__, slow_th, fast_th);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_airtime_ctrl_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, group;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].at_on = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_feature_all(pad);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_feature_all(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bw_ctrl_en(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, group;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].bw_on = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_feature_all(pad);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_feature_all(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_schedule_ctrl(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 rv, sch_type, sch_policy;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &sch_type, &sch_policy);

		if ((rv > 1) && (sch_type <= 1) && (sch_policy <= 1)) {
			INT ret = vow_set_schedule_ctrl(pad, (UINT8)sch_type, (UINT8)sch_policy);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}

			MTWF_PRINT("%s: sch_type %u sch_policy %u.\n", __func__,
					  pad->vow_sch_cfg.sch_type, pad->vow_sch_cfg.sch_policy);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT show_vow_schedule_ctrl(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	MTWF_PRINT("%s: sch_type %u sch_policy %u.\n", __func__,
			  pad->vow_sch_cfg.sch_type, pad->vow_sch_cfg.sch_policy);

	return TRUE;
}

INT set_vow_sta_ac_priority(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, sta;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &sta, &val);

		if ((rv > 1) && (IS_WCID_VALID(pad, sta)) && (val < 4)) {
			BOOLEAN ret;
			/* set AC change rule */
			pad->vow_sta_cfg[sta].ac_change_rule = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_sta(pad, sta, ENUM_VOW_DRR_CTRL_FIELD_STA_PRIORITY);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_sta(pad, sta, ENUM_VOW_DRR_CTRL_FIELD_STA_PRIORITY);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "sta %d W ENUM_VOW_DRR_PRIORITY_CFG_ITEM failed.\n", sta);
				return FALSE;
			}

			MTWF_PRINT("%s: sta %d W AC change rule %d.\n", __func__, sta, pad->vow_sta_cfg[sta].ac_change_rule);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_sta_dwrr_quantum_id(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, sta, ac;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &sta, &ac, &val);

		if ((rv > 2) && (IS_WCID_VALID(pad, sta)) && (ac < 4) && (val < 8)) {
			INT ret;

			pad->vow_sta_cfg[sta].dwrr_quantum[ac] = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_sta(pad, sta, ENUM_VOW_DRR_CTRL_FIELD_STA_AC0_QUA_ID + ac);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_sta(pad, sta, ENUM_VOW_DRR_CTRL_FIELD_STA_AC0_QUA_ID + ac);
			MTWF_PRINT("%s: set sta %d, ac %d, quantum id %u.\n", __func__, sta, ac, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bss_dwrr_quantum(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, group;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].dwrr_quantum = val;
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_00 + group);
			MTWF_PRINT("%s: set group %d, quantum id %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_dwrr_max_wait_time(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
		RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pad->vow_cfg.sta_max_wait_time = val;
			/* ret = vow_set_sta(pad, 0xFF, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_DEFICIT_BOUND); */
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_sta_DWRR_max_time(pad);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_sta_DWRR_max_time(pad);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_min_rate(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_rate = val;
			pad->vow_bss_cfg[group].min_rate_token = vow_convert_rate_token(pad, VOW_MIN, group);
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_RATE_TOKEN_CFG_ITEM);
			MTWF_PRINT("%s: group %d set rate %u\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_rate(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_rate = val;
			pad->vow_bss_cfg[group].max_rate_token = vow_convert_rate_token(pad, VOW_MAX, group);
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_RATE_TOKEN_CFG_ITEM);
			MTWF_PRINT("%s: group %d set %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_min_ratio(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */


	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].min_airtime_ratio = val;
			pad->vow_bss_cfg[group].min_airtime_token = vow_convert_airtime_token(pad, VOW_MIN, group);
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MIN_AIRTIME_TOKEN_CFG_ITEM);
			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set group command failed.\n");
				return FALSE;
			}
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_RATIO_CTRL, group);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_RATIO_CTRL, group);
			MTWF_PRINT("%s: group %d set %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_max_ratio(
	IN  PRTMP_ADAPTER pad,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pad->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pad->vow_bss_cfg[group].max_airtime_ratio = val;
			pad->vow_bss_cfg[group].max_airtime_token = vow_convert_airtime_token(pad, VOW_MAX, group);
			ret = vow_set_group(pad, group, ENUM_BSSGROUP_CTRL_MAX_AIRTIME_TOKEN_CFG_ITEM);
			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set group command failed.\n");
				return FALSE;
			}
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_RATIO_CTRL, group);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_at_estimator_group(pad, ENUM_AT_PROC_EST_GROUP_RATIO_CTRL, group);
			MTWF_PRINT("%s: group %d set %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_counter_clr(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_rx_airtime(pAd, ENUM_RX_AT_BITWISE_CTRL, ENUM_RX_AT_BITWISE_SUB_TYPE_AIRTIME_CLR);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_rx_airtime(pAd, ENUM_RX_AT_BITWISE_CTRL, ENUM_RX_AT_BITWISE_SUB_TYPE_AIRTIME_CLR);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_airtime_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pAd->vow_rx_time_cfg.rx_time_en = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_rx_airtime(pAd, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_rx_airtime(pAd, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_early_end_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0)) {
			INT ret;

			pAd->vow_rx_time_cfg.rx_early_end_en = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_rx_airtime(pAd, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_EARLYEND_EN);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_rx_airtime(pAd, ENUM_RX_AT_FEATURE_CTRL, ENUM_RX_AT_FEATURE_SUB_TYPE_EARLYEND_EN);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_ed_offset(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0) && (val < 32)) {
			INT ret;

			pAd->vow_rx_time_cfg.ed_offset = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_rx_airtime(pAd, ENUM_RX_AT_TIMER_VALUE_CTRL, ENUM_RX_AT_TIME_VALUE_SUB_TYPE_ED_OFFSET_CTRL);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_rx_airtime(pAd, ENUM_RX_AT_TIMER_VALUE_CTRL, ENUM_RX_AT_TIME_VALUE_SUB_TYPE_ED_OFFSET_CTRL);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_rx_obss_backoff(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0) && (val <= 0xFFFF)) {
			INT ret;

			pAd->vow_rx_time_cfg.obss_backoff = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_backoff_time(pAd, ENUM_RX_AT_OBSS);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_backoff_time(pAd, ENUM_RX_AT_OBSS);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_rx_wmm_backoff(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 wmm, ac, val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &wmm, &ac, &val);

		if ((rv > 2) && (wmm < VOW_MAX_WMM_SET_NUM) && (ac < WMM_NUM_OF_AC) && (val <= 0xFFFF)) {
			INT ret;

			pAd->vow_rx_time_cfg.wmm_backoff[wmm][ac] = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_backoff_time(pAd, wmm);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_backoff_time(pAd, wmm);
			MTWF_PRINT("%s: wmm %d ac %d set %u.\n", __func__, wmm, ac, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_non_qos_backoff(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if ((rv > 0) && (val <= 0xFFFFF)) {
			INT ret;

			pAd->vow_rx_time_cfg.non_qos_backoff = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_backoff_time(pAd, ENUM_RX_AT_NON_QOS);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_backoff_time(pAd, ENUM_RX_AT_NON_QOS);
			MTWF_PRINT("%s: set %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_om_wmm_backoff(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 ac, val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &ac, &val);

		if ((rv > 0) && (ac < WMM_NUM_OF_AC) && (val <= 0xFFFFF)) {
			INT ret;

			pAd->vow_rx_time_cfg.om_wmm_backoff[ac] = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_backoff_time(pAd, ENUM_RX_AT_WMM_GROUP_STA);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_backoff_time(pAd, ENUM_RX_AT_WMM_GROUP_STA);
			MTWF_PRINT("set ac %d, val = %u.\n", ac, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_repeater_wmm_backoff(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 ac, val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &ac, &val);

		if ((rv > 0) && (ac < WMM_NUM_OF_AC) && (val <= 0xFFFFF)) {
			INT ret;

			pAd->vow_rx_time_cfg.repeater_wmm_backoff[ac] = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_backoff_time(pAd, ENUM_RX_AT_WMM_GROUP_PEPEATER);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_backoff_time(pAd, ENUM_RX_AT_WMM_GROUP_PEPEATER);
			MTWF_PRINT("%s: set ac %d, val = %u.\n", __func__, ac, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_bss_wmmset(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 bss_idx, val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &bss_idx, &val);

		if ((rv > 1) && (val < VOW_MAX_WMM_SET_NUM) && (bss_idx < VOW_MAX_GROUP_NUM)) {
			INT ret;

			pAd->vow_rx_time_cfg.bssid2wmm_set[bss_idx] = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_mbss2wmm_map(pAd, bss_idx);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_mbss2wmm_map(pAd, bss_idx);
			MTWF_PRINT("%s: bss_idx %d set %u.\n", __func__, bss_idx, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_rx_om_wmm_select(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 om_idx, val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &om_idx, &val);

		if ((rv > 1) && (om_idx < 4)) { /* FIXME: enum --> 4 */
			INT ret;

			pAd->vow_rx_time_cfg.wmm_backoff_sel[om_idx] = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_wmm_selection(pAd, om_idx);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_wmm_selection(pAd, om_idx);
			MTWF_PRINT("%s: OM MAC index %d set %u.\n", __func__, om_idx, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

BOOLEAN halUmacVoWChargeBwToken(
	IN UINT_8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT_16 u2ChargeLenValue,
	IN UINT_16 u2ChargeTimeValue
)
{
	UINT32 reg;

	if ((ucBssGroup >= UMAC_BSS_GROUP_NUMBER) ||
		((fgChargeMode != TRUE) && (fgChargeMode != FALSE)))
		return FALSE;

	reg = ((((UINT32)u2ChargeLenValue << UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_OFFSET) & UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_MASK) |
		   (((UINT32)u2ChargeTimeValue << UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_OFFSET) & UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_MASK));
	RTMP_IO_WRITE32(pvow_pad->hdev_ctrl, UMAC_DRR_TABLE_WDATA0, reg);
	reg = ((UMAC_DRR_TABLE_CTRL0_EXEC_MASK) |
		   ((UMAC_CHARGE_BW_TOKEN_OP_MASK | (((UINT32)fgChargeMode << (ffs(UMAC_CHARGE_ADD_MODE_BIT_MASK) - 1)) & UMAC_CHARGE_ADD_MODE_BIT_MASK)) << UMAC_DRR_TABLE_CTRL0_MODE_OP_OFFSET) |
		   ((ucBssGroup << UMAC_CHARGE_MODE_BSS_GROUP_OFFSET) & UMAC_CHARGE_MODE_BSS_GROUP_MASK));
	RTMP_IO_WRITE32(pvow_pad->hdev_ctrl, UMAC_DRR_TABLE_CTRL0, reg);
	return TRUE;
}

BOOLEAN halUmacVoWChargeBwTokenLength(
	IN UINT8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeLenValue
)
{
	return halUmacVoWChargeBwToken(ucBssGroup, fgChargeMode, u2ChargeLenValue, 0);
}



BOOLEAN halUmacVoWChargeBwTokenTime(
	IN UINT8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeTimeValue
)
{
	return halUmacVoWChargeBwToken(ucBssGroup, fgChargeMode, 0, u2ChargeTimeValue);
}

BOOLEAN halUmacVoWChargeBwDrr(
	IN UINT8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeLenValue,
	IN UINT16 u2ChargeTimeValue
)
{
	UINT32 reg;

	if ((ucBssGroup >= UMAC_BSS_GROUP_NUMBER) ||
		((fgChargeMode != TRUE) && (fgChargeMode != FALSE)))
		return FALSE;

	reg = ((((UINT32)u2ChargeLenValue << UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_OFFSET) & UMAC_CHARGE_MODE_WDATA_CHARGE_LENGTH_MASK) |
		   (((UINT32)u2ChargeTimeValue << UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_OFFSET) & UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_MASK));
	RTMP_IO_WRITE32(pvow_pad->hdev_ctrl, UMAC_DRR_TABLE_WDATA0, reg);
	reg = ((UMAC_DRR_TABLE_CTRL0_EXEC_MASK) |
		   ((UMAC_CHARGE_BW_DRR_OP_MASK | (fgChargeMode << UMAC_CHARGE_ADD_MODE_BIT_MASK)) << UMAC_DRR_TABLE_CTRL0_MODE_OP_OFFSET) |
		   ((ucBssGroup << UMAC_CHARGE_MODE_BSS_GROUP_OFFSET) & UMAC_CHARGE_MODE_BSS_GROUP_MASK));
	RTMP_IO_WRITE32(pvow_pad->hdev_ctrl, UMAC_DRR_TABLE_CTRL0, reg);
	return TRUE;
}

BOOLEAN halUmacVoWChargeBwDrrLength(
	IN UINT8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeLenValue
)
{
	return halUmacVoWChargeBwDrr(ucBssGroup, fgChargeMode, u2ChargeLenValue, 0);
}

BOOLEAN halUmacVoWChargeBwDrrTime(
	IN UINT8 ucBssGroup,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeTimeValue
)
{
	return halUmacVoWChargeBwDrr(ucBssGroup, fgChargeMode, 0, u2ChargeTimeValue);
}


BOOLEAN halUmacVoWChargeAitTimeDRR(
	IN UINT16 u2StaID,
	IN UINT8 ucAcId,
	IN BOOLEAN fgChargeMode,
	IN UINT16 u2ChargeValue
)
{
	UINT32 reg;

	if ((u2StaID > WTBL_MAX_NUM(pvow_pad)) ||
		(ucAcId >= 4) ||
		((fgChargeMode != TRUE) && (fgChargeMode != FALSE)))
		return FALSE;

	reg = (((UINT32)u2ChargeValue << UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_OFFSET) & UMAC_CHARGE_MODE_WDATA_CHARGE_TIME_MASK);
	RTMP_IO_WRITE32(pvow_pad->hdev_ctrl, UMAC_DRR_TABLE_WDATA0, reg);
	reg = ((UMAC_DRR_TABLE_CTRL0_EXEC_MASK) |
		   ((UMAC_CHARGE_AIRTIME_DRR_OP_MASK | (((UINT32)fgChargeMode << (ffs(UMAC_CHARGE_ADD_MODE_BIT_MASK)-1) & UMAC_CHARGE_ADD_MODE_BIT_MASK))) << UMAC_DRR_TABLE_CTRL0_MODE_OP_OFFSET) |
		   ((u2StaID << UMAC_CHARGE_MODE_STA_ID_OFFSET) & UMAC_CHARGE_MODE_STA_ID_MASK) |
		   ((ucAcId << UMAC_CHARGE_MODE_QUEUE_ID_OFFSET) & UMAC_CHARGE_MODE_QUEUE_ID_MASK));
	RTMP_IO_WRITE32(pvow_pad->hdev_ctrl, UMAC_DRR_TABLE_CTRL0, reg);
	return TRUE;
}



INT set_vow_charge_sta_dwrr(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, ac, mode, val, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d-%d", &sta, &mode, &ac, &val);

		if ((rv > 3) && (IS_WCID_VALID(pAd, sta)) && (ac < WMM_NUM_OF_AC)) {
			halUmacVoWChargeAitTimeDRR(sta, ac, mode, val);
			MTWF_PRINT("%s: sta%d/ac%d %c charge--> %u.\n", __func__, sta, ac, mode == 0 ? 'd' : 'a', val);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_charge_bw_time(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, mode, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &group, &mode, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			halUmacVoWChargeBwTokenTime(group, mode, val);
			MTWF_PRINT("%s: group%d %c charge--> %u.\n", __func__, group, mode == 0 ? 'd' : 'a', val);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_charge_bw_len(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, mode, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &group, &mode, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			halUmacVoWChargeBwTokenLength(group, mode, val);
			MTWF_PRINT("%s: group%d %c charge--> %u.\n", __func__, group, mode == 0 ? 'd' : 'a', val);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_charge_bw_dwrr(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val, mode, rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &group, &mode, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) {
			halUmacVoWChargeBwDrrTime(group, mode, val);
			MTWF_PRINT("%s: group%d %c charge--> %u.\n", __func__, group, mode == 0 ? 'd' : 'a', val);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_sta_psm(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, psm, rv;
	INT ret = TRUE;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (arg) {
		rv = sscanf(arg, "%d-%d", &sta, &psm);

		if ((rv > 1) && (IS_WCID_VALID(pAd, sta))) {
			UINT32 offset, reg = 0;

			if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON) {
				ret = chip_dbg->set_sta_psm(pAd, sta, psm);
				if (ret != TRUE)
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"set psm failed\n");
			} else {
				/* clear PSM update mask bit31 */
				RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WTBLON, 0x80000000);
				/* set PSM bit in WTBL DW3 bit 30 */
				offset = (sta << 8) | 0x3000c;
				RTMP_IO_READ32(pAd->hdev_ctrl, offset, &reg);

				if (psm) {
					reg |= 0x40000000;
					RTMP_IO_WRITE32(pAd->hdev_ctrl, offset, reg);
				} else {
					reg &= ~0x40000000;
					RTMP_IO_WRITE32(pAd->hdev_ctrl, offset, reg);
				}
			}

			MTWF_PRINT("%s: sta%d psm--> %u.\n", __func__, sta, psm);
		} else
			ret = FALSE;
	} else
		ret = FALSE;

	return ret;
}



INT set_vow_monitor_sta(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &sta);

		if ((rv > 0) && (IS_WCID_VALID(pAd, sta))) {
			pAd->vow_monitor_sta = sta;
			MTWF_PRINT("%s: monitor sta%d.\n", __func__, sta);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_monitor_bss(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 bss, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &bss);

		if ((rv > 0) && (bss <= 16)) {
			pAd->vow_monitor_bss = bss;
			MTWF_PRINT("%s: monitor bss%d.\n", __func__, bss);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_monitor_mbss(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 bss, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &bss);

		if ((rv > 0) && (bss < 16)) {
			pAd->vow_monitor_mbss = bss;
			MTWF_PRINT("%s: monitor mbss%d.\n", __func__, bss);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_avg_num(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 num, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &num);

		if ((rv > 0) && (num < 1000)) {
			pAd->vow_avg_num = num;
			MTWF_PRINT("%s: average numer %d.\n", __func__, num);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_dvt_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 en, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &en);

		if (rv > 0) {
			if (en == 0) {
				vow_reset(pAd);
				vow_variable_reset(pAd);
				vow_init(pAd);
			}
			pAd->vow_dvt_en = en;
			MTWF_PRINT("%s: DVT enable %d.\n", __func__, pAd->vow_dvt_en);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_show_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 en, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &en);

		if (rv > 0) {
			pAd->vow_show_en = en;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				uni_vmd_vow_set_feature_all(pAd);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				vow_set_feature_all(pAd);
			MTWF_PRINT("%s: DVT Show enable %d.\n", __func__, pAd->vow_show_en);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_help(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	MTWF_PRINT("======== Group table =========\n"
			  "vow_min_rate_token = <group>-<token>\n"
			  "vow_max_rate_token = <group>-<token>\n"
			  "vow_min_airtime_token = <group>-<token>\n"
			  "vow_max_airtime_token = <group>-<token>\n"
			  "vow_min_rate_bucket = <group>-<byte> 1K\n"
			  "vow_max_rate_bucket = <group>-<byte> 1K\n"
			  "vow_min_airtime_bucket = <group>-<time> 1.024\n"
			  "vow_max_airtime_bucket = <group>-<time> 1.024\n"
			  "vow_max_wait_time = <group>-<time> 1.024\n"
			  "vow_max_backlog_size = <group>-<byte> 1K\n"
			  "======== Control =============\n"
			  "vow_bw_enable = <0/1> 0:disable, 1:enable\n"
			  "vow_refill_en = <0/1> 0:disable, 1:enable\n"
			  "vow_airtime_fairness_en = <0/1> 0:disable, 1:enable\n"
			  "vow_txop_switch_bss_en = <0/1> 0:disable, 1:enable\n"
			  "vow_dbdc_search_rule = <band>-<0/1> 0:WMM AC, 1:WMM set\n"
			  "vow_refill_period = <n> 2^n\n"
			  "vow_bss_enable = <group>-<0/1> 0:disable, 1:enable\n"
			  "vow_spl_sta_num = <sta num> in txop\n"
			  "vow_airtime_control_en = <group>-<0/1> 0:disable, 1:enable\n"
			  "vow_bw_control_en = <group>-<0/1> 0:disable, 1:enable\n"
			  "vow_sch_ctrl = <type>-<policy> (type->0:algo,1:hw, policy->0:SRR,1:WRR)\n");
	MTWF_PRINT("======== Group others =============\n"
			  "vow_bss_dwrr_quantum = <group>-<time> 256us\n"
			  "vow_group_dwrr_max_wait_time = <time> 256us\n"
			  "vow_group2band_map = <group>-<band>\n"
			  "======== Station table =============\n"
			  "vow_sta_dwrr_quantum = <Qid>-<val> 256us\n"
			  "vow_sta_dwrr_quantum_id = <wlanidx>-<WMMA AC>-<Qid>\n"
			  "vow_sta_ac_priority = <wlanidx>-<0/1/2> 0:disable, 1:BE, 2:BK\n"
			  "vow_sta_pause = <wlanidx>-<0/1> 0: normal, 1: pause\n"
			  "vow_sta_psm = <wlanidx>-<0/1> 0: normal, 1: power save\n"
			  "vow_sta_group = <wlanidx>-<group>\n"
			  "vow_dwrr_max_wait_time = <time> 256us\n"
			  "======== User Config =============\n"
			  "vow_min_rate = <group>-<Mbps>\n"
			  "vow_max_rate = <group>-<Mbps>\n"
			  "vow_min_ratio = <group>-<%%>\n"
			  "vow_max_ratio = <group>-<%%>\n");
	MTWF_PRINT("======== Rx Config =============\n"
			  "vow_rx_counter_clr = <n>\n"
			  "vow_rx_airtime_en = <0/1> 0:dieable, 1:enable\n"
			  "vow_rx_early_end_en = <0/1> 0:dieable, 1:enable\n"
			  "vow_rx_ed_offset = <val> 1.024(5b)\n"
			  "vow_rx_obss_backoff = <val> 1.024(16b)\n"
			  "vow_rx_wmm_backoff = <WMM set>-<WMM AC>-<val>\n"
			  "vow_om_wmm_backoff = <WMM AC>-<val>\n"
			  "vow_repeater_wmm_backoff = <WMM AC>-<val>\n"
			  "vow_rx_non_qos_backoff = <val>\n"
			  "vow_rx_bss_wmmset = <MBSS idx>-<0/1/2/3>\n"
			  "vow_rx_om_wmm_sel = <OM idx>-<val> 0:RX WMM(1to1), 1:OM wmm\n"
			  "======== Airtime estimator =============\n"
			  "vow_at_est_en = <0/1> 0:dieable, 1:enable\n"
			  "vow_at_mon_period = <period> ms\n");
	MTWF_PRINT("======== Badnode detector =============\n"
			  "vow_bn_en = <0/1> 0:dieable, 1:enable\n"
			  "vow_bn_mon_period = <period> ms\n"
			  "vow_bn_fallback_th = <count>\n"
			  "vow_bn_per_th = <TX PER>\n"
			  "======== Airtime counter test =============\n"
			  "vow_counter_test = <0/1> 0:dieable, 1:enable\n"
			  "vow_counter_test_period = <period> ms\n"
			  "vow_counter_test_band = <band>\n"
			  "vow_counter_test_avgcnt = <average num> sec\n"
			  "vow_counter_test_target = <wlanidx>\n"
			  "======== DVT =============\n"
			  "vow_dvt_en = <0/1> 0:dieable, 1:enable\n"
			  "vow_monitor_sta = <STA num>\n"
			  "vow_show_sta = <STA num>\n"
			  "vow_monitor_bss = <BSS num>\n"
			  "vow_monitor_mbss = <MBSS num>\n"
			  "vow_show_mbss = <MBSS num>\n"
			  "vow_avg_num = <average num> sec\n"
			  "======== RED ===========\n"
			  "vow_set_red_en = <0/1> 0:disable, 1:enable\n"
			  "vow_set_red_show_sta = <STA num>\n"
			  "vow_set_red_tar_delay = <tarDelay> us\n"
			  "======== WATF ===========\n"
			  "vow_watf_en = <0/1> 0:disable, 1:enable\n"
			  "vow_watf_q = <level>-<quantum> unit 256us\n"
			  "vow_watf_add_entry = <level>-<Addr>\n"
			  "vow_watf_del_entry = <Addr>\n"
			 );
	return TRUE;
}


/* commands for show */
INT set_vow_show_sta(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &sta);

		if ((rv > 0) && (IS_WCID_VALID(pAd, sta))) {
			pAd->vow_show_sta = sta;
			MTWF_PRINT("%s: show station up to %d.\n", __func__, pAd->vow_show_sta);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_show_mbss(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 mbss, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &mbss);

		if ((rv > 0) && (mbss <= 16)) {
			pAd->vow_show_mbss = mbss;
			MTWF_PRINT("%s: show MBSS up to %d.\n", __func__, pAd->vow_show_mbss);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT show_vow_dump_vow(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	vow_dump_umac_CRs(pAd);
	return TRUE;
}

INT show_vow_dump_sta(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, val1 = 0, val2 = 0, val3 = 0, val4 = 0, val;
	UINT32 rv;
	UINT32 opmode = 0, start, end;

	if (arg == NULL)
		opmode = 0x20;
	else {
		rv = sscanf(arg, "0x%x", &opmode);
		if (rv < 1)
			rv = sscanf(arg, "%d", &opmode);
		if (rv < 1)
			opmode = 0x22;
	}
	if ((opmode == 0x22) || (opmode == 0x23)) {
		start = pAd->vow_gen.VOW_STA_SETTING_BEGIN;
		end = pAd->vow_gen.VOW_STA_SETTING_END;
	} else {
		start = 0;
		end = WTBL_MAX_NUM(pAd);
	}

	for (sta = start; sta < end; sta++) {
		val = 0x80000000 | (((UINT8)(opmode))<<16);
		if (opmode == 0x00)
			val |= ((sta << 2) | 0x01);
		else
		val |= sta;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x8388, val);
		udelay(100);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8350, &val1);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8354, &val2);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8358, &val3);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x835c, &val4);
		MTWF_PRINT("STA%d: 0x%08X, 0x%08X, 0x%08X, 0x%08X.\n",
					sta, val1, val2, val3, val4);
	}

	return TRUE;
}

INT show_vow_dump_bss_bitmap(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val1 = 0, val2 = 0, val3 = 0, val4 = 0, val;
	UINT32 rv;
	UINT32 opmode = 0, start, end;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (arg == NULL)
		opmode = 0x42;
	else {
		rv = sscanf(arg, "0x%x", &opmode);
		if (rv < 1)
			rv = sscanf(arg, "%d", &opmode);
		if (rv < 1)
			opmode = 0x42;
	}

	if ((opmode == 0x22) || (opmode == 0x23)) {
		start = pAd->vow_gen.VOW_STA_BITMAP_BEGIN;
		end = pAd->vow_gen.VOW_STA_BITMAP_END;
	} else {
		start = 0;
		end = UMAC_BSS_GROUP_NUMBER;
	}

	if ((pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG) && chip_dbg->show_bss_bitmap) {
		chip_dbg->show_bss_bitmap(pAd, start, end - 1);
		return TRUE;
	}

	for (group = start; group < end; group++) {
		if (pAd->vow_gen.VOW_GEN == VOW_GEN_1)
			val = 0x80440000;
		else
			val = 0x80000000 | (((UINT8)(opmode))<<16);

		val |= group;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x8388, val);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8350, &val1);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8354, &val2);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8358, &val3);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x835c, &val4);
		MTWF_PRINT("Group%d BitMap: 0x%08X, 0x%08X, 0x%08X, 0x%08X.\n", group, val1, val2, val3, val4);
#if (MAX_LEN_OF_MAC_TABLE > 128)
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8690, &val1);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8694, &val2);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8698, &val3);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x869c, &val4);
		MTWF_PRINT("Group%d BitMap: 0x%08X, 0x%08X, 0x%08X, 0x%08X.\n", group, val1, val2, val3, val4);
#endif
	}

	return TRUE;
}

/* dump group token setting */
INT show_vow_dump_bss(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val1 = 0, val2 = 0, val3 = 0, val4 = 0, val = 0;
	UINT32 rv;
	UINT32 opmode = 0, start, end;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (arg == NULL) {
		if (pAd->vow_gen.VOW_GEN == VOW_GEN_1)
			opmode = 0x44;
		else
			opmode = 0x40;
	} else {
		rv = sscanf(arg, "0x%x", &opmode);
		if (rv < 1)
			rv = sscanf(arg, "%d", &opmode);
		if (rv < 1) {
			if (pAd->vow_gen.VOW_GEN == VOW_GEN_1)
				opmode = 0x44;
			else
				opmode = 0x22;
		}
	}
	if ((opmode == 0x44) || (opmode == 0x45) || (opmode == 0x22)) {
		start = VOW_BSS_SETTING_BEGIN;
		end = VOW_BSS_SETTING_END;
	} else {
		start = 0;
		end = UMAC_BSS_GROUP_NUMBER;
	}

	if ((pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG) && chip_dbg->show_bss_setting) {
		chip_dbg->show_bss_setting(pAd, start, end - 1);
		return TRUE;
	}

	for (group = start; group < end; group++) {
		val = 0x80000000|(((UINT8)(opmode))<<16); /* move from BSS sysram to STA sysram */
		val |= group;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x8388, val);
		udelay(100);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8350, &val1);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8354, &val2);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x8358, &val3);
		RTMP_IO_READ32(pAd->hdev_ctrl, 0x835c, &val4);
		MTWF_PRINT("Group%d Config: 0x%08X, 0x%08X, 0x%08X, 0x%08X.\n", group, val1, val2, val3, val4);
	}

	return TRUE;
}

INT vow_show_bss_atoken(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group = 0, val1 = 0, val2 = 0, val3 = 0, val4 = 0, val;
	UINT32 rv;
	UINT32 addr_offset, val_offset;

	if (arg) {
		rv = sscanf(arg, "%d", &group);

		if ((rv > 0) && (group < VOW_MAX_GROUP_NUM)) {
			val = 0x80440000;
			val |= (group + pAd->vow_gen.VOW_BSS_TOKEN_OFFSET);

				addr_offset = 0x8388;
				val_offset = 0x8350;

			RTMP_IO_WRITE32(pAd->hdev_ctrl, addr_offset, val);
			udelay(100);
			RTMP_IO_READ32(pAd->hdev_ctrl, val_offset, &val1);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0x4), &val2);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0x8), &val3);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0xc), &val4);

			MTWF_PRINT("Group%d airtime token: max 0x%08X, min 0x%08X.\n", group, val2, val1);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT vow_show_bss_ltoken(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val1 = 0, val2 = 0, val3 = 0, val4 = 0, val;
	UINT32 rv;
	UINT32 addr_offset, val_offset;

	if (arg) {
		rv = sscanf(arg, "%d", &group);

		if ((rv > 0) && (group < VOW_MAX_GROUP_NUM)) {
			val = 0x80440000;
			val |= (group + pAd->vow_gen.VOW_BSS_TOKEN_OFFSET);

				addr_offset = 0x8388;
				val_offset = 0x8350;

			RTMP_IO_WRITE32(pAd->hdev_ctrl, addr_offset, val);
			udelay(100);
			RTMP_IO_READ32(pAd->hdev_ctrl, val_offset, &val1);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0x4), &val2);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0x8), &val3);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0xc), &val4);

			MTWF_PRINT("Group%d length token: max 0x%08X, min 0x%08X\n", group, val4, val3);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT vow_show_bss_dtoken(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 group, val1_ori = 0, val1_sign_ext, val2_ori = 0, val2_sign_ext, val;
	UINT32 rv;
	UINT32 addr_offset, val_offset;

	if (arg) {
		rv = sscanf(arg, "%d", &group);

		if ((rv > 0) && (group < VOW_MAX_GROUP_NUM)) {
			if (pAd->vow_gen.VOW_GEN == VOW_GEN_1)
				val = 0x80460000;
			else
				val = 0x80480000;
			val |= group;

				addr_offset = 0x8388;
				val_offset = 0x8350;

			RTMP_IO_WRITE32(pAd->hdev_ctrl, addr_offset, val);
			udelay(100);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0x8), &val1_ori);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0xc), &val2_ori);

			val1_sign_ext = val1_ori;
			val2_sign_ext = val2_ori;

			if (val1_ori >> 17)
				val1_sign_ext = ~(0x3FFFF - val1_ori) + 1;
			if (val2_ori >> 17)
				val2_sign_ext = ~(0x3FFFF - val2_ori) + 1;

			MTWF_PRINT("Group%d airtime token: max %d(0x%08X), min %d(0x%08X)\n",
				group, val1_sign_ext, val1_ori, val2_sign_ext, val2_ori);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT vow_show_sta_dtoken(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta = 0, val1 = 0, val2 = 0, val3 = 0, val4 = 0, val;
	UINT32 val1_sign_ext, val2_sign_ext, val3_sign_ext, val4_sign_ext;
	UINT32 rv;
	UINT32 addr_offset, val_offset;

	if (arg) {
		rv = sscanf(arg, "%d", &sta);

		if ((rv > 0) && (IS_WCID_VALID(pAd, sta))) {
			val = 0x80000000;
			val |= (sta << 2);

				addr_offset = 0x8388;
				val_offset = 0x8350;

			/* BK */
			RTMP_IO_WRITE32(pAd->hdev_ctrl, addr_offset, val);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0x8), &val1);
			/* BE */
			val |= 1;
			RTMP_IO_WRITE32(pAd->hdev_ctrl, addr_offset, val);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0x8), &val2);
			/* VI */
			val |= 2;
			RTMP_IO_WRITE32(pAd->hdev_ctrl, addr_offset, val);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0x8), &val3);
			/* VO */
			val |= 3;
			RTMP_IO_WRITE32(pAd->hdev_ctrl, addr_offset, val);
			RTMP_IO_READ32(pAd->hdev_ctrl, (val_offset + 0x8), &val4);

			val1_sign_ext = val1;
			val2_sign_ext = val2;
			val3_sign_ext = val3;
			val4_sign_ext = val4;

			if (val1 >> 17)
				val1_sign_ext = ~(0x3FFFF - val1) + 1;
			if (val2 >> 17)
				val2_sign_ext = ~(0x3FFFF - val2) + 1;
			if (val3 >> 17)
				val3_sign_ext = ~(0x3FFFF - val3) + 1;
			if (val4 >> 17)
				val4_sign_ext = ~(0x3FFFF - val4) + 1;

			MTWF_PRINT("Sta%d deficit token: ac0 %d(0x%08X), ac1 %d(0x%08X), ac2 %d(0x%08X), ac3 %d(0x%08X)\n",
				sta, val1_sign_ext, val1, val2_sign_ext, val2, val3_sign_ext, val3, val4_sign_ext, val4);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT show_vow_rx_time(
	IN  RTMP_ADAPTER * pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 counter[4];
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		counter[0] = uni_cmd_vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME, 0);
		counter[1] = uni_cmd_vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME, 1);
		counter[2] = uni_cmd_vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME, 0);
		counter[3] = uni_cmd_vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME, 1);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		counter[0] = vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME, 0);
		counter[1] = vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME, 1);
		counter[2] = vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME, 0);
		counter[3] = vow_get_rx_time_counter(pAd, ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME, 1);
	}
	MTWF_PRINT("%s: nonwifi %u/%u, obss %u/%u.\n", __func__, counter[0], counter[1], counter[2], counter[3]);
	return TRUE;
}

INT show_vow_sta_conf(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 sta, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &sta);

		if ((rv > 0) && (IS_WCID_VALID(pAd, sta))) {
			UINT32 pri, q;
			CHAR * pri_str[] = {"No change.", "To BE.", "To BK."};

			MTWF_PRINT("%s: ************ sta%d ***********\n", __func__, sta);
			/* group */
			MTWF_PRINT("Group --> %u\n", pAd->vow_sta_cfg[sta].group);
			/* priority */
			pri = pAd->vow_sta_cfg[sta].ac_change_rule;
			MTWF_PRINT("Priority --> %s(%u)\n", pri_str[pri], pri);
			/* airtime quantum for AC */
			q = pAd->vow_sta_cfg[sta].dwrr_quantum[WMM_AC_BK];
			MTWF_PRINT("Ac0 --> %uus(%u)\n", (pAd->vow_cfg.vow_sta_dwrr_quantum[q] << 8), q);
			q = pAd->vow_sta_cfg[sta].dwrr_quantum[WMM_AC_BE];
			MTWF_PRINT("Ac1 --> %uus(%u)\n", (pAd->vow_cfg.vow_sta_dwrr_quantum[q] << 8), q);
			q = pAd->vow_sta_cfg[sta].dwrr_quantum[WMM_AC_VI];
			MTWF_PRINT("Ac2 --> %uus(%u)\n", (pAd->vow_cfg.vow_sta_dwrr_quantum[q] << 8), q);
			q = pAd->vow_sta_cfg[sta].dwrr_quantum[WMM_AC_VO];
			MTWF_PRINT("Ac3 --> %uus(%u)\n", (pAd->vow_cfg.vow_sta_dwrr_quantum[q] << 8), q);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT show_vow_all_sta_conf(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 i;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
	int ret;

	for (i = 0; i < wtbl_max_num; i++) {
		CHAR str[4];

		ret = snprintf(str, sizeof(str), "%d", i);
		if (os_snprintf_error(sizeof(str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"snprintf error!\n");
			return FALSE;
		}
		show_vow_sta_conf(pAd, str);
	}

	return TRUE;
}

INT show_vow_bss_conf(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 bss, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &bss);

		if ((rv > 0) && (bss < VOW_MAX_GROUP_NUM)) {
			UINT32 val;

			MTWF_PRINT("%s: ************** Group%d **********\n", __func__, bss);
			/* per BSS bw control */
			val = pAd->vow_cfg.per_bss_enable & (1UL << bss);
			MTWF_PRINT("BW control --> %s(%d)\n", val == 0 ? "Disable" : "Enable", val);
			/* per BSS airtime control */
			val = pAd->vow_bss_cfg[bss].at_on;
			/* val = halUmacVoWGetDisablePerBssCheckTimeTokenFeature(bss); */
			MTWF_PRINT("Airtime control --> %s(%d)\n", val == 0 ? "Disable" : "Enable", val);
			/* per BSS TP control */
			/* val = halUmacVoWGetDisablePerBssCheckLengthTokenFeature(bss); */
			val = pAd->vow_bss_cfg[bss].bw_on;
			MTWF_PRINT("Rate control --> %s(%d)\n", val == 0 ? "Disable" : "Enable", val);
			/* Rate Setting */
			MTWF_PRINT("Rate --> %u/%uMbps\n", pAd->vow_bss_cfg[bss].max_rate, pAd->vow_bss_cfg[bss].min_rate);
			/* Airtime Setting */
			MTWF_PRINT("Airtime ratio --> %u/%u %%\n", pAd->vow_bss_cfg[bss].max_airtime_ratio, pAd->vow_bss_cfg[bss].min_airtime_ratio);
			/* Rate token */
			MTWF_PRINT("Rate token --> %u Byte(%u)/%u Byte(%u)\n",
					  (pAd->vow_bss_cfg[bss].max_rate_token >> 3),
					  pAd->vow_bss_cfg[bss].max_rate_token,
					  (pAd->vow_bss_cfg[bss].min_rate_token >> 3),
					  pAd->vow_bss_cfg[bss].min_rate_token);
			MTWF_PRINT("Rate bucket --> %u Byte/%u Byte\n",
					  pAd->vow_bss_cfg[bss].max_ratebucket_size << 10,
					  pAd->vow_bss_cfg[bss].min_ratebucket_size << 10);
			/* Airtime token */
			MTWF_PRINT("Airtime token --> %u us(%u)/%u us(%u)\n",
					  (pAd->vow_bss_cfg[bss].max_airtime_token >> 3),
					  pAd->vow_bss_cfg[bss].max_airtime_token,
					  (pAd->vow_bss_cfg[bss].min_airtime_token >> 3),
					  pAd->vow_bss_cfg[bss].min_airtime_token);
			MTWF_PRINT("Airtime bucket --> %u us/%u us\n",
					  pAd->vow_bss_cfg[bss].max_airtimebucket_size << 10,
					  pAd->vow_bss_cfg[bss].min_airtimebucket_size << 10);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT show_vow_all_bss_conf(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 i;
	int ret;

	for (i = 0; i < VOW_MAX_GROUP_NUM; i++) {
		CHAR str[4];

		ret = snprintf(str, sizeof(str), "%d", i);
		if (os_snprintf_error(sizeof(str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"snprintf error!\n");
			return FALSE;
		}
		show_vow_bss_conf(pAd, str);
	}

	return TRUE;
}

INT vow_show_queue_status(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{

	return TRUE;
}

INT show_vow_near_far(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	NEAR_FAR_CTRL_T *near_far_ctrl = &pAd->vow_misc_cfg.near_far_ctrl;
	UINT32 band_idx;

	MTWF_PRINT("adjust_en = %d\n", near_far_ctrl->adjust_en);
	MTWF_PRINT("slow_phy_th = %d\n", near_far_ctrl->slow_phy_th);
	MTWF_PRINT("fast_phy_th = %d\n", near_far_ctrl->fast_phy_th);

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++)
		MTWF_PRINT("band(%d) near_far_txop_running = %d\n",
			 band_idx, pAd->txop_ctl[band_idx].near_far_txop_running);

	return TRUE;
}

INT show_vow_help(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	MTWF_PRINT("vow_rx_time (Non-wifi/OBSS)\n"
			  "vow_sta_conf = <wlanidx>\n"
			  "vow_sta_conf\n"
			  "vow_bss_conf = <group>\n"
			  "vow_all_bss_conf\n"
			  "vow_dump_sta (raw)\n"
			  "vow_dump_bss_bitmap (raw)\n"
			  "vow_dump_bss (raw)\n"
			  "vow_dump_vow (raw)\n"
			  "vow_show_sta_dtoken = <wlanidx> DWRR\n"
			  "vow_show_bss_dtoken = <group> DWRR\n"
			  "vow_show_bss_atoken = <group> airtime\n"
			  "vow_show_bss_ltoken = <group> length\n"
			  "vow_sch_ctrl(type->0:algo,1:hw, policy->0:SRR,1:WRR)\n");
	return TRUE;
}
/* airtime estimator */
INT set_vow_at_est_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) { /*  */
			INT ret;

			pAd->vow_at_est.at_estimator_en = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_at_estimator(pAd, ENUM_AT_RPOCESS_ESTIMATE_MODULE_CTRL);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_at_estimator(pAd, ENUM_AT_RPOCESS_ESTIMATE_MODULE_CTRL);
			MTWF_PRINT("%s: value %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_at_mon_period(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) { /*  */
			INT ret;

			pAd->vow_at_est.at_monitor_period = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_at_estimator(pAd, ENUM_AT_PROC_EST_MONITOR_PERIOD_CTRL);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_at_estimator(pAd, ENUM_AT_PROC_EST_MONITOR_PERIOD_CTRL);
			MTWF_PRINT("%s: period %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "%s: set command failed.\n", __func__);
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_group2band_map(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, group, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d-%d", &group, &val);

		if ((rv > 1) && (group < VOW_MAX_GROUP_NUM)) { /*  */
			INT ret;

			pAd->vow_bss_cfg[group].band_idx = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_cmd_vow_set_at_estimator_group(pAd, ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING, group);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_at_estimator_group(pAd, ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING, group);
			MTWF_PRINT("%s: group %d, band %u.\n", __func__, group, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}
/* bad node detetor */
INT set_vow_bn_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) { /*  */
			INT ret;

			pAd->vow_badnode.bn_en = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support) {
				AsicNotSupportFunc(pAd, __func__);
				ret = TRUE;
			} else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_bad_node(pAd, ENUM_AT_PROC_BAD_NODE_FEATURE_CTRL);
			MTWF_PRINT("%s: value %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bn_mon_period(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) {
			INT ret;

			pAd->vow_badnode.bn_monitor_period = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support) {
				AsicNotSupportFunc(pAd, __func__);
				ret = TRUE;
			} else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_bad_node(pAd, ENUM_AT_PROC_BAD_NODE_MONITOR_PERIOD_CTRL);
			MTWF_PRINT("%s: period %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bn_fallback_th(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) { /*  */
			INT ret;

			pAd->vow_badnode.bn_fallback_threshold = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support) {
				AsicNotSupportFunc(pAd, __func__);
				ret = TRUE;
			} else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_bad_node(pAd, ENUM_AT_PROC_BAD_NODE_FALLBACK_THRESHOLD);
			MTWF_PRINT("%s: period %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_bn_per_th(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) {
			INT ret;

			pAd->vow_badnode.bn_per_threshold = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support) {
				AsicNotSupportFunc(pAd, __func__);
				ret = TRUE;
			} else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_bad_node(pAd, ENUM_AT_PROC_BAD_NODE_PER_THRESHOLD);
			MTWF_PRINT("%s: per %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_counter_test_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 on, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &on);

		if (rv > 0) {
			MtCmdSetVoWCounterCtrl(pAd, 1, on);
			MTWF_PRINT("%s: on = %d\n", __func__, on);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_counter_test_period(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 period, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &period);

		if (rv > 0) {
			MtCmdSetVoWCounterCtrl(pAd, 2, period);
			MTWF_PRINT("%s: period = %d\n", __func__, period);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_counter_test_band(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 band, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &band);

		if (rv > 0) {
			MtCmdSetVoWCounterCtrl(pAd, 3, band);
			MTWF_PRINT("%s: band = %d\n", __func__, band);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_counter_test_avgcnt(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 avgcnt, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &avgcnt);

		if (rv > 0) {
			MtCmdSetVoWCounterCtrl(pAd, 4, avgcnt);
			MTWF_PRINT("%s: avgcnt = %d\n", __func__, avgcnt);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_counter_test_target(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 target, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &target);

		if (rv > 0) {
			MtCmdSetVoWCounterCtrl(pAd, 5, target);
			MTWF_PRINT("%s: target = %d\n", __func__, target);
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT show_vow_watf_info(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	VOW_WATF	*pwatf;
	UINT8		macAddr[MAC_ADDR_LEN];
	INT			level, Num;

	pwatf = &pAd->vow_watf_mac[0];
	MTWF_PRINT("======== WATF Information ========\n");
	MTWF_PRINT("vow_watf_en: %d\n", pAd->vow_watf_en);
	MTWF_PRINT("vow_watf_q_lv0: %d\n", pAd->vow_watf_q_lv0);
	MTWF_PRINT("vow_watf_q_lv1: %d\n", pAd->vow_watf_q_lv1);
	MTWF_PRINT("vow_watf_q_lv2: %d\n", pAd->vow_watf_q_lv2);
	MTWF_PRINT("vow_watf_q_lv3: %d\n", pAd->vow_watf_q_lv3);

	for (level = 0; level < VOW_WATF_LEVEL_NUM; level++) {
		MTWF_PRINT("======== WATF LV%d's MAC Address List ========\n", level);

		for (Num = 0; Num < pwatf->Num; Num++) {
			NdisMoveMemory(&macAddr, pwatf->Entry[Num].Addr, MAC_ADDR_LEN);
			MTWF_PRINT("Entry %d: "MACSTR"\n",
					  Num, MAC2STR(macAddr));
		}

		pwatf++;
	}

	return TRUE;
}


INT set_vow_watf_en(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 val, rv, ret;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg) {
		rv = sscanf(arg, "%d", &val);

		if (rv > 0) {
			pAd->vow_watf_en = val;
#ifdef WIFI_UNIFIED_COMMAND
			if (pChipCap->uni_cmd_support)
				ret = uni_vmd_vow_set_feature_all(pAd);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				ret = vow_set_feature_all(pAd);
#ifdef RED_SUPPORT
			if (pAd->red_mcu_offload)
				MtCmdCr4Set(pAd,
					    CR4_SET_ID_RED_ENTER_LOW_FREE_PLE_MODE,
					    pAd->vow_cfg.en_bw_ctrl
					    || (pAd->vow_cfg.en_airtime_fairness
						&& pAd->vow_watf_en), 0);
#endif
			MTWF_PRINT("%s: vow_watf_en is set to %u.\n", __func__, val);

			if (ret) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "set command failed.\n");
				return FALSE;
			}
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_watf_q(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT8		*pwatf_string;
	UINT32		val, rv, level;
	int ret;

	if (arg && vow_watf_is_enabled(pAd)) {
		rv = sscanf(arg, "%d-%d", &level, &val);

		if (rv > 1) {
			os_alloc_mem(NULL, (UCHAR **)&pwatf_string, 32);
			if (pwatf_string == NULL)
				return FALSE;

			switch (level) {
			case 0: {
				pAd->vow_watf_q_lv0 =  val;
				ret = snprintf(pwatf_string, 32, "%d-%d", 0, pAd->vow_watf_q_lv0);
				if (os_snprintf_error(32, ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"snprintf error!\n");
					break;
				}
				set_vow_sta_dwrr_quantum(pAd, pwatf_string);
				MTWF_PRINT("vow_watf_q_lv0 is set to %d\n", pAd->vow_watf_q_lv0);
				break;
			}

			case 1: {
				pAd->vow_watf_q_lv1 =  val;
				ret = snprintf(pwatf_string, 32, "%d-%d", 1, pAd->vow_watf_q_lv1);
				if (os_snprintf_error(32, ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"snprintf error!\n");
					break;
				}
				set_vow_sta_dwrr_quantum(pAd, pwatf_string);
				MTWF_PRINT("vow_watf_q_lv1 is set to %d\n", pAd->vow_watf_q_lv1);
				break;
			}

			case 2: {
				pAd->vow_watf_q_lv2 =  val;
				ret = snprintf(pwatf_string, 32, "%d-%d", 2, pAd->vow_watf_q_lv2);
				if (os_snprintf_error(32, ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"snprintf error!\n");
					break;
				}
				set_vow_sta_dwrr_quantum(pAd, pwatf_string);
				MTWF_PRINT("vow_watf_q_lv2 is set to %d\n", pAd->vow_watf_q_lv2);
				break;
			}

			case 3: {
				pAd->vow_watf_q_lv3 =  val;
				ret = snprintf(pwatf_string, 32, "%d-%d", 3, pAd->vow_watf_q_lv3);
				if (os_snprintf_error(32, ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"snprintf error!\n");
					break;
				}
				set_vow_sta_dwrr_quantum(pAd, pwatf_string);
				MTWF_PRINT("vow_watf_q_lv3 is set to %d\n", pAd->vow_watf_q_lv3);
				break;
			}

			default:
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"vow_watf_q_lv is setting fail.\n");
			}

			if (pwatf_string != NULL)
				os_free_mem(pwatf_string);
		} else {
			MTWF_PRINT("Wrong format, vow_watf_q=[Level]-[Quantum]\n"
					  "[Level] should be among 0 to 3 !\n"
					  "[Quantum] unit is 256us.\n");
			return FALSE;
		}
	} else
		return FALSE;

	return TRUE;
}


INT set_vow_watf_add_entry(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32			rv, level;
	UINT32			tmp_macAddr[MAC_ADDR_LEN];
	UINT8			macAddr[MAC_ADDR_LEN];
	VOW_WATF		*pwatf, *ptmpwatf;
	INT			i, j;
	BOOLEAN		isDuplicate = FALSE;

	if (arg) {
		rv = sscanf(arg, "%d-%02x:%02x:%02x:%02x:%02x:%02x",
					&level, &tmp_macAddr[0], &tmp_macAddr[1], &tmp_macAddr[2], &tmp_macAddr[3], &tmp_macAddr[4], &tmp_macAddr[5]);
		pwatf = &pAd->vow_watf_mac[level];
		ptmpwatf = &pAd->vow_watf_mac[0];

		if ((rv == 7) && (level < VOW_WATF_LEVEL_NUM)) {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				macAddr[i] = tmp_macAddr[i];

			for (i = 0; i < VOW_WATF_LEVEL_NUM; i++) {
				for (j = 0; j < ptmpwatf->Num; j++)
					if (memcmp(ptmpwatf->Entry[j].Addr, &macAddr, MAC_ADDR_LEN) == 0) {
						isDuplicate = TRUE;
						MTWF_PRINT("This MAC Address "MACSTR" is duplicate.\n",
								  MAC2STR(macAddr));
						break;
					}

				ptmpwatf++;
			}

			if (!isDuplicate) {
				NdisMoveMemory(pwatf->Entry[pwatf->Num++].Addr, &macAddr, MAC_ADDR_LEN);
				MTWF_PRINT("The entry Level %d - "MACSTR" is set complete!\n",
						  level, MAC2STR(macAddr));
			}
		} else {
			MTWF_PRINT("Wrong format, vow_watf_add_entry=[Level]-[Addr]:[Addr]:[Addr]:[Addr]:[Addr]:[Addr]\n"
					  "[Level] should be among 0 to 3 !\n");
			return FALSE;
		}
	} else
		return FALSE;

	return TRUE;
}

INT set_vow_watf_del_entry(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32			rv, level;
	UINT32			tmp_macAddr[MAC_ADDR_LEN];
	UINT8			macAddr[MAC_ADDR_LEN];
	UINT8			nullAddr[MAC_ADDR_LEN];
	VOW_WATF		*pwatf;
	INT			i;
	INT				j = 0;
	BOOLEAN		isFound = FALSE;

	if (arg) {
		rv = sscanf(arg, "%d-%02x:%02x:%02x:%02x:%02x:%02x",
					&level, &tmp_macAddr[0], &tmp_macAddr[1], &tmp_macAddr[2], &tmp_macAddr[3], &tmp_macAddr[4], &tmp_macAddr[5]);
		pwatf = &pAd->vow_watf_mac[level];
		NdisZeroMemory(nullAddr, MAC_ADDR_LEN);

		if ((rv == 7) && (level < VOW_WATF_LEVEL_NUM)) {
			for (i = 0; i < MAC_ADDR_LEN; i++)
				macAddr[i] = tmp_macAddr[i];

			for (i = 0; i < pwatf->Num; i++) {
				if (memcmp(pwatf->Entry[i].Addr, &macAddr, MAC_ADDR_LEN) == 0) {
					isFound = TRUE;
					NdisZeroMemory(pwatf->Entry[i].Addr, MAC_ADDR_LEN);
					MTWF_PRINT("The entry "MACSTR" founded will be deleted!\n",
							  MAC2STR(macAddr));
					break;
				}
			}

			if (!isFound) {
				MTWF_PRINT("The entry "MACSTR" is not in the list!\n", MAC2STR(macAddr));
			} else {
				for (i = 0; i < pwatf->Num; i++) {
					if (memcmp(pwatf->Entry[i].Addr, &nullAddr, MAC_ADDR_LEN) == 0)
						continue;
					else
						NdisMoveMemory(&(pAd->vow_watf_mac[level].Entry[j++].Addr), pwatf->Entry[i].Addr, MAC_ADDR_LEN);
				}

				pwatf->Num--;
			}
		} else {
			MTWF_PRINT("Wrong format, vow_watf_add_entry=[Level]-[Addr]:[Addr]:[Addr]:[Addr]:[Addr]:[Addr]\n"
					  "[Level] should be among 0 to 3 !\n");
			return FALSE;
		}
	} else
		return FALSE;

	return TRUE;
}

VOID set_vow_watf_sta_dwrr(
	PRTMP_ADAPTER pAd,
	UINT8 *Addr,
	UINT16 Wcid)
{
	VOW_WATF		*pwatf;
	UINT8			i, j;
	UINT8			level = 0;
	BOOLEAN		isFound = FALSE;

	if (vow_watf_is_enabled(pAd)) {
		pwatf = &pAd->vow_watf_mac[0];

		for (i = 0; i < VOW_WATF_LEVEL_NUM; i++) {
			for (j = 0; j < pwatf->Num; j++) {
				if (memcmp(pwatf->Entry[j].Addr, Addr, MAC_ADDR_LEN) == 0) {
					isFound = TRUE;
					level = i;
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "This MAC Address "MACSTR" is found in list.\n",
							  MAC2STR(Addr));
					break;
				}
			}

			pwatf++;
		}

		if (isFound) {
			for (i = 0; i < 4; i++)
				pAd->vow_sta_cfg[Wcid].dwrr_quantum[i] = level;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Update STA %d's DWRR quantum with LV%d\n", Wcid, level);
		} else {
			for (i = 0; i < 4; i++)
				pAd->vow_sta_cfg[Wcid].dwrr_quantum[i] = level;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Update STA %d's DWRR quantum with default LV%d\n", Wcid, level);
		}
	}
}


VOID vow_reset_watf(
	IN PRTMP_ADAPTER pad)
{
	/* When interface down, it will reset WATF table. */
	VOW_WATF	*pwatf;
	INT	i, j;

	if (pad->vow_watf_en) {
		pwatf = &pad->vow_watf_mac[0];

		for (i = 0; i < VOW_WATF_LEVEL_NUM; i++) {
			for (j = 0; j < pwatf->Num; j++)
				NdisZeroMemory(pwatf->Entry[j].Addr, MAC_ADDR_LEN);

			pwatf->Num = 0;
			pwatf++;
		}
	}
}


INT show_vow_info(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	MTWF_PRINT("======== VOW Control Information ========\n");
	MTWF_PRINT("ATC Enbale: %d\n", pAd->vow_cfg.en_bw_ctrl);
	MTWF_PRINT("ATF Enbale: %d\n", pAd->vow_cfg.en_airtime_fairness);
	MTWF_PRINT("WATF Enable: %d\n", pAd->vow_watf_en);
	MTWF_PRINT("en_bw_refill: %d\n", pAd->vow_cfg.en_bw_refill);
	MTWF_PRINT("en_txop_no_change_bss: %d\n", pAd->vow_cfg.en_txop_no_change_bss);
	MTWF_PRINT("dbdc0_search_rule: %d\n", pAd->vow_cfg.dbdc0_search_rule);
	MTWF_PRINT("dbdc1_search_rule: %d\n", pAd->vow_cfg.dbdc1_search_rule);
	MTWF_PRINT("refill_period: %d\n", pAd->vow_cfg.refill_period);
	MTWF_PRINT("SPL sta num: %d\n", pAd->vow_misc_cfg.spl_sta_count);
	MTWF_PRINT("======== VOW Max Deficit Information ========\n");
	MTWF_PRINT("VOW Max Deficit(unit 256us): %d\n", pAd->vow_cfg.sta_max_wait_time);
	MTWF_PRINT("======== VOW Quantum Information ========\n");
	MTWF_PRINT("Quantum ID 0 value(unit 256us): %d\n", pAd->vow_cfg.vow_sta_dwrr_quantum[0]);
	MTWF_PRINT("Quantum ID 1 value(unit 256us): %d\n", pAd->vow_cfg.vow_sta_dwrr_quantum[1]);
	MTWF_PRINT("Quantum ID 2 value(unit 256us): %d\n", pAd->vow_cfg.vow_sta_dwrr_quantum[2]);
	MTWF_PRINT("Quantum ID 3 value(unit 256us): %d\n", pAd->vow_cfg.vow_sta_dwrr_quantum[3]);
	return TRUE;
}

VOID vow_reset_dvt(
	IN PRTMP_ADAPTER pad)
{
	/* When interface down, it will reset DVT parameters. */
	pad->vow_dvt_en = 0;
	NdisZeroMemory(vow_tx_time, sizeof(vow_tx_time));
	NdisZeroMemory(vow_rx_time, sizeof(vow_rx_time));
	NdisZeroMemory(vow_tx_ok, sizeof(vow_tx_ok));
	NdisZeroMemory(vow_tx_fail, sizeof(vow_tx_fail));
	NdisZeroMemory(vow_last_tx_time, sizeof(vow_last_tx_time));
	NdisZeroMemory(vow_last_rx_time, sizeof(vow_last_rx_time));
	NdisZeroMemory(vow_tx_bss_byte, sizeof(vow_tx_bss_byte));
	NdisZeroMemory(vow_rx_bss_byte, sizeof(vow_rx_bss_byte));
	NdisZeroMemory(vow_tx_mbss_byte, sizeof(vow_tx_mbss_byte));
	NdisZeroMemory(vow_rx_mbss_byte, sizeof(vow_rx_mbss_byte));
	vow_sum_tx_rx_time = 0;
	vow_avg_sum_time = 0;
	vow_idx = 0;
	vow_ampdu_cnt = 0;
	vow_interval = 0;
	vow_last_free_cnt = 0;
}

VOID vow_display_info_periodic(
	IN  PRTMP_ADAPTER pAd)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (pAd->vow_show_en == 1) {
		CHAR i, ac, bw;
		UINT32 wtbl_offset;
		UINT32 tx_sum, tx = 0, rx_sum, rx = 0, tx_ok[2], tx_fail[2],
		tx_cnt = 0, tx_ok_sum, tx_fail_sum, tx_diff_time, rx_diff_time;
		UINT32 cnt = 0, free_cnt = 0;
		UINT32 offset;
		UINT32 ple_stat[ALL_CR_NUM_OF_ALL_AC + 1] = {0}, pg_flow_ctrl[8] = {0};
		UINT32 sta_pause[16] = {0}, dis_sta_map[16] = {0};
		UINT32 ple_txcmd_stat;
		INT32 k, l;
		UINT32 counter[2] = {0};
		UINT32 at_info[4] = {0};
		UINT32 addr = 0, phymode, rate, DW6 = 0, DW10 = 0;
		UINT32 ple_stat_num;
		UINT32 total_nonempty_cnt = 0;

		vow_idx++;

		/* airtime */
		for (i = 0; i <= pAd->vow_monitor_sta; i++) {
			wtbl_offset = (i << 8) | 0x3004C;
			tx_sum = rx_sum = 0;

			if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON) {
				for (ac = 0; ac < 4; ac++) {
					tx = chip_dbg->get_sta_airtime(pAd, i, ac, TRUE);
					tx_sum += tx;
					rx = chip_dbg->get_sta_airtime(pAd, i, ac, FALSE);
					rx_sum += rx;
				}
			} else {
				for (ac = 0; ac < 4; ac++) {
					RTMP_IO_READ32(pAd->hdev_ctrl, wtbl_offset + (ac << 3), &tx);
					tx_sum += tx;
					RTMP_IO_READ32(pAd->hdev_ctrl, wtbl_offset + (ac << 3) + 4, &rx);
					rx_sum += rx;
				}
			}

			/* clear WTBL airtime statistic */
			tx_diff_time = tx_sum - vow_last_tx_time[i];
			rx_diff_time = rx_sum - vow_last_rx_time[i];
			vow_tx_time[i] += tx_diff_time;
			vow_rx_time[i] += rx_diff_time;
			vow_last_tx_time[i] = tx_sum;
			vow_last_rx_time[i] = rx_sum;
			vow_sum_tx_rx_time += tx_diff_time + rx_diff_time;
			vow_avg_sum_time += tx_diff_time + rx_diff_time;

			if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON) {
				addr = chip_dbg->get_sta_addr(pAd, i);
				addr = addr & 0xffff;
				DW10 = chip_dbg->get_sta_rate(pAd, i);
				phymode = (DW10 & 0x3c0) >> 6;
				rate = DW10 & 0x3f;
			} else {
				wtbl_offset = (i << 8) | 0x30000;
				RTMP_IO_READ32(pAd->hdev_ctrl, wtbl_offset, &addr);
				addr = addr & 0xffff;
				wtbl_offset = (i << 8) | 0x30018;
				RTMP_IO_READ32(pAd->hdev_ctrl, wtbl_offset, &DW6);
				phymode = (DW6 & 0x1c0) >> 6;
				rate = DW6 & 0x3f;
			}

			if (i <= pAd->vow_show_sta) {
				MTWF_PRINT("sta%d: tx -> %u, rx -> %u, vow_idx %d\n",
						 i, tx_diff_time, rx_diff_time, vow_idx);
				MTWF_PRINT("sta%d: addr %x:%x, Mode %d, MCS %d, vow_idx %d\n",
						 i, (addr & 0xff), (addr >> 8), phymode, rate, vow_idx);
			}

			if (vow_idx == pAd->vow_avg_num) {
				MTWF_PRINT("AVG sta%d: tx -> %u(%u), rx -> %u(%u)\n", i,
						 vow_tx_time[i]/pAd->vow_avg_num,
						 vow_tx_time[i],
						 vow_rx_time[i]/pAd->vow_avg_num,
						 vow_rx_time[i]);
				vow_avg_sum_time = 0;
				vow_tx_time[i] = 0;
				vow_rx_time[i] = 0;
			}
		}

		MTWF_PRINT("Total Airtime: %u\n", vow_sum_tx_rx_time);
		vow_sum_tx_rx_time = 0;

		/* tx counter */
		for (i = 1; i <= pAd->vow_monitor_sta; i++) {
			wtbl_offset = (i << 8) | 0x30040;
			tx_ok_sum = tx_fail_sum = 0;

			for (bw = 0; bw < 2; bw++) {
				if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON)
					tx_cnt = chip_dbg->get_sta_tx_cnt(pAd, i, bw);
				else
					RTMP_IO_READ32(pAd->hdev_ctrl, wtbl_offset + (bw << 2), &tx_cnt);

				tx_ok_sum += (tx_cnt & 0xffff);
				tx_ok[bw] = (tx_cnt & 0xffff);
				tx_fail_sum += ((tx_cnt >> 16) & 0xffff);
				tx_fail[bw] = ((tx_cnt >> 16) & 0xffff);
			}

			if (i <= pAd->vow_show_sta) {
				MTWF_PRINT("sta%d: tx cnt -> %u/%u, tx fail -> %u/%u, vow_idx %d\n",
						 i, tx_ok[0], tx_ok[1], tx_fail[0], tx_fail[1], vow_idx);
			}

			vow_tx_ok[i] += tx_ok_sum;
			vow_tx_fail[i] += tx_fail_sum;

			if (vow_idx == pAd->vow_avg_num) {
				MTWF_PRINT("AVG sta%d: tx cnt -> %u(%u), tx fail -> %u(%u)\n", i,
						  vow_tx_ok[i]/pAd->vow_avg_num,
						  vow_tx_ok[i],
						  vow_tx_fail[i]/pAd->vow_avg_num,
						  vow_tx_fail[i]);
				vow_tx_ok[i] = 0;
				vow_tx_fail[i] = 0;
			}
		}

		/* throughput */
		for (i = 0; i <= pAd->vow_monitor_bss; i++) {
			UINT32 txb = 0, rxb = 0;
			if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON) {
				RTMP_IO_READ32(pAd->hdev_ctrl, 0x31110 + (i << 2), &txb);
				RTMP_IO_READ32(pAd->hdev_ctrl, 0x31130 + (i << 2), &rxb);
			} else {
				RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON + 0x110 + (i << 2), &txb);
				RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON + 0x130 + (i << 2), &rxb);
			}
			MTWF_PRINT("BSS%d: tx byte -> %u, rx byte -> %u\n", i, txb, rxb);
			vow_tx_bss_byte[i] += txb;
			vow_rx_bss_byte[i] += rxb;

			if (vow_idx == pAd->vow_avg_num) {
				MTWF_PRINT("AVG bss%d: tx -> %u(%u), rx -> %u(%u)\n", i,
						  vow_tx_bss_byte[i]/pAd->vow_avg_num,
						  vow_tx_bss_byte[i],
						  vow_rx_bss_byte[i]/pAd->vow_avg_num,
						  vow_rx_bss_byte[i]);
				vow_tx_bss_byte[i] = 0;
				vow_rx_bss_byte[i] = 0;
			}
		}

		for (i = 0; i <= pAd->vow_monitor_mbss; i++) {
			UINT32 txb = 0, rxb = 0;
			if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON) {
				RTMP_IO_READ32(pAd->hdev_ctrl, 0x31240 + (i << 2), &txb);
				RTMP_IO_READ32(pAd->hdev_ctrl, 0x312c0 + (i << 2), &rxb);
			} else {
				RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON + 0x240 + (i << 2), &txb);
				RTMP_IO_READ32(pAd->hdev_ctrl, WF_WTBLON + 0x2C0 + (i << 2), &rxb);
			}
			if (i < pAd->vow_show_mbss)
				MTWF_PRINT("MBSS%d: tx byte -> %u, rx byte -> %u\n", i, txb, rxb);

			vow_tx_mbss_byte[i] += txb;
			vow_rx_mbss_byte[i] += rxb;

			if (vow_idx == pAd->vow_avg_num) {
				MTWF_PRINT("AVG mbss%d: tx -> %u(%u), rx -> %u(%u)\n", i,
						  vow_tx_mbss_byte[i]/pAd->vow_avg_num,
						  vow_tx_mbss_byte[i],
						  vow_rx_mbss_byte[i]/pAd->vow_avg_num,
						  vow_rx_mbss_byte[i]);
				vow_tx_mbss_byte[i] = 0;
				vow_rx_mbss_byte[i] = 0;
			}
		}

		/* read LPON free run counter */
		if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON)
			free_cnt = chip_dbg->get_lpon_frcr(pAd);
		else
			RTMP_IO_READ32(pAd->hdev_ctrl, LPON_FREE_RUN, &free_cnt);

		if (vow_last_free_cnt) {
			vow_interval += (free_cnt - vow_last_free_cnt);
			MTWF_PRINT("free count %d\n", free_cnt - vow_last_free_cnt);
			vow_last_free_cnt = free_cnt;
		}

		vow_last_free_cnt = free_cnt;
		/* read AMPDU count */
		if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON)
			offset = 0x2d038;
		else
			offset = MIB_M0SDR14;

		RTMP_IO_READ32(pAd->hdev_ctrl, offset, &cnt);
		vow_ampdu_cnt += cnt;
		MTWF_PRINT("AMPDU[Band0] count %d\n", cnt);
		if (pAd->CommonCfg.dbdc_mode) {
			RTMP_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR14 + 0x200, &cnt);
			vow_ampdu_cnt += cnt;
			MTWF_PRINT("AMPDU[Band1] count %d\n", cnt);
		}
		if (vow_idx == pAd->vow_avg_num) {
			MTWF_PRINT("total ampdu cnt -> %u, avg ampdu cnt --> %d\n",
					 vow_ampdu_cnt, vow_ampdu_cnt/pAd->vow_avg_num);
			MTWF_PRINT("total interval -> %u, avg interval --> %d\n",
					 vow_interval, vow_interval/pAd->vow_avg_num);
			vow_interval = 0;
			vow_idx = 0;
			vow_ampdu_cnt = 0;
		}

		/* Show obss/non-wifi airtime */
		if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON) {
			chip_dbg->get_obss_nonwifi_airtime(pAd, at_info);
			MTWF_PRINT("nonwifi %u/%u, obss %u/%u.\n", at_info[0], at_info[1], at_info[2], at_info[3]);
		} else {
			HW_IO_READ32(pAd->hdev_ctrl, 0x215B4, &counter[0]); /* nonwifi airtime counter - band 0 */
			HW_IO_READ32(pAd->hdev_ctrl, 0x215B8, &counter[1]); /* obss airtime counter - band 0 */
			MTWF_PRINT("nonwifi: %u, obss: %u.\n", counter[0], counter[1]);
		}

		/* PLE Info  Mark for Debug */
		if (pAd->vow_gen.VOW_GEN == VOW_GEN_FALCON) {
			chip_dbg->get_ple_acq_stat(pAd, ple_stat);
			chip_dbg->get_dis_sta_map(pAd, dis_sta_map);
			chip_dbg->get_sta_pause(pAd, sta_pause);
			total_nonempty_cnt = chip_dbg->show_sta_acq_info(pAd, ple_stat, sta_pause, dis_sta_map, 0);
			chip_dbg->get_ple_txcmd_stat(pAd, &ple_txcmd_stat);
			chip_dbg->show_txcmdq_info(pAd, ple_txcmd_stat);
		} else {
			HW_IO_READ32(pAd->hdev_ctrl, PLE_QUEUE_EMPTY, &ple_stat[0]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC0_QUEUE_EMPTY_0, &ple_stat[1]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC0_QUEUE_EMPTY_1, &ple_stat[2]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC0_QUEUE_EMPTY_2, &ple_stat[3]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC0_QUEUE_EMPTY_3, &ple_stat[4]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC1_QUEUE_EMPTY_0, &ple_stat[5]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC1_QUEUE_EMPTY_1, &ple_stat[6]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC1_QUEUE_EMPTY_2, &ple_stat[7]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC1_QUEUE_EMPTY_3, &ple_stat[8]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC2_QUEUE_EMPTY_0, &ple_stat[9]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC2_QUEUE_EMPTY_1, &ple_stat[10]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC2_QUEUE_EMPTY_2, &ple_stat[11]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC2_QUEUE_EMPTY_3, &ple_stat[12]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC3_QUEUE_EMPTY_0, &ple_stat[13]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC3_QUEUE_EMPTY_1, &ple_stat[14]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC3_QUEUE_EMPTY_2, &ple_stat[15]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC3_QUEUE_EMPTY_3, &ple_stat[16]);
#if (MAX_LEN_OF_MAC_TABLE > 128)
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC0_QUEUE_EMPTY_4, &ple_stat[17]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC0_QUEUE_EMPTY_5, &ple_stat[18]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC0_QUEUE_EMPTY_6, &ple_stat[19]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC0_QUEUE_EMPTY_7, &ple_stat[20]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC1_QUEUE_EMPTY_4, &ple_stat[21]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC1_QUEUE_EMPTY_5, &ple_stat[22]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC1_QUEUE_EMPTY_6, &ple_stat[23]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC1_QUEUE_EMPTY_7, &ple_stat[24]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC2_QUEUE_EMPTY_4, &ple_stat[25]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC2_QUEUE_EMPTY_5, &ple_stat[26]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC2_QUEUE_EMPTY_6, &ple_stat[27]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC2_QUEUE_EMPTY_7, &ple_stat[28]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC3_QUEUE_EMPTY_4, &ple_stat[29]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC3_QUEUE_EMPTY_5, &ple_stat[30]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC3_QUEUE_EMPTY_6, &ple_stat[31]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_AC3_QUEUE_EMPTY_7, &ple_stat[32]);
#endif
			HW_IO_READ32(pAd->hdev_ctrl, PLE_FREEPG_CNT, &pg_flow_ctrl[0]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_FREEPG_HEAD_TAIL, &pg_flow_ctrl[1]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_PG_HIF_GROUP, &pg_flow_ctrl[2]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_HIF_PG_INFO, &pg_flow_ctrl[3]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_PG_CPU_GROUP, &pg_flow_ctrl[4]);
			HW_IO_READ32(pAd->hdev_ctrl, PLE_CPU_PG_INFO, &pg_flow_ctrl[5]);
			HW_IO_READ32(pAd->hdev_ctrl, DIS_STA_MAP0, &dis_sta_map[0]);
			HW_IO_READ32(pAd->hdev_ctrl, DIS_STA_MAP1, &dis_sta_map[1]);
			HW_IO_READ32(pAd->hdev_ctrl, DIS_STA_MAP2, &dis_sta_map[2]);
			HW_IO_READ32(pAd->hdev_ctrl, DIS_STA_MAP3, &dis_sta_map[3]);
#if (MAX_LEN_OF_MAC_TABLE > 128)
			HW_IO_READ32(pAd->hdev_ctrl, DIS_STA_MAP4, &dis_sta_map[4]);
			HW_IO_READ32(pAd->hdev_ctrl, DIS_STA_MAP5, &dis_sta_map[5]);
			HW_IO_READ32(pAd->hdev_ctrl, DIS_STA_MAP6, &dis_sta_map[6]);
			HW_IO_READ32(pAd->hdev_ctrl, DIS_STA_MAP7, &dis_sta_map[7]);
#endif
			HW_IO_READ32(pAd->hdev_ctrl, STATION_PAUSE0, &sta_pause[0]);
			HW_IO_READ32(pAd->hdev_ctrl, STATION_PAUSE1, &sta_pause[1]);
			HW_IO_READ32(pAd->hdev_ctrl, STATION_PAUSE2, &sta_pause[2]);
			HW_IO_READ32(pAd->hdev_ctrl, STATION_PAUSE3, &sta_pause[3]);
#if (MAX_LEN_OF_MAC_TABLE > 128)
			HW_IO_READ32(pAd->hdev_ctrl, STATION_PAUSE4, &sta_pause[4]);
			HW_IO_READ32(pAd->hdev_ctrl, STATION_PAUSE5, &sta_pause[5]);
			HW_IO_READ32(pAd->hdev_ctrl, STATION_PAUSE6, &sta_pause[6]);
			HW_IO_READ32(pAd->hdev_ctrl, STATION_PAUSE7, &sta_pause[7]);
#endif
#if (MAX_LEN_OF_MAC_TABLE > 128)
			ple_stat_num = 32;
			total_nonempty_cnt = 0;
#else
			ple_stat_num = 16;
#endif
			for (l = 1; l <= ple_stat_num; l++) { /* show AC Q info */
				for (k = 0; k < 32; k++) {
					UINT32 hfid, tfid, pktcnt, ac_num; /* ctrl = 0; */
					UINT32 sta_num = k + ((l-1) % 4) * 32 + ((l-1)>>4)*128,
						fl_que_ctrl[3] = {0};
					struct wifi_dev *wdev = wdev_search_by_wcid(pAd, sta_num);
					UINT32 wmmidx = 0;

					/* ple queue empty check */
					if (((ple_stat[l] & (0x1 << k)) >> k) != 0)
						continue;

					if (!IS_WCID_VALID(pAd, sta_num))
						break;

					ac_num = (sta_num < 128) ? ((l-1) >> 2) : ((l-17) >> 2);
					if (wdev)
						wmmidx = HcGetWmmIdx(pAd, wdev);

					MTWF_PRINT("STA%d AC%d: ", sta_num, ac_num);
					fl_que_ctrl[0] |= (0x1 << 31);
					fl_que_ctrl[0] |= (0x2 << 14);
					fl_que_ctrl[0] |= (ac_num << 8);
					fl_que_ctrl[0] |= sta_num;
					HW_IO_WRITE32(pAd->hdev_ctrl, PLE_FL_QUE_CTRL_0, fl_que_ctrl[0]);
					HW_IO_READ32(pAd->hdev_ctrl, PLE_FL_QUE_CTRL_2, &fl_que_ctrl[1]);
					HW_IO_READ32(pAd->hdev_ctrl, PLE_FL_QUE_CTRL_3, &fl_que_ctrl[2]);
					hfid = fl_que_ctrl[1] & 0xfff;
					tfid = (fl_que_ctrl[1] & 0xfff << 16) >> 16;
					pktcnt = fl_que_ctrl[2] & 0xfff;
					total_nonempty_cnt++;
					if (total_nonempty_cnt <= 20)
						MTWF_PRINT("tail/head fid = 0x%03x/0x%03x, pkt cnt = %x\n",
							 tfid, hfid, pktcnt);
				}
			}
		}

		MTWF_PRINT("**************[nonempty STAs : %d]************************\n",
				total_nonempty_cnt);
	}
}

VOID vow_avg_pkt_len_reset(struct _RTMP_ADAPTER *ad)
{
	ad->vow_mcli_ctl.pkt_avg_len = 0;
	ad->vow_mcli_ctl.sta_nums = 0;
}

/*
 *
 */
VOID vow_avg_pkt_len_calculate(struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = entry->wdev->sys_handle;
	UINT32 avg_pkt_len = 0;
	struct multi_cli_ctl *mctrl = &ad->vow_mcli_ctl;

	if (entry->avg_tx_pkts > 0)
		avg_pkt_len = (UINT32)(entry->AvgTxBytes / entry->avg_tx_pkts);

	if ((avg_pkt_len > VERIWAVE_INVALID_PKT_LEN_HIGH) ||
		(avg_pkt_len < VERIWAVE_INVALID_PKT_LEN_LOW))
		return;
	/*moving average for pkt avg length*/
	mctrl->pkt_avg_len =
		((mctrl->pkt_avg_len * mctrl->sta_nums) + avg_pkt_len) / (mctrl->sta_nums + 1);
	mctrl->sta_nums++;
}
#endif /* VOW_SUPPORT */
