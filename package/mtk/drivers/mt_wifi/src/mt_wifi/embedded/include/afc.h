/*
 * Copyright (c) [2022], MediaTek Inc. All rights reserved.
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
	afc.h
*/

#ifndef __AFC_H__
#define __AFC_H__

#ifdef CONFIG_6G_AFC_SUPPORT
/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/
#include "rt_config.h"
/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/
#define TLV_HEADER 2

#define OP_CLASS_131 131
#define OP_CLASS_132 132
#define OP_CLASS_133 133
#define OP_CLASS_134 134
#define OP_CLASS_135 135
#define OP_CLASS_136 136
#define OP_CLASS_137 137

#define MAX_20MHZ_CHANNEL_IN_6G_UNII   24
#define MAX_40MHZ_CHANNEL_IN_6G_UNII   12
#define MAX_80MHZ_CHANNEL_IN_6G_UNII   6
#define MAX_160MHZ_CHANNEL_IN_6G_UNII  3

#define UNII_7_BW20_START_INX  (MAX_20MHZ_CHANNEL_IN_6G_UNII + 5)
#define UNII_7_BW40_START_INX  (MAX_40MHZ_CHANNEL_IN_6G_UNII + 3)
#define UNII_7_BW80_START_INX  (MAX_80MHZ_CHANNEL_IN_6G_UNII + 2)
#define UNII_7_BW160_START_INX (MAX_160MHZ_CHANNEL_IN_6G_UNII + 1)

#define UNII_5_STARTING_FREQ 5945
#define UNII_7_STARTING_FREQ 6525
#define BW20_MHZ             20
#define AFC_STATUS_LEN 4
enum AFC_6G_UNII {
	BAND_6G_UNII_5,
	BAND_6G_UNII_7,
	BAND_6G_UNII_NUM
};

enum AFC_RESPONSE_TAG {
	VERSION = 0,
	ASI_RESPONSE,
	FREQ_INFO,
	CHANNELS_ALLOWED,
	OPER_CLASS,
	CHANNEL_LIST,
	EXPIRY_TIME,
	RESPONSE
};

/*******************************************************************************
 *    MACRO
 ******************************************************************************/

/*******************************************************************************
 *    TYPES
 ******************************************************************************/

struct afc_device_info {
	UINT8 regionCode[6];
	UINT8 glblOperClassNum;
	UINT8 glblOperClass[15];
	UINT16 lowFrequency;
	UINT16 highFrequency;
	short minDesiredPower;
};

struct afc_response {
	UINT16 status;
	UINT8 Resv[2];
	UINT8 data[0];
};

struct AFC_TX_PWR_INFO {
	INT8 max_psd_bw20[MAX_20MHZ_CHANNEL_IN_6G_UNII];
	INT8 max_eirp_bw20[MAX_20MHZ_CHANNEL_IN_6G_UNII];
	INT8 max_eirp_bw40[MAX_40MHZ_CHANNEL_IN_6G_UNII];
	INT8 max_eirp_bw80[MAX_80MHZ_CHANNEL_IN_6G_UNII];
	INT8 max_eirp_bw160[MAX_160MHZ_CHANNEL_IN_6G_UNII];
};

struct AFC_RESPONSE_DATA {
	struct AFC_TX_PWR_INFO afc_txpwr_info[BAND_6G_UNII_NUM];
	UINT32 expiry_time;
	UINT16 response_code;
};


/*******************************************************************************
 *    GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *    FUNCTION PROTOTYPES
 ******************************************************************************/

/** SET **/
INT afc_cmd_set_afc_params(IN struct _RTMP_ADAPTER *pAd, IN RTMP_STRING * arg);

/** GET **/
INT afc_cmd_show_afc_params(IN struct _RTMP_ADAPTER *pAd, IN RTMP_STRING * arg);

/* Event Handler andes_mt.c */
VOID event_afc_handler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT_32 Length);

INT set_afc_event(struct _RTMP_ADAPTER *pAd, char *arg);

void afc_update_params_from_response(struct _RTMP_ADAPTER *pAd, UINT8 *buf_data, UINT32 buf_len);

int afc_daemon_response(struct _RTMP_ADAPTER *pAd, struct __RTMP_IOCTL_INPUT_STRUCT *wrq);

int afc_daemon_channel_info(struct _RTMP_ADAPTER *pAd, struct __RTMP_IOCTL_INPUT_STRUCT *wrq);

#endif /* CONFIG_6G_AFC_SUPPORT */
#endif /* __AFC_H__ */

