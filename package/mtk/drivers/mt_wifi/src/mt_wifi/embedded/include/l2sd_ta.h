/*=============================================================================
//             INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//      Copyright (c) 2009-2010 Intel Corporation. All Rights Reserved.
//
//  File Name: l2sd_ta.h
//  Description:
//===========================================================================*/

#ifndef _L2SD_TA_H_
#define _L2SD_TA_H_

#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"

/* Currently 20 bytes are needed for Query and 24 bytes are needed for Trigger */
#define WIDI_QUERY_TRIGGER_VE_LEN			24
#define WIDI_DEVICE_NAME_LEN				32
#define WIDI_PRIMARY_DEV_LEN				8
#define WIDI_SERVICE_VE_LEN					23
#define WIDI_P2P_IP_VE_LEN					20

/* Status definition */
#define WIDI_DISASSOCIATED					0
#define WIDI_ASSOCIATED						1
#define WIDI_WPS_STATUS_FAIL				2
#define WIDI_WPS_STATUS_SUCCESS				3
#define WIDI_DEAUTHENTICATED				4
/* P2P Connection status: P2P Uses the l2sd_callback_link_stat and notify_assoc_stat*/
#define WIDI_P2P_PROVISION_REQ_RECV			5
#define WIDI_P2P_GO_REQ_RECIVED				6
#define WIDI_P2P_GO_CONFIRM_RECEIVED		7
#define WIDI_P2P_ASSOCIATED					8
#define WIDI_P2P_WPS_STATUS_FAIL			9
#define WIDI_P2P_WPS_STATUS_SUCESS			10
#define WIDI_P2P_DEAUTHENTICATED			11
#define WIDI_P2P_INVITE_REQ_RECEIVED		13
#define WIDI_P2P_INVITE_RSP_SENT			14
#define WIDI_P2P_WPS_SUCCESS                15

/* Msg type */
#define WIDI_MSG_TYPE_QUERY_OR_TRIGGER		1
#define WIDI_MSG_TYPE_SERVICE				2
#define WIDI_MSG_TYPE_ASSOC_STATUS			3
#define WIDI_MSG_TYPE_P2P_PROBE				4
#define WIDI_MSG_TYPE_EDID					5
#define WIDI_MSG_TYPE_P2P_IP				6
#define WIDI_MSG_TYPE_P2P_GO_CONFIRM		7
#define WIDI_MSG_TYPE_P2P_WFD_IE			8

#define INTEL_SMI_CODE	"\x00\x01\x57"
#define INTEL_OUI_CODE	"\x00\x17\x35"
/* Intel SMI (0x00:0x01:0x57) */
#define IS_INTEL_SMI(_x)			((_x)[0] == 0x00 && (_x)[1] == 0x01 && (_x)[2] == 0x57)
/* Intel OUI (0x00:0x17:0x35) */
#define IS_INTEL_OUI(_x)			((_x)[0] == 0x00 && (_x)[1] == 0x17 && (_x)[2] == 0x35)

#define	WPS_VENDOR_EXT_CODE				0x1049
#define	WPS_VENDOR_EXT_WIDI_SRC_CODE	0x1055
#define	WPS_VENDOR_EXT_WIDI_SINK_CODE	0x106A

typedef struct _WIDI_VENDOR_EXT {
	UCHAR VendorExt[WIDI_QUERY_TRIGGER_VE_LEN];
	struct _WIDI_VENDOR_EXT *pNext;
} WIDI_VENDOR_EXT, *PWIDI_VENDOR_EXT;

typedef struct GNU_PACKED _WIDI_QUERY_OR_TRIGGER_MSG {
	UINT8 type;
	UCHAR src_mac[MAC_ADDR_LEN];
	UCHAR channel;
	UCHAR ssid_len;
	UCHAR ssid[MAX_LEN_OF_SSID];
	UCHAR vendorExt[WIDI_QUERY_TRIGGER_VE_LEN];
} WIDI_QUERY_OR_TRIGGER_MSG, *PWIDI_QUERY_OR_TRIGGER_MSG;

typedef struct GNU_PACKED _WIDI_SERVICE_MSG {
	UINT8 type;
	UCHAR dst_mac[MAC_ADDR_LEN];
	UCHAR channel;
	CHAR  device_name[WIDI_DEVICE_NAME_LEN];
	UCHAR primary_dev[WIDI_PRIMARY_DEV_LEN];
	UCHAR ext[WIDI_SERVICE_VE_LEN];
} WIDI_SERVICE_MSG, *PWIDI_SERVICE_MSG;

typedef struct GNU_PACKED _WIDI_ASSOC_MSG {
	UINT8 type;
	UCHAR peer_mac[MAC_ADDR_LEN];
	UCHAR assoc_stat;
	UINT8 ssid_len;
	UINT8 ssid[MAX_LEN_OF_SSID];
} WIDI_ASSOC_MSG, *PWIDI_ASSOC_MSG;

#ifdef P2P_SUPPORT
typedef struct GNU_PACKED _WIDI_P2P_EDID_DATA {
	UINT8 type;
	UINT16 mfg_code;
	UINT16 prod_code;
	UINT32 serial_num;
	UINT8 cap_flag;
	UINT8 hori_size;
	UINT8 vert_size;
} WIDI_P2P_EDID_DATA, *PWIDI_P2P_EDID_DATA;

typedef struct GNU_PACKED _WIDI_P2P_PROBE_WPS_IE_MSG {
	UINT8 type;
	UINT8 src_mac[ETH_ALEN];
	UINT8 channel;
	UINT8 ssid_len;
	UINT8 ssid[MAX_LEN_OF_SSID];
	UINT8 category_id;
} WIDI_P2P_PROBE_WPS_IE_MSG, *PWIDI_P2P_PROBE_WPS_IE_MSG;

typedef struct GNU_PACKED _WIDI_P2P_IP {
	UINT8 type;
	UINT8 src_mac[ETH_ALEN];
	UINT8 channel;
	UINT8 ssid_len;
	UINT8 ssid[MAX_LEN_OF_SSID];
	UINT8 qa_ta_ext[WIDI_P2P_IP_VE_LEN];
} WIDI_P2P_IP, *PWIDI_P2P_IP;

#endif /* P2P_SUPPORT */
#endif /* _L2SD_TA_H */

