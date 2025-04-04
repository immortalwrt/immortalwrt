
/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2020, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************
*/

#ifndef _DOT11_EID_H_
#define _DOT11_EID_H_

/* Element IDs */
#define EID_SSID 0
#define EID_SUPPORTED_RATES_AND_BSS_MEMBERSHIP_SELECTORS 1
#define EID_DSSS_PARAM_SET 3
#define EID_CF_PARAMETER_SET 4
#define EID_TIM 5
#define EID_IBSS_PARAM_SET 6
#define EID_COUNTRY 7
#define EID_REQ 10
#define EID_BSS_LOAD 11
#define EID_EDCA_PARAM_SET 12
#define EID_TSPEC 13
#define EID_TCLAS 14
#define EID_SCHEDULE 15
#define EID_CHALLENGE_TEXT 16
#define EID_POWER_CONSTRAINT 32
#define EID_POWER_CAP 33
#define EID_TPC_REQ 34
#define EID_TPC_REPORT 35
#define EID_SUPPORTED_CHANNEL 36
#define EID_CHANNEL_SWITCH_ANNOUNCE 37
#define EID_MEASUREMENT_REQ 38
#define EID_MEASUREMENT_REPORT 39
#define EID_QUIET 40
#define EID_IBSS_DFS 41
#define EID_ERP 42
#define EID_TS_DELAY 43
#define EID_TCLAS_PROCESS 44
#define EID_HT_CAPS 45
#define EID_QOS_CAP 46
#define EID_RSN 48
#define EID_EXTENDED_SUPPORTED_RATES_AND_BSS_MEMBERSHIP_SELECTORS 50
#define EID_AP_CHANNEL_REPORT 51
#define EID_NEIGHBOR_REPORT 52
#define EID_RCPI 53
#define EID_MOBILITY_DOMAIN 54
#define EID_FAST_BSS_TRANSITION 55
#define EID_TIMEOUT_INTERVAL 56
#define EID_RIC_DATA 57
#define EID_DSE_REG_LOCATION 58
#define EID_SUPPORTED_OPERATING_CLASS 59
#define EID_EXTENDED_CHANNEL_SWITCH_ANNOUNCE 60
#define EID_HT_OPERATION 61
#define EID_SECONDARY_CHANNEL_OFFSET 62
#define EID_BSS_AVG_ACCESS_DELAY 63
#define EID_ANTENNA 64
#define EID_RSNI 65
#define EID_MEASUREMENT_PILOT_TRANS 66
#define EID_BSS_AVAILABLE_ADMISSION_CAP 67
#define EID_BSS_AC_ACCESS_DELAY 68
#define EID_TIME_ADV 69
#define EID_RM_ENABLE_CAP 70
#define EID_MULTIPLE_BSSID 71
#define EID_2040_BSS_COEX 72
#define EID_2040_BSS_INTOLERANT_CHANNEL_REPORT 73
#define EID_OVERLAPPING_BSS_SCAN_PARAM 74
#define EID_RIC_DESC 75
#define EID_MGMT_MIC 76
#define EID_EVT_REQ 78
#define EID_EVT_REPORT 79
#define EID_DIAGNOSTIC_REQ 80
#define EID_DIAGNOSTIC_REPORT 81
#define EID_LOCATION_PARAM 82
#define EID_NONTRANSMITTED_BSSID_CAP 83
#define EID_SSID_LIST 84
#define EID_MULTIPLE_BSSID_INDEX 85
#define EID_FMS_DESC 86
#define EID_FMS_REQ 87
#define EID_FMS_RESPONSE 88
#define EID_QOS_TRAFFIC_CAP 89
#define EID_BSS_MAX_IDLE_PERIOD 90
#define EID_TFS_REQ 91
#define EID_TFS_RESPONSE 92
#define EID_WNM_SLEEP_MODE 93
#define EID_TIM_BROADCAST_REQ 94
#define EID_TIM_BROADCAST_RESPONSE 95
#define EID_COLLOCATED_INTF_REPORT 96
#define EID_CHANNEL_USAGE 97
#define EID_TIME_ZONE 98
#define EID_DMS_REQ 99
#define EID_DMS_RESPONSE 100
#define EID_LINK_IDENTIFIER 101
#define EID_WAKEUP_SCHEDULE 102
#define EID_CHANNEL_SWITCH_TIMING 104
#define EID_PTI_CTRL 105
#define EID_TPU_BUFFER_STATUS 106
#define EID_INTERWORKING 107
#define EID_ADV_PROTOCOL 108
#define EID_EXPEDITED_BW_REQ 109
#define EID_QOS_MAP 110
#define EID_ROAMING_CONSORTIUM 111
#define EID_BCN_TIMING 120
#define EID_EXTENDED_CAPS 127
#define EID_MIC 140
#define EID_DESTINATION_URI 141
#define EID_UAPSD_COEX 142
#define EID_AWAKE_WINDOW 157
#define EID_MULTI_BAND 158
#define EID_ADDBA_EXTENSION 159
#define EID_NEXT_PCP_LIST 160
#define EID_PCP_HANDOVER 161
#define EID_QUIET_PERIOD_REQ 175
#define EID_QUIET_PERIOD_RESPONSE 177
#define EID_QLOAD_REPORT 186
#define EID_VHT_CAPS 191
#define EID_VHT_OPERATION 192
#define EID_EXTENDED_BSS_LOAD 193
#define EID_WIDE_BW_CHANNEL_SWITCH 194
#define EID_TRANSMIT_POWER_ENVELOPE 195
#define EID_CHANNEL_SWITCH_WRAPPER 196
#define EID_AID 197
#define EID_QUIET_CHANNEL 198
#define EID_OPERATING_MODE_NOTIFICATION 199
#define EID_UPSIM 200
#define EID_REDUCED_NEIGHBOR_REPORT 201
#define EID_DEVICE_LOCATION 204
#define EID_WHITE_SPACE_MAP 205
#define EID_FINE_TIMING_MEASUREMENT_PARAM 206
#define EID_VENDOR_SPECIFIC 221
#define EID_EXTENSION 255

/* Extension Element IDs */
#define EID_EXT_FTM_SYNC_INFO 9
#define EID_EXT_EXTENDED_REQ 10
#define EID_EXT_ESTMATED_SERVICE_PARAM 11
#define EID_EXT_FUTURE_CHANNEL_GUIDANCE 14

#endif /*_DOT11_EID_H_*/
